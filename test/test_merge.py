import subprocess

import pytest


# TODO: Have a different "person" for the commit and for the merge
# TODO: Test "unborn" case, but how ?
def test_merge_fast_forward(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    checkout_cmd = [git2cpp_path, "checkout", "-b", "foregone"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout.returncode == 0

    file_path = xtl_path / "mook_file.txt"
    file_path.write_text("blablabla")

    add_cmd = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "test commit"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    checkout_cmd_2 = [git2cpp_path, "checkout", "master"]
    p_checkout_2 = subprocess.run(
        checkout_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout_2.returncode == 0

    merge_cmd = [git2cpp_path, "merge", "foregone"]
    p_merge = subprocess.run(merge_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_merge.returncode == 0
    assert "Fast-forward" in p_merge.stdout

    log_cmd = [git2cpp_path, "log", "--format=full", "--max-count", "1"]
    p_log = subprocess.run(log_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0
    assert "Author:	Jane Doe" in p_log.stdout
    # assert "Commit:	John Doe" in p_log.stdout
    assert (xtl_path / "mook_file.txt").exists()

    merge_cmd_2 = [git2cpp_path, "merge", "foregone"]
    p_merge_2 = subprocess.run(
        merge_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_merge_2.returncode == 0
    assert p_merge_2.stdout == "Already up-to-date\n"


def test_merge_commit(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    checkout_cmd = [git2cpp_path, "checkout", "-b", "foregone"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout.returncode == 0

    file_path = xtl_path / "mook_file.txt"
    file_path.write_text("blablabla")

    add_cmd = [git2cpp_path, "add", "mook_file.txt"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "test commit foregone"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    checkout_cmd_2 = [git2cpp_path, "checkout", "master"]
    p_checkout_2 = subprocess.run(
        checkout_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout_2.returncode == 0

    file_path_2 = xtl_path / "mook_file_2.txt"
    file_path_2.write_text("BLABLABLA")

    add_cmd_2 = [git2cpp_path, "add", "mook_file_2.txt"]
    p_add_2 = subprocess.run(add_cmd_2, capture_output=True, cwd=xtl_path, text=True)
    assert p_add_2.returncode == 0

    commit_cmd_2 = [git2cpp_path, "commit", "-m", "test commit master"]
    p_commit_2 = subprocess.run(
        commit_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_commit_2.returncode == 0

    merge_cmd = [git2cpp_path, "merge", "foregone"]
    p_merge = subprocess.run(merge_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_merge.returncode == 0

    log_cmd = [git2cpp_path, "log", "--format=full", "--max-count", "2"]
    p_log = subprocess.run(log_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_log.returncode == 0
    assert "Author:	Jane Doe" in p_log.stdout
    # assert "Commit:	John Doe" in p_log.stdout
    assert "Johan" not in p_log.stdout
    assert (xtl_path / "mook_file.txt").exists()
    assert (xtl_path / "mook_file_2.txt").exists()

    merge_cmd_2 = [git2cpp_path, "merge", "foregone"]
    p_merge_2 = subprocess.run(
        merge_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_merge_2.returncode == 0
    assert p_merge_2.stdout == "Already up-to-date\n"


@pytest.mark.parametrize("flag", ["--abort", "--quit", "--continue"])
def test_merge_conflict(xtl_clone, commit_env_config, git2cpp_path, tmp_path, flag):
    assert (tmp_path / "xtl").exists()
    xtl_path = tmp_path / "xtl"

    checkout_cmd = [git2cpp_path, "checkout", "-b", "foregone"]
    p_checkout = subprocess.run(
        checkout_cmd, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout.returncode == 0

    file_path = xtl_path / "mook_file.txt"
    file_path.write_text("blablabla")

    file_path_2 = xtl_path / "mook_file_2.txt"
    file_path_2.write_text("Second file")

    add_cmd = [git2cpp_path, "add", "--all"]
    p_add = subprocess.run(add_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_add.returncode == 0

    commit_cmd = [git2cpp_path, "commit", "-m", "test commit foregone"]
    p_commit = subprocess.run(commit_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_commit.returncode == 0

    checkout_cmd_2 = [git2cpp_path, "checkout", "master"]
    p_checkout_2 = subprocess.run(
        checkout_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_checkout_2.returncode == 0

    file_path.write_text("BLABLABLA")

    add_cmd_2 = [git2cpp_path, "add", "mook_file.txt"]
    p_add_2 = subprocess.run(add_cmd_2, capture_output=True, cwd=xtl_path, text=True)
    assert p_add_2.returncode == 0

    commit_cmd_2 = [git2cpp_path, "commit", "-m", "test commit master"]
    p_commit_2 = subprocess.run(
        commit_cmd_2, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_commit_2.returncode == 0

    merge_cmd = [git2cpp_path, "merge", "foregone"]
    p_merge = subprocess.run(merge_cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p_merge.returncode == 0
    assert "conflict: " in p_merge.stdout

    flag_cmd = [git2cpp_path, "merge", flag]
    if flag == "--abort":
        for answer in {"y", ""}:
            p_abort = subprocess.run(
                flag_cmd, input=answer, capture_output=True, cwd=xtl_path, text=True
            )
            assert p_abort.returncode == 0
            assert (xtl_path / "mook_file.txt").exists()
            text = (xtl_path / "mook_file.txt").read_text()
            if answer == "y":
                assert "BLA" in text
                assert "bla" not in text
            else:
                assert "Abort." in p_abort.stdout

    elif flag == "--quit":
        pass
        # p_quit = subprocess.run(flag_cmd, capture_output=True, cwd=xtl_path, text=True)
        # assert p_quit.returncode == 0
        # assert (xtl_path / "mook_file.txt").exists()
        # with open(xtl_path / "mook_file.txt") as f:
        #     lines = f.readlines()
        #     assert "<<<<<<< HEAD" in lines[0]
        #     assert ">>>>>>> foregone" in lines[-1]

        # p_merge_2 = subprocess.run(
        #     merge_cmd, capture_output=True, cwd=xtl_path, text=True
        # )
        # assert p_merge_2.returncode != 0
        # print(p_merge_2.stdout)
        # assert "error: Merging is not possible because you have unmerged files." in p_merge_2.stdout

    elif flag == "--continue":
        # Create another branch pointing to the same commit (alias branch).
        # This checks the merge behaviour when a different branch name points to the same commit.
        branch_alias_cmd = [git2cpp_path, "branch", "foregone_alias"]
        p_branch_alias = subprocess.run(
            branch_alias_cmd, capture_output=True, cwd=xtl_path, text=True
        )
        assert p_branch_alias.returncode == 0

        file_path.write_text("blablabla")

        cmd_add = [git2cpp_path, "add", "mook_file.txt"]
        p_add = subprocess.run(cmd_add, cwd=xtl_path, text=True)
        assert p_add.returncode == 0

        p_continue = subprocess.run(
            flag_cmd, capture_output=True, cwd=xtl_path, text=True
        )
        assert p_continue.returncode == 0

        log_cmd = [git2cpp_path, "log", "--format=full", "--max-count", "2"]
        p_log = subprocess.run(log_cmd, capture_output=True, cwd=xtl_path, text=True)
        assert p_log.returncode == 0
        assert "Author:	Jane Doe" in p_log.stdout
        # assert "Commit:	John Doe" in p_log.stdout
        assert "Johan" not in p_log.stdout
        assert (xtl_path / "mook_file.txt").exists()
        assert (xtl_path / "mook_file_2.txt").exists()

        merge_cmd_2 = [git2cpp_path, "merge", "foregone"]
        p_merge_2 = subprocess.run(
            merge_cmd_2, capture_output=True, cwd=xtl_path, text=True
        )
        assert p_merge_2.returncode == 0
        assert p_merge_2.stdout == "Already up-to-date\n"
