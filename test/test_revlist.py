import subprocess

import pytest


def test_revlist(xtl_clone, git_config, git2cpp_path, tmp_path, monkeypatch):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    cmd = [
        git2cpp_path,
        "rev-list",
        "35955995424eb9699bb604b988b5270253b1fccc",
        "--max-count",
        "4",
    ]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "da1754dd6" in p.stdout
