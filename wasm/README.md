# Building and testing git2cpp in WebAssembly

This directory contains everything needed to build the local `git2cpp` source code as an
WebAssembly [Emscripten-forge](https://emscripten-forge.org/) package, create local
[cockle](https://github.com/jupyterlite/cockle) and
[JupyterLite terminal](https://github.com/jupyterlite/terminal) deployments that run in a browser,
and test the WebAssembly build.

It works on Linux and macOS but not Windows.

There are 6 sub-directories:

- `recipe`: build local `git2cpp` source code into an Emscripten-forge package.
- `cockle-deploy`: create a `cockle` deployment in the `serve` directory.
- `lite-deploy`: create a JupyterLite `terminal` deployment in the `serve` directory.
- `serve`: where the two deployments are served from.
- `test-cockle`: test the WebAssembly build in a `cockle` deployment.
- `test-lite`: test the WebAssembly build in a JupyterLite `terminal` deployment.

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

The available `cmake` options are:

- `USE_RECIPE_PATCHES`: Use patches from emscripten-forge recipe or not, default is `ON`
- `USE_COCKLE_RELEASE`: Use latest cockle release rather than repo main branch, default is `OFF`

For example, to run `cmake` but without using emscripten-forge recipe patches use:

```bash
cmake . -DUSE_RECIPE_PATCHES=OFF
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

If you want to remotely access git servers using `git2cpp` subcommands such as `clone`, `fetch`
and `push`, then serve with a local CORS proxy using:

```bash
make serve-with-cors
```

which will use the CORS proxy on `http://localhost:8881/`.

## Rebuild

After making changes to the local `git2cpp` source code you can rebuild the WebAssembly package,
both deployments and test code using from the `wasm` directory:

```bash
make rebuild
```

## Test

There are two separate test runs which both run the tests in the top-level `test` directory with
various monkey patching so that `git2cpp` commands are executed in the browser:

- `test-cockle` runs each command directly in a `cockle` shell.
- `test-lite` types each command into a JupyterLite terminal using the browser UI. This is slower
  than `test-cockle` but is closer to how users run `git2cpp`.

To run them use from the `wasm` directory:

```bash
make test-cockle
make test-lite
```

If there are problems running the tests then ensure you have the latest `playwright` browser installed:

```bash
playwright install chromium
```

To run all of the tests including those that require the `GIT2CPP_TEST_PRIVATE_TOKEN`:

```bash
GIT2CPP_TEST_PRIVATE_TOKEN=<this-is-the-personal-access-token> make test-cockle
GIT2CPP_TEST_PRIVATE_TOKEN=<this-is-the-personal-access-token> make test-lite
```

You can run a specific test from the top-level `test` directory (not the `wasm/test-cockle` or
`wasm/test-lite` directories) by setting `GIT2CPP_TEST_WASM` to either `cockle` or `lite`:

```bash
GIT2CPP_TEST_WASM=cockle pytest -v test_git.py::test_version
GIT2CPP_TEST_WASM=lite pytest -v test_git.py::test_version
```

### Watching the tests in a browser

The tests use [pytest-playwright](https://playwright.dev/python/docs/test-runners) so its command
line options can be used to see what is happening in the browser, for both `cockle` and `lite`:

- `--headed` shows the browser window, and `--slowmo <ms>` slows down each browser operation.
- `PWDEBUG=1` opens the Playwright Inspector to step through browser operations (use with `-s`).
- `--tracing on` (or `retain-on-failure`) records a trace which can be viewed using
  `playwright show-trace test-results/<test-name>/trace.zip`.
- `--video on` (or `retain-on-failure`) records a video of each test in `test-results`.

For example:

```bash
GIT2CPP_TEST_WASM=lite pytest -v --headed --slowmo 100 test_git.py::test_version
```

### Manually running the test servers

If wasm tests are failing it can be helpful to run the test servers and manually run `cockle`
commands to help understand the problem. For `cockle` use:

```bash
cd test-cockle
npm run serve
```

This will start both the test server on port 8000 and the CORS server on port 8881. Open a browser
at http://localhost:8000/ and to run a command such as `ls -l` open the dev console and enter the
following at the prompt: `await window.cockle.shellRun('ls -al')`. The generated output will appear
in a new `<div>` in the web page in a format similar to that returned by Python's
`subprocess.run()`.

For JupyterLite use:

```bash
cd test-lite
npm run serve
```

This will start both the JupyterLite server on port 8001 and the CORS server on port 8881. Open a
browser at http://localhost:8001/ and open a terminal to run commands.
