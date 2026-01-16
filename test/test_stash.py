import subprocess

import pytest


def test_stash_push(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    p = xtl_path / "mook_file.txt"
    p.write_text("blabla")

    cmd_add = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    stash_path = xtl_path / ".git/refs/stash"
    assert not stash_path.exists()

    cmd_stash = [git2cpp_path, "stash"]
    p_stash = subprocess.run(cmd_stash, capture_output=True, cwd=xtl_path, text=True)
    assert p_stash.returncode == 0
    assert stash_path.exists()


def test_stash_list(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    p = xtl_path / "mook_file.txt"
    p.write_text("blabla")

    cmd_add = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    cmd_list = [git2cpp_path, "stash", "list"]
    p_list = subprocess.run(cmd_list, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert "stash@{0}" not in p_list.stdout

    cmd_stash = [git2cpp_path, "stash"]
    p_stash = subprocess.run(cmd_stash, capture_output=True, cwd=xtl_path, text=True)
    assert p_stash.returncode == 0

    p_list_2 = subprocess.run(cmd_list, capture_output=True, cwd=xtl_path, text=True)
    assert p_list_2.returncode == 0
    assert "stash@{0}" in p_list_2.stdout


@pytest.mark.parametrize("index_flag", ["", "--index"])
def test_stash_pop(xtl_clone, commit_env_config, git2cpp_path, tmp_path, index_flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    index = 0 if index_flag == "" else 1

    for i in range(index + 1):
        p = xtl_path / f"mook_file_{i}.txt"
        p.write_text(f"blabla{i}")

        cmd_add = [git2cpp_path, "add", f"mook_file_{i}.txt"]
        p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
        assert p_add.returncode == 0

        cmd_stash = [git2cpp_path, "stash"]
        p_stash = subprocess.run(
            cmd_stash, capture_output=True, cwd=xtl_path, text=True
        )
        assert p_stash.returncode == 0

        cmd_status = [git2cpp_path, "status"]
        p_status = subprocess.run(
            cmd_status, capture_output=True, cwd=xtl_path, text=True
        )
        assert p_status.returncode == 0
        assert "mook_file" not in p_status.stdout

    cmd_pop = [git2cpp_path, "stash", "pop"]
    if index_flag != "":
        cmd_pop.append(index_flag)
        cmd_pop.append("1")
    p_pop = subprocess.run(cmd_pop, capture_output=True, cwd=xtl_path, text=True)
    assert p_pop.returncode == 0
    assert "mook_file_0" in p_pop.stdout
    assert "Dropped refs/stash@{" + str(index) + "}" in p_pop.stdout

    cmd_list = [git2cpp_path, "stash", "list"]
    p_list = subprocess.run(cmd_list, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    if index_flag == "":
        assert p_list.stdout == ""
    else:
        assert "stash@{0}" in p_list.stdout
        assert "stash@{1}" not in p_list.stdout


@pytest.mark.parametrize("index_flag", ["", "--index"])
def test_stash_apply(xtl_clone, commit_env_config, git2cpp_path, tmp_path, index_flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    index = 0 if index_flag == "" else 1

    for i in range(index + 1):
        p = xtl_path / f"mook_file_{i}.txt"
        p.write_text(f"blabla{i}")

        cmd_add = [git2cpp_path, "add", f"mook_file_{i}.txt"]
        p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
        assert p_add.returncode == 0

        cmd_stash = [git2cpp_path, "stash"]
        p_stash = subprocess.run(
            cmd_stash, capture_output=True, cwd=xtl_path, text=True
        )
        assert p_stash.returncode == 0

        cmd_status = [git2cpp_path, "status"]
        p_status = subprocess.run(
            cmd_status, capture_output=True, cwd=xtl_path, text=True
        )
        assert p_status.returncode == 0
        assert "mook_file" not in p_status.stdout

    cmd_apply = [git2cpp_path, "stash", "apply"]
    if index_flag != "":
        cmd_apply.append(index_flag)
        cmd_apply.append("1")
    p_apply = subprocess.run(cmd_apply, capture_output=True, cwd=xtl_path, text=True)
    assert p_apply.returncode == 0
    assert "mook_file_0" in p_apply.stdout

    cmd_list = [git2cpp_path, "stash", "list"]
    p_list = subprocess.run(cmd_list, capture_output=True, cwd=xtl_path, text=True)
    assert p_list.returncode == 0
    assert "stash@{0}" in p_list.stdout
    if index_flag != "":
        assert "stash@{1}" in p_list.stdout
