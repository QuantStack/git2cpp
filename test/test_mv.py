import subprocess

import pytest


def test_mv_basic(git2cpp_path, tmp_path):
    """Test basic mv operation to rename a file"""
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    # Create a test file
    test_file = tmp_path / "test_file.txt"
    test_file.write_text("test content")

    # Add the file to git
    add_cmd = [git2cpp_path, "add", "test_file.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    # Move/rename the file
    mv_cmd = [git2cpp_path, "mv", "test_file.txt", "renamed_file.txt"]
    p_mv = subprocess.run(mv_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_mv.returncode == 0

    # Verify the file was moved
    assert not test_file.exists()
    assert (tmp_path / "renamed_file.txt").exists()

    # Check git status
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_status.returncode == 0
    assert "new file:   renamed_file.txt" in p_status.stdout


def test_mv_to_subdirectory(git2cpp_path, tmp_path):
    """Test moving a file to a subdirectory"""
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    # Create a test file
    test_file = tmp_path / "move_me.txt"
    test_file.write_text("content to move")

    # Add the file to git
    add_cmd = [git2cpp_path, "add", "move_me.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    # Create a subdirectory
    test_dir = tmp_path / "test"
    test_dir.mkdir()

    # Move the file to existing subdirectory
    mv_cmd = [git2cpp_path, "mv", "move_me.txt", "test/move_me.txt"]
    p_mv = subprocess.run(mv_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_mv.returncode == 0

    # Verify the file was moved
    assert not test_file.exists()
    assert (test_dir / "move_me.txt").exists()

    # Check git status
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_status.returncode == 0
    assert "new file:   test/move_me.txt" in p_status.stdout


def test_mv_destination_exists_without_force(git2cpp_path, tmp_path):
    """Test that mv fails when destination exists without --force flag"""
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    # Create source file
    source_file = tmp_path / "source.txt"
    source_file.write_text("source content")

    # Create destination file
    dest_file = tmp_path / "destination.txt"
    dest_file.write_text("destination content")

    # Add both files to git
    add_cmd = [git2cpp_path, "add", "source.txt", "destination.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    # Try to move without force - should fail
    mv_cmd = [git2cpp_path, "mv", "source.txt", "destination.txt"]
    p_mv = subprocess.run(mv_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_mv.returncode != 0
    assert "destination already exists" in p_mv.stderr

    # Verify source file still exists
    assert source_file.exists()
    assert dest_file.exists()


@pytest.mark.parametrize("force_flag", ["-f", "--force"])
def test_mv_destination_exists_with_force(git2cpp_path, tmp_path, force_flag):
    """Test that mv succeeds when destination exists with --force flag"""
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    # Create source file
    source_file = tmp_path / "source.txt"
    source_file.write_text("source content")

    # Create destination file
    dest_file = tmp_path / "destination.txt"
    dest_file.write_text("destination content")

    # Add both files to git
    add_cmd = [git2cpp_path, "add", "source.txt", "destination.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    # Move with force - should succeed
    mv_cmd = [git2cpp_path, "mv", force_flag, "source.txt", "destination.txt"]
    p_mv = subprocess.run(mv_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_mv.returncode == 0

    # Verify source file was moved
    assert not source_file.exists()
    assert dest_file.exists()
    assert dest_file.read_text() == "source content"


def test_mv_nonexistent_source(git2cpp_path, tmp_path):
    """Test that mv fails when source file doesn't exist"""
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    # Try to move a file that doesn't exist
    mv_cmd = [git2cpp_path, "mv", "nonexistent.txt", "destination.txt"]
    p_mv = subprocess.run(mv_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_mv.returncode != 0


def test_mv_multiple_files(commit_env_config, git2cpp_path, tmp_path):
    """Test moving multiple files sequentially"""
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    # Create test files
    file1 = tmp_path / "file1.txt"
    file1.write_text("content 1")
    file2 = tmp_path / "file2.txt"
    file2.write_text("content 2")

    # Add files to git
    add_cmd = [git2cpp_path, "add", "file1.txt", "file2.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    # Commit the files
    commit_cmd = [git2cpp_path, "commit", "-m", "Add test files"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_commit.returncode == 0

    # Move first file
    mv_cmd1 = [git2cpp_path, "mv", "file1.txt", "renamed1.txt"]
    p_mv1 = subprocess.run(mv_cmd1, capture_output=True, cwd=tmp_path, text=True)
    assert p_mv1.returncode == 0

    # Move second file
    mv_cmd2 = [git2cpp_path, "mv", "file2.txt", "renamed2.txt"]
    p_mv2 = subprocess.run(mv_cmd2, capture_output=True, cwd=tmp_path, text=True)
    assert p_mv2.returncode == 0

    # Verify both files were moved
    assert not file1.exists()
    assert not file2.exists()
    assert (tmp_path / "renamed1.txt").exists()
    assert (tmp_path / "renamed2.txt").exists()


def test_mv_and_commit(commit_env_config, git2cpp_path, tmp_path):
    """Test moving a file and committing the change"""
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    # Create a test file
    test_file = tmp_path / "original.txt"
    test_file.write_text("original content")

    # Add and commit the file
    add_cmd = [git2cpp_path, "add", "original.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "Add original file"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_commit.returncode == 0

    # Move the file
    mv_cmd = [git2cpp_path, "mv", "original.txt", "moved.txt"]
    p_mv = subprocess.run(mv_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_mv.returncode == 0

    # Check status before commit
    status_cmd = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_status.returncode == 0
    assert "Changes to be committed" in p_status.stdout

    # Commit the move
    commit_cmd2 = [git2cpp_path, "commit", "-m", "Move file"]
    p_commit2 = subprocess.run(
        commit_cmd2, capture_output=True, cwd=tmp_path, text=True
    )
    assert p_commit2.returncode == 0

    # Verify the file is in the new location
    assert not (tmp_path / "original.txt").exists()
    assert (tmp_path / "moved.txt").exists()


def test_mv_nogit(git2cpp_path, tmp_path):
    """Test that mv fails when not in a git repository"""
    # Create a test file outside a git repo
    test_file = tmp_path / "test.txt"
    test_file.write_text("test content")

    # Try to mv without being in a git repo
    mv_cmd = [git2cpp_path, "mv", "test.txt", "moved.txt"]
    p_mv = subprocess.run(mv_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_mv.returncode != 0


def test_mv_preserve_content(git2cpp_path, tmp_path):
    """Test that file content is preserved after mv"""
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    # Create a test file with specific content
    test_content = "This is important content that should be preserved"
    test_file = tmp_path / "important.txt"
    test_file.write_text(test_content)

    # Add the file to git
    add_cmd = [git2cpp_path, "add", "important.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    # Move the file
    mv_cmd = [git2cpp_path, "mv", "important.txt", "preserved.txt"]
    p_mv = subprocess.run(mv_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_mv.returncode == 0

    # Verify content is preserved
    moved_file = tmp_path / "preserved.txt"
    assert moved_file.exists()
    assert moved_file.read_text() == test_content
