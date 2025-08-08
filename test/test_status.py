# from pathlib import Path
import os
import subprocess

import pytest


working_dir = 'test/data/xtl'

@pytest.mark.parametrize("short_flag", ["", "-s", "--short"])
@pytest.mark.parametrize("long_flag", ["", "--long"])
def test_status_new_file(xtl_clone, git2cpp_path, short_flag, long_flag):
    with open("./test/data/xtl/mook_file.txt", "x"):   # Untracked files
        pass

    with open("./test/data/xtl/CMakeLists.txt", "a") as f:   # Changes not staged for commit / modified
        f.write("blablabla")

    os.remove("./test/data/xtl/README.md")   # Changes not staged for commit / deleted

    cmd = [git2cpp_path, 'status']
    if short_flag != "":
        cmd.append(short_flag)
    if long_flag != "":
        cmd.append(long_flag)
    p = subprocess.run(cmd, capture_output=True, cwd=working_dir, text=True)
    assert p.returncode == 0

    if (long_flag == "--long") or ((long_flag == "") & (short_flag == "")):
        assert "On branch master" in p.stdout
        assert "Changes not staged for commit" in p.stdout
        assert "Untracked files" in p.stdout
        assert "deleted" in p.stdout
        assert "modified" in p.stdout

    elif short_flag in ["-s", "--short"]:
        assert " M " in p.stdout
        assert " D " in p.stdout
        assert "?? " in p.stdout


@pytest.mark.parametrize("short_flag", ["", "-s", "--short"])
@pytest.mark.parametrize("long_flag", ["", "--long"])
def test_status_add_file(xtl_clone, git2cpp_path, short_flag, long_flag):
    with open("./test/data/xtl/mook_file.txt", "x"):   # Changes to be committed / new file
        pass

    os.remove("./test/data/xtl/README.md")   # Changes to be committed / deleted

    cmd_add = [git2cpp_path, 'add', "--all"]
    p = subprocess.run(cmd_add, cwd=working_dir, text=True)
    assert p.returncode == 0

    cmd_status = [git2cpp_path, 'status']
    if short_flag != "":
        cmd_status.append(short_flag)
    if long_flag != "":
        cmd_status.append(long_flag)
    p = subprocess.run(cmd_status, capture_output=True, cwd=working_dir, text=True)

    if (long_flag == "--long") or ((long_flag == "") & (short_flag == "")):
        assert "Changes to be committed" in p.stdout
        assert "Changes not staged for commit" not in p.stdout
        assert "Untracked files" not in p.stdout
        assert "new file" in p.stdout
        assert "deleted" in p.stdout

    elif short_flag in ["-s", "--short"]:
        assert "A  " in p.stdout
        assert "D  " in p.stdout
