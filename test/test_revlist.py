import subprocess

import pytest


def test_revlist(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    cmd = [
        git2cpp_path,
        "rev-list",
        "35955995424eb9699bb604b988b5270253b1fccc",
        "--max-count",
        "2",
    ]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "da1754dd6" in p.stdout
    assert "2da8e13ef" not in p.stdout
