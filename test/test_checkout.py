import os
import subprocess

import pytest


working_dir = 'test/data/xtl'

def test_checkout(xtl_clone, git2cpp_path):
    create_cmd = [git2cpp_path, 'branch', 'foregone']
    subprocess.run(create_cmd, capture_output=True, cwd=working_dir, text=True)

    checkout_cmd = [git2cpp_path, 'checkout', 'foregone']
    p = subprocess.run(checkout_cmd, capture_output=True, cwd=working_dir, text=True)
    assert(p.stdout == '');

    branch_cmd = [git2cpp_path, 'branch']
    p2 = subprocess.run(branch_cmd, capture_output=True, cwd=working_dir, text=True)
    assert(p2.stdout == '* foregone\n  master\n')

    checkout_cmd[2] = 'master'
    subprocess.run(checkout_cmd, capture_output=True, cwd=working_dir, text=True)

    del_cmd = [git2cpp_path, 'branch', '-d', 'foregone']
    subprocess.run(del_cmd, cwd=working_dir, text=True)


def test_checkout_b(xtl_clone, git2cpp_path):
    checkout_cmd = [git2cpp_path, 'checkout', '-b', 'foregone']
    p = subprocess.run(checkout_cmd, capture_output=True, cwd=working_dir, text=True)
    assert(p.stdout == '');

    branch_cmd = [git2cpp_path, 'branch']
    p2 = subprocess.run(branch_cmd, capture_output=True, cwd=working_dir, text=True)
    assert(p2.stdout == '* foregone\n  master\n')

    checkout_cmd.remove('-b')
    checkout_cmd[2] = 'master'
    subprocess.run(checkout_cmd, cwd=working_dir, text=True)

    del_cmd = [git2cpp_path, 'branch', '-d', 'foregone']
    subprocess.run(del_cmd, cwd=working_dir, text=True)
