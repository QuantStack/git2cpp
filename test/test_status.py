# from pathlib import Path
import os
import subprocess

import pytest
from .conftest import GIT2CPP_TEST_WASM


@pytest.mark.parametrize("short_flag", ["", "-s", "--short"])
@pytest.mark.parametrize("long_flag", ["", "--long"])
def test_status_new_file(xtl_clone, git2cpp_path, tmp_path, short_flag, long_flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    p = xtl_path / "mook_file.txt"  # Untracked files
    p.write_text("")

    pw = xtl_path / "CMakeLists.txt"  # Changes not staged for commit / modified
    pw.write_text("blablabla")

    os.remove(xtl_path / "README.md")  # Changes not staged for commit / deleted

    cmd = [git2cpp_path, "status"]
    if short_flag != "":
        cmd.append(short_flag)
    if long_flag != "":
        cmd.append(long_flag)
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True, check=True)

    if (long_flag == "--long") or ((long_flag == "") & (short_flag == "")):
        assert "On branch master" in p.stdout
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
    if not GIT2CPP_TEST_WASM:
        # TODO: fix this in wasm build
        assert p.returncode != 0
    assert "error: could not find repository at" in p.stderr


@pytest.mark.parametrize("short_flag", ["", "-s", "--short"])
@pytest.mark.parametrize("long_flag", ["", "--long"])
def test_status_add_file(xtl_clone, git2cpp_path, tmp_path, short_flag, long_flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    p = xtl_path / "mook_file.txt"  # Changes to be committed / new file
    p.write_text("")

    os.remove(xtl_path / "README.md")  # Changes to be committed / deleted

    cmd_add = [git2cpp_path, "add", "--all"]
    p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    cmd_status = [git2cpp_path, "status"]
    if short_flag != "":
        cmd_status.append(short_flag)
    if long_flag != "":
        cmd_status.append(long_flag)
    p_status = subprocess.run(cmd_status, capture_output=True, cwd=xtl_path, text=True)
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

    cmd = [git2cpp_path, "init"]
    p = subprocess.run(cmd, cwd=tmp_path)
    assert p.returncode == 0

    cmd_status = [git2cpp_path, "status"]
    p_status = subprocess.run(cmd_status, capture_output=True, cwd=tmp_path, text=True)
    assert p_status.returncode == 0
    assert "On branch ma" in p_status.stdout   # "main" locally, but "master" in the CI
    assert "No commit yet" in p_status.stdout
    assert "Nothing to commit, working tree clean" in p_status.stdout


def test_status_clean_tree(xtl_clone, git2cpp_path, tmp_path):
    """Test 'Nothing to commit, working tree clean' message"""
    xtl_path = tmp_path / "xtl"

    cmd = [git2cpp_path, "status"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)

    assert p.returncode == 0
    assert "On branch master" in p.stdout
    assert "Nothing to commit, working tree clean" in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
def test_status_rename_detection(xtl_clone, git2cpp_path, tmp_path, short_flag):
    """Test that renamed files are detected correctly"""
    xtl_path = tmp_path / "xtl"

    # Rename a file using git mv or by moving and staging
    old_readme = xtl_path / "README.md"
    new_readme = xtl_path / "README_renamed.md"

    # Move the README file
    os.rename(old_readme, new_readme)

    # Move/rename the LICENCE file using mv
    cmd_mv = [git2cpp_path, "mv", "LICENSE", "LICENSE_renamed"]
    subprocess.run(cmd_mv, capture_output=True, cwd=xtl_path, check=True)

    # Stage both the deletion and addition
    cmd_add = [git2cpp_path, "add", "--all"]
    subprocess.run(cmd_add, capture_output=True, cwd=xtl_path, check=True)

    # Check status
    cmd_status = [git2cpp_path, "status"]
    if short_flag == "-s":
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0

    # Should show as renamed, not as deleted + new file
    assert "README.md -> README_renamed.md" in p.stdout
    assert "LICENSE -> LICENSE_renamed" in p.stdout
    if short_flag == "-s":
        assert "R  " in p.stdout
    else:
        assert "renamed:" in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
def test_status_mixed_changes(xtl_clone, git2cpp_path, tmp_path, short_flag):
    """Test status with both staged and unstaged changes"""
    xtl_path = tmp_path / "xtl"

    # Create a new file and stage it
    staged_file = xtl_path / "staged.txt"
    staged_file.write_text("staged content")

    # Deleted a file staged
    del_file = xtl_path / "README.md"
    os.remove(del_file)

    # Stage the two previous files
    subprocess.run([git2cpp_path, "add", "staged.txt", "README.md"], cwd=xtl_path)

    # Modify an existing file without staging
    unstaged_file = xtl_path / "CMakeLists.txt"
    unstaged_file.write_text("unstaged changes")

    # Create an untracked file
    untracked_file = xtl_path / "untracked.txt"
    untracked_file.write_text("untracked")

    cmd_status = [git2cpp_path, "status"]
    if short_flag == "-s":
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=xtl_path, text=True)

    assert p.returncode == 0
    if short_flag == "-s":
        assert "A  staged.txt" in p.stdout
        assert "D  README.md" in p.stdout
        assert " M CMakeLists.txt" in p.stdout
        assert "?? untracked.txt" in p.stdout
    else:
        assert "Changes to be committed" in p.stdout
        assert "new file:   staged.txt" in p.stdout
        assert "deleted:   README.md" in p.stdout
        assert "Changes not staged for commit" in p.stdout
        assert "modified:   CMakeLists.txt" in p.stdout
        assert "Untracked files" in p.stdout
        assert "untracked.txt" in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
