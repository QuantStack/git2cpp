import subprocess
import time

import pytest


# TODO: Have a different "person" for the commit and for the merge
# TODO: Test "unborn" case, but how ?
def test_merge_fast_forward(xtl_clone, git_config, git2cpp_path, tmp_path, monkeypatch):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    checkout_cmd = [git2cpp_path, "checkout", "-b", "foregone"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout.returncode == 0

    p = xtl_path / "mook_file.txt"
    p.write_text("blablabla")

    add_cmd = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "test commit"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    checkout_cmd_2 = [git2cpp_path, "checkout", "master"]
    p_checkout_2 = subprocess.run(
        checkout_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout_2.returncode == 0

    merge_cmd = [git2cpp_path, "merge", "foregone"]
    p_merge = subprocess.run(merge_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_merge.returncode == 0
    assert "Fast-forward" in p_merge.stdout

    log_cmd = [git2cpp_path, "log", "--format=full", "--max-count", "1"]
    p_log = subprocess.run(log_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0
    assert "Author:	Jane Doe" in p_log.stdout
    # assert "Commit:	John Doe" in p_log.stdout
    assert (xtl_path / "mook_file.txt").exists()

    merge_cmd_2 = [git2cpp_path, "merge", "foregone"]
    p_merge_2 = subprocess.run(
        merge_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_merge_2.returncode == 0
    assert p_merge_2.stdout == "Already up-to-date\n"


def test_merge(xtl_clone, git_config, git2cpp_path, tmp_path, monkeypatch):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    checkout_cmd = [git2cpp_path, "checkout", "-b", "foregone"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout.returncode == 0

    p = xtl_path / "mook_file.txt"
    p.write_text("blablabla")

    add_cmd = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "test commit foregone"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    checkout_cmd_2 = [git2cpp_path, "checkout", "master"]
    p_checkout_2 = subprocess.run(
        checkout_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout_2.returncode == 0

    p = xtl_path / "mook_file_2.txt"
    p.write_text("BLABLABLA")

    add_cmd_2 = [git2cpp_path, "add", "mook_file_2.txt"]
    p_add_2 = subprocess.run(add_cmd_2, capture_output=True, cwd=xtl_path, text=True)
    assert p_add_2.returncode == 0

    commit_cmd_2 = [git2cpp_path, "commit", "-m", "test commit master"]
    p_commit_2 = subprocess.run(
        commit_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_commit_2.returncode == 0

    merge_cmd = [git2cpp_path, "merge", "foregone"]
    p_merge = subprocess.run(merge_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_merge.returncode == 0

    log_cmd = [git2cpp_path, "log", "--format=full", "--max-count", "2"]
    p_log = subprocess.run(log_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0
    assert "Author:	Jane Doe" in p_log.stdout
    # assert "Commit:	John Doe" in p_log.stdout
    assert "Johan" not in p_log.stdout
    assert (xtl_path / "mook_file.txt").exists()
    assert (xtl_path / "mook_file.txt").exists()

    merge_cmd_2 = [git2cpp_path, "merge", "foregone"]
    p_merge_2 = subprocess.run(
        merge_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_merge_2.returncode == 0
    assert p_merge_2.stdout == "Already up-to-date\n"
