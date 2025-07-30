import os
import subprocess

import pytest


@pytest.mark.parametrize("all_flag", ["", "-A", "--all", "--no-ignore-removal"])
def test_add(git2cpp_path, all_flag):
    with open("./test/mook_file.txt", "x"):
        pass

    cmd_add = [git2cpp_path, 'add', "test/mook_file.txt"]
    subprocess.run(cmd_add, capture_output=True, text=True)

    cmd_status = [git2cpp_path, 'status', "--long"]
    p_status = subprocess.run(cmd_status, capture_output=True, text=True)

    assert "Changes to be committed" in p_status.stdout
    assert "new file" in p_status.stdout

    cmd_commit = [git2cpp_path, 'commit', "-m", "test commit"]
    subprocess.run(cmd_commit, capture_output=True, text=True)

    cmd_status_2 = [git2cpp_path, 'status', "--long"]
    p_status_2 = subprocess.run(cmd_status_2, capture_output=True, text=True)

    assert "mook_file" not in p_status_2.stdout

    cmd_reset = [git2cpp_path, 'reset', "--hard", "HEAD~1"]
    subprocess.run(cmd_reset, capture_output=True, text=True)
