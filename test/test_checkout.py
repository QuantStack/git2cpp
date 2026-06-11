import subprocess

import pytest

from .conftest import strip_ansi_colours


def test_checkout(repo_init_with_commit, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    create_cmd = [git2cpp_path, "branch", "foregone"]
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_create.returncode == 0

    checkout_cmd = [git2cpp_path, "checkout", "foregone"]
    p_checkout = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_checkout.returncode == 0
    assert "Switched to branch 'foregone'" in p_checkout.stdout

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_branch.returncode == 0
    assert p_branch.stdout == "* foregone\n  main\n"

    checkout_cmd[2] = "main"
    p_checkout2 = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_checkout2.returncode == 0
    assert "Switched to branch 'main'" in p_checkout2.stdout


def test_checkout_b(repo_init_with_commit, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    checkout_cmd = [git2cpp_path, "checkout", "-b", "foregone"]
    p_checkout = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_checkout.returncode == 0
    assert "Switched to a new branch 'foregone'" in p_checkout.stdout

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_branch.returncode == 0
    assert p_branch.stdout == "* foregone\n  main\n"

    checkout_cmd.remove("-b")
    checkout_cmd[2] = "main"
    p_checkout2 = subprocess.run(checkout_cmd, cwd=tmp_path, text=True)
    assert p_checkout2.returncode == 0

    p_branch2 = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_branch2.returncode == 0
    assert p_branch2.stdout == "  foregone\n* main\n"


def test_checkout_B_force_create(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test checkout -B to force create or reset a branch"""
    assert (tmp_path / "initial.txt").exists()

    # Create a branch first
    create_cmd = [git2cpp_path, "branch", "resetme"]
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_create.returncode == 0

    # Use -B to reset it (should not fail even if branch exists)
    checkout_cmd = [git2cpp_path, "checkout", "-B", "resetme"]
    p_checkout = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_checkout.returncode == 0
    assert "Switched to a new branch 'resetme'" in p_checkout.stdout

    # Verify we're on the branch
    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_branch.returncode == 0
    assert "* resetme" in p_branch.stdout


def test_checkout_invalid_branch(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that checkout fails gracefully with invalid branch name"""
    assert (tmp_path / "initial.txt").exists()

    # Try to checkout non-existent branch
    checkout_cmd = [git2cpp_path, "checkout", "nonexistent"]
    p_checkout = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)

    # Should fail with error message
    assert p_checkout.returncode != 0
    assert (
        "error: pathspec 'nonexistent' did not match any file(s) known to git" in p_checkout.stderr
    )


def test_checkout_with_unstaged_changes(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that checkout shows unstaged changes when switching branches"""
    initial_file = tmp_path / "initial.txt"
    assert (initial_file).exists()

    # Create a new branch
    create_cmd = [git2cpp_path, "branch", "newbranch"]
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_create.returncode == 0

    # Modify a file (unstaged change)
    initial_file.write_text("Modified content")

    # Checkout - should succeed and show the modified file status
    checkout_cmd = [git2cpp_path, "checkout", "newbranch"]
    p_checkout = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)

    # Should succeed and show status
    assert p_checkout.returncode == 0
    p_checkout.stdout = strip_ansi_colours(p_checkout.stdout)
    assert " M initial.txt" in p_checkout.stdout
    assert "Switched to branch 'newbranch'" in p_checkout.stdout


@pytest.mark.parametrize("force_flag", ["", "-f", "--force"])
def test_checkout_refuses_overwrite(
    repo_init_with_commit, commit_env_config, git2cpp_path, tmp_path, force_flag
):
    """Test that checkout refuses to switch when local changes would be overwritten, and switches when using --force"""
    initial_file = tmp_path / "initial.txt"
    assert (initial_file).exists()

    # Create a new branch and switch to it
    create_cmd = [git2cpp_path, "checkout", "-b", "newbranch"]
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_create.returncode == 0

    # Modify initial.txt and commit it on newbranch
    initial_file.write_text("Content on newbranch")

    add_cmd = [git2cpp_path, "add", "initial.txt"]
    subprocess.run(add_cmd, cwd=tmp_path, text=True, check=True)

    commit_cmd = [git2cpp_path, "commit", "-m", "Change on newbranch"]
    subprocess.run(commit_cmd, cwd=tmp_path, text=True, check=True)

    # Switch back to default branch
    checkout_default_cmd = [git2cpp_path, "checkout", "main"]
    p_default = subprocess.run(checkout_default_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_default.returncode == 0

    # Now modify initial.txt locally (unstaged) on default branch
    initial_file.write_text("Local modification on main")

    # Try to checkout newbranch
    checkout_cmd = [git2cpp_path, "checkout"]
    if force_flag != "":
        checkout_cmd.append(force_flag)
    checkout_cmd.append("newbranch")
    p_checkout = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)

    if force_flag == "":
        assert p_checkout.returncode != 0
        assert (
            "Your local changes to the following files would be overwritten by checkout:"
            in p_checkout.stdout
        )
        assert "initial.txt" in p_checkout.stdout
        assert (
            "Please commit your changes or stash them before you switch branches"
            in p_checkout.stdout
        )

        # Verify we're still on default branch (didn't switch)
        branch_cmd = [git2cpp_path, "branch"]
        p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
        assert "* main" in p_branch.stdout
    else:
        assert "Switched to branch 'newbranch'" in p_checkout.stdout

        # Verify we switched to newbranch
        branch_cmd = [git2cpp_path, "branch"]
        p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
        assert "* newbranch" in p_branch.stdout


def test_checkout_file_restores_modified_file(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that checkout -- <file> discards working tree changes"""
    initial_file = tmp_path / "initial.txt"
    original_content = initial_file.read_text()

    # Modify the file (unstaged)
    initial_file.write_text("Modified content")
    assert initial_file.read_text() == "Modified content"

    # Restore it via checkout -- <file>
    checkout_cmd = [git2cpp_path, "checkout", "--", "initial.txt"]
    p = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode == 0
    assert initial_file.read_text() == original_content


def test_checkout_file_restores_multiple_files(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that checkout -- <file1> <file2> restores multiple files at once"""
    initial_file = tmp_path / "initial.txt"

    # Create and commit a second file first
    second_file = tmp_path / "second.txt"
    second_file.write_text("second content")

    add_cmd = [git2cpp_path, "add", "second.txt"]
    subprocess.run(add_cmd, cwd=tmp_path, text=True, check=True)
    commit_cmd = [git2cpp_path, "commit", "-m", "Add second file"]
    subprocess.run(commit_cmd, cwd=tmp_path, text=True, check=True)

    original_initial = initial_file.read_text()
    original_second = second_file.read_text()

    # Modify both files
    initial_file.write_text("dirty initial")
    second_file.write_text("dirty second")

    checkout_cmd = [git2cpp_path, "checkout", "--", "initial.txt", "second.txt"]
    p = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode == 0
    assert initial_file.read_text() == original_initial
    assert second_file.read_text() == original_second


def test_checkout_file_does_not_affect_other_files(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that checkout -- <file> only touches the specified file"""
    initial_file = tmp_path / "initial.txt"
    original_initial = initial_file.read_text()

    # Create and commit a second file
    second_file = tmp_path / "second.txt"
    second_file.write_text("second content")

    add_cmd = [git2cpp_path, "add", "second.txt"]
    subprocess.run(add_cmd, cwd=tmp_path, text=True, check=True)
    commit_cmd = [git2cpp_path, "commit", "-m", "Add second file"]
    subprocess.run(commit_cmd, cwd=tmp_path, text=True, check=True)

    # Modify both files
    initial_file.write_text("dirty initial")
    second_file.write_text("dirty second")

    # Only restore initial.txt
    checkout_cmd = [git2cpp_path, "checkout", "--", "initial.txt"]
    p = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode == 0
    assert initial_file.read_text() == original_initial
    assert second_file.read_text() == "dirty second"


def test_checkout_file_does_not_change_branch(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that checkout -- <file> does not move HEAD or change the current branch"""
    initial_file = tmp_path / "initial.txt"
    original_initial = initial_file.read_text()

    initial_file.write_text("dirty")

    checkout_cmd = [git2cpp_path, "checkout", "--", "initial.txt"]
    p = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    assert initial_file.read_text() == original_initial

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_branch.returncode == 0
    assert "* main" in p_branch.stdout


def test_checkout_file_nonexistent_path_fails(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that checkout -- <nonexistent> fails with a non-zero exit code"""
    checkout_cmd = [git2cpp_path, "checkout", "--", "doesnotexist.txt"]
    p = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode != 0


def test_checkout_file_no_paths_fails(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that checkout -- with no file arguments fails"""
    checkout_cmd = [git2cpp_path, "checkout", "--"]
    p = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode != 0
    assert "no branch or file specified" in p.stderr


def test_checkout_branch_file_restores_modified_file(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that checkout <branch> -- <file> restores the file from the branch."""
    initial_file = tmp_path / "initial.txt"

    # Create a new commit on main so the branch switch is meaningful
    second_file = tmp_path / "second.txt"
    second_file.write_text("second content")
    subprocess.run([git2cpp_path, "add", "second.txt"], cwd=tmp_path, text=True, check=True)
    subprocess.run(
        [git2cpp_path, "commit", "-m", "Add second file"], cwd=tmp_path, text=True, check=True
    )

    # Create and switch to feature branch
    subprocess.run([git2cpp_path, "checkout", "-b", "feature"], cwd=tmp_path, text=True, check=True)

    # Modify the file on feature branch and commit it
    initial_file.write_text("feature content")
    subprocess.run([git2cpp_path, "add", "initial.txt"], cwd=tmp_path, text=True, check=True)
    subprocess.run(
        [git2cpp_path, "commit", "-m", "Change initial on feature"],
        cwd=tmp_path,
        text=True,
        check=True,
    )

    # Go back to main and dirty the file
    subprocess.run([git2cpp_path, "checkout", "main"], cwd=tmp_path, text=True, check=True)
    initial_file.write_text("local dirty content")

    # Restore only initial.txt from feature
    checkout_cmd = [git2cpp_path, "checkout", "feature", "initial.txt"]
    p = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode == 0
    assert initial_file.read_text() == "feature content"

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_branch.returncode == 0
    assert "* main" in p_branch.stdout


def test_checkout_branch_multiple_files_restores_all(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test that checkout <branch> -- <file1> <file2> restores multiple files from the branch."""
    initial_file = tmp_path / "initial.txt"

    second_file = tmp_path / "second.txt"
    second_file.write_text("second content")
    subprocess.run([git2cpp_path, "add", "second.txt"], cwd=tmp_path, text=True, check=True)
    subprocess.run(
        [git2cpp_path, "commit", "-m", "Add second file"], cwd=tmp_path, text=True, check=True
    )

    # Create feature branch and modify both files there
    subprocess.run([git2cpp_path, "checkout", "-b", "feature"], cwd=tmp_path, text=True, check=True)
    initial_file.write_text("feature initial")
    second_file.write_text("feature second")
    subprocess.run(
        [git2cpp_path, "add", "initial.txt", "second.txt"], cwd=tmp_path, text=True, check=True
    )
    subprocess.run(
        [git2cpp_path, "commit", "-m", "Change both files on feature"],
        cwd=tmp_path,
        text=True,
        check=True,
    )

    # Return to main and dirty both files
    subprocess.run([git2cpp_path, "checkout", "main"], cwd=tmp_path, text=True, check=True)
    initial_file.write_text("dirty main initial")
    second_file.write_text("dirty main second")

    # Restore both files from feature
    checkout_cmd = [git2cpp_path, "checkout", "feature", "initial.txt", "second.txt"]
    p = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)

    assert p.returncode == 0
    assert initial_file.read_text() == "feature initial"
    assert second_file.read_text() == "feature second"

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_branch.returncode == 0
    assert "* main" in p_branch.stdout


def test_checkout_tag(repo_init_with_commit, git2cpp_path, tmp_path):
    """checkout <tag> should detach HEAD at the tag commit."""
    # Create a tag pointing to HEAD
    tag_cmd = [git2cpp_path, "tag", "v1.0"]
    p_tag = subprocess.run(tag_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_tag.returncode == 0

    # Switch to the tag
    checkout_cmd = [git2cpp_path, "checkout", "v1.0"]
    p_checkout = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_checkout.returncode == 0
    assert "detached HEAD" in p_checkout.stdout

    # Verify we're detached
    current_branch_cmd = [git2cpp_path, "branch", "--show-current"]
    p_current = subprocess.run(current_branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_current.returncode == 0
    assert p_current.stdout.strip() == ""

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_branch.returncode == 0
    assert "*" not in p_branch.stdout
