# from pathlib import Path
import os
import subprocess

import pytest

from .conftest import strip_ansi_colours


@pytest.mark.parametrize("short_flag", ["", "-s", "--short"])
@pytest.mark.parametrize("long_flag", ["", "--long"])
def test_status_new_file(repo_init_with_commit, git2cpp_path, tmp_path, short_flag, long_flag):
    assert (tmp_path / "initial.txt").exists()

    p = tmp_path / "mook_file.txt"  # Untracked files
    p.write_text("")

    pw = tmp_path / "initial.txt"  # Changes not staged for commit / modified
    pw.write_text("blablabla")

    deletable = tmp_path / "to_delete.txt"
    deletable.write_text("delete me")
    subprocess.run([git2cpp_path, "add", "to_delete.txt"], cwd=tmp_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "Add deletable"], cwd=tmp_path, check=True)
    os.remove(deletable)  # Changes not staged for commit / deleted

    cmd = [git2cpp_path, "status"]
    if short_flag != "":
        cmd.append(short_flag)
    if long_flag != "":
        cmd.append(long_flag)
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True, check=True)

    if (long_flag == "--long") or ((long_flag == "") & (short_flag == "")):
        assert "On branch main" in p.stdout
        assert "Changes not staged for commit" in p.stdout
        assert "Untracked files" in p.stdout
        assert "deleted" in p.stdout
        assert "modified" in p.stdout

    elif short_flag in ["-s", "--short"]:
        assert " M " in p.stdout
        assert " D " in p.stdout
        assert "?? " in p.stdout


