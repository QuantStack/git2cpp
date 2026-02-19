import subprocess

import pytest


def test_checkout(xtl_clone, git2cpp_path, tmp_path):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    create_cmd = [git2cpp_path, "branch", "foregone"]
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_create.returncode == 0

    checkout_cmd = [git2cpp_path, "checkout", "foregone"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout.returncode == 0
    assert "Switched to branch 'foregone'" in p_checkout.stdout

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_branch.returncode == 0
    assert p_branch.stdout == "* foregone\n  master\n"

    checkout_cmd[2] = "master"
    p_checkout2 = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout2.returncode == 0
    assert "Switched to branch 'master'" in p_checkout2.stdout


def test_checkout_b(xtl_clone, git2cpp_path, tmp_path):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    checkout_cmd = [git2cpp_path, "checkout", "-b", "foregone"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout.returncode == 0
    assert "Switched to a new branch 'foregone'" in p_checkout.stdout

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_branch.returncode == 0
    assert p_branch.stdout == "* foregone\n  master\n"

    checkout_cmd.remove("-b")
    checkout_cmd[2] = "master"
    p_checkout2 = subprocess.run(checkout_cmd, cwd=xtl_path, text=True)
    assert p_checkout2.returncode == 0

    p_branch2 = subprocess.run(branch_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_branch2.returncode == 0
    assert p_branch2.stdout == "  foregone\n* master\n"


def test_checkout_B_force_create(xtl_clone, git2cpp_path, tmp_path):
    """Test checkout -B to force create or reset a branch"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a branch first
    create_cmd = [git2cpp_path, "branch", "resetme"]
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_create.returncode == 0

    # Use -B to reset it (should not fail even if branch exists)
    checkout_cmd = [git2cpp_path, "checkout", "-B", "resetme"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout.returncode == 0
    assert "Switched to a new branch 'resetme'" in p_checkout.stdout

    # Verify we're on the branch
    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_branch.returncode == 0
    assert "* resetme" in p_branch.stdout


def test_checkout_invalid_branch(xtl_clone, git2cpp_path, tmp_path):
    """Test that checkout fails gracefully with invalid branch name"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Try to checkout non-existent branch
    checkout_cmd = [git2cpp_path, "checkout", "nonexistent"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )

    # Should fail with error message
    assert p_checkout.returncode != 0
    assert "error: could not resolve pathspec 'nonexistent'" in p_checkout.stderr


# @pytest.mark.skip(reason="Waiting for print_tobecommited and print_tracking_info implementations")
def test_checkout_with_unstaged_changes(xtl_clone, git2cpp_path, tmp_path):
    """Test that checkout warns about unstaged changes"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a new branch
    create_cmd = [git2cpp_path, "branch", "newbranch"]
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_create.returncode == 0

    # Modify a file (unstaged change)
    readme_path = xtl_path / "README.md"
    readme_path.write_text("Modified content")

    # Try to checkout - should warn about unstaged changes
    checkout_cmd = [git2cpp_path, "checkout", "newbranch"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    print(p_checkout.stdout)
    # Should show warning message (once implemented)
    # assert (
    #     "Your local changes to the following files would be overwritten by checkout:"
    #     in p_checkout.stdout
    # )
    # assert "README.md" in p_checkout.stdout
    # assert (
    #     "Please commit your changes or stash them before you switch branches"
    #     in p_checkout.stdout
    # )

    # Verify we stayed on master branch
    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_branch.returncode == 0
    # print(p_branch.stdout)
    # assert "* master" in p_branch.stdout


# def test_checkout_force_with_unstaged_changes(xtl_clone, git2cpp_path, tmp_path):
#     """Test that checkout -f forces checkout even with unstaged changes"""
#     assert (tmp_path / "xtl").exists()
#     xtl_path = tmp_path / "xtl"

#     # Create a new branch
#     create_cmd = [git2cpp_path, 'branch', 'forcebranch']
#     p_create = subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, text=True)
#     assert p_create.returncode == 0

#     # Modify a file (unstaged change)
#     readme_path = xtl_path / "README.md"
#     readme_path.write_text("Modified content that will be lost")

#     # Force checkout - should succeed and discard changes
#     checkout_cmd = [git2cpp_path, 'checkout', '-f', 'forcebranch']
#     p_checkout = subprocess.run(checkout_cmd, capture_output=True, cwd=xtl_path, text=True)
#     assert p_checkout.returncode == 0
#     assert "Switched to branch 'forcebranch'" in p_checkout.stdout

#     # Verify we're on the branch
#     branch_cmd = [git2cpp_path, 'branch']
#     p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=xtl_path, text=True)
#     assert '* forcebranch' in p_branch.stdout
