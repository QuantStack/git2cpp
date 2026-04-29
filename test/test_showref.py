import re
import subprocess


def test_showref_list(repo_init_with_commit, git2cpp_path, tmp_path):
    """`show-ref` lists the repository references (heads present after init+commit)."""
    cmd = [git2cpp_path, "show-ref"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    # repo_init_with_commit in conftest creates the branch "main"
    assert "refs/heads/main" in p.stdout


def test_showref_includes_tag(repo_init_with_commit, git2cpp_path, tmp_path):
    """A created tag appears in show-ref output as refs/tags/<name>."""
    # create a lightweight tag using the CLI under test
    subprocess.run([git2cpp_path, "tag", "v1.0"], cwd=tmp_path, check=True)

    p = subprocess.run([git2cpp_path, "show-ref"], capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    assert "refs/tags/v1.0" in p.stdout


def test_showref_line_format(repo_init_with_commit, git2cpp_path, tmp_path):
    """Each line of show-ref is: <40-hex-oid> <refname>."""
    p = subprocess.run([git2cpp_path, "show-ref"], capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode == 0
    print(p.stdout)

    hex_re = re.compile(r"^[0-9a-f]{40}$")
    for line in p.stdout.splitlines():
        line = line.strip()
        if not line:
            continue
        parts = line.split()
        # Expect at least two tokens: oid and refname
        assert len(parts) >= 2
        oid, refname = parts[0], parts[1]
        assert hex_re.match(oid), f"OID not a 40-char hex: {oid!r}"
        assert refname.startswith("refs/"), f"Refname does not start with refs/: {refname!r}"


def test_showref_nogit(git2cpp_path, tmp_path):
    """Running show-ref outside a repository returns an error and non-zero exit."""
    cmd = [git2cpp_path, "show-ref"]
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path, text=True)
    assert p.returncode != 0
    assert "error: could not find repository at" in p.stderr
