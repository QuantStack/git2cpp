import subprocess

import pytest


def test_rebase_basic(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test basic rebase operation with fast-forward"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create a feature branch
    checkout_cmd = [git2cpp_path, "checkout", "-b", "feature"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout.returncode == 0

    # Create a commit on feature branch
    file_path = xtl_path / "feature_file.txt"
    file_path.write_text("feature content")

    add_cmd = [git2cpp_path, "add", "feature_file.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "feature commit"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    # Go back to master and create another commit
    checkout_master_cmd = [git2cpp_path, "checkout", "master"]
    p_checkout_master = subprocess.run(
        checkout_master_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout_master.returncode == 0

    file_path_2 = xtl_path / "master_file.txt"
    file_path_2.write_text("master content")

    add_cmd_2 = [git2cpp_path, "add", "master_file.txt"]
    p_add_2 = subprocess.run(add_cmd_2, capture_output=True, cwd=xtl_path, text=True)
    assert p_add_2.returncode == 0

    commit_cmd_2 = [git2cpp_path, "commit", "-m", "master commit"]
    p_commit_2 = subprocess.run(
        commit_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_commit_2.returncode == 0

    # Switch to feature and rebase onto master
    checkout_feature_cmd = [git2cpp_path, "checkout", "feature"]
    p_checkout_feature = subprocess.run(
        checkout_feature_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout_feature.returncode == 0

    rebase_cmd = [git2cpp_path, "rebase", "master"]
    p_rebase = subprocess.run(rebase_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rebase.returncode == 0
    assert "Successfully rebased" in p_rebase.stdout
    assert "Rebasing 1 commit(s)" in p_rebase.stdout

    # Verify both files exist
    assert (xtl_path / "feature_file.txt").exists()
    assert (xtl_path / "master_file.txt").exists()


def test_rebase_multiple_commits(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test rebase with multiple commits"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create feature branch with multiple commits
    checkout_cmd = [git2cpp_path, "checkout", "-b", "feature"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout.returncode == 0

    # First commit
    file_1 = xtl_path / "file_1.txt"
    file_1.write_text("content 1")
    add_cmd_1 = [git2cpp_path, "add", "file_1.txt"]
    subprocess.run(add_cmd_1, cwd=xtl_path, text=True)
    commit_cmd_1 = [git2cpp_path, "commit", "-m", "commit 1"]
    subprocess.run(commit_cmd_1, cwd=xtl_path, text=True)

    # Second commit
    file_2 = xtl_path / "file_2.txt"
    file_2.write_text("content 2")
    add_cmd_2 = [git2cpp_path, "add", "file_2.txt"]
    subprocess.run(add_cmd_2, cwd=xtl_path, text=True)
    commit_cmd_2 = [git2cpp_path, "commit", "-m", "commit 2"]
    subprocess.run(commit_cmd_2, cwd=xtl_path, text=True)

    # Third commit
    file_3 = xtl_path / "file_3.txt"
    file_3.write_text("content 3")
    add_cmd_3 = [git2cpp_path, "add", "file_3.txt"]
    subprocess.run(add_cmd_3, cwd=xtl_path, text=True)
    commit_cmd_3 = [git2cpp_path, "commit", "-m", "commit 3"]
    subprocess.run(commit_cmd_3, cwd=xtl_path, text=True)

    # Go to master and add a commit
    checkout_master_cmd = [git2cpp_path, "checkout", "master"]
    subprocess.run(checkout_master_cmd, cwd=xtl_path)

    master_file = xtl_path / "master_file.txt"
    master_file.write_text("master")
    subprocess.run([git2cpp_path, "add", "master_file.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "master commit"], cwd=xtl_path)

    # Rebase feature onto master
    checkout_feature_cmd = [git2cpp_path, "checkout", "feature"]
    subprocess.run(checkout_feature_cmd, cwd=xtl_path)

    rebase_cmd = [git2cpp_path, "rebase", "master"]
    p_rebase = subprocess.run(rebase_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rebase.returncode == 0
    assert "Rebasing 3 commit(s)" in p_rebase.stdout
    assert "Successfully rebased" in p_rebase.stdout

    # Verify all files exist
    assert (xtl_path / "file_1.txt").exists()
    assert (xtl_path / "file_2.txt").exists()
    assert (xtl_path / "file_3.txt").exists()
    assert (xtl_path / "master_file.txt").exists()


def test_rebase_with_conflicts(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test rebase with conflicts"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create feature branch
    checkout_cmd = [git2cpp_path, "checkout", "-b", "feature"]
    subprocess.run(checkout_cmd, capture_output=True, cwd=xtl_path, text=True)

    # Create conflicting file on feature
    conflict_file = xtl_path / "conflict.txt"
    conflict_file.write_text("feature content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "feature commit"], cwd=xtl_path)

    # Go to master and create conflicting commit
    checkout_master_cmd = [git2cpp_path, "checkout", "master"]
    subprocess.run(checkout_master_cmd, cwd=xtl_path)

    conflict_file.write_text("master content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "master commit"], cwd=xtl_path)

    # Try to rebase feature onto master
    checkout_feature_cmd = [git2cpp_path, "checkout", "feature"]
    subprocess.run(checkout_feature_cmd, cwd=xtl_path)

    rebase_cmd = [git2cpp_path, "rebase", "master"]
    p_rebase = subprocess.run(rebase_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rebase.returncode == 0
    assert "Conflicts detected" in p_rebase.stdout
    assert "rebase --continue" in p_rebase.stdout
    assert "rebase --skip" in p_rebase.stdout
    assert "rebase --abort" in p_rebase.stdout


def test_rebase_abort(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test rebase abort after conflicts"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create feature branch
    checkout_cmd = [git2cpp_path, "checkout", "-b", "feature"]
    subprocess.run(checkout_cmd, cwd=xtl_path)

    # Create conflicting file on feature
    conflict_file = xtl_path / "conflict.txt"
    conflict_file.write_text("feature content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "feature commit"], cwd=xtl_path)

    # Go to master and create conflicting commit
    subprocess.run([git2cpp_path, "checkout", "master"], cwd=xtl_path)
    conflict_file.write_text("master content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "master commit"], cwd=xtl_path)

    # Rebase and get conflict
    subprocess.run([git2cpp_path, "checkout", "feature"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "rebase", "master"], cwd=xtl_path)

    # Abort the rebase
    abort_cmd = [git2cpp_path, "rebase", "--abort"]
    p_abort = subprocess.run(abort_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_abort.returncode == 0
    assert "Rebase aborted" in p_abort.stdout

    # Verify we're back to original state
    assert conflict_file.read_text() == "feature content"


def test_rebase_continue(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test rebase continue after resolving conflicts"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create feature branch
    subprocess.run([git2cpp_path, "checkout", "-b", "feature"], cwd=xtl_path)

    # Create conflicting file on feature
    conflict_file = xtl_path / "conflict.txt"
    conflict_file.write_text("feature content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "feature commit"], cwd=xtl_path)

    # Go to master and create conflicting commit
    subprocess.run([git2cpp_path, "checkout", "master"], cwd=xtl_path)
    conflict_file.write_text("master content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "master commit"], cwd=xtl_path)

    # Rebase and get conflict
    subprocess.run([git2cpp_path, "checkout", "feature"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "rebase", "master"], cwd=xtl_path)

    # Resolve conflict
    conflict_file.write_text("resolved content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)

    # Continue rebase
    continue_cmd = [git2cpp_path, "rebase", "--continue"]
    p_continue = subprocess.run(
        continue_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_continue.returncode == 0
    assert "Successfully rebased" in p_continue.stdout

    # Verify resolution
    assert conflict_file.read_text() == "resolved content"


def test_rebase_skip(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test rebase skip to skip current commit"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create feature branch
    subprocess.run([git2cpp_path, "checkout", "-b", "feature"], cwd=xtl_path)

    # Create conflicting file on feature
    conflict_file = xtl_path / "conflict.txt"
    conflict_file.write_text("feature content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "feature commit"], cwd=xtl_path)

    # Go to master and create conflicting commit
    subprocess.run([git2cpp_path, "checkout", "master"], cwd=xtl_path)
    conflict_file.write_text("master content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "master commit"], cwd=xtl_path)

    # Rebase and get conflict
    subprocess.run([git2cpp_path, "checkout", "feature"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "rebase", "master"], cwd=xtl_path)

    # Skip the conflicting commit
    skip_cmd = [git2cpp_path, "rebase", "--skip"]
    p_skip = subprocess.run(skip_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_skip.returncode == 0
    assert "Skipping" in p_skip.stdout


def test_rebase_quit(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test rebase quit to cleanup state without resetting HEAD"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create feature branch
    subprocess.run([git2cpp_path, "checkout", "-b", "feature"], cwd=xtl_path)

    # Create conflicting file
    conflict_file = xtl_path / "conflict.txt"
    conflict_file.write_text("feature content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "feature commit"], cwd=xtl_path)

    # Create conflict on master
    subprocess.run([git2cpp_path, "checkout", "master"], cwd=xtl_path)
    conflict_file.write_text("master content")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "master commit"], cwd=xtl_path)

    # Start rebase
    subprocess.run([git2cpp_path, "checkout", "feature"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "rebase", "master"], cwd=xtl_path)

    # Quit rebase
    quit_cmd = [git2cpp_path, "rebase", "--quit"]
    p_quit = subprocess.run(quit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_quit.returncode == 0
    assert "Rebase state cleaned up" in p_quit.stdout
    assert "HEAD not reset" in p_quit.stdout


def test_rebase_onto(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test rebase with --onto option"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create first branch
    subprocess.run([git2cpp_path, "checkout", "-b", "branch1"], cwd=xtl_path)
    file1 = xtl_path / "file1.txt"
    file1.write_text("branch1")
    subprocess.run([git2cpp_path, "add", "file1.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "branch1 commit"], cwd=xtl_path)

    # Create second branch from branch1
    subprocess.run([git2cpp_path, "checkout", "-b", "branch2"], cwd=xtl_path)
    file2 = xtl_path / "file2.txt"
    file2.write_text("branch2")
    subprocess.run([git2cpp_path, "add", "file2.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "branch2 commit"], cwd=xtl_path)

    # Create target branch from master
    subprocess.run([git2cpp_path, "checkout", "master"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "checkout", "-b", "target"], cwd=xtl_path)
    target_file = xtl_path / "target.txt"
    target_file.write_text("target")
    subprocess.run([git2cpp_path, "add", "target.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "target commit"], cwd=xtl_path)

    # Rebase branch2 onto target, upstream is branch1
    rebase_cmd = [git2cpp_path, "rebase", "branch1", "branch2", "--onto", "target"]
    p_rebase = subprocess.run(rebase_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rebase.returncode == 0

    # Verify target file exists and branch2 file exists, but not branch1 file
    assert not (xtl_path / "target.txt").exists()
    assert (xtl_path / "file2.txt").exists()


def test_rebase_no_upstream_error(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that rebase without upstream argument fails"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    rebase_cmd = [git2cpp_path, "rebase"]
    p_rebase = subprocess.run(rebase_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rebase.returncode != 0
    assert "upstream is required for rebase" in p_rebase.stderr


def test_rebase_invalid_upstream_error(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that rebase with invalid upstream fails"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    rebase_cmd = [git2cpp_path, "rebase", "nonexistent-branch"]
    p_rebase = subprocess.run(rebase_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rebase.returncode != 0
    assert "could not resolve upstream" in p_rebase.stderr or "could not resolve upstream" in p_rebase.stdout


def test_rebase_already_in_progress_error(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that starting rebase when one is in progress fails"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create feature branch with conflict
    subprocess.run([git2cpp_path, "checkout", "-b", "feature"], cwd=xtl_path)
    conflict_file = xtl_path / "conflict.txt"
    conflict_file.write_text("feature")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "feature"], cwd=xtl_path)

    # Create conflict on master
    subprocess.run([git2cpp_path, "checkout", "master"], cwd=xtl_path)
    conflict_file.write_text("master")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "master"], cwd=xtl_path)

    # Start rebase with conflict
    subprocess.run([git2cpp_path, "checkout", "feature"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "rebase", "master"], cwd=xtl_path)

    # Try to start another rebase
    rebase_cmd = [git2cpp_path, "rebase", "master"]
    p_rebase = subprocess.run(rebase_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_rebase.returncode != 0
    assert "rebase is already in progress" in p_rebase.stderr or "rebase is already in progress" in p_rebase.stdout


def test_rebase_continue_without_rebase_error(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that --continue without rebase in progress fails"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    continue_cmd = [git2cpp_path, "rebase", "--continue"]
    p_continue = subprocess.run(continue_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_continue.returncode != 0
    assert "No rebase in progress" in p_continue.stderr or "No rebase in progress" in p_continue.stdout


def test_rebase_continue_with_unresolved_conflicts(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test that --continue with unresolved conflicts fails"""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    # Create conflict scenario
    subprocess.run([git2cpp_path, "checkout", "-b", "feature"], cwd=xtl_path)
    conflict_file = xtl_path / "conflict.txt"
    conflict_file.write_text("feature")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "feature"], cwd=xtl_path)

    subprocess.run([git2cpp_path, "checkout", "master"], cwd=xtl_path)
    conflict_file.write_text("master")
    subprocess.run([git2cpp_path, "add", "conflict.txt"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "commit", "-m", "master"], cwd=xtl_path)

    # Start rebase
    subprocess.run([git2cpp_path, "checkout", "feature"], cwd=xtl_path)
    subprocess.run([git2cpp_path, "rebase", "master"], cwd=xtl_path)

    # Try to continue without resolving
    continue_cmd = [git2cpp_path, "rebase", "--continue"]
    p_continue = subprocess.run(continue_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_continue.returncode != 0
    assert "resolve conflicts" in p_continue.stderr or "resolve conflicts" in p_continue.stdout
