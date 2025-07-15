import os
import subprocess

import pytest


@pytest.mark.parametrize("all_flag", ["-A", "--all", "--no-ignore-removal"])
def test_add(git2cpp_path, all_flag):
    with open("./test/mook_file.txt", "x") as f:
        pass
    f.close()

    cmd_add = [git2cpp_path, 'add']
    if all_flag != "":
        cmd_add.append(all_flag)
    subprocess.run(cmd_add, capture_output=True, text=True)

    cmd_status = [git2cpp_path, 'status', "--long"]
    p_status = subprocess.run(cmd_status, capture_output=True, text=True)

    print(p_status.stdout)
    # assert "Changes to be committed" in p.stdout
    # assert "Changes not staged for commit" in p.stdout
    # assert "Untracked files" in p.stdout
    # assert "new file" in p.stdout
    # assert "deleted" in p.stdout
    # assert "modified" in p.stdout
    #
    os.remove("./test/mook_file.txt")
