import subprocess

import pytest


def test_revparse_bare(git2cpp_path, tmp_path, run_in_tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, "init", "--bare"]
    p = subprocess.run(cmd, cwd=tmp_path)
    assert p.returncode == 0

    cmd2 = [git2cpp_path, "rev-parse", "--is-bare-repository"]
    p2 = subprocess.run(cmd2, capture_output=True, text=True, cwd=tmp_path)
    assert p2.returncode == 0
    assert p2.stdout == "true\n"


def test_revparse_shallow(git2cpp_path, tmp_path, run_in_tmp_path):
    url = "https://github.com/xtensor-stack/xtl.git"
    cmd = [git2cpp_path, "clone", "--depth", "1", url]
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=tmp_path)
    assert p.returncode == 0
    assert (tmp_path / "xtl").exists()

    xtl_path = tmp_path / "xtl"
    cmd2 = [git2cpp_path, "rev-parse", "--is-shallow-repository"]
    p2 = subprocess.run(cmd2, capture_output=True, text=True, cwd=xtl_path)
    assert p2.returncode == 0
    assert p2.stdout == "true\n"
