import os
import subprocess
from pathlib import Path

import pytest

GIT2CPP_TEST_WASM = os.getenv('GIT2CPP_TEST_WASM') == "1"

if GIT2CPP_TEST_WASM:
    from .conftest_wasm import *

# Fixture to run test in current tmp_path
@pytest.fixture
def run_in_tmp_path(tmp_path):
    original_cwd = os.getcwd()
    os.chdir(tmp_path)
    yield
    os.chdir(original_cwd)


@pytest.fixture(scope="session")
def git2cpp_path():
    if GIT2CPP_TEST_WASM:
        return 'git2cpp'
    else:
        return Path(__file__).parent.parent / 'build' / 'git2cpp'

@pytest.fixture
def xtl_clone(git2cpp_path, tmp_path, run_in_tmp_path):
    url = "https://github.com/xtensor-stack/xtl.git"
    clone_cmd = [git2cpp_path, "clone", url]
    p = subprocess.run(clone_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0


@pytest.fixture
def commit_env_config(monkeypatch):
    config = {
        "GIT_AUTHOR_NAME": "Jane Doe",
        "GIT_AUTHOR_EMAIL": "jane.doe@blabla.com",
        "GIT_COMMITTER_NAME": "Jane Doe",
        "GIT_COMMITTER_EMAIL": "jane.doe@blabla.com"
    }
    for key, value in config.items():
        if GIT2CPP_TEST_WASM:
            subprocess.run(["export", f"{key}='{value}'"], check=True)
        else:
            monkeypatch.setenv(key, value)
