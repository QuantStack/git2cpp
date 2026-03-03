import subprocess

import pytest


@pytest.mark.parametrize("all_flag", ["", "-A", "--all", "--no-ignore-removal"])
def test_add(git2cpp_path, tmp_path, all_flag):
    cmd_init = [git2cpp_path, "init", "."]
    p_init = subprocess.run(cmd_init, capture_output=True, cwd=tmp_path)
    assert p_init.returncode == 0

    p = tmp_path / "mook_file.txt"
    p.write_text("")

    p2 = tmp_path / "mook_file_2.txt"
    p2.write_text("")

    cmd_add = [git2cpp_path, "add"]
    if all_flag != "":
        cmd_add.append(all_flag)
    else:
        cmd_add.append("mook_file.txt")
    p_add = subprocess.run(cmd_add, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    cmd_status = [git2cpp_path, "status", "--long"]
    p_status = subprocess.run(cmd_status, cwd=tmp_path, capture_output=True, text=True)
    assert p_status.returncode == 0

    assert "Changes to be committed" in p_status.stdout
    assert "new file" in p_status.stdout
    if all_flag != "":
        assert "Untracked files" not in p_status.stdout
    else:
        assert "Untracked files" in p_status.stdout


def test_add_nogit(git2cpp_path, tmp_path):
    p = tmp_path / "mook_file.txt"
    p.write_text("")

    cmd_add = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=tmp_path, text=True, capture_output=True)
    assert p_add.returncode != 0
    assert "error: could not find repository at" in p_add.stderr
