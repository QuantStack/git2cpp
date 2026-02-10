import subprocess

import pytest


def test_rm_basic_file(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test basic rm operation to remove a file"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a test file
    test_file = xtl_path / "test_file.txt"
    test_file.write_text("test content")

    # Add and commit the file
    add_cmd = [git2cpp_path, "add", "test_file.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "Add test file"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    # Remove the file
    rm_cmd = [git2cpp_path, "rm", "test_file.txt"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode == 0

    # Verify the file was removed from working tree
    assert not test_file.exists()

    # Check git status
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_status.returncode == 0
    assert "Changes to be committed" in p_status.stdout
    assert "deleted" in p_status.stdout


def test_rm_multiple_files(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test removing multiple files at once"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create test files
    file1 = xtl_path / "file1.txt"
    file1.write_text("content 1")
    file2 = xtl_path / "file2.txt"
    file2.write_text("content 2")

    # Add and commit files
    add_cmd = [git2cpp_path, "add", "file1.txt", "file2.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "Add test files"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    # Remove both files
    rm_cmd = [git2cpp_path, "rm", "file1.txt", "file2.txt"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode == 0

    # Verify both files were removed
    assert not file1.exists()
    assert not file2.exists()

    # Check git status
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_status.returncode == 0
    assert "Changes to be committed" in p_status.stdout
    assert "deleted" in p_status.stdout


def test_rm_directory_without_recursive_flag(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that rm fails when trying to remove a directory without -r flag"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a directory with a file
    test_dir = xtl_path / "test_dir"
    test_dir.mkdir()
    test_file = test_dir / "file.txt"
    test_file.write_text("content")

    # Add and commit the file
    add_cmd = [git2cpp_path, "add", "test_dir/file.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "Add test directory"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    # Try to remove directory without -r flag - should fail
    rm_cmd = [git2cpp_path, "rm", "test_dir"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode != 0
    assert "not removing" in p_rm.stderr and "recursively without -r" in p_rm.stderr

    # Verify directory still exists
    assert test_dir.exists()
    assert test_file.exists()


def test_rm_directory_with_recursive_flag(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test removing a directory with -r flag"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a directory with files
    test_dir = xtl_path / "test_dir"
    test_dir.mkdir()
    file1 = test_dir / "file1.txt"
    file1.write_text("content 1")
    file2 = test_dir / "file2.txt"
    file2.write_text("content 2")

    # Add and commit the files
    add_cmd = [git2cpp_path, "add", "test_dir/file1.txt", "test_dir/file2.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "Add test directory"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    # Remove directory with -r flag - should succeed
    rm_cmd = [git2cpp_path, "rm", "-r", "test_dir"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode == 0

    # Check git status
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_status.returncode == 0
    assert "Changes to be committed" in p_status.stdout
    assert "deleted" in p_status.stdout


def test_rm_and_commit(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test removing a file and committing the change"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a test file
    test_file = xtl_path / "to_remove.txt"
    test_file.write_text("content to remove")

    # Add and commit the file
    add_cmd = [git2cpp_path, "add", "to_remove.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "Add file to remove"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    # Remove the file
    rm_cmd = [git2cpp_path, "rm", "to_remove.txt"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode == 0

    # Check status before commit
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_status.returncode == 0
    assert "Changes to be committed" in p_status.stdout
    assert "deleted" in p_status.stdout

    # Commit the removal
    commit_cmd2 = [git2cpp_path, "commit", "-m", "Remove file"]
    p_commit2 = subprocess.run(commit_cmd2, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit2.returncode == 0

    # Verify the file is gone
    assert not test_file.exists()

    # Check status after commit
    status_cmd2 = [git2cpp_path, "status", "--long"]
    p_status2 = subprocess.run(status_cmd2, capture_output=True, cwd=xtl_path, text=True)
    assert p_status2.returncode == 0
    assert "to_remove.txt" not in p_status2.stdout


def test_rm_nonexistent_file(xtl_clone, git2cpp_path, tmp_path):
    """Test that rm fails when file doesn't exist"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Try to remove a file that doesn't exist
    rm_cmd = [git2cpp_path, "rm", "nonexistent.txt"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode != 0


def test_rm_untracked_file(xtl_clone, git2cpp_path, tmp_path):
    """Test that rm fails when trying to remove an untracked file"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create an untracked file
    untracked_file = xtl_path / "untracked.txt"
    untracked_file.write_text("untracked content")

    # Try to remove untracked file - should fail
    rm_cmd = [git2cpp_path, "rm", "untracked.txt"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode != 0


def test_rm_staged_file(xtl_clone, git2cpp_path, tmp_path):
    """Test removing a file that was added but not yet committed"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a test file
    test_file = xtl_path / "staged.txt"
    test_file.write_text("staged content")

    # Add the file (but don't commit)
    add_cmd = [git2cpp_path, "add", "staged.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    # Remove the file
    rm_cmd = [git2cpp_path, "rm", "staged.txt"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode == 0

    # Verify the file was removed
    assert not test_file.exists()

    # Check git status - should show nothing staged
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_status.returncode == 0
    assert "Changes to be committed" not in p_status.stdout
    assert "staged.txt" not in p_status.stdout


def test_rm_file_in_subdirectory(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test removing a file in a subdirectory"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Use existing subdirectory
    test_file = xtl_path / "include" / "test.txt"
    test_file.write_text("test content")

    # Add and commit the file
    add_cmd = [git2cpp_path, "add", "include/test.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "Add file in subdirectory"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    # Remove the file
    rm_cmd = [git2cpp_path, "rm", "include/test.txt"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode == 0

    # Verify the file was removed
    assert not test_file.exists()

    # Check git status
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_status.returncode == 0
    assert "Changes to be committed" in p_status.stdout
    assert "deleted" in p_status.stdout


def test_rm_nogit(git2cpp_path, tmp_path):
    """Test that rm fails when not in a git repository"""
    # Create a test file outside a git repo
    test_file = tmp_path / "test.txt"
    test_file.write_text("test content")

    # Try to rm without being in a git repo
    rm_cmd = [git2cpp_path, "rm", "test.txt"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_rm.returncode != 0


def test_rm_nested_directory_recursive(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test removing a nested directory structure with -r flag"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create nested directory structure
    nested_dir = xtl_path / "level1" / "level2"
    nested_dir.mkdir(parents=True)
    file1 = xtl_path / "level1" / "file1.txt"
    file1.write_text("content 1")
    file2 = nested_dir / "file2.txt"
    file2.write_text("content 2")

    # Add and commit the files
    add_cmd = [git2cpp_path, "add", "level1/file1.txt", "level1/level2/file2.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "Add nested structure"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    # Remove the directory tree with -r flag
    rm_cmd = [git2cpp_path, "rm", "-r", "level1"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode == 0

    # Check git status
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_status.returncode == 0
    assert "Changes to be committed" in p_status.stdout
    assert "deleted" in p_status.stdout


def test_rm_mixed_files_and_directory(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test removing both individual files and directories in one command"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a file and a directory with contents
    single_file = xtl_path / "single.txt"
    single_file.write_text("single file")

    test_dir = xtl_path / "remove_dir"
    test_dir.mkdir()
    dir_file = test_dir / "file.txt"
    dir_file.write_text("file in dir")

    # Add and commit everything
    add_cmd = [git2cpp_path, "add", "single.txt", "remove_dir/file.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "Add mixed content"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    # Remove both file and directory
    rm_cmd = [git2cpp_path, "rm", "-r", "single.txt", "remove_dir"]
    p_rm = subprocess.run(rm_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rm.returncode == 0

    # Verify everything was removed
    assert not single_file.exists()

    # Check git status
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_status.returncode == 0
    assert "Changes to be committed" in p_status.stdout
    assert "deleted" in p_status.stdout
