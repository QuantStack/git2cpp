# Extra fixtures used for wasm testing in a JupyterLite terminal, including some that override the
# default pytest fixtures. Commands are run by typing them into a terminal, with stdout and stderr
# written to files in the JupyterLite drive that are then read back, and the exit code read from the
# terminal output.
import atexit
import os
import pathlib
import re
import signal
import subprocess
import time
from functools import partial

import pytest
from playwright.sync_api import Browser, ConsoleMessage, Page

from .conftest_common import (
    MockPath,
    maybe_wrap_arg,
    os_chdir,
    os_remove,
    os_rename,
    os_symlink,
    tmp_path,  # noqa: F401 Fixture used by tests
)

LITE_URL = "http://localhost:8001/lab/index.html"

# Files in the JupyterLite drive that a command's stdout and stderr are written to.
# Terminal path /drive/<name> corresponds to JupyterLite contents path <name>.
STDOUT_FILE = "lite_stdout.txt"
STDERR_FILE = "lite_stderr.txt"

# Command line that writes the exit code of the previous command line and the shell's cwd to the
# terminal, as lines such as '?=0' and 'PWD=/drive'.
# Quote the grep pattern as cockle treats an unquoted '?' as a glob wildcard.
EXIT_CODE_AND_PWD_LINE = "env | grep -E '^(\\?|PWD)='"

# Collect the output of the open terminal so that we can tell when a command line has finished.
COLLECT_OUTPUT_JS = """
() => {
  const widget = Array.from(window.jupyterapp.shell.widgets('main')).find(w => w.content?.session);
  window.liteTest = { output: '' };
  widget.content.session.messageReceived.connect((_, msg) => {
    if (msg.type === 'stdout') {
      window.liteTest.output += msg.content[0];
    }
  });
}
"""

# Read the contents of text files. They are not deleted as the next command overwrites them.
READ_FILES_JS = """
async paths => {
  const contents = window.jupyterapp.serviceManager.contents;
  return await Promise.all(
    paths.map(async path => (await contents.get(path, { content: true, format: 'text' })).content)
  );
}
"""

# Shut down the open terminal's session.
SHUTDOWN_TERMINAL_JS = """
async () => {
  const widget = Array.from(window.jupyterapp.shell.widgets('main')).find(w => w.content?.session);
  await widget.content.session.shutdown();
}
"""

# Recursively delete everything in the JupyterLite drive.
CLEAR_DRIVE_JS = """
async () => {
  const contents = window.jupyterapp.serviceManager.contents;
  async function remove(path) {
    const model = await contents.get(path, { content: true });
    if (model.type === 'directory') {
      for (const child of model.content) {
        await remove(child.path);
      }
    }
    if (path !== '') {
      await contents.delete(path);
    }
  }
  await remove('');
}
"""


class ConsoleWatcher:
    """Collect console messages from the page and its workers, as cockle logs from a worker."""

    def __init__(self, page: Page):
        self._page = page
        self.messages: list[str] = []
        page.on("console", self._on_console)
        page.on("worker", lambda worker: worker.on("console", self._on_console))

    def _on_console(self, msg: ConsoleMessage):
        self.messages.append(msg.text)

    def clear(self):
        self.messages.clear()

    def wait_for(self, prefix: str, timeout_ms: int = 60_000):
        start = time.monotonic()
        while not any(m.startswith(prefix) for m in self.messages):
            if (time.monotonic() - start) * 1000 > timeout_ms:
                raise TimeoutError(f"Timeout waiting for console message starting '{prefix}'")
            # Waiting on the page allows playwright to dispatch console events.
            self._page.wait_for_timeout(100)


@pytest.fixture(scope="session", autouse=True)
def run_web_server():
    cwd = pathlib.Path(__file__).parent.parent / "wasm/test-lite"
    with open(cwd / "serve.log", "w") as f:
        # Start in a new process group so that the whole process tree (npm, npx and node) can be
        # killed, otherwise the node server continues running and holding the port.
        proc = subprocess.Popen(
            ["npm", "run", "serve"], stdout=f, stderr=f, cwd=cwd, start_new_session=True
        )
        # Also kill the servers at exit in case pytest aborts without running fixture teardown,
        # such as after an INTERNALERROR.
        atexit.register(kill_process_group, proc)
        # Wait a bit until server ready to receive connections.
        time.sleep(1)
        yield
        kill_process_group(proc)


