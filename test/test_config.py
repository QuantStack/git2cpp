import subprocess

import pytest
from .conftest import GIT2CPP_TEST_WASM


def test_config_list(commit_env_config, git2cpp_path, tmp_path):
    cmd_init = [git2cpp_path, "init", "--bare", str(tmp_path)]
    p_init = subprocess.run(cmd_init, capture_output=True)
    assert p_init.returncode == 0

    cmd_list = [git2cpp_path, "config", "list"]
    p_list = subprocess.run(cmd_list, capture_output=True, cwd=tmp_path, text=True)
    assert p_list.returncode == 0
    assert "core.bare=true" in p_list.stdout
    assert "remote" not in p_list.stdout


def test_config_get(git2cpp_path, tmp_path):
    cmd_init = [git2cpp_path, "init", "--bare", str(tmp_path)]
    p_init = subprocess.run(cmd_init, capture_output=True)
    assert p_init.returncode == 0

    cmd_get = [git2cpp_path, "config", "get", "core.bare"]
    p_get = subprocess.run(cmd_get, capture_output=True, cwd=tmp_path, text=True)
    assert p_get.returncode == 0
    assert p_get.stdout == "true\n"


def test_config_set(commit_env_config, git2cpp_path, tmp_path):
    cmd_init = [git2cpp_path, "init", "--bare", str(tmp_path)]
    p_init = subprocess.run(cmd_init, capture_output=True)
    assert p_init.returncode == 0

    cmd_set = [git2cpp_path, "config", "set", "code.bare", "false"]
    p_set = subprocess.run(cmd_set, cwd=tmp_path, text=True)
    assert p_set.returncode == 0

    cmd_get = [git2cpp_path, "config", "get", "code.bare"]
    p_get = subprocess.run(cmd_get, capture_output=True, cwd=tmp_path, text=True)
    assert p_get.returncode == 0
    assert p_get.stdout == "false\n"


def test_config_unset(git2cpp_path, tmp_path):
    cmd_init = [git2cpp_path, "init", "--bare", str(tmp_path)]
    p_init = subprocess.run(cmd_init, capture_output=True)
    assert p_init.returncode == 0

    cmd_get = [git2cpp_path, "config", "unset", "core.bare"]
    p_get = subprocess.run(cmd_get, capture_output=True, cwd=tmp_path, text=True)
    assert p_get.returncode == 0

    cmd_get = [git2cpp_path, "config", "get", "core.bare"]
    p_get = subprocess.run(cmd_get, capture_output=True, cwd=tmp_path, text=True)
    if not GIT2CPP_TEST_WASM:
        # TODO: fix this in wasm build
        assert p_get.returncode != 0
    assert p_get.stderr == "error: config value 'core.bare' was not found\n"
