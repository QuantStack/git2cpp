# Building and testing git2cpp in WebAssembly

This directory contains everything needed to build the local `git2cpp` source code as an
WebAssembly [Emscripten-forge](https://emscripten-forge.org/) package, create local
[cockle](https://github.com/jupyterlite/cockle) and
[JupyterLite terminal](https://github.com/jupyterlite/terminal) deployments that run in a browser,
and test the WebAssembly build.

It works on Linux and macOS but not Windows.

There are 5 sub-directories:

- `recipe`: build local `git2cpp` source code into an Emscripten-forge package.
- `cockle-deploy`: create a `cockle` deployment in the `serve` directory.
- `lite-deploy`: create a JupyterLite `terminal` deployment in the `serve` directory.
- `serve`: where the two deployments are served from.
- `test`: test the WebAssembly build.

## Build and deploy

The build, deploy and test process uses a separate `micromamba` environment defined in
`wasm-environment.yml`. To set this up use from within this directory:

```bash
micromamba create -f wasm-environment.yml
micromamba activate git2cpp-wasm
```

Then to build the WebAssembly package, both deployments and the testing resources use:

```bash
cmake .
make
```

The built emscripten-forge package will be file named something like `git2cpp-0.0.5-h7223423_1.tar.bz2`
in the directory `recipe/em-force-recipes/output/emscripten-wasm32`.

The local deployments in the `serve` directory can be manually checked using:

```bash
make serve
```

and open a web browser at http://localhost:8080/. Confirm that the local build of `git2cpp` is being
used in the two deployments by running `cockle-config package` at the command line, the output
should be something like:

<img alt="cockle-config output" src="cockle-config.png">

Note that the `source` for the `git2cpp` package is the local filesystem rather than from
`prefix.dev`. The version number of `git2cpp` in this table is not necessarily correct as it is the
version number of the current Emscripten-forge recipe rather than the version of the local `git2cpp`
source code which can be checked using `git2cpp -v` at the `cockle`/`terminal` command line.

## Test

To test the WebAssembly build use:

```bash
make test
```

This runs (some of) the tests in the top-level `test` directory with various monkey patching so that
`git2cpp` commands are executed in the browser. If there are problems running the tests then ensure
you have the latest `playwright` browser installed:


```bash
playwright install chromium
```

## Rebuild

After making changes to the local `git2cpp` source code you can rebuild the WebAssembly package,
both deployments and test code using:

```bash
make rebuild
```
