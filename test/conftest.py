import os
from pathlib import Path
import pytest

# Fixture to run test in current tmp_path
@pytest.fixture
def run_in_tmp_path(tmp_path):
    original_cwd = os.getcwd()
    os.chdir(tmp_path)
    yield
    os.chdir(original_cwd)

@pytest.fixture(scope='session')
def git2cpp_path():
    return Path(__file__).parent.parent / 'build' / 'git2cpp'

@pytest.fixture
def rename_git():
    os.rename("test/data/status_data/embedded_git/", "test/data/status_data/.git/")
    yield
    os.rename("test/data/status_data/.git/", "test/data/status_data/embedded_git/")
