import os
import subprocess

import pytest


@pytest.mark.parametrize("all_flag", ["", "-A", "--all", "--no-ignore-removal"])
def test_commit(rename_git, git2cpp_path, all_flag):
    checkout_cmd = [git2cpp_path, 'checkout', '-b', 'commit_test_branch']
    subprocess.run(checkout_cmd, cwd="test/data/status_data", text=True)

    with open("./test/data/status_data/mook_file.txt", "x"):
        pass

    cmd_add = [git2cpp_path, 'add', "mook_file.txt"]
    subprocess.run(cmd_add, cwd="test/data/status_data", text=True)

    cmd_status = [git2cpp_path, 'status', "--long"]
    p_status = subprocess.run(cmd_status, capture_output=True, cwd="test/data/status_data", text=True)

    assert "Changes to be committed" in p_status.stdout
    assert "new file" in p_status.stdout

    cmd_commit = [git2cpp_path, 'commit', "-m", "test commit"]
    subprocess.run(cmd_commit, cwd="test/data/status_data", text=True)

    cmd_status_2 = [git2cpp_path, 'status', "--long"]
    p_status_2 = subprocess.run(cmd_status_2, capture_output=True, cwd="test/data/status_data", text=True)

    assert "mook_file" not in p_status_2.stdout

    cmd_reset = [git2cpp_path, 'reset', "--hard", "HEAD~1"]
    subprocess.run(cmd_reset, cwd="test/data/status_data", text=True)

    checkout_cmd.remove('-b')
    checkout_cmd[2] = 'main'
    subprocess.run(checkout_cmd, cwd="test/data/status_data", text=True)

    del_cmd = [git2cpp_path, 'branch', '-d', 'commit_test_branch']
    subprocess.run(del_cmd, cwd="test/data/status_data", text=True)
