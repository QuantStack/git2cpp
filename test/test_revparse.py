import subprocess

import pytest

def test_revparse(git2cpp_path, tmp_path, run_in_tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, 'init', '--bare']
    p = subprocess.run(cmd, cwd = tmp_path)

    cmd2 = [git2cpp_path, 'rev-parse', '--is-bare-repository']
    p2 = subprocess.run(cmd2, capture_output=True, text=True, cwd = tmp_path)

    assert p2.returncode == 0
    assert p2.stdout == 'true\n'
