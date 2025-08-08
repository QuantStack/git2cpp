import os
import subprocess

import pytest


working_dir = 'test/data/xtl'

@pytest.mark.parametrize("all_flag", ["", "-A", "--all", "--no-ignore-removal"])
def test_commit(xtl_clone, git_config, git2cpp_path, monkeypatch, all_flag):
    with open("./test/data/xtl/mook_file.txt", "x"):
        pass

    cmd_add = [git2cpp_path, 'add', "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=working_dir, text=True)
    assert p_add.returncode == 0

    cmd_status = [git2cpp_path, 'status', "--long"]
    p_status = subprocess.run(cmd_status, capture_output=True, cwd=working_dir, text=True)
    assert p_status.returncode == 0
    assert "Changes to be committed" in p_status.stdout
    assert "new file" in p_status.stdout

    cmd_commit = [git2cpp_path, 'commit', "-m", "test commit"]
    p_commit = subprocess.run(cmd_commit, cwd=working_dir, text=True)
    assert p_commit.returncode == 0

    cmd_status_2 = [git2cpp_path, 'status', "--long"]
    p_status_2 = subprocess.run(cmd_status_2, capture_output=True, cwd=working_dir, text=True)
    assert p_status_2.returncode == 0
    assert "mook_file" not in p_status_2.stdout
