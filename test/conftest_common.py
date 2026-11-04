# Code shared by wasm testing in cockle (conftest_cockle.py) and JupyterLite (conftest_lite.py).
import pathlib
import re
import subprocess

import pytest


class MockPath(pathlib.Path):
    def __init__(self, path: str = ""):
        super().__init__(path)

    def exists(self) -> bool:
        p = subprocess.run(["stat", str(self)])
        return p.returncode == 0

    def is_dir(self) -> bool:
        p = subprocess.run(["stat", "-c", "%F", str(self)], capture_output=True, text=True)
        return p.returncode == 0 and p.stdout.strip() == "directory"

    def is_file(self) -> bool:
        p = subprocess.run(["stat", "-c", "%F", str(self)], capture_output=True, text=True)
        return p.returncode == 0 and p.stdout.strip() == "regular file"

    def iterdir(self):
        p = subprocess.run(
            ["ls", str(self), "-a", "-1"], capture_output=True, text=True, check=True
        )
        for f in filter(lambda f: f not in ["", ".", ".."], re.split(r"\r?\n", p.stdout)):
            yield MockPath(self / f)

    def mkdir(self, *, parents=False):
        args = [str(self)]
        if parents:
            args.append("-p")
        subprocess.run(["mkdir"] + args, capture_output=True, text=True, check=True)

    def read_bytes(self) -> bytes:
        raise RuntimeError("Not implemented")

    def read_text(self) -> str:
        p = subprocess.run(["cat", str(self)], capture_output=True, text=True, check=True)
        text = p.stdout
        text = text.removesuffix("\n")
        return text

    def write_bytes(self, data: bytes):
        # Convert binary data to a string where each element is backslash-escaped so that we can
        # write to file in cockle using `echo -e <backslash-escaped data>`.
        encoded_string = "".join(f"\\x{d:02x}" for d in data)
        cmd = ["echo", "-e", encoded_string, ">", str(self)]
        subprocess.run(cmd, capture_output=True, text=True, check=True)
        return len(data)

    def write_text(self, data: str):
        # Note that in general it is not valid to direct output of a subprocess.run call to a file,
        # but we get away with it here as the command arguments are passed straight through to
        # cockle without being checked.
        data = data.removesuffix("\n")
        cmd = ["echo", data, ">", str(self)]
        subprocess.run(cmd, capture_output=True, text=True, check=True)
        return len(data)

    def __truediv__(self, other):
        if isinstance(other, str):
            return MockPath(f"{self}/{other}")
        raise RuntimeError("MockPath.__truediv__ only supports strings")


@pytest.fixture(scope="function")
def tmp_path() -> MockPath:
    # Assumes only one tmp_path needed per test.
    path = MockPath("/drive/tmp0")
    subprocess.run(["mkdir", str(path)], check=True)
    assert path.exists()
    assert path.is_dir()
    return path


def maybe_wrap_arg(s: str | MockPath) -> str:
    # An argument containing whitespace (including newlines) needs to be wrapped in quotes if
    # it is not already, due to how the command is passed to the shell as a single string.
    # Could do better here.
    s = str(s)
    if re.search(r"\s", s) and not s.endswith("'"):
        return "'" + s + "'"
    return s


def os_chdir(dir: str):
    subprocess.run(["cd", str(dir)], capture_output=True, check=True, text=True)


def os_getcwd():
    return subprocess.run(["pwd"], capture_output=True, check=True, text=True).stdout.strip()


def os_remove(file: str):
    return subprocess.run(["rm", str(file)], capture_output=True, check=True, text=True)


def os_rename(src: str | MockPath, dst: str | MockPath):
    return subprocess.run(["mv", str(src), str(dst)], capture_output=True, check=True, text=True)


def os_symlink(src: str | MockPath, dst: str | MockPath):
    return subprocess.run(
        ["ln", "-s", str(src), str(dst)], capture_output=True, check=True, text=True
    )
