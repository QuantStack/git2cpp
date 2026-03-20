# git2cpp
[![GithubActions](https://github.com/QuantStack/git2cpp/actions/workflows/test.yml/badge.svg)](https://github.com/QuantStack/git2cpp/actions/workflows/test.yml)
[![Documentation Status](http://readthedocs.org/projects/git2cpp/badge/?version=latest)](https://git2cpp.readthedocs.io/en/latest/?badge=latest)
[![Codecov](https://codecov.io/gh/QuantStack/git2cpp/graph/badge.svg)](https://app.codecov.io/gh/QuantStack/git2cpp)

This is a C++ wrapper of [libgit2](https://libgit2.org/) to provide a command-line interface (CLI)
to `git` functionality. The intended use is in WebAssembly in-browser terminals (see
[cockle](https://github.com/jupyterlite/cockle) and
[JupyterLite terminal](https://github.com/jupyterlite/terminal) projects) but it can be compiled and
used on any POSIX-compliant system.

See `overview.md` for further details.

## Development workflow

### Build

Developer's workflow using `micromamba` to manage the dependencies:

```bash
micromamba create -f dev-environment.yml
micromamba activate git2cpp-dev
cmake -Bbuild -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX
cd build
make -j8
```

The `git2cpp` executable can then be run, e.g. `./git2cpp -v`.

### Test

The CLI is tested using `python`. From the top-level directory:

```bash
pytest -v
```

Some tests access the private repository at https://github.com/QuantStack/git2cpp-test-private using
a fine-grained github Personal Access Token (PAT). These tests are skipped by default. To run them
you will need to obtain the PAT from one of the maintainers, and run the tests as follows:

```bash
GIT2CPP_TEST_PRIVATE_TOKEN=<this-is-the-personal-access-token> pytest -v
```

### pre-commit

`pre-commit` runs automatically on `git commit`. To run it manually use:

```bash
pre-commit run --all-files
```

## WebAssembly build and deployment

The `wasm` directory contains everything needed to build the local `git2cpp` source code as an
WebAssembly [Emscripten-forge](https://emscripten-forge.org/) package, create local
[cockle](https://github.com/jupyterlite/cockle) and
[JupyterLite terminal](https://github.com/jupyterlite/terminal) deployments that run in a browser,
and test the WebAssembly build.

See the `README.md` in the `wasm` directory for further details.

The latest `cockle` and JupyterLite `terminal` deployments using `git2cpp` are available at
[https://quantstack.net/git2cpp](https://quantstack.net/git2cpp)

## Documentation

The project documentation is generated from the `git2cpp` help pages. To build the documentation
locally first build `git2cpp` as usual as described above, then install the documentation
dependencies:

```bash
micromamba install myst-parser sphinx sphinx-book-theme
```

and build the documentation:

```bash
cd docs
make html
```

The top-level documentation page will be `docs/_build/html/index.html`
