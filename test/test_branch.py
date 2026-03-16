import subprocess


def test_branch_list(repo_init_with_commit, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    cmd = [git2cpp_path, "branch"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    assert "* main" in p.stdout


def test_branch_create_delete(repo_init_with_commit, git2cpp_path, tmp_path):
    assert (tmp_path / "initial.txt").exists()

    create_cmd = [git2cpp_path, "branch", "foregone"]
    p_create = subprocess.run(create_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_create.returncode == 0

    list_cmd = [git2cpp_path, "branch"]
    p_list = subprocess.run(list_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_list.returncode == 0
    assert "  foregone\n* main" in p_list.stdout

    del_cmd = [git2cpp_path, "branch", "-d", "foregone"]
    p_del = subprocess.run(del_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_del.returncode == 0

    p_list2 = subprocess.run(list_cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_list2.returncode == 0
    assert "* main" in p_list2.stdout


def test_branch_nogit(git2cpp_path, tmp_path):
    cmd = [git2cpp_path, "branch"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode != 0
    assert "error: could not find repository at" in p.stderr


def test_branch_new_repo(git2cpp_path, tmp_path, run_in_tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, "init"]
    subprocess.run(cmd, cwd=tmp_path, check=True)

    branch_cmd = [git2cpp_path, "branch"]
    p_branch = subprocess.run(branch_cmd, cwd=tmp_path)

    assert p_branch.returncode == 0


def test_branch_list_flag(repo_init_with_commit, git2cpp_path, tmp_path):
    """Explicit -l/--list flag behaves the same as bare 'branch'."""
    assert (tmp_path / "initial.txt").exists()

    subprocess.run([git2cpp_path, "branch", "feature-a"], cwd=tmp_path, check=True)

    for flag in ["-l", "--list"]:
        cmd = [git2cpp_path, "branch", flag]
        p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
        assert p.returncode == 0
        assert "  feature-a" in p.stdout
        assert "* main" in p.stdout


def test_branch_list_all(xtl_clone, git2cpp_path, tmp_path):
    """The -a/--all flag lists both local and remote-tracking branches."""
    xtl_path = tmp_path / "xtl"

    cmd = [git2cpp_path, "branch", "-a"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "* master" in p.stdout
    assert "origin/" in p.stdout


def test_branch_list_remotes(xtl_clone, git2cpp_path, tmp_path):
    """The -r/--remotes flag lists only remote-tracking branches."""
    xtl_path = tmp_path / "xtl"

    cmd = [git2cpp_path, "branch", "-r"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "origin/" in p.stdout
    # Local branch should NOT appear with * prefix
    assert "* master" not in p.stdout


def test_branch_create_already_exists(repo_init_with_commit, git2cpp_path, tmp_path):
    """Creating a branch that already exists should fail without --force."""
    assert (tmp_path / "initial.txt").exists()

    subprocess.run([git2cpp_path, "branch", "duplicate"], cwd=tmp_path, check=True)

    cmd = [git2cpp_path, "branch", "duplicate"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode != 0


def test_branch_create_force_overwrite(
    repo_init_with_commit, commit_env_config, git2cpp_path, tmp_path
):
    """--force allows overwriting an existing branch."""
    assert (tmp_path / "initial.txt").exists()

    subprocess.run([git2cpp_path, "branch", "my-branch"], cwd=tmp_path, check=True)

    # Add a second commit so HEAD moves forward
    (tmp_path / "second.txt").write_text("second")
    subprocess.run([git2cpp_path, "add", "second.txt"], cwd=tmp_path, check=True)
    subprocess.run([git2cpp_path, "commit", "-m", "Second commit"], cwd=tmp_path, check=True)

    # Without --force this would fail; with -f it should reset the branch to current HEAD
    cmd = [git2cpp_path, "branch", "-f", "my-branch"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0


def test_branch_delete_nonexistent(repo_init_with_commit, git2cpp_path, tmp_path):
    """Deleting a branch that doesn't exist should fail."""
    assert (tmp_path / "initial.txt").exists()

    cmd = [git2cpp_path, "branch", "-d", "no-such-branch"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode != 0


def test_branch_create_multiple(repo_init_with_commit, git2cpp_path, tmp_path):
    """Creating multiple branches and verifying they all appear in the listing."""
    assert (tmp_path / "initial.txt").exists()

    branches = ["alpha", "beta", "gamma"]
    for name in branches:
        p = subprocess.run(
            [git2cpp_path, "branch", name], capture_output=True, cwd=tmp_path, text=True
        )
        assert p.returncode == 0

    cmd = [git2cpp_path, "branch"]
    p_list = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p_list.returncode == 0
    for name in branches:
        assert f"  {name}" in p_list.stdout
    # Current branch is still starred
    assert "* main" in p_list.stdout


def test_branch_show_current(repo_init_with_commit, git2cpp_path, tmp_path):
    """--show-current prints the current branch name."""
    assert (tmp_path / "initial.txt").exists()

    cmd = [git2cpp_path, "branch", "--show-current"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    print(p.stdout)
    # Default branch after init is "main" or "master" depending on git config
    assert p.stdout.strip() == "main"


def test_branch_show_current_after_create_and_switch(repo_init_with_commit, git2cpp_path, tmp_path):
    """--show-current reflects the branch we switched to."""
    assert (tmp_path / "initial.txt").exists()

    subprocess.run([git2cpp_path, "checkout", "-b", "new-feature"], cwd=tmp_path, check=True)

    cmd = [git2cpp_path, "branch", "--show-current"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    assert p.stdout == "new-feature\n"


def test_branch_show_current_detached_head(repo_init_with_commit, git2cpp_path, tmp_path):
    """--show-current prints nothing when HEAD is detached."""
    assert (tmp_path / "initial.txt").exists()

    result = subprocess.run(
        [git2cpp_path, "rev-parse", "HEAD"],
        capture_output=True,
        cwd=tmp_path,
        text=True,
        check=True,
    )
    head_sha = result.stdout.strip()
    subprocess.run([git2cpp_path, "checkout", head_sha], cwd=tmp_path, check=True)

    cmd = [git2cpp_path, "branch", "--show-current"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    assert p.stdout == ""


def test_branch_show_current_new_repo(git2cpp_path, tmp_path, run_in_tmp_path):
    """--show-current prints the branch name even on a fresh repo with no commits (unborn HEAD)."""
    assert list(tmp_path.iterdir()) == []

    subprocess.run([git2cpp_path, "init", "-b", "main"], cwd=tmp_path, check=True)

    cmd = [git2cpp_path, "branch", "--show-current"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    # Default branch after init is "main" or "master" depending on git config
    assert p.stdout.strip() == "main"


def test_branch_show_current_nogit(git2cpp_path, tmp_path):
    """--show-current fails gracefully outside a git repository."""
    cmd = [git2cpp_path, "branch", "--show-current"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode != 0
    assert "error: could not find repository at" in p.stderr
