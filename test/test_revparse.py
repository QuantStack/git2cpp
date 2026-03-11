import subprocess

import pytest


def test_revparse_bare(git2cpp_path, tmp_path, run_in_tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, "init", "--bare"]
    p = subprocess.run(cmd, cwd=tmp_path)
    assert p.returncode == 0

    cmd2 = [git2cpp_path, "rev-parse", "--is-bare-repository"]
    p2 = subprocess.run(cmd2, capture_output=True, text=True, cwd=tmp_path)
    assert p2.returncode == 0
    assert p2.stdout == "true\n"


def test_revparse_shallow(git2cpp_path, tmp_path, run_in_tmp_path):
    url = "https://github.com/xtensor-stack/xtl.git"
    cmd = [git2cpp_path, "clone", "--depth", "1", url]
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=tmp_path)
    assert p.returncode == 0
    assert (tmp_path / "xtl").exists()

    xtl_path = tmp_path / "xtl"
    cmd2 = [git2cpp_path, "rev-parse", "--is-shallow-repository"]
    p2 = subprocess.run(cmd2, capture_output=True, text=True, cwd=xtl_path)
    assert p2.returncode == 0
    assert p2.stdout == "true\n"

    cmd3 = [
        git2cpp_path,
        "rev-parse",
        "--is-shallow-repository",
        "--is-bare-repository",
    ]
    p3 = subprocess.run(cmd3, capture_output=True, text=True, cwd=xtl_path)
    assert p3.returncode == 0
    assert p3.stdout == "true\nfalse\n"


def test_revparse_multiple_revs(repo_init_with_commit, git2cpp_path, tmp_path):
    """Test one sha per line is printed when multiple revisions are provided"""
    assert (tmp_path / "initial.txt").exists()

    (tmp_path / "second.txt").write_text("second")
    subprocess.run(
        [git2cpp_path, "add", "second.txt"],
        capture_output=True,
        text=True,
        cwd=tmp_path,
        check=True,
    )
    subprocess.run(
        [git2cpp_path, "commit", "-m", "Second commit"],
        capture_output=True,
        text=True,
        cwd=tmp_path,
        check=True,
    )

    p = subprocess.run(
        [git2cpp_path, "rev-parse", "HEAD", "HEAD~1"],
        capture_output=True,
        text=True,
        cwd=tmp_path,
    )
    assert p.returncode == 0

    lines = p.stdout.splitlines()
    print()
    assert len(lines) == 2
    assert all(len(x) == 40 for x in lines)
    assert lines[0] != lines[1]


def test_revparse_multiple_opts(git2cpp_path, tmp_path, run_in_tmp_path):
    """Test the options are printed in order"""
    url = "https://github.com/xtensor-stack/xtl.git"
    cmd = [git2cpp_path, "clone", "--depth", "2", url]
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=tmp_path)
    assert p.returncode == 0
    assert (tmp_path / "xtl").exists()

    xtl_path = tmp_path / "xtl"

    p = subprocess.run(
        [
            git2cpp_path,
            "rev-parse",
            "HEAD",
            "--is-shallow-repository",
            "--is-bare-repository",
            "HEAD~1",
        ],
        capture_output=True,
        text=True,
        cwd=xtl_path,
    )
    assert p.returncode == 0

    lines = p.stdout.splitlines()
    assert len(lines) == 4
    assert len(lines[0]) == 40
    assert len(lines[3]) == 40
    assert lines[0] != lines[1]
    assert "true" in lines[1]
    assert "false" in lines[2]


def test_revparse_errors(repo_init_with_commit, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    rev_cmd = [git2cpp_path, "rev-parse", "HEAD~1"]
    p_rev = subprocess.run(rev_cmd, capture_output=True, text=True, cwd=tmp_path)
    assert p_rev.returncode == 129
    assert "bad revision" in p_rev.stderr

    opt_cmd = [git2cpp_path, "rev-parse", "--parseopt"]
    p_opt = subprocess.run(opt_cmd, capture_output=True, text=True, cwd=tmp_path)
    assert p_opt.returncode != 0
    assert "The following argument was not expected:" in p_opt.stderr
