import subprocess

import pytest


@pytest.mark.parametrize("all_flag", ["", "-A", "--all", "--no-ignore-removal"])
def test_commit(xtl_clone, commit_env_config, git2cpp_path, tmp_path, all_flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    p = xtl_path / "mook_file.txt"
    p.write_text("")

    cmd_add = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    cmd_status = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(cmd_status, capture_output=True, cwd=xtl_path, text=True)
    assert p_status.returncode == 0

    assert "Changes to be committed" in p_status.stdout
    assert "new file" in p_status.stdout

    cmd_commit = [git2cpp_path, "commit", "-m", "test commit"]
    p_commit = subprocess.run(cmd_commit, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    cmd_status_2 = [git2cpp_path, "status", "--long"]
    p_status_2 = subprocess.run(
        cmd_status_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_status_2.returncode == 0
    assert "mook_file" not in p_status_2.stdout


@pytest.mark.parametrize("commit_msg", ["Added file", ""])
def test_commit_message_via_stdin(commit_env_config, git2cpp_path, tmp_path, run_in_tmp_path, commit_msg):
    cmd = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd)
    assert p_init.returncode == 0

    (tmp_path / "file.txt").write_text("Some text")

    cmd_add = [git2cpp_path, "add", "file.txt"]
    p_add = subprocess.run(cmd_add)
    assert p_add.returncode == 0

    cmd_commit = [git2cpp_path, "commit"]
    p_commit = subprocess.run(cmd_commit, text=True, capture_output=True, input=commit_msg)

    if commit_msg == "":
        # No commit message
        assert p_commit.returncode != 0
        assert "Aborting, no commit message specified" in p_commit.stderr
    else:
        # Valid commit message
        assert p_commit.returncode == 0

        cmd_log = [git2cpp_path, "log"]
        p_log = subprocess.run(cmd_log, text=True, capture_output=True)
        assert p_log.returncode == 0
        lines = p_log.stdout.splitlines()

        assert "commit" in lines[0]
        assert "Author:" in lines[1]
        assert "Date" in lines[2]
        assert commit_msg in lines[4]
