import subprocess
from pathlib import Path


def test_init_in_directory(git2cpp_path, tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, "init", "--bare", str(tmp_path)]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    assert p.stdout.startswith("Initialized empty Git repository in ")
    assert p.stdout.strip().endswith("/")

    assert sorted(map(lambda path: path.name, tmp_path.iterdir())) == [
        "HEAD",
        "config",
        "description",
        "hooks",
        "info",
        "objects",
        "refs",
    ]

    # TODO: check this is a valid git repo


def test_init_in_cwd(git2cpp_path, tmp_path, run_in_tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []
    assert Path.cwd() == tmp_path

    cmd = [git2cpp_path, "init", "--bare"]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    assert p.stdout.startswith("Initialized empty Git repository in ")
    assert p.stdout.strip().endswith("/")

    assert sorted(map(lambda path: path.name, tmp_path.iterdir())) == [
        "HEAD",
        "config",
        "description",
        "hooks",
        "info",
        "objects",
        "refs",
    ]

    # TODO: check this is a valid git repo


def test_init_not_bare(git2cpp_path, tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []
    assert not (tmp_path / ".git" / "HEAD").exists()
    cmd = [git2cpp_path, "init", "."]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    assert p.stdout.startswith("Initialized empty Git repository in ")
    assert p.stdout.strip().endswith(".git/")

    # Directory contains just .git directory.
    assert sorted(map(lambda path: path.name, tmp_path.iterdir())) == [".git"]
    # .git directory is a valid repo.
    assert sorted(map(lambda path: path.name, (tmp_path / ".git").iterdir())) == [
        "HEAD",
        "config",
        "description",
        "hooks",
        "info",
        "objects",
        "refs",
    ]

    # Would like to use `git2cpp status` but it complains that 'refs/heads/master' not found
    cmd = [git2cpp_path, "log"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path)
    assert p.returncode == 0
    assert b"does not have any commits yet" in p.stdout


def test_init_reinitializes_existing_repo_message(git2cpp_path, tmp_path):
    cmd = [git2cpp_path, "init", "."]

    p1 = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p1.returncode == 0

    p2 = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p2.returncode == 0
    assert p2.stderr == ""
    assert p2.stdout.startswith("Reinitialized existing Git repository in ")
    assert p2.stdout.strip().endswith("/.git/")


def test_error_on_unknown_option(git2cpp_path):
    cmd = [git2cpp_path, "init", "--unknown"]
    p = subprocess.run(cmd, capture_output=True)
    assert p.returncode == 109
    assert p.stdout == b""
    assert p.stderr.startswith(b"The following argument was not expected: --unknown")


def test_error_on_repeated_directory(git2cpp_path):
    cmd = [git2cpp_path, "init", "abc", "def"]
    p = subprocess.run(cmd, capture_output=True)
    assert p.returncode == 109
    assert p.stdout == b""
    assert p.stderr.startswith(b"The following argument was not expected: def")


def test_init_creates_missing_parent_directories(git2cpp_path, tmp_path):
    # Parent "does-not-exist" does not exist yet.
    repo_dir = tmp_path / "does-not-exist" / "repo"
    assert not repo_dir.parent.exists()

    cmd = [git2cpp_path, "init", "--bare", str(repo_dir)]
    p = subprocess.run(cmd, capture_output=True, text=True)

    assert p.returncode == 0
    assert p.stdout.startswith("Initialized empty Git repository in ")
    assert p.stdout.strip().endswith("/")
    assert ".git" not in p.stdout

    assert repo_dir.exists()
    assert sorted(p.name for p in repo_dir.iterdir()) == [
        "HEAD",
        "config",
        "description",
        "hooks",
        "info",
        "objects",
        "refs",
    ]


def test_init_initial_branch_non_bare(git2cpp_path, tmp_path):
    cmd = [git2cpp_path, "init", "-b", "main", "."]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    assert p.stderr == ""
    assert p.stdout.startswith("Initialized empty Git repository in ")
    assert p.stdout.strip().endswith("/.git/")

    head = (tmp_path / ".git" / "HEAD").read_text()
    assert "refs/heads/main" in head


def test_init_initial_branch_bare(git2cpp_path, tmp_path):
    cmd = [git2cpp_path, "init", "--bare", "-b", "main", str(tmp_path)]
    p = subprocess.run(cmd, capture_output=True, text=True)
    assert p.returncode == 0
    assert p.stderr == ""
    assert p.stdout.startswith("Initialized empty Git repository in ")
    assert p.stdout.strip().endswith("/")

    head = (tmp_path / "HEAD").read_text()
    assert "refs/heads/main" in head