def test_status_typechange(xtl_clone, git2cpp_path, tmp_path, short_flag):
    """Test status shows typechange (file to symlink or vice versa)"""
    # Note: This test may need to be skipped on Windows
    if os.name == 'nt':
        pytest.skip("Symlink test not reliable on Windows")

    xtl_path = tmp_path / "xtl"

    # Remove a file and replace with a symlink
    test_file = xtl_path / "README.md"
    os.remove(test_file)
    os.symlink("CMakeLists.txt", test_file)

    cmd_status = [git2cpp_path, "status"]
    if short_flag == "-s":
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=xtl_path, text=True)

    assert p.returncode == 0
    # Should show typechange in unstaged changes
    if short_flag == "-s":
        assert " T " in p.stdout
    else:
        assert "Changes not staged for commit" in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
def test_status_untracked_directory(xtl_clone, git2cpp_path, tmp_path, short_flag):
    """Test that untracked directories are shown with trailing slash"""
    xtl_path = tmp_path / "xtl"

    # Create a directory with files
    new_dir = xtl_path / "new_directory"
    new_dir.mkdir()
    (new_dir / "file1.txt").write_text("content1")
    (new_dir / "file2.txt").write_text("content2")

    cmd_status = [git2cpp_path, "status"]
    if short_flag == "-s":
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=xtl_path, text=True)

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
    subprocess.run([git2cpp_path, "init"], cwd=repo_path)

    # Create initial commit
    test_file = repo_path / "file.txt"
    test_file.write_text("initial")
    subprocess.run([git2cpp_path, "add", "file.txt"], cwd=repo_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "initial"], cwd=repo_path, check=True)

    # Clone it to create remote tracking
    clone_path = tmp_path / "clone"
    subprocess.run(["git", "clone", str(repo_path), str(clone_path)], check=True)

    # Make a commit in clone
    clone_file = clone_path / "file2.txt"
    clone_file.write_text("new file")
    subprocess.run([git2cpp_path, "add", "file2.txt"], cwd=clone_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "second commit"], cwd=clone_path, check=True)

    # Check status
    cmd_status = [git2cpp_path, "status"]
    if (short_flag == "-s"):
        cmd_status.append(short_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=clone_path, text=True)

    assert p.returncode == 0
    if short_flag == "-s":
        assert "...origin/ma" in p.stdout   # "main" locally, but "master" in the CI
        assert "[ahead 1]" in p.stdout
    else:
        assert "Your branch is ahead of" in p.stdout
        assert "by 1 commit" in p.stdout
        assert 'use "git push"' in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s"])
@pytest.mark.parametrize("branch_flag", ["-b", "--branch"])
def test_status_with_branch_and_tracking(commit_env_config, git2cpp_path, tmp_path, short_flag, branch_flag):
    """Test short format with branch flag shows tracking info"""
    # Create a repository with remote tracking
    repo_path = tmp_path / "repo"
    repo_path.mkdir()

    subprocess.run([git2cpp_path, "init"], cwd=repo_path)
    test_file = repo_path / "file.txt"
    test_file.write_text("initial")
    subprocess.run([git2cpp_path, "add", "file.txt"], cwd=repo_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "initial"], cwd=repo_path, check=True)

    # Clone it
    clone_path = tmp_path / "clone"
    subprocess.run(["git", "clone", str(repo_path), str(clone_path)])

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
        assert "## ma" in p.stdout   # "main" locally, but "master" in the CI
        assert "[ahead 1]" in p.stdout
    else:
        assert "On branch ma" in p.stdout   # "main" locally, but "master" in the CI
        assert "Your branch is ahead of 'origin/ma" in p.stdout   # "main" locally, but "master" in the CI
        assert "1 commit." in p.stdout


def test_status_all_headers_shown(xtl_clone, git2cpp_path, tmp_path):
    """Test that all status headers can be shown together"""
    xtl_path = tmp_path / "xtl"

    # Changes to be committed
    staged = xtl_path / "staged.txt"
    staged.write_text("staged")
    subprocess.run([git2cpp_path, "add", "staged.txt"], cwd=xtl_path)

    # Changes not staged
    modified = xtl_path / "CMakeLists.txt"
    modified.write_text("modified")

    # Untracked
    untracked = xtl_path / "untracked.txt"
    untracked.write_text("untracked")

    cmd_status = [git2cpp_path, "status"]
    p = subprocess.run(cmd_status, capture_output=True, cwd=xtl_path, text=True)

    assert p.returncode == 0
    assert "On branch master" in p.stdout
    assert "Changes to be committed:" in p.stdout
    assert 'use "git reset HEAD <file>..." to unstage' in p.stdout
    assert "Changes not staged for commit:" in p.stdout
    assert 'use "git add <file>..." to update what will be committed' in p.stdout
    assert "Untracked files:" in p.stdout
    assert 'use "git add <file>..." to include in what will be committed' in p.stdout
