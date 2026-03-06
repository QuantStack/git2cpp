import pytest
import subprocess


@pytest.mark.parametrize("protocol", ["http", "https"])
def test_fetch_private_repo(git2cpp_path, tmp_path, run_in_tmp_path, private_test_repo, protocol):
    # Note that http succeeds by redirecting to https.
    init_cmd = [git2cpp_path, "init", "."]
    p_init = subprocess.run(init_cmd, capture_output=True, text=True)
    assert p_init.returncode == 0
    assert (tmp_path / ".git").exists()

    url = private_test_repo['https_url' if protocol == 'https' else 'http_url']
    remote_cmd = [git2cpp_path, "remote", "add", "origin", url]
    p_remote = subprocess.run(remote_cmd, capture_output=True, text=True)
    assert p_remote.returncode == 0

    # First fetch with wrong password which fails, then correct password which succeeds.
    username = "abc"  # Can be any non-empty string.
    password = private_test_repo['token']
    input = f"{username}\nwrong_password\n{username}\n{password}"
    fetch_cmd = [git2cpp_path, "fetch", "origin"]
    p_fetch = subprocess.run(fetch_cmd, capture_output=True, text=True, input=input)
    assert p_fetch.returncode == 0

    branch_cmd = [git2cpp_path, "branch", "--all"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, text=True)
    assert p_branch.returncode == 0
    assert "origin/main" in p_branch.stdout
