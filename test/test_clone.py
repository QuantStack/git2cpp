import pytest
import subprocess

xtl_url = "https://github.com/xtensor-stack/xtl.git"


def test_clone(git2cpp_path, tmp_path, run_in_tmp_path):
    clone_cmd = [git2cpp_path, "clone", xtl_url]
    p_clone = subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_clone.returncode == 0

    assert (tmp_path / "xtl").exists()
    assert (tmp_path / "xtl/include").exists()


def test_clone_is_bare(git2cpp_path, tmp_path, run_in_tmp_path):
    clone_cmd = [git2cpp_path, "clone", "--bare", xtl_url]
    p_clone = subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_clone.returncode == 0

    assert (tmp_path / "xtl").is_dir()

    status_cmd = [git2cpp_path, "status"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=tmp_path / "xtl", text=True)
    assert p_status.returncode != 0
    assert "This operation is not allowed against bare repositories" in p_status.stderr

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path / "xtl", text=True)
    assert p_branch.returncode == 0
    assert p_branch.stdout.strip() == "* master"


def test_clone_shallow(git2cpp_path, tmp_path, run_in_tmp_path):
    clone_cmd = [git2cpp_path, "clone", "--depth", "1", xtl_url]
    p_clone = subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_clone.returncode == 0
    assert (tmp_path / "xtl").exists()

    xtl_path = tmp_path / "xtl"

    cmd_log = [git2cpp_path, "log"]
    p_log = subprocess.run(cmd_log, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0
    assert p_log.stdout.count("Author") == 1


@pytest.mark.parametrize("protocol", ["http", "https"])
def test_clone_private_repo(git2cpp_path, tmp_path, run_in_tmp_path, private_test_repo, protocol):
    # Succeeds with correct credentials.
    # Note that http succeeds by redirecting to https.
    username = "abc"  # Can be any non-empty string.
    password = private_test_repo['token']
    input = f"{username}\n{password}"
    repo_path = tmp_path / private_test_repo['repo_name']
    url = private_test_repo['https_url' if protocol == 'https' else 'http_url']

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


def test_clone_private_repo_fails_then_succeeds(
        git2cpp_path, tmp_path, run_in_tmp_path, private_test_repo
):
    # Fails with wrong credentials, then succeeds with correct ones.
    username = "xyz"  # Can be any non-empty string.
    password = private_test_repo['token']
    input = "\n".join(["wrong1", "wrong2", username, password])
    repo_path = tmp_path / private_test_repo['repo_name']

    clone_cmd = [git2cpp_path, "clone", private_test_repo['https_url']]
    p_clone = subprocess.run(clone_cmd, capture_output=True, text=True, input=input)
    assert p_clone.returncode == 0
    assert repo_path.exists()
    # Two requests for username and password.
    assert p_clone.stdout.count("Username:") == 2
    assert p_clone.stdout.count("Password:") == 2

    status_cmd = [git2cpp_path, "status"]
    p_status = subprocess.run(status_cmd, cwd=repo_path, capture_output=True, text=True)
    assert p_status.returncode == 0
    assert "On branch main" in p_status.stdout
    assert "Your branch is up to date with 'origin/main'" in p_status.stdout


def test_clone_private_repo_fails_on_no_username(
    git2cpp_path, tmp_path, run_in_tmp_path, private_test_repo
):
    input = ""
    repo_path = tmp_path / private_test_repo['repo_name']

    clone_cmd = [git2cpp_path, "clone", private_test_repo['https_url']]
    p_clone = subprocess.run(clone_cmd, capture_output=True, text=True, input=input)

    assert p_clone.returncode != 0
    assert "No username specified" in p_clone.stderr
    assert not repo_path.exists()
    assert p_clone.stdout.count("Username:") == 1
    assert p_clone.stdout.count("Password:") == 0


def test_clone_private_repo_fails_on_no_password(
    git2cpp_path, tmp_path, run_in_tmp_path, private_test_repo
):
    input = "username\n"  # Note no password after the \n
    repo_path = tmp_path / private_test_repo['repo_name']

    clone_cmd = [git2cpp_path, "clone", private_test_repo['https_url']]
    p_clone = subprocess.run(clone_cmd, capture_output=True, text=True, input=input)

    assert p_clone.returncode != 0
    assert "No password specified" in p_clone.stderr
    assert not repo_path.exists()
    assert p_clone.stdout.count("Username:") == 1
    assert p_clone.stdout.count("Password:") == 1