def kill_process_group(proc: subprocess.Popen):
    if proc.poll() is None:
        os.killpg(proc.pid, signal.SIGTERM)
        proc.wait()


@pytest.fixture(scope="session")
def lite_page(run_web_server, browser: Browser) -> Page:
    # Load JupyterLite once per session as it is much slower than opening a new terminal.
    context = browser.new_context()
    page = context.new_page()
    page.goto(LITE_URL)
    page.locator(".jp-LauncherCard", has_text="Terminal").wait_for()
    yield page
    context.close()


@pytest.fixture(scope="session")
def lite_console_watcher(lite_page: Page) -> ConsoleWatcher:
    return ConsoleWatcher(lite_page)


class LiteTerminal:
    """The open terminal and the directories that commands are run in."""

    def __init__(self, page: Page):
        self.page = page
        # Current directory of the terminal's shell.
        self.shell_cwd: str = ""
        # Current directory of the mocked Python process, only changed by os.chdir. Commands that
        # do not specify a cwd are run here.
        self.process_cwd: str = ""


@pytest.fixture(scope="function", autouse=True)
def load_page(lite_page: Page, lite_console_watcher: ConsoleWatcher) -> LiteTerminal:
    # Open a new terminal from the launcher at the start of every test and wait until the shell
    # has written its first prompt, so that each test has a new shell.
    page = lite_page
    lite_console_watcher.clear()
    launcher_card = page.locator(".jp-LauncherCard", has_text="Terminal")
    if not launcher_card.is_visible():
        # Previous test closed its terminal so open a new launcher.
        page.locator('[data-command="launcher:create"]').first.click()
    launcher_card.click()
    page.locator(".jp-Terminal .xterm").wait_for()
    # Cockle checks the terminal theme immediately before writing the first prompt.
    lite_console_watcher.wait_for("Cockle theme change")
    page.evaluate(COLLECT_OUTPUT_JS)
    page.locator(".jp-Terminal .xterm").click()

    terminal = LiteTerminal(page)
    _, terminal.shell_cwd = read_exit_code_and_pwd(page)
    terminal.process_cwd = terminal.shell_cwd

    yield terminal

    # Shut down the terminal, which closes it, and remove everything from the drive.
    page.evaluate(SHUTDOWN_TERMINAL_JS)
    page.locator(".jp-Terminal").wait_for(state="detached")
    page.evaluate(CLEAR_DRIVE_JS)


def type_text(page: Page, text: str):
    # Type text into the terminal, using the Enter and Backspace keys for newline and delete
    # characters respectively.
    for chunk in re.split(r"([\n\x7f])", text):
        if chunk == "\n":
            page.keyboard.press("Enter")
        elif chunk == "\x7f":
            page.keyboard.press("Backspace")
        elif chunk:
            page.keyboard.type(chunk)


def run_line(page: Page, line: str, input: str | None = None, timeout_ms: int = 600_000) -> str:
    # Type a command line into the terminal, optionally followed by stdin input for the command,
    # wait for the next prompt and return the terminal output.
    page.evaluate("() => { window.liteTest.output = ''; }")
    page.keyboard.type(line)
    page.keyboard.press("Enter")
    if input is not None:
        # The shell enables buffered stdin for the command immediately it starts running, after
        # which typed input is buffered until the command reads it. There is no signal for this
        # so wait a short time before typing.
        page.wait_for_timeout(200)
        if not input.endswith("\n"):
            input += "\n"
        type_text(page, input)
    page.wait_for_function("() => window.liteTest.output.includes('js-shell:')", timeout=timeout_ms)
    return page.evaluate("() => window.liteTest.output")


