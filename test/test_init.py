from pathlib import Path
import subprocess


def test_init_in_directory(git2cpp_path, tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, 'init', '--bare', str(tmp_path)]
    p = subprocess.run(cmd, capture_output=True)
    assert p.returncode == 0
    assert p.stdout == b''
    assert p.stderr == b''

    assert sorted(map(lambda path: path.name, tmp_path.iterdir())) == [
        'HEAD', 'config', 'description', 'hooks', 'info', 'objects', 'refs'
    ]

    # TODO: check this is a valid git repo


def test_init_in_cwd(git2cpp_path, tmp_path, run_in_tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []
    assert Path.cwd() == tmp_path

    cmd = [git2cpp_path, 'init', '--bare']
    p = subprocess.run(cmd, capture_output=True)
    assert p.returncode == 0
    assert p.stdout == b''
    assert p.stderr == b''

    assert sorted(map(lambda path: path.name, tmp_path.iterdir())) == [
        'HEAD', 'config', 'description', 'hooks', 'info', 'objects', 'refs'
    ]

    # TODO: check this is a valid git repo


def test_init_not_bare(git2cpp_path, tmp_path):
    # tmp_path exists and is empty.
    assert list(tmp_path.iterdir()) == []

    cmd = [git2cpp_path, 'init', '.']
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path)
    assert p.returncode == 0
    assert p.stdout == b''
    assert p.stderr == b''

    # Directory contains just .git directory.
    assert sorted(map(lambda path: path.name, tmp_path.iterdir())) == ['.git']
    # .git directory is a valid repo.
    assert sorted(map(lambda path: path.name, (tmp_path / '.git').iterdir())) == [
        'HEAD', 'config', 'description', 'hooks', 'info', 'objects', 'refs'
    ]

    # Would like to use `git2cpp status` but it complains that 'refs/heads/master' not found
    cmd = [git2cpp_path, 'log']
    p = subprocess.run(cmd, capture_output=True, cwd=tmp_path)
    assert p.returncode == 0
    assert p.stdout == b''
    assert p.stderr == b''


def test_error_on_unknown_option(git2cpp_path):
    cmd = [git2cpp_path, 'init', '--unknown']
    p = subprocess.run(cmd, capture_output=True)
    assert p.returncode == 109
    assert p.stdout == b''
    assert p.stderr.startswith(b"The following argument was not expected: --unknown")


def test_error_on_repeated_directory(git2cpp_path):
    cmd = [git2cpp_path, 'init', 'abc', 'def']
    p = subprocess.run(cmd, capture_output=True)
    assert p.returncode == 109
    assert p.stdout == b''
    assert p.stderr.startswith(b"The following argument was not expected: def")
