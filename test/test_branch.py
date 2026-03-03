import subprocess

import pytest


def test_branch_list(repo_init_with_commit, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    cmd = [git2cpp_path, "branch"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    assert "* ma" in p.stdout


def test_branch_create_delete(repo_init_with_commit, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    create_cmd = [git2cpp_path, "branch", "foregone"]
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_create.returncode == 0

    list_cmd = [git2cpp_path, "branch"]
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_list.returncode == 0
    assert "  foregone\n* ma" in p_list.stdout

    del_cmd = [git2cpp_path, "branch", "-d", "foregone"]
    p_del = subprocess.run(del_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_del.returncode == 0

    p_list2 = subprocess.run(list_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_list2.returncode == 0
    assert "* ma" in p_list2.stdout


def test_branch_nogit(git2cpp_path, tmp_path):
    cmd = [git2cpp_path, "branch"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode != 0
    assert "error: could not find repository at" in p.stderr


def test_branch_new_repo(git2cpp_path, tmp_path, run_in_tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, "init"]
    subprocess.run(cmd, cwd=tmp_path, check=True)

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, cwd=tmp_path)

    assert p_branch.returncode == 0
