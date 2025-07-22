import os
import subprocess

import pytest

def test_checkout(rename_git, git2cpp_path):
    create_cmd = [git2cpp_path, 'branch', 'foregone']
    subprocess.run(create_cmd, capture_output=True, cwd="test/data/status_data", text=True)

    checkout_cmd = [git2cpp_path, 'checkout', 'foregone']
    p = subprocess.run(checkout_cmd, capture_output=True, cwd="test/data/status_data", text=True)
    assert(p.stdout == '');

    branch_cmd = [git2cpp_path, 'branch']
    p2 = subprocess.run(branch_cmd, capture_output=True, cwd="test/data/status_data", text=True)
    assert(p2.stdout == '* foregone\n  main\n')

    checkout_cmd[2] = 'main'
    subprocess.run(checkout_cmd, capture_output=True, cwd="test/data/status_data", text=True)

    del_cmd = [git2cpp_path, 'branch', '-d', 'foregone']
    subprocess.run(del_cmd, cwd="test/data/status_data", text=True)

def test_checkout_b(rename_git, git2cpp_path):
    checkout_cmd = [git2cpp_path, 'checkout', '-b', 'foregone']
    p = subprocess.run(checkout_cmd, capture_output=True, cwd="test/data/status_data", text=True)
    assert(p.stdout == '');

    branch_cmd = [git2cpp_path, 'branch']
    p2 = subprocess.run(branch_cmd, capture_output=True, cwd="test/data/status_data", text=True)
    assert(p2.stdout == '* foregone\n  main\n')

    checkout_cmd.remove('-b')
    checkout_cmd[2] = 'main'
    subprocess.run(checkout_cmd, cwd="test/data/status_data", text=True)

    del_cmd = [git2cpp_path, 'branch', '-d', 'foregone']
    subprocess.run(del_cmd, cwd="test/data/status_data", text=True)

