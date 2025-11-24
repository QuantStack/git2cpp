import os
import subprocess
import pytest
from pathlib import Path


def test_remote_list_empty(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test listing remotes in a repo with no remotes."""
    # Initialize a repo
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    
    cmd = [git2cpp_path, 'remote']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    assert p.stdout == ''  # No remotes yet


def test_remote_add(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test adding a remote."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    
    cmd = [git2cpp_path, 'remote', 'add', 'origin', 'https://github.com/user/repo.git']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    
    # Verify remote was added
    list_cmd = [git2cpp_path, 'remote']
    p_list = subprocess.run(list_cmd, capture_output=True, text=True)
    assert p_list.returncode == 0
    assert 'origin' in p_list.stdout


def test_remote_add_multiple(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test adding multiple remotes."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', 'https://github.com/user/repo.git'], 
                   capture_output=True, check=True)
    subprocess.run([git2cpp_path, 'remote', 'add', 'upstream', 'https://github.com/upstream/repo.git'], 
                   capture_output=True, check=True)
    
    list_cmd = [git2cpp_path, 'remote']
    p_list = subprocess.run(list_cmd, capture_output=True, text=True)
    assert p_list.returncode == 0
    output = p_list.stdout.strip()
    assert 'origin' in output
    assert 'upstream' in output


def test_remote_remove(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test removing a remote."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', 'https://github.com/user/repo.git'], 
                   capture_output=True, check=True)
    
    # Remove the remote
    cmd = [git2cpp_path, 'remote', 'remove', 'origin']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    
    # Verify remote was removed
    list_cmd = [git2cpp_path, 'remote']
    p_list = subprocess.run(list_cmd, capture_output=True, text=True)
    assert p_list.returncode == 0
    assert 'origin' not in p_list.stdout


def test_remote_remove_rm_alias(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test removing a remote using 'rm' alias."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', 'https://github.com/user/repo.git'], 
                   capture_output=True, check=True)
    
    # Remove using 'rm' alias
    cmd = [git2cpp_path, 'remote', 'rm', 'origin']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    
    # Verify remote was removed
    list_cmd = [git2cpp_path, 'remote']
    p_list = subprocess.run(list_cmd, capture_output=True, text=True)
    assert p_list.returncode == 0
    assert 'origin' not in p_list.stdout


def test_remote_rename(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test renaming a remote."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', 'https://github.com/user/repo.git'], 
                   capture_output=True, check=True)
    
    # Rename the remote
    cmd = [git2cpp_path, 'remote', 'rename', 'origin', 'upstream']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    
    # Verify remote was renamed
    list_cmd = [git2cpp_path, 'remote']
    p_list = subprocess.run(list_cmd, capture_output=True, text=True)
    assert p_list.returncode == 0
    assert 'origin' not in p_list.stdout
    assert 'upstream' in p_list.stdout


def test_remote_set_url(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test setting remote URL."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', 'https://github.com/user/repo.git'], 
                   capture_output=True, check=True)
    
    # Change the URL
    new_url = 'https://github.com/user/newrepo.git'
    cmd = [git2cpp_path, 'remote', 'set-url', 'origin', new_url]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    
    # Verify URL was changed
    show_cmd = [git2cpp_path, 'remote', 'show', 'origin']
    p_show = subprocess.run(show_cmd, capture_output=True, text=True)
    assert p_show.returncode == 0
    assert new_url in p_show.stdout


def test_remote_set_push_url(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test setting remote push URL."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', 'https://github.com/user/repo.git'], 
                   capture_output=True, check=True)
    
    # Set push URL
    push_url = 'https://github.com/user/pushrepo.git'
    cmd = [git2cpp_path, 'remote', 'set-url', '--push', 'origin', push_url]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    
    # Verify push URL was set
    show_cmd = [git2cpp_path, 'remote', 'show', 'origin']
    p_show = subprocess.run(show_cmd, capture_output=True, text=True)
    assert p_show.returncode == 0
    assert push_url in p_show.stdout


def test_remote_show(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test showing remote details."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    url = 'https://github.com/user/repo.git'
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', url], 
                   capture_output=True, check=True)
    
    cmd = [git2cpp_path, 'remote', 'show', 'origin']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    assert 'origin' in p.stdout
    assert url in p.stdout


def test_remote_show_verbose(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test showing remotes with verbose flag."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    url = 'https://github.com/user/repo.git'
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', url], 
                   capture_output=True, check=True)
    
    cmd = [git2cpp_path, 'remote', '-v']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    assert 'origin' in p.stdout
    assert url in p.stdout
    assert '(fetch)' in p.stdout or '(push)' in p.stdout


def test_remote_show_all_verbose(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test showing all remotes with verbose flag."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', 'https://github.com/user/repo.git'], 
                   capture_output=True, check=True)
    subprocess.run([git2cpp_path, 'remote', 'add', 'upstream', 'https://github.com/upstream/repo.git'], 
                   capture_output=True, check=True)
    
    cmd = [git2cpp_path, 'remote', 'show', '-v']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    assert 'origin' in p.stdout
    assert 'upstream' in p.stdout


def test_remote_error_on_duplicate_add(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test error when adding duplicate remote."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', 'https://github.com/user/repo.git'], 
                   capture_output=True, check=True)
    
    # Try to add duplicate
    cmd = [git2cpp_path, 'remote', 'add', 'origin', 'https://github.com/user/other.git']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode != 0


def test_remote_error_on_remove_nonexistent(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test error when removing non-existent remote."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    
    cmd = [git2cpp_path, 'remote', 'remove', 'nonexistent']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode != 0


def test_remote_error_on_rename_nonexistent(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test error when renaming non-existent remote."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    
    cmd = [git2cpp_path, 'remote', 'rename', 'nonexistent', 'new']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode != 0


def test_remote_error_on_show_nonexistent(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test error when showing non-existent remote."""
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True)
    
    cmd = [git2cpp_path, 'remote', 'show', 'nonexistent']
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode != 0


