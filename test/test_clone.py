import subprocess
from .conftest import GIT2CPP_TEST_WASM

url = "https://github.com/xtensor-stack/xtl.git"


def test_clone(git2cpp_path, tmp_path, run_in_tmp_path):
    clone_cmd = [git2cpp_path, "clone", url]
    p_clone = subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_clone.returncode == 0

    assert (tmp_path / "xtl").exists()
    assert (tmp_path / "xtl/include").exists()


def test_clone_is_bare(git2cpp_path, tmp_path, run_in_tmp_path):
    clone_cmd = [git2cpp_path, "clone", "--bare", url]
    p_clone = subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_clone.returncode == 0

    assert (tmp_path / "xtl").is_dir()

    status_cmd = [git2cpp_path, "status"]
    p_status = subprocess.run(status_cmd, capture_output=True, cwd=tmp_path / "xtl", text=True)
    if not GIT2CPP_TEST_WASM:
        # TODO: fix this in wasm build
        assert p_status.returncode != 0
    assert "This operation is not allowed against bare repositories" in p_status.stderr

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, capture_output=True, cwd=tmp_path / "xtl", text=True)
    assert p_branch.returncode == 0
    assert p_branch.stdout.strip() == "* master"


def test_clone_shallow(git2cpp_path, tmp_path, run_in_tmp_path):
    clone_cmd = [git2cpp_path, "clone", "--depth", "1", url]
    p_clone = subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_clone.returncode == 0
    assert (tmp_path / "xtl").exists()

    xtl_path = tmp_path / "xtl"

    cmd_log = [git2cpp_path, "log"]
    p_log = subprocess.run(cmd_log, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0
    assert p_log.stdout.count("Author") == 1
