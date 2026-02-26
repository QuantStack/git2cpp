import subprocess
from uuid import uuid4


def test_push_private_repo(git2cpp_path, tmp_path, run_in_tmp_path, private_test_repo):
    # Unique branch name to avoid branch name collisions on remote repo.
    branch_name = f"test-{uuid4()}"

    # Start of test follows test_clone_private_repo, then creates a new local branch and pushes
    # that to the remote.
    username = "abc"  # Can be any non-empty string.
    password = private_test_repo['token']
    input = f"{username}\n{password}"
    repo_path = tmp_path / private_test_repo['repo_name']
    url = private_test_repo['https_url']

    clone_cmd = [git2cpp_path, "clone", url]
    p_clone = subprocess.run(clone_cmd, capture_output=True, text=True, input=input)
    assert p_clone.returncode == 0
    assert repo_path.exists()
    # Single request for username and password.
    assert p_clone.stdout.count("Username:") == 1
    assert p_clone.stdout.count("Password:") == 1

    status_cmd = [git2cpp_path, "status"]
    p_status = subprocess.run(status_cmd, cwd=repo_path, capture_output=True, text=True)
    assert p_status.returncode == 0
    assert "On branch main" in p_status.stdout
    assert "Your branch is up to date with 'origin/main'" in p_status.stdout

    checkout_cmd = [git2cpp_path, "checkout", "-b", branch_name]
    p_checkout = subprocess.run(checkout_cmd, cwd=repo_path, capture_output=True, text=True)
    assert p_checkout.returncode == 0

    p_status = subprocess.run(status_cmd, cwd=repo_path, capture_output=True, text=True)
    assert p_status.returncode == 0
    assert f"On branch {branch_name}" in p_status.stdout

    (repo_path / "new_file.txt").write_text("Some text")
    add_cmd = [git2cpp_path, "add", "new_file.txt"]
    p_add = subprocess.run(add_cmd, cwd=repo_path, capture_output=True, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "This is my commit message"]
    p_commit = subprocess.run(commit_cmd, cwd=repo_path, capture_output=True, text=True)
    assert p_commit.returncode == 0

    log_cmd = [git2cpp_path, "log", "-n1"]
    p_log = subprocess.run(log_cmd, cwd=repo_path, capture_output=True, text=True)
    assert p_log.returncode == 0
    assert p_log.stdout.count("Author:") == 1
    assert p_log.stdout.count("Date:") == 1
    assert p_log.stdout.count("This is my commit message") == 1

    # push with incorrect credentials to check it fails, then with correct to check it works.
    input = f"${username}\ndef\n{username}\n{password}"
    push_cmd = [git2cpp_path, "push", "origin"]
    p_push = subprocess.run(push_cmd, cwd=repo_path, capture_output=True, text=True, input=input)
    assert p_push.returncode == 0
    assert p_push.stdout.count("Username:") == 2
    assert p_push.stdout.count("Password:") == 2
    assert "Pushed to origin" in p_push.stdout
