import pytest
import re
import subprocess
from .conftest import GIT2CPP_TEST_WASM


@pytest.mark.parametrize("arg", ['-v', '--version'])
def test_version(git2cpp_path, arg):
    cmd = [git2cpp_path, arg]
    p = subprocess.run(cmd, capture_output=True)
    assert p.returncode == 0
    assert p.stderr == b''
    assert p.stdout.startswith(b'git2cpp version ')
    assert p.stdout.count(b'\n') == 1


def test_error_on_unknown_option(git2cpp_path):
    cmd = [git2cpp_path, '--unknown']
    p = subprocess.run(cmd, capture_output=True)
    assert p.returncode == 109
    assert p.stdout == b''
    assert p.stderr.startswith(b"The following argument was not expected: --unknown")


@pytest.mark.skipif(not GIT2CPP_TEST_WASM, reason="Only test in WebAssembly")
def test_cockle_config(git2cpp_path):
    # Check cockle-config shows git2cpp is available.
    cmd = ["cockle-config", "module", "git2cpp"]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    lines = [line for line in re.split(r"\r?\n", p.stdout) if len(line) > 0]
    assert len(lines) == 5
    assert lines[1] == "│ module  │ package │ cached │"
    assert lines[3] == "│ git2cpp │ git2cpp │        │"

    p = subprocess.run([git2cpp_path, "-v"], capture_output=True, text=True)
    assert p.returncode == 0

    # Check git2cpp module has been cached.
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    lines = [line for line in re.split(r"\r?\n", p.stdout) if len(line) > 0]
    assert len(lines) == 5
    assert lines[1] == "│ module  │ package │ cached │"
    assert lines[3] == "│ git2cpp │ git2cpp │ yes    │"
