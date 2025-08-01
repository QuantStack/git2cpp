import os
from pathlib import Path
import pytest
import subprocess
from genericpath import exists


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
def xtl_clone(git2cpp_path):
    url = 'https://github.com/xtensor-stack/xtl.git'
    clone_working_dir = 'test/data'

    clone_cmd = [git2cpp_path, 'clone', url]
    subprocess.run(clone_cmd, capture_output=True, cwd = clone_working_dir, text=True)

    yield

    cleanup_cmd = ['rm', '-rf', 'xtl']
    subprocess.run(cleanup_cmd, capture_output=True, cwd = clone_working_dir, text=True)

@pytest.fixture
def git_config(git2cpp_path):
    gitconfig_path = "~/.gitconfig"
    if not(os.path.isfile(gitconfig_path)):
        with open("~/.gitconfig", "a") as f:
            f.write("[user]\n name = Jane Doe\n email = jane.doe@blabla.com")

        yield

        os.remove("test/data/.gitconfig")
