import subprocess

import pytest

def test_tag_list_empty(xtl_clone, git2cpp_path, tmp_path):
    """Test listing tags when there are no tags."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    cmd = [git2cpp_path, 'tag']
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "0.2.0" in p.stdout


def test_tag_create_lightweight(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test creating a lightweight tag."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a lightweight tag
    create_cmd = [git2cpp_path, 'tag', 'v1.0.0']
    subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, text=True, check=True)

    # List tags to verify it was created
    list_cmd = [git2cpp_path, 'tag']
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert 'v1.0.0' in p_list.stdout


def test_tag_create_annotated(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test creating an annotated tag."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create an annotated tag
    create_cmd = [git2cpp_path, 'tag', '-m', 'Release version 1.0', 'v1.0.0']
    subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, text=True, check=True)

    # List tags to verify it was created
    list_cmd = [git2cpp_path, 'tag', "-n", "1"]
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert 'v1.0.0' in p_list.stdout
    assert 'Release version 1.0' in p_list.stdout


def test_tag_create_on_specific_commit(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test creating a tag on a specific commit."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Get the commit SHA before creating new commit
    old_head_cmd = ['git', 'rev-parse', 'HEAD']
    p_old_head = subprocess.run(old_head_cmd, capture_output=True, cwd=xtl_path, text=True)
    old_head_sha = p_old_head.stdout.strip()

    # Create a commit first
    file_path = xtl_path / "test_file.txt"
    file_path.write_text("test content")

    add_cmd = [git2cpp_path, 'add', 'test_file.txt']
    subprocess.run(add_cmd, cwd=xtl_path, check=True)

    commit_cmd = [git2cpp_path, 'commit', '-m', 'test commit']
    subprocess.run(commit_cmd, cwd=xtl_path, check=True)

    # Get new HEAD commit SHA
    new_head_cmd = ['git', 'rev-parse', 'HEAD']
    p_new_head = subprocess.run(new_head_cmd, capture_output=True, cwd=xtl_path, text=True)
    new_head_sha = p_new_head.stdout.strip()

    # Verify we actually created a new commit
    assert old_head_sha != new_head_sha

    # Create tag on HEAD
    tag_cmd = [git2cpp_path, 'tag', 'v1.0.0', 'HEAD']
    subprocess.run(tag_cmd, capture_output=True, cwd=xtl_path, check=True)

    # Verify tag exists
    list_cmd = [git2cpp_path, 'tag']
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert 'v1.0.0' in p_list.stdout

    # Get commit SHA that the tag points to
    tag_sha_cmd = ['git', 'rev-parse', 'v1.0.0^{commit}']
    p_tag_sha = subprocess.run(tag_sha_cmd, capture_output=True, cwd=xtl_path, text=True)
    tag_sha = p_tag_sha.stdout.strip()

    # Verify tag points to new HEAD, not old HEAD
    assert tag_sha == new_head_sha
    assert tag_sha != old_head_sha


def test_tag_delete(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test deleting a tag."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a tag
    create_cmd = [git2cpp_path, 'tag', 'v1.0.0']
    subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, text=True, check=True)

    # Delete the tag
    delete_cmd = [git2cpp_path, 'tag', '-d', 'v1.0.0']
    p_delete = subprocess.run(delete_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_delete.returncode == 0
    assert "Deleted tag 'v1.0.0'" in p_delete.stdout

    # Verify tag is gone
    list_cmd = [git2cpp_path, 'tag']
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert 'v1.0.0' not in p_list.stdout


def test_tag_delete_nonexistent(xtl_clone, git2cpp_path, tmp_path):
    """Test deleting a tag that doesn't exist."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Try to delete non-existent tag
    delete_cmd = [git2cpp_path, 'tag', '-d', 'nonexistent']
    p_delete = subprocess.run(delete_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_delete.returncode != 0
    assert "not found" in p_delete.stderr


@pytest.mark.parametrize("list_flag", ["-l", "--list"])
def test_tag_list_with_flag(xtl_clone, commit_env_config, git2cpp_path, tmp_path, list_flag):
    """Test listing tags with -l or --list flag."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a tag
    tag_cmd = [git2cpp_path, 'tag', 'v1.0.0']
    subprocess.run(tag_cmd, capture_output=True, cwd=xtl_path, text=True)

    # List tags
    list_cmd = [git2cpp_path, 'tag', list_flag]
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert 'v1.0.0' in p_list.stdout


def test_tag_list_with_pattern(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test listing tags with a pattern."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create tags with different prefixes
    tag_cmd_1 = [git2cpp_path, 'tag', 'v1.0.0']
    subprocess.run(tag_cmd_1, capture_output=True, cwd=xtl_path, text=True)

    tag_cmd_2 = [git2cpp_path, 'tag', 'v1.0.1']
    subprocess.run(tag_cmd_2, capture_output=True, cwd=xtl_path, text=True)

    tag_cmd_3 = [git2cpp_path, 'tag', 'release-1.0']
    subprocess.run(tag_cmd_3, capture_output=True, cwd=xtl_path, text=True)

    # List only tags matching pattern
    list_cmd = [git2cpp_path, 'tag', '-l', 'v1.0*']
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert 'v1.0.0' in p_list.stdout
    assert 'v1.0.1' in p_list.stdout
    assert 'release-1.0' not in p_list.stdout


def test_tag_list_with_message_lines(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test listing tags with message lines (-n flag)."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create an annotated tag with a message
    create_cmd = [git2cpp_path, 'tag', '-m', 'First line\nSecond line\nThird line\nForth line', 'v1.0.0']
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_create.returncode == 0

    # List tags with message lines
    list_cmd = [git2cpp_path, 'tag', '-n', '3', '-l']
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert 'v1.0.0' in p_list.stdout
    assert 'First line' in p_list.stdout
    assert 'Second line' in p_list.stdout
    assert 'Third line' in p_list.stdout
    assert 'Forth line' not in p_list.stdout


@pytest.mark.parametrize("force_flag", ["-f", "--force"])
def test_tag_force_replace(xtl_clone, commit_env_config, git2cpp_path, tmp_path, force_flag):
    """Test replacing an existing tag with -f or --force flag."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create initial tag
    create_cmd_1 = [git2cpp_path, 'tag', 'v1.0.0']
    subprocess.run(create_cmd_1, capture_output=True, cwd=xtl_path, text=True, check=True)

    # Try to create same tag without force (should fail)
    create_cmd_2 = [git2cpp_path, 'tag', 'v1.0.0']
    p_create_2 = subprocess.run(create_cmd_2, capture_output=True, cwd=xtl_path)
    assert p_create_2.returncode != 0

    # Create same tag with force (should succeed)
    create_cmd_3 = [git2cpp_path, 'tag', force_flag, 'v1.0.0']
    p_create_3 = subprocess.run(create_cmd_3, capture_output=True, cwd=xtl_path, text=True)
    assert p_create_3.returncode == 0


def test_tag_nogit(git2cpp_path, tmp_path):
    """Test tag command outside a git repository."""
    cmd = [git2cpp_path, 'tag']
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode != 0


def test_tag_annotated_no_message(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test creating an annotated tag without a message should fail."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a commit with a known message
    file_path = xtl_path / "test_file.txt"
    file_path.write_text("test content")

    add_cmd = [git2cpp_path, 'add', 'test_file.txt']
    subprocess.run(add_cmd, cwd=xtl_path, check=True)

    commit_cmd = [git2cpp_path, 'commit', '-m', 'my specific commit message']
    subprocess.run(commit_cmd, cwd=xtl_path, check=True)

    # Create tag with empty message (should create lightweight tag)
    create_cmd = [git2cpp_path, 'tag', '-m', '', 'v1.0.0']
    subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, check=True)

     # List tag with messages - lightweight tag shows commit message
    list_cmd = [git2cpp_path, 'tag', '-n', '1']
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert 'v1.0.0' in p_list.stdout
    # Lightweight tag shows the commit message, not a tag message
    assert 'my specific commit message' in p_list.stdout


def test_tag_multiple_create_and_list(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test creating multiple tags and listing them."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create multiple tags
    tags = ['v1.0.0', 'v1.0.1', 'v1.1.0', 'v2.0.0']
    for tag in tags:
        create_cmd = [git2cpp_path, 'tag', tag]
        subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, check=True)

    # List all tags
    list_cmd = [git2cpp_path, 'tag']
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0

    # Verify all tags are in the list
    for tag in tags:
        assert tag in p_list.stdout


def test_tag_on_new_commit(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test creating tags on new commits."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Tag the current commit
    tag_cmd_1 = [git2cpp_path, 'tag', 'before-change']
    subprocess.run(tag_cmd_1, cwd=xtl_path, check=True)

    # Make a new commit
    file_path = xtl_path / "new_file.txt"
    file_path.write_text("new content")

    add_cmd = [git2cpp_path, 'add', 'new_file.txt']
    subprocess.run(add_cmd, cwd=xtl_path, check=True)

    commit_cmd = [git2cpp_path, 'commit', '-m', 'new commit']
    subprocess.run(commit_cmd, cwd=xtl_path, check=True)

    # Tag the new commit
    tag_cmd_2 = [git2cpp_path, 'tag', 'after-change']
    subprocess.run(tag_cmd_2, cwd=xtl_path, check=True)

    # List tags
    list_cmd = [git2cpp_path, 'tag']
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert 'before-change' in p_list.stdout
    assert 'after-change' in p_list.stdout
