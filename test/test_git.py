import pytest
import subprocess


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
