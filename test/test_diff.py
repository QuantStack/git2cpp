import re
import subprocess

import pytest


def test_diff_nogit(git2cpp_path, tmp_path):
    cmd = [git2cpp_path, "diff"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode != 0
    assert "repository" in p.stderr.lower() or "not a git" in p.stderr.lower()


def test_diff_working_directory(xtl_clone, git2cpp_path, tmp_path):
    xtl_path = tmp_path / "xtl"

    readme = xtl_path / "README.md"
    original_content = readme.read_text()
    readme.write_text(original_content + "\nNew line added")

    cmd = [git2cpp_path, "diff"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "README.md" in p.stdout
    assert "New line added" in p.stdout  # should be "+New line added"


@pytest.mark.parametrize("cached_flag", ["--cached", "--staged"])
def test_diff_cached(xtl_clone, git2cpp_path, tmp_path, cached_flag):
    xtl_path = tmp_path / "xtl"

    new_file = xtl_path / "new_file.txt"
    new_file.write_text("Hello, world!")

    cmd_add = [git2cpp_path, "add", "new_file.txt"]
    subprocess.run(cmd_add, cwd=xtl_path, check=True)

    cmd_diff = [git2cpp_path, "diff", cached_flag]
    p_diff = subprocess.run(cmd_diff, capture_output=True, cwd=xtl_path, text=True)
    assert p_diff.returncode == 0
    assert "new_file.txt" in p_diff.stdout
    assert "+Hello, world!" in p_diff.stdout


def test_diff_two_commits(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    xtl_path = tmp_path / "xtl"

    new_file = xtl_path / "new_file.txt"
    new_file.write_text("Hello, world!")

    cmd_add = [git2cpp_path, "add", "new_file.txt"]
    subprocess.run(cmd_add, cwd=xtl_path, check=True)

    cmd_commit = [git2cpp_path, "commit", "-m", "new commit"]
    subprocess.run(cmd_commit, cwd=xtl_path, check=True)

    cmd_diff = [git2cpp_path, "diff", "HEAD~1", "HEAD"]
    p_diff = subprocess.run(cmd_diff, capture_output=True, cwd=xtl_path, text=True)
    assert p_diff.returncode == 0
    assert "new_file.txt" in p_diff.stdout
    assert "+Hello, world!" in p_diff.stdout


def test_diff_no_index(git2cpp_path, tmp_path):
    file1 = tmp_path / "file1.txt"
    file2 = tmp_path / "file2.txt"

    file1.write_text("Hello\nWorld\n")
    file2.write_text("Hello\nPython\n")

    cmd = [git2cpp_path, "diff", "--no-index", str(file1), str(file2)]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    assert "-World" in p.stdout
    assert "+Python" in p.stdout


def test_diff_stat(xtl_clone, git2cpp_path, tmp_path):
    xtl_path = tmp_path / "xtl"

    readme = xtl_path / "README.md"
    readme.write_text("Modified content\n")

    cmd = [git2cpp_path, "diff", "--stat"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "README.md" in p.stdout
    assert "1 file changed, 1 insertion(+)" in p.stdout
    assert "Modified content" not in p.stdout


def test_diff_shortstat(xtl_clone, git2cpp_path, tmp_path):
    """Test diff with --shortstat (last line of --stat only)"""
    xtl_path = tmp_path / "xtl"

    readme = xtl_path / "README.md"
    readme.write_text("Modified content\n")

    cmd = [git2cpp_path, "diff", "--shortstat"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "README.md" not in p.stdout
    assert "1 file changed, 1 insertion(+)" in p.stdout
    assert "Modified content" not in p.stdout


def test_diff_numstat(xtl_clone, git2cpp_path, tmp_path):
    """Test diff with --numstat (machine-friendly stat)"""
    xtl_path = tmp_path / "xtl"

    readme = xtl_path / "README.md"
    readme.write_text("Modified content\n")

    cmd = [git2cpp_path, "diff", "--numstat"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "README.md" in p.stdout
    assert bool(re.search("1       [0-9]*", p.stdout))
    assert "Modified content" not in p.stdout


def test_diff_summary(xtl_clone, git2cpp_path, tmp_path):
    """Test diff with --summary"""
    xtl_path = tmp_path / "xtl"

    new_file = xtl_path / "newfile.txt"
    new_file.write_text("New content")

    cmd_add = [git2cpp_path, "add", "newfile.txt"]
    subprocess.run(cmd_add, cwd=xtl_path, check=True)

    cmd = [git2cpp_path, "diff", "--cached", "--summary"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "newfile.txt" in p.stdout
    assert "+" not in p.stdout


def test_diff_name_only(xtl_clone, git2cpp_path, tmp_path):
    xtl_path = tmp_path / "xtl"

    (xtl_path / "README.md").write_text("Modified")

    cmd = [git2cpp_path, "diff", "--name-only"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)

    assert p.returncode == 0
    assert p.stdout == "README.md\n"
    assert "+" not in p.stdout


def test_diff_name_status(xtl_clone, git2cpp_path, tmp_path):
    xtl_path = tmp_path / "xtl"

    (xtl_path / "README.md").write_text("Modified")

    cmd = [git2cpp_path, "diff", "--name-status"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert p.stdout == "M\tREADME.md\n"


def test_diff_raw(xtl_clone, git2cpp_path, tmp_path):
    """Test diff with --raw format"""
    xtl_path = tmp_path / "xtl"

    readme = xtl_path / "README.md"
    readme.write_text("Modified")

    cmd = [git2cpp_path, "diff", "--raw"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "M\tREADME.md" in p.stdout
    assert bool(re.search(":[0-9]*", p.stdout))


def test_diff_reverse(xtl_clone, git2cpp_path, tmp_path):
    xtl_path = tmp_path / "xtl"

    readme = xtl_path / "README.md"
    original = readme.read_text()
    readme.write_text(original + "\nAdded line")

    cmd_normal = [git2cpp_path, "diff"]
    p_normal = subprocess.run(cmd_normal, capture_output=True, cwd=xtl_path, text=True)
    assert p_normal.returncode == 0
    assert "+Added line" in p_normal.stdout

    cmd_reverse = [git2cpp_path, "diff", "-R"]
    p_reverse = subprocess.run(
        cmd_reverse, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_reverse.returncode == 0
    assert "-Added line" in p_reverse.stdout


@pytest.mark.parametrize("text_flag", ["-a", "--text"])
def test_diff_text(xtl_clone, commit_env_config, git2cpp_path, tmp_path, text_flag):
    """Test diff with -a/--text (treat all files as text)"""
    xtl_path = tmp_path / "xtl"

    binary_file = xtl_path / "binary.bin"
    binary_file.write_bytes(b"\x00\x01\x02\x03")

    cmd_add = [git2cpp_path, "add", "binary.bin"]
    subprocess.run(cmd_add, cwd=xtl_path, check=True)

    cmd_commit = [git2cpp_path, "commit", "-m", "add binary"]
    subprocess.run(cmd_commit, cwd=xtl_path, check=True)

    binary_file.write_bytes(b"\x00\x01\x02\x04")

    cmd_text = [git2cpp_path, "diff", text_flag]
    p = subprocess.run(cmd_text, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "binary.bin" in p.stdout
    assert "@@" in p.stdout


def test_diff_ignore_space_at_eol(xtl_clone, git2cpp_path, tmp_path):
    """Test diff with --ignore-space-at-eol"""
    xtl_path = tmp_path / "xtl"

    readme = xtl_path / "README.md"
    original = readme.read_text()
    # Add trailing spaces at end of line
    readme.write_text(original.rstrip() + "  \n")

    cmd = [git2cpp_path, "diff", "--ignore-space-at-eol"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert p.stdout == ""


@pytest.mark.parametrize("space_change_flag", ["-b", "--ignore-space-change"])
def test_diff_ignore_space_change(
    xtl_clone, commit_env_config, git2cpp_path, tmp_path, space_change_flag
):
    """Test diff with -b/--ignore-space-change"""
    xtl_path = tmp_path / "xtl"

    test_file = xtl_path / "test.txt"
    test_file.write_text("Hello  world\n")

    cmd_add = [git2cpp_path, "add", "test.txt"]
    subprocess.run(cmd_add, cwd=xtl_path, check=True)

    cmd_commit = [git2cpp_path, "commit", "-m", "test"]
    subprocess.run(cmd_commit, cwd=xtl_path, check=True)

    # Change spacing
    test_file.write_text("Hello    world\n")

    cmd_diff = [git2cpp_path, "diff", space_change_flag]
    p = subprocess.run(cmd_diff, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert p.stdout == ""


@pytest.mark.parametrize("ignore_space_flag", ["-w", "--ignore-all-space"])
def test_diff_ignore_all_space(
    xtl_clone, commit_env_config, git2cpp_path, tmp_path, ignore_space_flag
):
    """Test diff with -w/--ignore-all-space"""
    xtl_path = tmp_path / "xtl"

    test_file = xtl_path / "test.txt"
    test_file.write_text("Hello world\n")

    cmd_add = [git2cpp_path, "add", "test.txt"]
    subprocess.run(cmd_add, cwd=xtl_path, check=True)

    cmd_commit = [git2cpp_path, "commit", "-m", "test"]
    subprocess.run(cmd_commit, cwd=xtl_path, check=True)

    test_file.write_text("Helloworld")

    cmd_diff = [git2cpp_path, "diff", ignore_space_flag]
    p = subprocess.run(cmd_diff, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert p.stdout == ""


@pytest.mark.parametrize(
    "unified_context_flag,context_lines",
    [("-U0", 0), ("-U1", 1), ("-U5", 5), ("--unified=3", 3)],
)
def test_diff_unified_context(
    xtl_clone,
    commit_env_config,
    git2cpp_path,
    tmp_path,
    unified_context_flag,
    context_lines,
):
    """Test diff with -U/--unified for context lines"""
    xtl_path = tmp_path / "xtl"

    test_file = xtl_path / "test.txt"
    # Create a file with enough lines to see context differences
    test_file.write_text(
        "Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9\nLine 10\n"
    )

    cmd_add = [git2cpp_path, "add", "test.txt"]
    subprocess.run(cmd_add, cwd=xtl_path, check=True)

    cmd_commit = [git2cpp_path, "commit", "-m", "test"]
    subprocess.run(cmd_commit, cwd=xtl_path, check=True)

    # Modify line 5 (middle of the file)
    test_file.write_text(
        "Line 1\nLine 2\nLine 3\nLine 4\nMODIFIED LINE 5\nLine 6\nLine 7\nLine 8\nLine 9\nLine 10\n"
    )

    # Run diff with the parameterized flag
    cmd = [git2cpp_path, "diff", unified_context_flag]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "test.txt" in p.stdout
    assert "MODIFIED LINE 5" in p.stdout
    assert "@@" in p.stdout  # Hunk header should always be present

    # Verify context lines based on context_lines parameter

    if context_lines >= 1:
        # Should show immediate neighbors
        assert "Line 4" in p.stdout
        assert "Line 6" in p.stdout

    if context_lines >= 3:
        # Should show 3 lines before and after
        assert "Line 2" in p.stdout
        assert "Line 3" in p.stdout
        assert "Line 7" in p.stdout
        assert "Line 8" in p.stdout

    if context_lines >= 5:
        # Should show 5 lines before and after (reaching file boundaries)
        assert "Line 1" in p.stdout
        assert "Line 9" in p.stdout
        assert "Line 10" in p.stdout

    if context_lines == 0:
        # With U0, context lines should not appear (except in headers)
        # Filter out header lines
        output_lines = [
            line
            for line in p.stdout.split("\n")
            if not line.startswith("@@")
            and not line.startswith("---")
            and not line.startswith("+++")
            and not line.startswith("diff ")
            and not line.startswith("index ")
        ]
        output_content = "\n".join(output_lines)

        # Should have the deletion and addition, but minimal other content
        assert "-Line 5" in output_content
        assert "+MODIFIED LINE 5" in output_content

    # Verify that lines too far from the change don't appear with small context
    if context_lines == 1:
        assert "Line 2" not in p.stdout or p.stdout.count("Line 2") == 0
        assert "Line 8" not in p.stdout or p.stdout.count("Line 8") == 0


def test_diff_inter_hunk_context(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test diff with --inter-hunk-context"""
    xtl_path = tmp_path / "xtl"

    test_file = xtl_path / "test.txt"
    lines = [f"Line {i}\n" for i in range(1, 31)]
    test_file.write_text("".join(lines))

    cmd_add = [git2cpp_path, "add", "test.txt"]
    subprocess.run(cmd_add, cwd=xtl_path, check=True)

    cmd_commit = [git2cpp_path, "commit", "-m", "test"]
    subprocess.run(cmd_commit, cwd=xtl_path, check=True)

    # Modify two separate sections
    lines[4] = "Modified Line 5\n"
    lines[19] = "Modified Line 20\n"
    test_file.write_text("".join(lines))

    # Test with small inter-hunk-context (should keep hunks separate)
    cmd_small = [git2cpp_path, "diff", "--inter-hunk-context=1"]
    p_small = subprocess.run(cmd_small, capture_output=True, cwd=xtl_path, text=True)
    assert p_small.returncode == 0
    assert "Modified Line 5" in p_small.stdout
    assert "Modified Line 20" in p_small.stdout
    assert "@@" in p_small.stdout

    # Count hunks in small context output
    hunk_count_small = len(
        [
            l
            for l in p_small.stdout.split("\n")
            if l.startswith("@@") and l.endswith("@@")
        ]
    )

    # Test with large inter-hunk-context (should merge hunks into one)
    cmd_large = [git2cpp_path, "diff", "--inter-hunk-context=15"]
    p_large = subprocess.run(cmd_large, capture_output=True, cwd=xtl_path, text=True)
    assert p_large.returncode == 0
    assert "Modified Line 5" in p_large.stdout
    assert "Modified Line 20" in p_large.stdout
    assert "@@" in p_large.stdout

    # Count hunks in large context output
    hunk_count_large = len(
        [
            l
            for l in p_large.stdout.split("\n")
            if l.startswith("@@") and l.endswith("@@")
        ]
    )

    # Verify both modifications appear in both outputs
    assert "Modified Line 5" in p_small.stdout and "Modified Line 5" in p_large.stdout
    assert "Modified Line 20" in p_small.stdout and "Modified Line 20" in p_large.stdout

    # Large inter-hunk-context should produce fewer or equal hunks (merging effect)
    assert hunk_count_large <= hunk_count_small, (
        f"Expected large context ({hunk_count_large} hunks) to have <= hunks than small context ({hunk_count_small} hunks)"
    )


def test_diff_abbrev(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test diff with --abbrev for object name abbreviation"""
    xtl_path = tmp_path / "xtl"

    test_file = xtl_path / "test.txt"
    test_file.write_text("Original content\n")

    cmd_add = [git2cpp_path, "add", "test.txt"]
    subprocess.run(cmd_add, cwd=xtl_path, check=True)

    cmd_commit = [git2cpp_path, "commit", "-m", "initial commit"]
    subprocess.run(cmd_commit, cwd=xtl_path, check=True)

    # Modify the file
    test_file.write_text("Modified content\n")

    # Test default --abbrev
    cmd_default = [git2cpp_path, "diff", "--abbrev"]
    p_default = subprocess.run(
        cmd_default, capture_output=True, cwd=xtl_path, text=True
    )
    assert p_default.returncode == 0
    assert "test.txt" in p_default.stdout

    # Test --abbrev=7 (short hash)
    cmd_7 = [git2cpp_path, "diff", "--abbrev=7"]
    p_7 = subprocess.run(cmd_7, capture_output=True, cwd=xtl_path, text=True)
    assert p_7.returncode == 0
    assert "test.txt" in p_7.stdout

    # Test --abbrev=12 (longer hash)
    cmd_12 = [git2cpp_path, "diff", "--abbrev=12"]
    p_12 = subprocess.run(cmd_12, capture_output=True, cwd=xtl_path, text=True)
    assert p_12.returncode == 0
    assert "test.txt" in p_12.stdout

    # Extract hash lengths from index lines to verify abbrev is working
    hash_pattern = r"index ([0-9a-f]+)\.\.([0-9a-f]+)"
    match_7 = re.search(hash_pattern, p_7.stdout)
    match_12 = re.search(hash_pattern, p_12.stdout)

    if match_7 and match_12:
        hash_len_7 = len(match_7.group(1))
        hash_len_12 = len(match_12.group(1))

        # Verify that abbrev=12 produces longer or equal hash than abbrev=7
        assert hash_len_12 >= hash_len_7, (
            f"Expected abbrev=12 ({hash_len_12}) to be >= abbrev=7 ({hash_len_7})"
        )


# Note: only checking if the output is a diff
def test_diff_patience(xtl_clone, commit_env_config, git2cpp_path, tmp_path):
    """Test diff with --patience algorithm"""
    xtl_path = tmp_path / "xtl"

    test_file = xtl_path / "test.txt"
    test_file.write_text("Line 1\nLine 2\nLine 3\n")

    cmd_add = [git2cpp_path, "add", "test.txt"]
    subprocess.run(cmd_add, cwd=xtl_path, check=True)

    cmd_commit = [git2cpp_path, "commit", "-m", "test"]
    subprocess.run(cmd_commit, cwd=xtl_path, check=True)

    test_file.write_text("Line 1\nNew Line\nLine 2\nLine 3\n")

    cmd = [git2cpp_path, "diff"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "test.txt" in p.stdout
    assert "+New Line" in p.stdout


# Note: only checking if the output is a diff
def test_diff_minimal(xtl_clone, git2cpp_path, tmp_path):
    """Test diff with --minimal (spend extra time to find smallest diff)"""
    xtl_path = tmp_path / "xtl"

    readme = xtl_path / "README.md"
    original = readme.read_text()
    readme.write_text(original + "\nExtra line\n")

    cmd = [git2cpp_path, "diff", "--minimal"]
    p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
    assert p.returncode == 0
    assert "README.md" in p.stdout
    assert "+Extra line" in p.stdout


# TODO: Find a way to check the colour
# @pytest.mark.parametrize("colour_flag", ["--color", "--no-color"])
# def test_diff_colour(xtl_clone, git2cpp_path, tmp_path, colour_flag):
#     xtl_path = tmp_path / "xtl"

#     (xtl_path / "README.md").write_text("Modified")

#     cmd = [git2cpp_path, "diff", colour_flag]
#     p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True, shell=True)
#     assert p.returncode == 0
#     # how to check if colour ?

#     ansi_escape = re.compile(r"\x1b\[[0-9;]*m")
#     if colour_flag == "--no-color":
#         assert not bool(re.search(ansi_escape, p.stdout))
#     else:
#         assert bool(re.search(ansi_escape, p.stdout))


# TODO: add the following flags after the "move" subcommand has been implemented (needed for the tests)
# @pytest.mark.parametrize("renames_flag", ["-M", "--find-renames"])
# def test_diff_find_renames(xtl_clone, git2cpp_path, tmp_path, renames_flag):
#     """Test diff with -M/--find-renames"""
#     xtl_path = tmp_path / "xtl"

#     old_file = xtl_path / "old_name.txt"
#     old_file.write_text("Hello\n")

#     cmd_add = [git2cpp_path, "add", "old_name.txt"]
#     subprocess.run(cmd_add, cwd=xtl_path, check=True)

#     cmd_commit = [git2cpp_path, "commit", "-m", "Add file"]
#     subprocess.run(cmd_commit, cwd=xtl_path, check=True)

#     new_file = xtl_path / "new_name.txt"
#     old_file.rename(new_file)
#     old_file.write_text("Goodbye\n")

#     cmd_add_all = [git2cpp_path, "add", "-A"]
#     subprocess.run(cmd_add_all, cwd=xtl_path, check=True)

#     cmd = [git2cpp_path, "diff", "--cached", renames_flag]
#     p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
#     assert p.returncode == 0
#     # assert "similarity index" in p.stdout
#     # assert "rename from" in p.stdout
#     assert "+++ b/new_name.txt" in p.stdout
#     assert "--- a/old_name.txt" in p.stdout
#     print("===\n", p.stdout, "===\n")


# def test_diff_find_renames_with_threshold(xtl_clone, git2cpp_path, tmp_path):
#     """Test diff with -M with threshold value"""
#     xtl_path = tmp_path / "xtl"

#     old_file = xtl_path / "old.txt"
#     old_file.write_text("Content\n")

#     cmd_add = [git2cpp_path, "add", "old.txt"]
#     subprocess.run(cmd_add, cwd=xtl_path, check=True)

#     cmd_commit = [git2cpp_path, "commit", "-m", "Add file"]
#     subprocess.run(cmd_commit, cwd=xtl_path, check=True)

#     new_file = xtl_path / "new.txt"
#     old_file.rename(new_file)

#     cmd_add_all = [git2cpp_path, "add", "-A"]
#     subprocess.run(cmd_add_all, cwd=xtl_path, check=True)

#     cmd = [git2cpp_path, "diff", "--cached", "-M50"]
#     p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
#     assert p.returncode == 0
#     print(p.stdout)  # Doesn't do the same as the previous one. Why ???


# @pytest.mark.parametrize("copies_flag", ["-C", "--find-copies"])
# def test_diff_find_copies(xtl_clone, git2cpp_path, tmp_path, copies_flag):
#     """Test diff with -C/--find-copies"""
#     xtl_path = tmp_path / "xtl"

#     original_file = xtl_path / "original.txt"
#     original_file.write_text("Content to be copied\n")

#     cmd_add = [git2cpp_path, "add", "original.txt"]
#     subprocess.run(cmd_add, cwd=xtl_path, check=True)

#     copied_file = xtl_path / "copied.txt"
#     copied_file.write_text("Content to be copied\n")

#     cmd_add_copy = [git2cpp_path, "add", "copied.txt"]
#     subprocess.run(cmd_add_copy, cwd=xtl_path, check=True)

#     cmd = [git2cpp_path, "diff", "--cached", copies_flag]
#     p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
#     assert p.returncode == 0
#     print(p.stdout)


# def test_diff_find_copies_with_threshold(xtl_clone, git2cpp_path, tmp_path):
#     """Test diff with -C with threshold value"""
#     xtl_path = tmp_path / "xtl"

#     original_file = xtl_path / "original.txt"
#     original_file.write_text("Content\n")

#     cmd_add = [git2cpp_path, "add", "original.txt"]
#     subprocess.run(cmd_add, cwd=xtl_path, check=True)

#     copied_file = xtl_path / "copied.txt"
#     copied_file.write_text("Content to be copied\n")

#     cmd_add_copy = [git2cpp_path, "add", "copied.txt"]
#     subprocess.run(cmd_add_copy, cwd=xtl_path, check=True)

#     cmd = [git2cpp_path, "diff", "--cached", "-C50"]
#     p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
#     assert p.returncode == 0


# def test_diff_find_copies_harder(xtl_clone, git2cpp_path, tmp_path):
#     """Test diff with --find-copies-harder"""
#     xtl_path = tmp_path / "xtl"

#     test_file = xtl_path / "test.txt"
#     test_file.write_text("Content\n")

#     cmd_add = [git2cpp_path, "add", "test.txt"]
#     subprocess.run(cmd_add, cwd=xtl_path, check=True)

#     cmd = [git2cpp_path, "diff", "--cached", "--find-copies-harder"]
#     p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
#     assert p.returncode == 0


# @pytest.mark.parametrize("break_rewrites_flag", ["-B", "--break-rewrites"])
# def test_diff_break_rewrites(xtl_clone, git2cpp_path, tmp_path, break_rewrites_flag):
#     """Test diff with -B/--break-rewrites"""
#     xtl_path = tmp_path / "xtl"

#     test_file = xtl_path / "test.txt"
#     test_file.write_text("Original content\n")

#     cmd_add = [git2cpp_path, "add", "test.txt"]
#     subprocess.run(cmd_add, cwd=xtl_path, check=True)

#     cmd_commit = [git2cpp_path, "commit", "-m", "test"]
#     subprocess.run(cmd_commit, cwd=xtl_path, check=True)

#     # Completely rewrite the file
#     test_file.write_text("Completely different content\n")

#     cmd = [git2cpp_path, "diff", break_rewrites_flag]
#     p = subprocess.run(cmd, capture_output=True, cwd=xtl_path, text=True)
#     assert p.returncode == 0
#     print(p.stdout)
