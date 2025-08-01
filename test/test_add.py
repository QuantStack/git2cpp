import os
import subprocess

import pytest


working_dir = 'test/data/xtl'

@pytest.mark.parametrize("all_flag", ["", "-A", "--all", "--no-ignore-removal"])
def test_add(xtl_clone, git2cpp_path, all_flag):
    with open("./test/data/xtl/mook_file.txt", "x"):
        pass

    with open("./test/data/xtl/mook_file_2.txt", "x"):
        pass

    cmd_add = [git2cpp_path, 'add']
    if all_flag != "":
        cmd_add.append(all_flag)
    else:
        cmd_add.append("mook_file.txt")
    subprocess.run(cmd_add, cwd=working_dir, text=True)

    cmd_status = [git2cpp_path, 'status', "--long"]
    p_status = subprocess.run(cmd_status, cwd=working_dir, capture_output=True, text=True)

    assert "Changes to be committed" in p_status.stdout
    assert "new file" in p_status.stdout
    if all_flag != "":
        assert "Untracked files" not in p_status.stdout
    else:
        assert "Untracked files" in p_status.stdout

    os.remove("./test/data/xtl/mook_file.txt")
    os.remove("./test/data/xtl/mook_file_2.txt")

    # undo the add, to leave the test directory at the end the same as it was at the start
    subprocess.run(cmd_add, cwd=working_dir, capture_output=True, text=True)
