import subprocess

import pytest


def test_revlist(repo_init_with_commit, commit_env_config, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    p = tmp_path / "initial.txt"
    p.write_text("commit2")
    subprocess.run([git2cpp_path, "add", "initial.txt"], cwd=tmp_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "commit 2"], cwd=tmp_path, check=True)

    p.write_text("commit3")
    subprocess.run([git2cpp_path, "add", "initial.txt"], cwd=tmp_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "commit 3"], cwd=tmp_path, check=True)

    cmd = [git2cpp_path, "rev-list", "HEAD", "--max-count", "2"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0

    lines = [line for line in p.stdout.splitlines() if line.strip()]
    assert len(lines) == 2
    assert all(len(oid) == 40 for oid in lines)
    assert lines[0] != lines[1]
