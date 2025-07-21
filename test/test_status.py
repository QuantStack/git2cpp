# from pathlib import Path
import os
import subprocess

import pytest


@pytest.fixture
def rename_git():
    os.rename("test/data/status_data/embedded_git/", "test/data/status_data/.git/")
    yield
    os.rename("test/data/status_data/.git/", "test/data/status_data/embedded_git/")


@pytest.mark.parametrize("short_flag", ["", "-s", "--short"])
@pytest.mark.parametrize("long_flag", ["", "--long"])
def test_status_format(rename_git, git2cpp_path, short_flag, long_flag):
    cmd = [git2cpp_path, 'status']
    if short_flag != "":
        cmd.append(short_flag)
    if long_flag != "":
        cmd.append(long_flag)
    p = subprocess.run(cmd, capture_output=True, cwd="test/data/status_data", text=True)

    if (long_flag == "--long") or ((long_flag == "") & (short_flag == "")):
        assert "Changes to be committed" in p.stdout
        assert "Changes not staged for commit" in p.stdout
        assert "Untracked files" in p.stdout
        assert "new file" in p.stdout
        assert "deleted" in p.stdout
        assert "modified" in p.stdout

    elif short_flag in ["-s", "--short"]:
        assert "D  " in p.stdout
        assert " M " in p.stdout
        assert " D " in p.stdout
        assert "?? " in p.stdout
