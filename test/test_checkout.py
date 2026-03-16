import subprocess

import pytest


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
    assert "error: could not resolve pathspec 'nonexistent'" in p_checkout.stderr


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
    subprocess.run(add_cmd, cwd=tmp_path, text=True)

    commit_cmd = [git2cpp_path, "commit", "-m", "Change on newbranch"]
    subprocess.run(commit_cmd, cwd=tmp_path, text=True)

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
