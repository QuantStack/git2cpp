# from pathlib import Path
import os
import subprocess

import pytest
from .conftest import GIT2CPP_TEST_WASM


@pytest.mark.parametrize("short_flag", ["", "-s", "--short"])
@pytest.mark.parametrize("long_flag", ["", "--long"])
def test_status_new_file(xtl_clone, git2cpp_path, tmp_path, short_flag, long_flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    p = xtl_path / "mook_file.txt"  # Untracked files
    p.write_text("")

    pw = xtl_path / "CMakeLists.txt"  # Changes not staged for commit / modified
    pw.write_text("blablabla")

    os.remove(xtl_path / "README.md")  # Changes not staged for commit / deleted

    cmd = [git2cpp_path, "status"]
    if short_flag != "":
        cmd.append(short_flag)
    if long_flag != "":
        cmd.append(long_flag)
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)

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


def test_status_nogit(git2cpp_path, tmp_path):
    cmd = [git2cpp_path, "status"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    if not GIT2CPP_TEST_WASM:
        # TODO: fix this in wasm build
        assert p.returncode != 0
    assert "error: could not find repository at" in p.stderr


@pytest.mark.parametrize("short_flag", ["", "-s", "--short"])
@pytest.mark.parametrize("long_flag", ["", "--long"])
def test_status_add_file(xtl_clone, git2cpp_path, tmp_path, short_flag, long_flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    p = xtl_path / "mook_file.txt"  # Changes to be committed / new file
    p.write_text("")

    os.remove(xtl_path / "README.md")  # Changes to be committed / deleted

    cmd_add = [git2cpp_path, "add", "--all"]
    p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    cmd_status = [git2cpp_path, "status"]
    if short_flag != "":
        cmd_status.append(short_flag)
    if long_flag != "":
        cmd_status.append(long_flag)
    p_status = subprocess.run(cmd_status, capture_output=True, cwd=xtl_path, text=True)
    assert p_status.returncode == 0

    if (long_flag == "--long") or ((long_flag == "") & (short_flag == "")):
        assert "Changes to be committed" in p_status.stdout
        assert "Changes not staged for commit" not in p_status.stdout
        assert "Untracked files" not in p_status.stdout
        assert "new file" in p_status.stdout
        assert "deleted" in p_status.stdout

    elif short_flag in ["-s", "--short"]:
        assert "A  " in p_status.stdout
        assert "D  " in p_status.stdout


def test_status_new_repo(git2cpp_path, tmp_path, run_in_tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, "init"]
    p = subprocess.run(cmd, cwd=tmp_path)
    assert p.returncode == 0

    status_cmd = [git2cpp_path, "status"]
    p_status = subprocess.run(status_cmd, cwd=tmp_path)
    assert p_status.returncode == 0
