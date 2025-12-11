import subprocess

import pytest

repo_url = "https://github.com/user/repo.git"


def test_remote_list_empty(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test listing remotes in a repo with no remotes."""
    # Initialize a repo
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    cmd = [git2cpp_path, "remote"]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    assert p.stdout == ""  # No remotes yet


def test_remote_add(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test adding a remote."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    missing_cmd = [git2cpp_path, "remote", "add", "origin"]
    p_missing = subprocess.run(missing_cmd, capture_output=True, text=True)
    assert p_missing.returncode != 0
    assert "usage: git remote add <name> <url>" in p_missing.stderr

    add_cmd = [git2cpp_path, "remote", "add", "origin", repo_url]
    p_add = subprocess.run(add_cmd, capture_output=True, text=True)
    assert p_add.returncode == 0

    # Verify remote was added
    list_cmd = [git2cpp_path, "remote"]
    p_list = subprocess.run(list_cmd, capture_output=True, text=True)
    assert p_list.returncode == 0
    assert "origin" in p_list.stdout


def test_remote_add_multiple(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test adding multiple remotes."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    add_origin_cmd = [git2cpp_path, "remote", "add", "origin", repo_url]
    p_add_origin = subprocess.run(add_origin_cmd, capture_output=True, check=True)
    assert p_add_origin.returncode == 0
    add_upstream_cmd = [
        git2cpp_path,
        "remote",
        "add",
        "upstream",
        "https://github.com/upstream/repo.git",
    ]
    p_add_upstream = subprocess.run(add_upstream_cmd, capture_output=True, check=True)
    assert p_add_upstream.returncode == 0

    list_cmd = [git2cpp_path, "remote"]
    p_list = subprocess.run(list_cmd, capture_output=True, text=True)
    assert p_list.returncode == 0
    output = p_list.stdout.strip()
    assert "origin" in output
    assert "upstream" in output


@pytest.mark.parametrize("remove", ["rm", "remove"])
def test_remote_remove(git2cpp_path, tmp_path, run_in_tmp_path, remove):
    """Test removing a remote."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    add_cmd = [git2cpp_path, "remote", "add", "origin", repo_url]
    p_add = subprocess.run(add_cmd, capture_output=True, check=True)
    assert p_add.returncode == 0

    # Remove the remote
    remove_cmd = [git2cpp_path, "remote", remove, "origin"]
    p_remove = subprocess.run(remove_cmd, capture_output=True, text=True)
    assert p_remove.returncode == 0

    # Verify remote was removed
    list_cmd = [git2cpp_path, "remote"]
    p_list = subprocess.run(list_cmd, capture_output=True, text=True)
    assert p_list.returncode == 0
    assert "origin" not in p_list.stdout


def test_remote_rename(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test renaming a remote."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    add_cmd = [git2cpp_path, "remote", "add", "origin", repo_url]
    p_add = subprocess.run(add_cmd, capture_output=True, check=True)
    assert p_add.returncode == 0

    # Rename the remote
    rename_cmd = [git2cpp_path, "remote", "rename", "origin", "upstream"]
    p_rename = subprocess.run(rename_cmd, capture_output=True, text=True)
    assert p_rename.returncode == 0

    # Verify remote was renamed
    list_cmd = [git2cpp_path, "remote"]
    p_list = subprocess.run(list_cmd, capture_output=True, text=True)
    assert p_list.returncode == 0
    assert "origin" not in p_list.stdout
    assert "upstream" in p_list.stdout


def test_remote_set_url(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test setting remote URL."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    add_cmd = [git2cpp_path, "remote", "add", "origin", repo_url]
    p_add = subprocess.run(add_cmd, capture_output=True, check=True)
    assert p_add.returncode == 0

    # Change the URL
    new_url = "https://github.com/user/newrepo.git"
    set_url_cmd = [git2cpp_path, "remote", "set-url", "origin", new_url]
    p_set_url = subprocess.run(set_url_cmd, capture_output=True, text=True)
    assert p_set_url.returncode == 0

    # Verify URL was changed
    show_cmd = [git2cpp_path, "remote", "show", "origin"]
    p_show = subprocess.run(show_cmd, capture_output=True, text=True)
    assert p_show.returncode == 0
    assert new_url in p_show.stdout


def test_remote_set_push_url(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test setting remote push URL."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    subprocess.run(
        [git2cpp_path, "remote", "add", "origin", repo_url],
        capture_output=True,
        check=True,
    )

    # Set push URL
    push_url = "https://github.com/user/pushrepo.git"
    cmd = [git2cpp_path, "remote", "set-url", "--push", "origin", push_url]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0

    # Verify push URL was set
    show_cmd = [git2cpp_path, "remote", "show", "origin"]
    p_show = subprocess.run(show_cmd, capture_output=True, text=True)
    assert p_show.returncode == 0
    assert push_url in p_show.stdout


def test_remote_show(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test showing remote details."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    subprocess.run(
        [git2cpp_path, "remote", "add", "origin", repo_url],
        capture_output=True,
        check=True,
    )

    cmd = [git2cpp_path, "remote", "show", "origin"]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    assert "origin" in p.stdout
    assert repo_url in p.stdout


def test_remote_show_verbose(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test showing remotes with verbose flag."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    subprocess.run(
        [git2cpp_path, "remote", "add", "origin", repo_url],
        capture_output=True,
        check=True,
    )

    cmd = [git2cpp_path, "remote", "-v"]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    assert "origin" in p.stdout
    assert repo_url in p.stdout
    assert "(fetch)" in p.stdout or "(push)" in p.stdout


def test_remote_show_all_verbose(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test showing all remotes with verbose flag."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    add_origin_cmd = [git2cpp_path, "remote", "add", "origin", repo_url]
    p_add_origin = subprocess.run(add_origin_cmd, capture_output=True, check=True)
    assert p_add_origin.returncode == 0
    add_upstream_cmd = [
        git2cpp_path,
        "remote",
        "add",
        "upstream",
        "https://github.com/upstream/repo.git",
    ]
    p_add_upstream = subprocess.run(add_upstream_cmd, capture_output=True, check=True)
    assert p_add_upstream.returncode == 0

    show_cmd = [git2cpp_path, "remote", "show", "-v"]
    p_show = subprocess.run(show_cmd, capture_output=True, text=True)
    assert p_show.returncode == 0
    assert "origin" in p_show.stdout
    assert "upstream" in p_show.stdout


def test_remote_error_on_duplicate_add(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test error when adding duplicate remote."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    add_cmd = [git2cpp_path, "remote", "add", "origin", repo_url]
    p_add = subprocess.run(add_cmd, capture_output=True, check=True)
    assert p_add.returncode == 0

    # Try to add duplicate
    add_dup_cmd = [
        git2cpp_path,
        "remote",
        "add",
        "origin",
        "https://github.com/user/other.git",
    ]
    p_add_dup = subprocess.run(add_dup_cmd, capture_output=True, text=True)
    assert p_add_dup.returncode != 0


def test_remote_error_on_remove_nonexistent(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test error when removing non-existent remote."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    cmd = [git2cpp_path, "remote", "remove", "nonexistent"]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode != 0


def test_remote_error_on_rename_nonexistent(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test error when renaming non-existent remote."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    cmd = [git2cpp_path, "remote", "rename", "nonexistent", "new"]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode != 0


def test_remote_error_on_show_nonexistent(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test error when showing non-existent remote."""
    p_init = subprocess.run([git2cpp_path, "init"], capture_output=True, check=True)
    assert p_init.returncode == 0

    cmd = [git2cpp_path, "remote", "show", "nonexistent"]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode != 0


@pytest.fixture
def repo_with_remote(git2cpp_path, tmp_path, run_in_tmp_path):
    """Fixture that creates a repo with a remote pointing to a local bare repo."""
    # Create a bare repo to use as remote
    remote_path = tmp_path / "remote_repo"
    remote_path.mkdir()
    init_cmd = [git2cpp_path, "init", "--bare", str(remote_path)]
    p_init = subprocess.run(init_cmd, capture_output=True, check=True)
    assert p_init.returncode == 0

    # Create a regular repo
    local_path = tmp_path / "local_repo"
    local_path.mkdir()

    # Initialize repo in the directory
    p_init_2 = subprocess.run(
        [git2cpp_path, "init"], capture_output=True, check=True, cwd=local_path
    )
    assert p_init_2.returncode == 0

    # Add remote
    add_cmd = [git2cpp_path, "remote", "add", "origin", str(remote_path)]
    p_add = subprocess.run(add_cmd, capture_output=True, check=True, cwd=local_path)
    assert p_add.returncode == 0

    return local_path, remote_path


def test_fetch_from_remote(git2cpp_path, repo_with_remote):
    """Test fetching from a remote."""
    local_path, remote_path = repo_with_remote

    # Note: This is a bare repo with no refs, so fetch will fail gracefully
    # For now, just test that fetch command runs (it will fail gracefully if no refs)
    cmd = [git2cpp_path, "fetch", "origin"]
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=local_path)
    # Fetch might succeed (empty) or fail (no refs), but shouldn't crash
    assert p.returncode in [0, 1]  # 0 for success, 1 for no refs/error


def test_fetch_default_origin(git2cpp_path, repo_with_remote):
    """Test fetching with default origin."""
    local_path, remote_path = repo_with_remote

    cmd = [git2cpp_path, "fetch"]
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=local_path)
    # Fetch might succeed (empty) or fail (no refs), but shouldn't crash
    assert p.returncode in [0, 1]


def test_remote_in_cloned_repo(xtl_clone, git2cpp_path, tmp_path):
    """Test that cloned repos have remotes configured."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    cmd = [git2cpp_path, "remote"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "origin" in p.stdout


def test_remote_show_in_cloned_repo(xtl_clone, git2cpp_path, tmp_path):
    """Test showing remote in cloned repo."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    cmd = [git2cpp_path, "remote", "show", "origin"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "origin" in p.stdout
    # Should contain URL information
    assert "http" in p.stdout or "git" in p.stdout or "https" in p.stdout


def test_push_local(xtl_clone, git_config, git2cpp_path, tmp_path, monkeypatch):
    """Test setting push on a local remote."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    checkout_cmd = [git2cpp_path, "checkout", "-b", "foregone"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, check=True, cwd=xtl_path
    )
    assert p_checkout.returncode == 0

    p = xtl_path / "mook_file.txt"
    p.write_text("")

    cmd_add = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    cmd_commit = [git2cpp_path, "commit", "-m", "test commit"]
    p_commit = subprocess.run(cmd_commit, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    url = "https://github.com/xtensor-stack/xtl.git"
    local_path = tmp_path / "local_repo"
    clone_cmd = [git2cpp_path, "clone", "--bare", url, local_path]
    p_clone = subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_clone.returncode == 0

    add_cmd = [git2cpp_path, "remote", "add", "local_repo", str(local_path)]
    p_add = subprocess.run(add_cmd, capture_output=True, check=True, cwd=xtl_path)
    assert p_add.returncode == 0

    cmd_push = [git2cpp_path, "push", "local_repo"]  # "foregone"
    p_push = subprocess.run(cmd_push, capture_output=True, check=True, cwd=xtl_path)
    assert p_push.returncode == 0

    list_cmd = [git2cpp_path, "branch"]
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=local_path, text=True)
    assert p_list.returncode == 0
    assert "foregone" in p_list.stdout
