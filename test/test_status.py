# from pathlib import Path
import os
import subprocess

import pytest


@pytest.fixture
def rename_git():
    os.rename("test/data/status_data/embeded_git/", "test/data/status_data/.git/")
    yield
    os.rename("test/data/status_data/.git/", "test/data/status_data/embeded_git/")


@pytest.mark.parametrize("short_flag", ["", "-s", "--short"])
@pytest.mark.parametrize("long_flag", ["", "--long"])
def test_status_format(rename_git, git2cpp_path, short_flag, long_flag):
    cmd = [git2cpp_path, 'status', short_flag, long_flag]
    p = subprocess.run(cmd, capture_output=True, cwd="test/data/status_data", text=True)

    if (long_flag == "--long") or ((long_flag == "") & (short_flag == "")):
        assert "Changes to be committed" in p.stdout
    elif short_flag in ["-s", "--short"]:
        pass

    print(p.stdout)
