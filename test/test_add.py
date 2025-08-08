import os
import subprocess

import pytest


@pytest.mark.parametrize("all_flag", ["", "-A", "--all", "--no-ignore-removal"])
def test_add(xtl_clone, git2cpp_path, tmp_path, all_flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    p = xtl_path / "mook_file.txt"
    p.write_text('')

    p2 = xtl_path / "mook_file_2.txt"
    p2.write_text('')

    cmd_add = [git2cpp_path, 'add']
    if all_flag != "":
        cmd_add.append(all_flag)
    else:
        cmd_add.append("mook_file.txt")
    p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    cmd_status = [git2cpp_path, 'status', "--long"]
    p_status = subprocess.run(cmd_status, cwd=xtl_path, capture_output=True, text=True)
    assert p_status.returncode == 0

    assert "Changes to be committed" in p_status.stdout
    assert "new file" in p_status.stdout
    if all_flag != "":
        assert "Untracked files" not in p_status.stdout
    else:
        assert "Untracked files" in p_status.stdout
