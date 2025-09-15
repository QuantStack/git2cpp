# Cockle deployment

This directory contains everything needed to build the local `git2cpp` source code as an
[Emscripten-forge](https://emscripten-forge.org/) package and create a local `cockle` deployment
that runs in a browser.

It works on Linux and macOS but not Windows.

It uses a separate `micromamba` environment defined in `deploy-environment.yml`. To set this up use:
```bash
micromamba create -f deploy-environment.yml
micromamba activate git2cpp-deploy
```

Then to build `git2cpp` and create the `cockle` deployment use:
```bash
make
```

This performs the following steps:

  1. Clones the `emscripten-forge/recipes` repository.
  2. Modifies the `git2cpp` recipe (using `modify_recipe.py`) to build from the local `git2cpp` source code in `../src/`.
  3. Builds the package from the recipe using `pixi`.
  4. Builds the `cockle` deployment in the `dist/` directory using the locally-built package.

The built package will be in the `em-forge-recipes/output/emscripten-wasm32` directory with a name
something like `git2cpp-0.0.3-h2072262_3.tar.bz2`. If you compare this with the latest
Emscripten-forge package on `https://prefix.dev/channels/emscripten-forge-dev/packages/git2cpp`,
the local package should have the same version number and the build number should be one higher.

To serve the `cockle` deployment use:
```bash
make serve
```

and open a web browser at the URL http://localhost:4500/. Run the `cockle-config` command (typing
`co`, then the tab key then enter should suffice). Amongst the displayed information should be the
`git2cpp` package showing that it is from a local directory such as
`file:///something-or-other/git2cpp/cockle-deploy/em-forge-recipes/output` rather than from
`prefix.dev` such as `https://repo.prefix.dev/emscripten-forge-dev`.

After making changes to `git2cpp` source code, to rebuild the package and deployment use:
```bash
make rebuild
```
and then re-serve using:
```bash
make serve
```
