import os
import subprocess

import pytest


def test_clone(git2cpp_path):
    url = 'https://github.com/xtensor-stack/xtl.git'
    working_dir = 'test/data'

    clone_cmd = [git2cpp_path, 'clone', url]
    subprocess.run(clone_cmd, capture_output=True, cwd = working_dir, text=True)

    assert os.path.exists(working_dir + '/xtl')
    assert os.path.exists(working_dir + '/xtl/include')

    cleanup_cmd = ['rm', '-rf', 'xtl']
    subprocess.run(cleanup_cmd, capture_output=True, cwd = working_dir, text=True)
