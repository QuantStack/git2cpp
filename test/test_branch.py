import os
import subprocess

import pytest

def test_branch_list(rename_git, git2cpp_path):
    cmd = [git2cpp_path, 'branch']
    p = subprocess.run(cmd, capture_output=True, cwd="test/data/status_data", text=True)
    assert(p.stdout == '* main\n')

def test_branch_create_delete(rename_git, git2cpp_path):
    create_cmd = [git2cpp_path, 'branch', 'foregone']
    subprocess.run(create_cmd, capture_output=True, cwd="test/data/status_data", text=True)
    list_cmd = [git2cpp_path, 'branch']
    p = subprocess.run(list_cmd, capture_output=True, cwd="test/data/status_data", text=True)
    assert(p.stdout == '  foregone\n* main\n')
    del_cmd = [git2cpp_path, 'branch', '-d', 'foregone']
    subprocess.run(del_cmd, capture_output=True, cwd="test/data/status_data", text=True)
    p2 = subprocess.run(list_cmd, capture_output=True, cwd="test/data/status_data", text=True)
    assert(p2.stdout == '* main\n')


