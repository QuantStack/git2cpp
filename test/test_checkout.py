import os
import subprocess

import pytest


def test_checkout(xtl_clone, git2cpp_path, tmp_path, run_in_tmp_path):
    create_cmd = [git2cpp_path, 'branch', 'foregone']
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_create.returncode == 0

    checkout_cmd = [git2cpp_path, 'checkout', 'foregone']
    p_checkout = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_checkout.returncode == 0
    assert(p_checkout.stdout == '');

    branch_cmd = [git2cpp_path, 'branch']
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_branch.returncode == 0
    assert(p_branch.stdout == '* foregone\n  master\n')

    checkout_cmd[2] = 'master'
    p_checkout2 = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_checkout2.returncode == 0


def test_checkout_b(xtl_clone, git2cpp_path, tmp_path, run_in_tmp_path):
    checkout_cmd = [git2cpp_path, 'checkout', '-b', 'foregone']
    p_checkout = subprocess.run(checkout_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_checkout.returncode == 0
    assert(p_checkout.stdout == '');

    branch_cmd = [git2cpp_path, 'branch']
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_branch.returncode == 0
    assert(p_branch.stdout == '* foregone\n  master\n')

    checkout_cmd.remove('-b')
    checkout_cmd[2] = 'master'
    p_checkout2 = subprocess.run(checkout_cmd, cwd=tmp_path, text=True)
    assert p_checkout2.returncode == 0
