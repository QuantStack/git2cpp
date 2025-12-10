import os
import subprocess

import pytest


def test_clone(git2cpp_path, tmp_path, run_in_tmp_path):
    url = "https://github.com/xtensor-stack/xtl.git"

    clone_cmd = [git2cpp_path, "clone", url]
    p_clone = subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_clone.returncode == 0

    assert os.path.exists(os.path.join(tmp_path, "xtl"))
    assert os.path.exists(os.path.join(tmp_path, "xtl/include"))


def test_clone_is_bare(git2cpp_path, tmp_path, run_in_tmp_path):
    url = "https://github.com/xtensor-stack/xtl.git"

    clone_cmd = [git2cpp_path, "clone", "--bare", url]
    p_clone = subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_clone.returncode == 0

    status_cmd = [git2cpp_path, "status"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_status.returncode != 0
