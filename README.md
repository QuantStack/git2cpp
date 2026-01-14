# git2cpp

This is a C++ wrapper of [libgit2](https://libgit2.org/) to provide a command-line interface (CLI)
to `git` functionality. The intended use is in WebAssembly in-browser terminals (see
[cockle](https://github.com/jupyterlite/cockle) and
[JupyterLite terminal](https://github.com/jupyterlite/terminal) projects) but it can be compiled and
used on any POSIX-compliant system.

See `overview.md` for further details.

Developer's workflow using `micromamba` to manage the dependencies:

```bash
micromamba create -f dev-environment.yml
micromamba activate git2cpp-dev
cmake -Bbuild -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX
cd build
make -j8
```

The `git2cpp` executable can then be run, e.g. `./git2cpp -v`.

The CLI is tested using `python`. From the top-level directory:

```bash
pytest -v
```

# WebAssembly build and deployment

The `wasm` directory contains everything needed to build the local `git2cpp` source code as an
WebAssembly [Emscripten-forge](https://emscripten-forge.org/) package, create local
[cockle](https://github.com/jupyterlite/cockle) and
[JupyterLite terminal](https://github.com/jupyterlite/terminal) deployments that run in a browser,
and test the WebAssembly build.

See the `README.md` in the `wasm` directory for further details.

The latest `cockle` and JupyterLite `terminal` deployments using `git2cpp` are available at
[https://quantstack.net/git2cpp](https://quantstack.net/git2cpp)
