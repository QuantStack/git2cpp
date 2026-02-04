import subprocess

import pytest
from .conftest import GIT2CPP_TEST_WASM


def test_branch_list(xtl_clone, git2cpp_path, tmp_path):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    cmd = [git2cpp_path, 'branch']
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert(p.stdout == '* master\n')


def test_branch_create_delete(xtl_clone, git2cpp_path, tmp_path):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    create_cmd = [git2cpp_path, 'branch', 'foregone']
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_create.returncode == 0

    list_cmd = [git2cpp_path, 'branch']
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert(p_list.stdout == '  foregone\n* master\n')

    del_cmd = [git2cpp_path, 'branch', '-d', 'foregone']
    p_del = subprocess.run(del_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_del.returncode == 0

    p_list2 = subprocess.run(list_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_list2.returncode == 0
    assert(p_list2.stdout == '* master\n')

def test_branch_nogit(git2cpp_path, tmp_path):
    cmd = [git2cpp_path, 'branch']
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    if not GIT2CPP_TEST_WASM:
        # TODO: fix this in wasm build
        assert p.returncode != 0
    assert "error: could not find repository at" in p.stderr


def test_branch_new_repo(git2cpp_path, tmp_path, run_in_tmp_path):
   # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, 'init']
    p = subprocess.run(cmd, cwd = tmp_path)

    branch_cmd = [git2cpp_path, 'branch']
    p_branch = subprocess.run(branch_cmd, cwd = tmp_path)

    assert p_branch.returncode == 0
