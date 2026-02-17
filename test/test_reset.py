import subprocess

import pytest


def test_reset(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    p = xtl_path / "mook_file.txt"
    p.write_text("")

    cmd_add = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    cmd_commit = [git2cpp_path, "commit", "-m", "test commit"]
    p_commit = subprocess.run(cmd_commit, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    cmd_log = [git2cpp_path, "log"]
    p_log = subprocess.run(cmd_log, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0
    assert "Jane Doe" in p_log.stdout

    cmd_reset = [git2cpp_path, "reset", "--hard", "HEAD~1"]
    p_reset = subprocess.run(cmd_reset, capture_output=True, cwd=xtl_path, text=True)
    assert p_reset.returncode == 0

    cmd_log_2 = [git2cpp_path, "log"]
    p_log = subprocess.run(cmd_log_2, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0
    assert "Jane Doe" not in p_log.stdout


def test_reset_nogit(git2cpp_path, tmp_path):
    cmd_reset = [git2cpp_path, "reset", "--hard", "HEAD~1"]
    p_reset = subprocess.run(cmd_reset, capture_output=True, cwd=tmp_path, text=True)
    assert p_reset.returncode != 0
    assert "error: could not find repository at" in p_reset.stderr
