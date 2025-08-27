import subprocess

import pytest

@pytest.mark.parametrize("format_flag", ["", "--format=full", "--format=fuller"])
def test_log(xtl_clone, git_config, git2cpp_path, tmp_path, monkeypatch, format_flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    p = xtl_path / "mook_file.txt"
    p.write_text('')

    cmd_add = [git2cpp_path, 'add', "mook_file.txt"]
    p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    cmd_commit = [git2cpp_path, 'commit', "-m", "test commit"]
    p_commit = subprocess.run(cmd_commit, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    cmd_log = [git2cpp_path, 'log']
    if format_flag != "":
        cmd_log.append(format_flag)
    p_log = subprocess.run(cmd_log, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0
    assert "Jane Doe" in p_log.stdout
    assert "test commit" in p_log.stdout

    if format_flag == "":
        assert "Commit" not in p_log.stdout
    else:
        assert "Commit" in p_log.stdout
        if format_flag == "--format=full":
            assert "Date" not in p_log.stdout
        else:
            assert "CommitDate" in p_log.stdout


@pytest.mark.parametrize("max_count_flag", ["", "-n", "--max-count"])
def test_max_count(xtl_clone, git_config, git2cpp_path, tmp_path, monkeypatch, max_count_flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    cmd_log = [git2cpp_path, 'log']
    if max_count_flag != "":
        cmd_log.append(max_count_flag)
        cmd_log.append("2")
    p_log = subprocess.run(cmd_log, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0

    if max_count_flag == "":
        assert p_log.stdout.count("Author") > 2
    else:
        assert p_log.stdout.count("Author") == 2
