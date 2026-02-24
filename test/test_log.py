import subprocess

import pytest


@pytest.mark.parametrize("format_flag", ["", "--format=full", "--format=fuller"])
def test_log(xtl_clone, commit_env_config, git2cpp_path, tmp_path, format_flag):
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
    if format_flag != "":
        cmd_log.append(format_flag)
    p_log = subprocess.run(cmd_log, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0
    assert "Jane Doe" in p_log.stdout
    assert "test commit" in p_log.stdout

    if format_flag == "":
        assert "Commit" not in p_log.stdout
    else:
        assert "Commit" in p_log.stdout
        if format_flag == "--format=full":
            assert "Date" not in p_log.stdout
        else:
            assert "CommitDate" in p_log.stdout


def test_log_nogit(commit_env_config, git2cpp_path, tmp_path):
    cmd_log = [git2cpp_path, "log"]
    p_log = subprocess.run(cmd_log, capture_output=True, cwd=tmp_path, text=True)
    assert p_log.returncode != 0
    assert "error: could not find repository at" in p_log.stderr


@pytest.mark.parametrize("max_count_flag", ["", "-n", "--max-count"])
def test_max_count(
    xtl_clone, commit_env_config, git2cpp_path, tmp_path, max_count_flag
):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    cmd_log = [git2cpp_path, "log"]
    if max_count_flag != "":
        cmd_log.append(max_count_flag)
        cmd_log.append("2")
    p_log = subprocess.run(cmd_log, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0

    if max_count_flag == "":
        assert p_log.stdout.count("Author") > 2
    else:
        assert p_log.stdout.count("Author") == 2


def test_log_with_head_reference(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that HEAD reference is shown on the latest commit."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a new commit
    p = xtl_path / "test_file.txt"
    p.write_text("test content")

    subprocess.run([git2cpp_path, "add", "test_file.txt"], cwd=xtl_path, check=True)
    subprocess.run(
        [git2cpp_path, "commit", "-m", "test commit"], cwd=xtl_path, check=True
    )

    # Run log with max count 1 to get only the latest commit
    p_log = subprocess.run(
        [git2cpp_path, "log", "-n", "1"], capture_output=True, cwd=xtl_path, text=True
    )
    assert p_log.returncode == 0

    # Check that HEAD reference is shown
    assert "HEAD ->" in p_log.stdout
    assert "master" in p_log.stdout or "main" in p_log.stdout


def test_log_with_tag(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that tags are shown in log output."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a commit and tag it
    p = xtl_path / "tagged_file.txt"
    p.write_text("tagged content")

    subprocess.run([git2cpp_path, "add", "tagged_file.txt"], cwd=xtl_path, check=True)
    subprocess.run(
        [git2cpp_path, "commit", "-m", "tagged commit"], cwd=xtl_path, check=True
    )

    # Create a tag (using git command since git2cpp might not have tag creation yet)
    subprocess.run(["git", "tag", "v1.0.0"], cwd=xtl_path, check=True)

    # Run log
    p_log = subprocess.run(
        [git2cpp_path, "log", "-n", "1"], capture_output=True, cwd=xtl_path, text=True
    )
    assert p_log.returncode == 0

    # Check that tag is shown
    assert "tag: v1.0.0" in p_log.stdout


def test_log_with_multiple_tags(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that multiple tags on the same commit are all shown."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a commit
    p = xtl_path / "multi_tag_file.txt"
    p.write_text("content")

    subprocess.run(
        [git2cpp_path, "add", "multi_tag_file.txt"], cwd=xtl_path, check=True
    )
    subprocess.run(
        [git2cpp_path, "commit", "-m", "multi tag commit"], cwd=xtl_path, check=True
    )

    # Create multiple tags
    subprocess.run(["git", "tag", "v1.0.0"], cwd=xtl_path, check=True)
    subprocess.run(["git", "tag", "stable"], cwd=xtl_path, check=True)
    subprocess.run(["git", "tag", "latest"], cwd=xtl_path, check=True)

    # Run log
    p_log = subprocess.run(
        [git2cpp_path, "log", "-n", "1"], capture_output=True, cwd=xtl_path, text=True
    )
    assert p_log.returncode == 0

    # Check that all tags are shown
    assert "tag: v1.0.0" in p_log.stdout
    assert "tag: stable" in p_log.stdout
    assert "tag: latest" in p_log.stdout


def test_log_with_annotated_tag(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that annotated tags are shown in log output."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a commit
    p = xtl_path / "annotated_tag_file.txt"
    p.write_text("content")

    subprocess.run(
        [git2cpp_path, "add", "annotated_tag_file.txt"], cwd=xtl_path, check=True
    )
    subprocess.run(
        [git2cpp_path, "commit", "-m", "annotated tag commit"], cwd=xtl_path, check=True
    )

    # Create an annotated tag
    subprocess.run(
        ["git", "tag", "-a", "v2.0.0", "-m", "Version 2.0.0"], cwd=xtl_path, check=True
    )

    # Run log
    p_log = subprocess.run(
        [git2cpp_path, "log", "-n", "1"], capture_output=True, cwd=xtl_path, text=True
    )
    assert p_log.returncode == 0

    # Check that annotated tag is shown
    assert "tag: v2.0.0" in p_log.stdout


def test_log_with_branch(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that branches are shown in log output."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a commit
    p = xtl_path / "branch_file.txt"
    p.write_text("content")

    subprocess.run([git2cpp_path, "add", "branch_file.txt"], cwd=xtl_path, check=True)
    subprocess.run(
        [git2cpp_path, "commit", "-m", "branch commit"], cwd=xtl_path, check=True
    )

    # Create a new branch pointing to HEAD
    subprocess.run(["git", "branch", "feature-branch"], cwd=xtl_path, check=True)

    # Run log
    p_log = subprocess.run(
        [git2cpp_path, "log", "-n", "1"], capture_output=True, cwd=xtl_path, text=True
    )
    assert p_log.returncode == 0

    # Check that both branches are shown (HEAD -> master/main and feature-branch)
    assert "feature-branch" in p_log.stdout


def test_log_with_remote_branches(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that remote branches are shown in log output."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # The xtl_clone fixture already has remote branches (origin/master, etc.)
    # Run log to check they appear
    p_log = subprocess.run(
        [git2cpp_path, "log", "-n", "1"], capture_output=True, cwd=xtl_path, text=True
    )
    assert p_log.returncode == 0

    # Check that origin remote branches are shown
    assert "origin/master" in p_log.stdout


def test_log_commit_without_references(
    xtl_clone, commit_env_config, git2cpp_path, tmp_path
):
    """Test that commits without any references don't show empty parentheses."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create two commits - the second one will have refs, the first won't
    for i in range(2):
        p = xtl_path / f"file_{i}.txt"
        p.write_text(f"content {i}")
        subprocess.run([git2cpp_path, "add", f"file_{i}.txt"], cwd=xtl_path, check=True)
        subprocess.run(
            [git2cpp_path, "commit", "-m", f"commit {i}"], cwd=xtl_path, check=True
        )

    # Run log with 2 commits
    p_log = subprocess.run(
        [git2cpp_path, "log", "-n", "2"], capture_output=True, cwd=xtl_path, text=True
    )
    assert p_log.returncode == 0

    # First commit line should have references
    lines = p_log.stdout.split("\n")
    first_commit_line = [l for l in lines if l.startswith("commit")][0]
    assert "(" in first_commit_line  # Has references

    # Second commit (older one) should not have empty parentheses
    second_commit_line = [l for l in lines if l.startswith("commit")][1]
    # Should either have no parentheses or have actual references
    if "(" in second_commit_line:
        # If it has parentheses, they shouldn't be empty
        assert "()" not in second_commit_line