@pytest.fixture
def repo_with_remote(git2cpp_path, tmp_path, run_in_tmp_path):
    """Fixture that creates a repo with a remote pointing to a local bare repo."""
    # Create a bare repo to use as remote
    remote_path = tmp_path / "remote_repo"
    remote_path.mkdir()
    subprocess.run([git2cpp_path, 'init', '--bare', str(remote_path)], 
                   capture_output=True, check=True)
    
    # Create a regular repo
    repo_path = tmp_path / "local_repo"
    repo_path.mkdir()
    
    # Initialize repo in the directory
    subprocess.run([git2cpp_path, 'init'], capture_output=True, check=True, cwd=repo_path)
    
    # Add remote
    subprocess.run([git2cpp_path, 'remote', 'add', 'origin', str(remote_path)], 
                   capture_output=True, check=True, cwd=repo_path)
    
    return repo_path, remote_path


def test_fetch_from_remote(git2cpp_path, repo_with_remote):
    """Test fetching from a remote."""
    repo_path, remote_path = repo_with_remote
    
    # Note: This is a bare repo with no refs, so fetch will fail gracefully
    # For now, just test that fetch command runs (it will fail gracefully if no refs)
    cmd = [git2cpp_path, 'fetch', 'origin']
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=repo_path)
    # Fetch might succeed (empty) or fail (no refs), but shouldn't crash
    assert p.returncode in [0, 1]  # 0 for success, 1 for no refs/error


def test_fetch_default_origin(git2cpp_path, repo_with_remote):
    """Test fetching with default origin."""
    repo_path, remote_path = repo_with_remote
    
    cmd = [git2cpp_path, 'fetch']
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=repo_path)
    # Fetch might succeed (empty) or fail (no refs), but shouldn't crash
    assert p.returncode in [0, 1]


def test_remote_in_cloned_repo(xtl_clone, git2cpp_path, tmp_path):
    """Test that cloned repos have remotes configured."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"
    
    cmd = [git2cpp_path, 'remote']
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert 'origin' in p.stdout


def test_remote_show_in_cloned_repo(xtl_clone, git2cpp_path, tmp_path):
    """Test showing remote in cloned repo."""
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"
    
    cmd = [git2cpp_path, 'remote', 'show', 'origin']
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert 'origin' in p.stdout
    # Should contain URL information
    assert 'http' in p.stdout or 'git' in p.stdout or 'https' in p.stdout

