# Extra fixtures used for wasm testing.
from functools import partial
from pathlib import Path
from playwright.sync_api import Page
import pytest
import subprocess
import time

@pytest.fixture(scope="session", autouse=True)
def run_web_server():
    with open('serve.log', 'w') as f:
        cwd = Path(__file__).parent.parent / 'wasm/test'
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

def subprocess_run(
    page: Page,
    cmd: list[str],
    *,
    capture_output: bool = False,
    cwd: str | None = None,
    text: bool | None = None
) -> subprocess.CompletedProcess:
    if cwd is not None:
        raise RuntimeError('cwd is not yet supported')

    proc = page.evaluate("async cmd => window.cockle.shellRun(cmd)", cmd)
    # TypeScript object is auto converted to Python dict.
    # Want to return subprocess.CompletedProcess, consider namedtuple if this fails in future.
    stdout = proc['stdout'] if capture_output else ''
    stderr = proc['stderr'] if capture_output else ''
    if not text:
        stdout = stdout.encode("utf-8")
        stderr = stderr.encode("utf-8")
    return subprocess.CompletedProcess(
        args=cmd,
        returncode=proc['returncode'],
        stdout=stdout,
        stderr=stderr
    )

@pytest.fixture(scope="function", autouse=True)
def mock_subprocess_run(page: Page, monkeypatch):
    monkeypatch.setattr(subprocess, "run", partial(subprocess_run, page))
