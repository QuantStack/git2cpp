import subprocess

import pytest

from .conftest import GIT2CPP_TEST_WASM


@pytest.mark.parametrize("all_flag", ["", "-A", "--all", "--no-ignore-removal"])
def test_commit(commit_env_config, git2cpp_path, tmp_path, all_flag):
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    p = tmp_path / "mook_file.txt"
    p.write_text("")

    cmd_add = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    cmd_status = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(cmd_status, capture_output=True, cwd=tmp_path, text=True)
    assert p_status.returncode == 0

    assert "Changes to be committed" in p_status.stdout
    assert "new file" in p_status.stdout

    cmd_commit = [git2cpp_path, "commit", "-m", "test commit"]
    p_commit = subprocess.run(cmd_commit, cwd=tmp_path, text=True)
    assert p_commit.returncode == 0

    cmd_status_2 = [git2cpp_path, "status", "--long"]
    p_status_2 = subprocess.run(cmd_status_2, capture_output=True, cwd=tmp_path, text=True)
    assert p_status_2.returncode == 0
    assert "mook_file" not in p_status_2.stdout


@pytest.mark.parametrize(
    "commit_msg_in,commit_msg_out",
    [
        ("Added file", "Added file"),
        ("", ""),
        ("ab\x7fc", "ac"),  # Check deletes previous character to prove using stdin line buffering
    ],
)
def test_commit_message_via_stdin(
    commit_env_config, git2cpp_path, tmp_path, run_in_tmp_path, commit_msg_in, commit_msg_out
):
    if not GIT2CPP_TEST_WASM and commit_msg_in != commit_msg_out:
        pytest.skip("Skip stdin delete test if not using webassembly")

    cmd = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd)
    assert p_init.returncode == 0

    (tmp_path / "file.txt").write_text("Some text")

    cmd_add = [git2cpp_path, "add", "file.txt"]
    p_add = subprocess.run(cmd_add)
    assert p_add.returncode == 0

    cmd_commit = [git2cpp_path, "commit"]
    p_commit = subprocess.run(
        cmd_commit, text=True, capture_output=True, input=commit_msg_in + "\n"
    )

    if commit_msg_out == "":
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
        assert commit_msg_out in lines[4]


def test_commit_no_changes_initial(commit_env_config, git2cpp_path, tmp_path):
    """Commit on fresh repo with no staged files should fail"""
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    # Do NOT add any files — attempt to commit immediately
    cmd_commit = [git2cpp_path, "commit", "-m", "empty commit"]
    p_commit = subprocess.run(cmd_commit, capture_output=True, cwd=tmp_path, text=True)

    # Should fail: nothing to commit
    assert p_commit.returncode != 0
    assert "nothing to commit" in p_commit.stderr or "nothing to commit" in p_commit.stdout


def test_commit_no_changes_after_first_commit(commit_env_config, git2cpp_path, tmp_path):
    """Commit twice without changes between commits should fail"""
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    # Create and commit a file
    (tmp_path / "file.txt").write_text("hello")
    subprocess.run([git2cpp_path, "add", "file.txt"], cwd=tmp_path, check=True)
    p_first = subprocess.run(
        [git2cpp_path, "commit", "-m", "first commit"], cwd=tmp_path, capture_output=True, text=True
    )
    assert p_first.returncode == 0

    # Try to commit again without any new changes
    p_second = subprocess.run(
        [git2cpp_path, "commit", "-m", "second commit (no changes)"],
        cwd=tmp_path,
        capture_output=True,
        text=True,
    )

    # Should fail: nothing to commit
    assert p_second.returncode != 0
    assert "nothing to commit" in p_second.stderr or "nothing to commit" in p_second.stdout
