import os
from pathlib import Path
import pytest
import subprocess


# Fixture to run test in current tmp_path
@pytest.fixture
def run_in_tmp_path(tmp_path):
    original_cwd = os.getcwd()
    os.chdir(tmp_path)
    yield
    os.chdir(original_cwd)


@pytest.fixture(scope="session")
def git2cpp_path():
    return Path(__file__).parent.parent / "build" / "git2cpp"


@pytest.fixture
def xtl_clone(git2cpp_path, tmp_path, run_in_tmp_path):
    url = "https://github.com/xtensor-stack/xtl.git"
    clone_cmd = [git2cpp_path, "clone", url]
    subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)


@pytest.fixture
def git_config(monkeypatch):
    monkeypatch.setenv("GIT_AUTHOR_NAME", "Jane Doe")
    monkeypatch.setenv("GIT_AUTHOR_EMAIL", "jane.doe@blabla.com")
    monkeypatch.setenv("GIT_COMMITTER_NAME", "Jane Doe")
    monkeypatch.setenv("GIT_COMMITTER_EMAIL", "jane.doe@blabla.com")
