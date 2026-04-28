# Test fixtures to confirm that wasm monkeypatching works correctly.

import re
import subprocess
from .conftest import GIT2CPP_TEST_WASM


def test_run_in_tmp_path(tmp_path, run_in_tmp_path):
    p = subprocess.run(["pwd"], capture_output=True, text=True, check=True)
    assert p.stdout.strip() == str(tmp_path)


def test_tmp_path(tmp_path):
    p = subprocess.run(["pwd"], capture_output=True, text=True, check=True, cwd=str(tmp_path))
    assert p.stdout.strip() == str(tmp_path)

    assert tmp_path.exists()
    assert tmp_path.is_dir()
    assert not tmp_path.is_file()

    assert sorted(tmp_path.iterdir()) == []
    subprocess.run(["mkdir", f"{tmp_path}/def"], capture_output=True, text=True, check=True)
    assert sorted(tmp_path.iterdir()) == [tmp_path / "def"]
    subprocess.run(["mkdir", f"{tmp_path}/abc"], capture_output=True, text=True, check=True)
    assert sorted(tmp_path.iterdir()) == [tmp_path / "abc", tmp_path / "def"]

    p = subprocess.run(["pwd"], capture_output=True, text=True, check=True, cwd=tmp_path.parent)
    assert p.stdout.strip() == str(tmp_path.parent)
    assert tmp_path in list(tmp_path.parent.iterdir())


def test_env_vars():
    # By default there should be not GIT_* env vars set.
    p = subprocess.run(["env"], capture_output=True, text=True, check=True)
    git_lines = sorted(filter(lambda f: f.startswith("GIT_"), re.split(r"\r?\n", p.stdout)))
    if GIT2CPP_TEST_WASM:
        assert git_lines == ["GIT_CORS_PROXY=http://localhost:8881/"]
    else:
        assert git_lines == []


def test_repo_init_with_commit(repo_init_with_commit, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()
    assert (tmp_path / "initial.txt").is_file()
    assert (tmp_path / "initial.txt").read_text() == "initial"

    git_dir = tmp_path / ".git"
    assert git_dir.exists()
    assert git_dir.is_dir()

    assert sorted(tmp_path.iterdir()) == [
        git_dir,
        tmp_path / "initial.txt",
    ]

    assert sorted(git_dir.iterdir()) == [
        git_dir / "HEAD",
        git_dir / "config",
        git_dir / "description",
        git_dir / "hooks",
        git_dir / "index",
        git_dir / "info",
        git_dir / "logs",
        git_dir / "objects",
        git_dir / "refs",
    ]

    cmd = [git2cpp_path, "log"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    lines = p.stdout.splitlines()
    assert "commit" in lines[0]
    assert lines[1].startswith("Author:")
    assert lines[2].startswith("Date:")
    assert "Initial commit" in lines[4]