def read_exit_code_and_pwd(page: Page) -> tuple[int | None, str]:
    # At the time of writing, the cockle in the JupyterLite terminal does not support '$?' and only
    # sets the exit code once a whole command line has finished, so read the exit code using a
    # second command line. This also reads the shell's cwd so that it can be tracked.
    # The next terminal release will support 'echo $?' instead.
    output = run_line(page, EXIT_CODE_AND_PWD_LINE)
    # Remove ANSI escape sequences such as colours, as grep is aliased to 'grep --color=auto'.
    output = re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", output)
    returncode = re.search(r"^\?=(-?\d+)\s*$", output, re.MULTILINE)
    pwd = re.search(r"^PWD=(.*?)\s*$", output, re.MULTILINE)
    if pwd is None:
        raise RuntimeError(f"Unable to read PWD from terminal output: {output!r}")
    # There is no exit code in a new shell before any command has been run.
    return None if returncode is None else int(returncode.group(1)), pwd.group(1)


def lite_shell_run(
    terminal: LiteTerminal, cmd: str, run_cwd: str, capture_output: bool, input: str | None = None
) -> dict:
    page = terminal.page

    # Change directory as part of the same command line, if necessary. The cockle in the JupyterLite
    # terminal does not support '&&' so if the cd fails the command is run in the wrong directory,
    # but this is detected afterwards from the shell's PWD.
    cd_prefix = "" if terminal.shell_cwd == run_cwd else f"cd {run_cwd}; "

    # If the command already redirects stdout, such as MockPath.write_text, do not redirect it
    # again as the last redirect wins.
    stdout_redirected = re.search(r"(^|\s)>>?(\s|$)", cmd) is not None
    stdout_redirect = "" if stdout_redirected else f" > /drive/{STDOUT_FILE}"
    run_line(page, f"{cd_prefix}{cmd}{stdout_redirect} 2> /drive/{STDERR_FILE}", input)

    returncode, terminal.shell_cwd = read_exit_code_and_pwd(page)
    if returncode is None:
        raise RuntimeError(f"Unable to read exit code of command: {cmd}")
    if cd_prefix and terminal.shell_cwd != run_cwd and not cmd.startswith("cd "):
        raise RuntimeError(f"Error setting cwd to {run_cwd}")

    stdout = stderr = ""
    if capture_output:
        paths = [STDERR_FILE] if stdout_redirected else [STDOUT_FILE, STDERR_FILE]
        contents = terminal.page.evaluate(READ_FILES_JS, paths)
        stderr = contents[-1]
        if not stdout_redirected:
            stdout = contents[0]
    return {"returncode": returncode, "stdout": stdout, "stderr": stderr}


def subprocess_run(
    terminal: LiteTerminal,
    cmd: list[str],
    *,
    capture_output: bool = False,
    check: bool = False,
    cwd: str | MockPath | None = None,
    input: str | None = None,
    text: bool | None = None,
) -> subprocess.CompletedProcess:
    run_cwd = terminal.process_cwd if cwd is None else str(cwd)
    proc = lite_shell_run(
        terminal, " ".join([maybe_wrap_arg(s) for s in cmd]), run_cwd, capture_output, input
    )

    # A successful cd without a specified cwd, such as from os.chdir, changes the process cwd.
    returncode = proc["returncode"]
    if cmd[0] == "cd" and cwd is None and returncode == 0:
        terminal.process_cwd = terminal.shell_cwd

    stdout = proc["stdout"]
    stderr = proc["stderr"]
    if not text:
        stdout = stdout.encode("utf-8")
        stderr = stderr.encode("utf-8")

    if check and returncode != 0:
        raise subprocess.CalledProcessError(returncode, cmd, stdout, stderr)

    return subprocess.CompletedProcess(
        args=cmd, returncode=returncode, stdout=stdout, stderr=stderr
    )


@pytest.fixture(scope="function", autouse=True)
def mock_subprocess_run(load_page: LiteTerminal, monkeypatch):
    monkeypatch.setattr(subprocess, "run", partial(subprocess_run, load_page))
    monkeypatch.setattr(os, "chdir", os_chdir)
    # Use the tracked cwd rather than running pwd, as os.getcwd is also called by pytest such as
    # when reporting a failure, when the terminal may still be running a command.
    monkeypatch.setattr(os, "getcwd", lambda: load_page.process_cwd)
    monkeypatch.setattr(os, "remove", os_remove)
    monkeypatch.setattr(os, "rename", os_rename)
    monkeypatch.setattr(os, "symlink", os_symlink)
