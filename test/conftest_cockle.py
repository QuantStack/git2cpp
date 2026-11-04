# Extra fixtures used for wasm testing, including some that override the default pytest fixtures.
import os
import pathlib
import signal
import subprocess
import time
from functools import partial

import pytest
from playwright.sync_api import Page

from .conftest_common import (
    MockPath,
    maybe_wrap_arg,
    os_chdir,
    os_getcwd,
    os_remove,
    os_rename,
    os_symlink,
    tmp_path,  # noqa: F401 Fixture used by tests
)


@pytest.fixture(scope="session", autouse=True)
def run_web_server():
    with open("serve.log", "w") as f:
        cwd = pathlib.Path(__file__).parent.parent / "wasm/test-cockle"
        # Start in a new process group so that the whole process tree (npm and node) can be
        # killed, otherwise the node server continues running and holding the port.
        proc = subprocess.Popen(
            ["npm", "run", "serve"], stdout=f, stderr=f, cwd=cwd, start_new_session=True
        )
        # Wait a bit until server ready to receive connections.
        time.sleep(1)
        yield
        os.killpg(proc.pid, signal.SIGTERM)
        proc.wait()


@pytest.fixture(scope="function", autouse=True)
def load_page(page: Page):
    # Load web page at start of every test.
    page.goto("http://localhost:8000")
    page.locator("#loaded").wait_for()


def subprocess_run(
    page: Page,
    cmd: list[str],
    *,
    capture_output: bool = False,
    check: bool = False,
    cwd: str | MockPath | None = None,
    input: str | None = None,
    text: bool | None = None,
) -> subprocess.CompletedProcess:
    shell_run = "async obj => await window.cockle.shellRun(obj.cmd, obj.input)"

    # Set cwd.
    if cwd is not None:
        proc = page.evaluate(shell_run, {"cmd": "pwd"})
        if proc["returncode"] != 0:
            raise RuntimeError("Error getting pwd")
        old_cwd = proc["stdout"].strip()
        if old_cwd == str(cwd):
            # cwd is already correct.
            cwd = None
        else:
            proc = page.evaluate(shell_run, {"cmd": f"cd {cwd}"})
            if proc["returncode"] != 0:
                raise RuntimeError(f"Error setting cwd to {cwd}")

    shell_run_args = {"cmd": " ".join([maybe_wrap_arg(s) for s in cmd]), "input": input}
    proc = page.evaluate(shell_run, shell_run_args)

    # TypeScript object is auto converted to Python dict.
    # Want to return subprocess.CompletedProcess, consider namedtuple if this fails in future.
    returncode = proc["returncode"]
    stdout = proc["stdout"] if capture_output else ""
    stderr = proc["stderr"] if capture_output else ""
    if not text:
        stdout = stdout.encode("utf-8")
        stderr = stderr.encode("utf-8")

    # Reset cwd.
    if cwd is not None:
        proc = page.evaluate(shell_run, {"cmd": "cd " + old_cwd})
        if proc["returncode"] != 0:
            raise RuntimeError(f"Error setting cwd to {old_cwd}")

    if check and returncode != 0:
        raise subprocess.CalledProcessError(returncode, cmd, stdout, stderr)

    return subprocess.CompletedProcess(
        args=cmd, returncode=returncode, stdout=stdout, stderr=stderr
    )


@pytest.fixture(scope="function", autouse=True)
def mock_subprocess_run(page: Page, monkeypatch):
    monkeypatch.setattr(subprocess, "run", partial(subprocess_run, page))
    monkeypatch.setattr(os, "chdir", os_chdir)
    monkeypatch.setattr(os, "getcwd", os_getcwd)
    monkeypatch.setattr(os, "remove", os_remove)
    monkeypatch.setattr(os, "rename", os_rename)
    monkeypatch.setattr(os, "symlink", os_symlink)
