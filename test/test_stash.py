import subprocess

import pytest


def test_stash_push(repo_init_with_commit, commit_env_config, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    p = tmp_path / "mook_file.txt"
    p.write_text("blabla")

    cmd_add = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    stash_path = tmp_path / ".git/refs/stash"
    assert not stash_path.exists()

    cmd_stash = [git2cpp_path, "stash"]
    p_stash = subprocess.run(cmd_stash, capture_output=True, cwd=tmp_path, text=True)
    assert p_stash.returncode == 0
    assert stash_path.exists()


def test_stash_list(repo_init_with_commit, commit_env_config, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    p = tmp_path / "mook_file.txt"
    p.write_text("blabla")

    cmd_add = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    cmd_list = [git2cpp_path, "stash", "list"]
    p_list = subprocess.run(cmd_list, capture_output=True, cwd=tmp_path, text=True)
    assert p_list.returncode == 0
    assert "stash@{0}" not in p_list.stdout

    cmd_stash = [git2cpp_path, "stash"]
    p_stash = subprocess.run(cmd_stash, capture_output=True, cwd=tmp_path, text=True)
    assert p_stash.returncode == 0

    p_list_2 = subprocess.run(cmd_list, capture_output=True, cwd=tmp_path, text=True)
    assert p_list_2.returncode == 0
    assert "stash@{0}" in p_list_2.stdout


@pytest.mark.parametrize("index_flag", ["", "--index"])
def test_stash_pop(
    repo_init_with_commit, commit_env_config, git2cpp_path, tmp_path, index_flag
):
    assert (tmp_path / "initial.txt").exists()

    index = 0 if index_flag == "" else 1

    for i in range(index + 1):
        p = tmp_path / f"mook_file_{i}.txt"
        p.write_text(f"blabla{i}")

        cmd_add = [git2cpp_path, "add", f"mook_file_{i}.txt"]
        p_add = subprocess.run(cmd_add, cwd=tmp_path, text=True)
        assert p_add.returncode == 0

        cmd_stash = [git2cpp_path, "stash"]
        p_stash = subprocess.run(
            cmd_stash, capture_output=True, cwd=tmp_path, text=True
        )
        assert p_stash.returncode == 0

        cmd_status = [git2cpp_path, "status"]
        p_status = subprocess.run(
            cmd_status, capture_output=True, cwd=tmp_path, text=True
        )
        assert p_status.returncode == 0
        assert "mook_file" not in p_status.stdout

    cmd_pop = [git2cpp_path, "stash", "pop"]
    if index_flag != "":
        cmd_pop.append(index_flag)
        cmd_pop.append("1")
    p_pop = subprocess.run(cmd_pop, capture_output=True, cwd=tmp_path, text=True)
    assert p_pop.returncode == 0
    assert "mook_file_0" in p_pop.stdout
    assert "Dropped refs/stash@{" + str(index) + "}" in p_pop.stdout

    cmd_list = [git2cpp_path, "stash", "list"]
    p_list = subprocess.run(cmd_list, capture_output=True, cwd=tmp_path, text=True)
    assert p_list.returncode == 0
    if index_flag == "":
        assert p_list.stdout == ""
    else:
        assert "stash@{0}" in p_list.stdout
        assert "stash@{1}" not in p_list.stdout


@pytest.mark.parametrize("index_flag", ["", "--index"])
def test_stash_apply(
    repo_init_with_commit, commit_env_config, git2cpp_path, tmp_path, index_flag
):
    assert (tmp_path / "initial.txt").exists()

    index = 0 if index_flag == "" else 1

    for i in range(index + 1):
        p = tmp_path / f"mook_file_{i}.txt"
        p.write_text(f"blabla{i}")

        cmd_add = [git2cpp_path, "add", f"mook_file_{i}.txt"]
        p_add = subprocess.run(cmd_add, cwd=tmp_path, text=True)
        assert p_add.returncode == 0

        cmd_stash = [git2cpp_path, "stash"]
        p_stash = subprocess.run(
            cmd_stash, capture_output=True, cwd=tmp_path, text=True
        )
        assert p_stash.returncode == 0

        cmd_status = [git2cpp_path, "status"]
        p_status = subprocess.run(
            cmd_status, capture_output=True, cwd=tmp_path, text=True
        )
        assert p_status.returncode == 0
        assert "mook_file" not in p_status.stdout

    cmd_apply = [git2cpp_path, "stash", "apply"]
    if index_flag != "":
        cmd_apply.append(index_flag)
        cmd_apply.append("1")
    p_apply = subprocess.run(cmd_apply, capture_output=True, cwd=tmp_path, text=True)
    assert p_apply.returncode == 0
    assert "mook_file_0" in p_apply.stdout

    cmd_list = [git2cpp_path, "stash", "list"]
    p_list = subprocess.run(cmd_list, capture_output=True, cwd=tmp_path, text=True)
    assert p_list.returncode == 0
    assert "stash@{0}" in p_list.stdout
    if index_flag != "":
        assert "stash@{1}" in p_list.stdout


def test_stash_show(repo_init_with_commit, commit_env_config, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    filename = "mook_show.txt"
    p = tmp_path / filename
    p.write_text("Hello")

    cmd_add = [git2cpp_path, "add", filename]
    p_add = subprocess.run(cmd_add, cwd=tmp_path, text=True)
    assert p_add.returncode == 0

    cmd_stash = [git2cpp_path, "stash"]
    p_stash = subprocess.run(cmd_stash, capture_output=True, cwd=tmp_path, text=True)
    assert p_stash.returncode == 0

    cmd_show = [git2cpp_path, "stash", "show", "--stat"]
    p_show = subprocess.run(cmd_show, capture_output=True, cwd=tmp_path, text=True)
    assert p_show.returncode == 0

    # A diffstat should mention the file and summary "file changed"
    assert filename in p_show.stdout
    assert "1 file changed" in p_show.stdout