def test_status_nogit(git2cpp_path, tmp_path):
    cmd = [git2cpp_path, "status"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode != 0
    assert "error: could not find repository at" in p.stderr


@pytest.mark.parametrize("short_flag", ["", "-s", "--short"])
@pytest.mark.parametrize("long_flag", ["", "--long"])
def test_status_add_file(repo_init_with_commit, git2cpp_path, tmp_path, short_flag, long_flag):
    assert (tmp_path / "initial.txt").exists()

    p = tmp_path / "mook_file.txt"  # Changes to be committed / new file
    p.write_text("")

    os.remove(tmp_path / "initial.txt")  # Changes to be committed / deleted

    cmd_add = [git2cpp_path, "add", "--all"]
    p_add = subprocess.run(cmd_add, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    cmd_status = [git2cpp_path, "status"]
    if short_flag != "":
        cmd_status.append(short_flag)
    if long_flag != "":
        cmd_status.append(long_flag)
    p_status = subprocess.run(cmd_status, capture_output=True, cwd=tmp_path, text=True)
    assert p_status.returncode == 0

    if (long_flag == "--long") or ((long_flag == "") & (short_flag == "")):
        assert "Changes to be committed" in p_status.stdout
        assert "Changes not staged for commit" not in p_status.stdout
        assert "Untracked files" not in p_status.stdout
        assert "new file" in p_status.stdout
        assert "deleted" in p_status.stdout

    elif short_flag in ["-s", "--short"]:
        assert "A  " in p_status.stdout
        assert "D  " in p_status.stdout


def test_status_new_repo(git2cpp_path, tmp_path, run_in_tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, "init", "-b", "main"]
    p = subprocess.run(cmd, cwd=tmp_path)
    assert p.returncode == 0

    cmd_status = [git2cpp_path, "status"]
    p_status = subprocess.run(cmd_status, capture_output=True, cwd=tmp_path, text=True)
    assert p_status.returncode == 0
    assert "On branch main" in p_status.stdout
    assert "No commit yet" in p_status.stdout
    assert "Nothing to commit, working tree clean" in p_status.stdout


def test_status_clean_tree(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test 'Nothing to commit, working tree clean' message"""
    assert (tmp_path / "initial.txt").exists()

    cmd = [git2cpp_path, "status"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode == 0
    assert "On branch main" in p.stdout
    assert "Nothing to commit, working tree clean" in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
def test_status_rename_detection(repo_init_with_commit, git2cpp_path, tmp_path, short_flag):
    """Test that renamed files are detected correctly"""
    assert (tmp_path / "initial.txt").exists()

    # Create tracked file to rename
    new_file = tmp_path / "other_file.txt"
    new_file.write_text("Another file")
    subprocess.run([git2cpp_path, "add", "other_file.txt"], cwd=tmp_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "Add file to rename"], cwd=tmp_path, check=True)

    # Rename a file by moving and staging
    # Move the initial file
    old_name = tmp_path / "initial.txt"
    new_name = tmp_path / "initial_renamed.txt"
    os.rename(old_name, new_name)

    # Move/rename the other file using mv
    cmd_mv = [git2cpp_path, "mv", "other_file.txt", "other_file_renamed.txt"]
    subprocess.run(cmd_mv, capture_output=True, cwd=tmp_path, check=True)

    # Stage both the deletion and addition
    cmd_add = [git2cpp_path, "add", "--all"]
    subprocess.run(cmd_add, capture_output=True, cwd=tmp_path, check=True)

    # Check status
    cmd_status = [git2cpp_path, "status"]
    if short_flag == "-s":
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0

    # Should show as renamed, not as deleted + new file
    assert "initial.txt -> initial_renamed.txt" in p.stdout
    assert "other_file.txt -> other_file_renamed.txt" in p.stdout
    if short_flag == "-s":
        assert "R  " in p.stdout
    else:
        assert "renamed:" in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
def test_status_mixed_changes(repo_init_with_commit, git2cpp_path, tmp_path, short_flag):
    """Test status with both staged and unstaged changes"""
    assert (tmp_path / "initial.txt").exists()

    # Create a new file and stage it
    staged_file = tmp_path / "staged.txt"
    staged_file.write_text("staged content")

    # Create a tracked file, then delete it (and stage the deletion)
    del_file = tmp_path / "to_delete.txt"
    del_file.write_text("delete me")
    subprocess.run([git2cpp_path, "add", "to_delete.txt"], cwd=tmp_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "Add deletable"], cwd=tmp_path, check=True)
    os.remove(del_file)

    # Stage the two previous files
    subprocess.run([git2cpp_path, "add", "staged.txt", "to_delete.txt"], cwd=tmp_path, check=True)

    # Modify an existing file without staging
    unstaged_file = tmp_path / "initial.txt"
    unstaged_file.write_text("unstaged changes")

    # Create an untracked file
    untracked_file = tmp_path / "untracked.txt"
    untracked_file.write_text("untracked")

    cmd_status = [git2cpp_path, "status"]
    if short_flag == "-s":
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode == 0

    p.stdout = strip_ansi_colours(p.stdout)
    if short_flag == "-s":
        assert "A  staged.txt" in p.stdout
        assert "D  to_delete.txt" in p.stdout
        assert " M initial.txt" in p.stdout
        assert "?? untracked.txt" in p.stdout
    else:
        assert "Changes to be committed" in p.stdout
        assert "new file:   staged.txt" in p.stdout
        assert "deleted:   to_delete.txt" in p.stdout
        assert "Changes not staged for commit" in p.stdout
        assert "modified:   initial.txt" in p.stdout
        assert "Untracked files" in p.stdout
        assert "untracked.txt" in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
def test_status_typechange(repo_init_with_commit, git2cpp_path, tmp_path, short_flag):
    """Test status shows typechange (file to symlink or vice versa)"""
    assert (tmp_path / "initial.txt").exists()

    # Remove a file and replace with a symlink
    test_file = tmp_path / "typechange"
    test_file.write_text("regular file")
    subprocess.run([git2cpp_path, "add", "typechange"], cwd=tmp_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "Add typechange"], cwd=tmp_path, check=True)
    os.remove(test_file)
    os.symlink("initial.txt", test_file)

    cmd_status = [git2cpp_path, "status"]
    if short_flag == "-s":
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode == 0
    # Should show typechange in unstaged changes
    if short_flag == "-s":
        assert " T " in p.stdout
    else:
        assert "Changes not staged for commit" in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
def test_status_untracked_directory(repo_init_with_commit, git2cpp_path, tmp_path, short_flag):
    """Test that untracked directories are shown with trailing slash"""
    assert (tmp_path / "initial.txt").exists()

    # Create a directory with files
    new_dir = tmp_path / "new_directory"
    new_dir.mkdir()
    (new_dir / "file1.txt").write_text("content1")
    (new_dir / "file2.txt").write_text("content2")

    cmd_status = [git2cpp_path, "status"]
    if short_flag == "-s":
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode == 0
    if short_flag == "-s":
        assert "?? " in p.stdout
    else:
        assert "Untracked files" in p.stdout
        # Directory should be shown with trailing slash, not individual files
        assert "new_directory/" in p.stdout
        assert "file1.txt" not in p.stdout
        assert "file2.txt" not in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
def test_status_ahead_of_upstream(commit_env_config, git2cpp_path, tmp_path, short_flag):
    """Test status when local branch is ahead of upstream"""
    # Create a repository with remote tracking
    repo_path = tmp_path / "repo"
    repo_path.mkdir()

    # Initialize repo
    subprocess.run([git2cpp_path, "init", "-b", "main"], cwd=repo_path, check=True)

    # Create initial commit
    test_file = repo_path / "file.txt"
    test_file.write_text("initial")
    subprocess.run([git2cpp_path, "add", "file.txt"], cwd=repo_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "initial"], cwd=repo_path, check=True)

    # Clone it to create remote tracking
    clone_path = tmp_path / "clone"
    subprocess.run([git2cpp_path, "clone", str(repo_path), str(clone_path)], check=True)

    # Make a commit in clone
    clone_file = clone_path / "file2.txt"
    clone_file.write_text("new file")
    subprocess.run([git2cpp_path, "add", "file2.txt"], cwd=clone_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "second commit"], cwd=clone_path, check=True)

    # Check status
    cmd_status = [git2cpp_path, "status"]
    if short_flag == "-s":
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=clone_path, text=True)

    assert p.returncode == 0
    if short_flag == "-s":
        assert "...origin/main" in p.stdout
        assert "[ahead 1]" in p.stdout
    else:
        assert "Your branch is ahead of 'origin/main'" in p.stdout
        assert "by 1 commit" in p.stdout
        assert 'use "git push"' in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
@pytest.mark.parametrize("branch_flag", ["-b", "--branch"])
def test_status_with_branch_and_tracking(
    commit_env_config, git2cpp_path, tmp_path, short_flag, branch_flag
):
    """Test short format with branch flag shows tracking info"""
    # Create a repository with remote tracking
    repo_path = tmp_path / "repo"
    repo_path.mkdir()

    subprocess.run([git2cpp_path, "init", "-b", "main"], cwd=repo_path)

    test_file = repo_path / "file.txt"
    test_file.write_text("initial")
    subprocess.run([git2cpp_path, "add", "file.txt"], cwd=repo_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "initial"], cwd=repo_path, check=True)

    # Clone it
    clone_path = tmp_path / "clone"
    subprocess.run([git2cpp_path, "clone", str(repo_path), str(clone_path)], check=True)

    # Make a commit
    clone_file = clone_path / "file2.txt"
    clone_file.write_text("new")
    subprocess.run([git2cpp_path, "add", "file2.txt"], cwd=clone_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "second"], cwd=clone_path, check=True)

    # Check short status with branch flag
    cmd_status = [git2cpp_path, "status", branch_flag]
    if short_flag == "-s":
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=clone_path, text=True)

    assert p.returncode == 0
    if short_flag == "-s":
        assert "## main" in p.stdout  # "main" locally, but "master" in the CI
        assert "[ahead 1]" in p.stdout
    else:
        assert "On branch main" in p.stdout  # "main" locally, but "master" in the CI
        assert (
            "Your branch is ahead of 'origin/main'" in p.stdout
        )  # "main" locally, but "master" in the CI
        assert "1 commit." in p.stdout


def test_status_all_headers_shown(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that all status headers can be shown together"""
    assert (tmp_path / "initial.txt").exists()

    # Changes to be committed
    staged = tmp_path / "staged.txt"
    staged.write_text("staged")
    subprocess.run([git2cpp_path, "add", "staged.txt"], cwd=tmp_path, check=True)

    # Changes not staged
    modified = tmp_path / "initial.txt"
    modified.write_text("modified")

    # Untracked
    untracked = tmp_path / "untracked.txt"
    untracked.write_text("untracked")

    cmd_status = [git2cpp_path, "status"]
    p = subprocess.run(cmd_status, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode == 0
    assert "On branch main" in p.stdout
    assert "Changes to be committed:" in p.stdout
    assert 'use "git reset HEAD <file>..." to unstage' in p.stdout
    assert "Changes not staged for commit:" in p.stdout
    assert 'use "git add <file>..." to update what will be committed' in p.stdout
    assert "Untracked files:" in p.stdout
    assert 'use "git add <file>..." to include in what will be committed' in p.stdout
