import os
import subprocess

import pytest


working_dir = 'test/data/xtl'

def test_branch_list(xtl_clone, git2cpp_path):
    cmd = [git2cpp_path, 'branch']
    p = subprocess.run(cmd, capture_output=True, cwd=working_dir, text=True)
    assert(p.stdout == '* master\n')


def test_branch_create_delete(xtl_clone, git2cpp_path):
    create_cmd = [git2cpp_path, 'branch', 'foregone']
    subprocess.run(create_cmd, capture_output=True, cwd=working_dir, text=True)
    list_cmd = [git2cpp_path, 'branch']
    p = subprocess.run(list_cmd, capture_output=True, cwd=working_dir, text=True)
    assert(p.stdout == '  foregone\n* master\n')

    del_cmd = [git2cpp_path, 'branch', '-d', 'foregone']
    subprocess.run(del_cmd, capture_output=True, cwd=working_dir, text=True)
    p2 = subprocess.run(list_cmd, capture_output=True, cwd=working_dir, text=True)
    assert(p2.stdout == '* master\n')
