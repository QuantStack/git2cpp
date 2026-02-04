# Extra fixtures used for wasm testing, including some that override the default pytest fixtures.
from functools import partial
import os
import pathlib
from playwright.sync_api import Page
import pytest
import re
import subprocess
import time


# Only include particular test files when testing wasm.
# This can be removed when all tests support wasm.
def pytest_ignore_collect(collection_path: pathlib.Path) -> bool:
    return collection_path.name not in [
        "test_add.py",
        "test_branch.py",
        "test_checkout.py"
        "test_clone.py",
        "test_commit.py",
        "test_config.py",
        "test_fixtures.py",
        "test_git.py",
        "test_init.py",
        "test_log.py",
        "test_merge.py",
        "test_rebase.py",
        "test_remote.py",
        "test_reset.py",
        "test_revlist.py",
        "test_revparse.py",
        "test_stash.py",
        "test_status.py",
    ]


@pytest.fixture(scope="session", autouse=True)
def run_web_server():
    with open('serve.log', 'w') as f:
        cwd = pathlib.Path(__file__).parent.parent / 'wasm/test'
        proc = subprocess.Popen(
            ['npm', 'run', 'serve'], stdout=f, stderr=f, cwd=cwd
        )
        # Wait a bit until server ready to receive connections.
        time.sleep(0.3)
        yield
        proc.terminate()


@pytest.fixture(scope="function", autouse=True)
def load_page(page: Page):
    # Load web page at start of every test.
    page.goto("http://localhost:8000")
    page.locator("#loaded").wait_for()


def os_chdir(dir: str):
    subprocess.run(["cd", str(dir)], capture_output=True, check=True, text=True)


def os_getcwd():
    return subprocess.run(["pwd"], capture_output=True, check=True, text=True).stdout.strip()


def os_remove(file: str):
    return subprocess.run(["rm", str(file)], capture_output=True, check=True, text=True)


class MockPath(pathlib.Path):
    def __init__(self, path: str = ""):
        super().__init__(path)

    def exists(self) -> bool:
        p = subprocess.run(['stat', str(self)])
        return p.returncode == 0

    def is_dir(self) -> bool:
        p = subprocess.run(['stat', '-c', '%F', str(self)], capture_output=True, text=True)
        return p.returncode == 0 and p.stdout.strip() == 'directory'

    def is_file(self) -> bool:
        p = subprocess.run(['stat', '-c', '%F', str(self)], capture_output=True, text=True)
        return p.returncode == 0 and p.stdout.strip() == 'regular file'

    def iterdir(self):
        p = subprocess.run(["ls", str(self), '-a', '-1'], capture_output=True, text=True, check=True)
        for f in filter(lambda f: f not in ['', '.', '..'], re.split(r"\r?\n", p.stdout)):
            yield MockPath(self / f)

    def mkdir(self):
        subprocess.run(["mkdir", str(self)], capture_output=True, text=True, check=True)

    def read_text(self) -> str:
        p = subprocess.run(["cat", str(self)], capture_output=True, text=True, check=True)
        text = p.stdout
        if text.endswith("\n"):
            text = text[:-1]
        return text

    def write_text(self, data: str):
        # Note that in general it is not valid to direct output of a subprocess.run call to a file,
        # but we get away with it here as the command arguments are passed straight through to
        # cockle without being checked.
        p = subprocess.run(["echo", data, ">", str(self)], capture_output=True, text=True)
        assert p.returncode == 0

    def __truediv__(self, other):
        if isinstance(other, str):
            return MockPath(f"{self}/{other}")
        raise RuntimeError("MockPath.__truediv__ only supports strings")


def subprocess_run(
    page: Page,
    cmd: list[str],
    *,
    capture_output: bool = False,
    check: bool = False,
    cwd: str | MockPath | None = None,
    input: str | None = None,
    text: bool | None = None
) -> subprocess.CompletedProcess:
    shell_run = "async obj => await window.cockle.shellRun(obj.cmd, obj.input)"

    # Set cwd.
    if cwd is not None:
        proc = page.evaluate(shell_run, { "cmd": "pwd" } )
        if proc['returncode'] != 0:
            raise RuntimeError("Error getting pwd")
        old_cwd = proc['stdout'].strip()
        if old_cwd == str(cwd):
            # cwd is already correct.
            cwd = None
        else:
            proc = page.evaluate(shell_run, { "cmd": f"cd {cwd}" } )
            if proc['returncode'] != 0:
                raise RuntimeError(f"Error setting cwd to {cwd}")

    def maybe_wrap_arg(s: str | MockPath) -> str:
        # An argument containing spaces needs to be wrapped in quotes if it is not already, due
        # to how the command is passed to cockle as a single string.
        # Could do better here.
        s = str(s)
        if ' ' in s and not s.endswith("'"):
            return "'" + s + "'"
        return s

    shell_run_args = {
        "cmd": " ".join([maybe_wrap_arg(s) for s in cmd]),
        "input": input
    }
    proc = page.evaluate(shell_run, shell_run_args)

    # TypeScript object is auto converted to Python dict.
    # Want to return subprocess.CompletedProcess, consider namedtuple if this fails in future.
    stdout = proc['stdout'] if capture_output else ''
    stderr = proc['stderr'] if capture_output else ''
    if not text:
        stdout = stdout.encode("utf-8")
        stderr = stderr.encode("utf-8")

    # Reset cwd.
    if cwd is not None:
        proc = page.evaluate(shell_run, { "cmd": "cd " + old_cwd } )
        if proc['returncode'] != 0:
            raise RuntimeError(f"Error setting cwd to {old_cwd}")

    if check and proc['returncode'] != 0:
        raise subprocess.CalledProcessError(proc['returncode'], cmd, stdout, stderr)

    return subprocess.CompletedProcess(
        args=cmd,
        returncode=proc['returncode'],
        stdout=stdout,
        stderr=stderr
    )


@pytest.fixture(scope="function")
def tmp_path() -> MockPath:
    # Assumes only one tmp_path needed per test.
    path = MockPath('/drive/tmp0')
    subprocess.run(['mkdir', str(path)], check=True)
    assert path.exists()
    assert path.is_dir()
    return path


@pytest.fixture(scope="function", autouse=True)
def mock_subprocess_run(page: Page, monkeypatch):
    monkeypatch.setattr(subprocess, "run", partial(subprocess_run, page))
    monkeypatch.setattr(os, "chdir", os_chdir)
    monkeypatch.setattr(os, "getcwd", os_getcwd)
    monkeypatch.setattr(os, "remove", os_remove)
