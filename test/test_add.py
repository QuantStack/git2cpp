import os
import subprocess

import pytest


@pytest.mark.parametrize("all_flag", ["", "-A", "--all", "--no-ignore-removal"])
def test_add(git2cpp_path, all_flag):
    with open("./test/mook_file.txt", "x") as f:
        pass

    with open("./test/mook_file_2.txt", "x") as f:
        pass

    cmd_add = [git2cpp_path, 'add']
    if all_flag != "":
        cmd_add.append(all_flag)
    else:
        cmd_add.append("test/mook_file.txt")
    subprocess.run(cmd_add, capture_output=True, text=True)

    cmd_status = [git2cpp_path, 'status', "--long"]
    p_status = subprocess.run(cmd_status, capture_output=True, text=True)

    assert "Changes to be committed" in p_status.stdout
    assert "new file" in p_status.stdout
    if all_flag != "":
        assert "Untracked files" not in p_status.stdout
    else:
        assert "Untracked files" in p_status.stdout

    os.remove("./test/mook_file.txt")
    os.remove("./test/mook_file_2.txt")

    # undo the add, to leave the test directory at the end the same as it was at the start
    subprocess.run(cmd_add, capture_output=True, text=True)
