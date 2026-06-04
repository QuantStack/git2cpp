# Overview

`git2cpp` is a C++ wrapper of [libgit2](https://libgit2.org/) to provide a command-line interface
(CLI) to `git` functionality. The intended use is in WebAssembly in-browser terminals (see the
[cockle](https://github.com/jupyterlite/cockle),
[JupyterLite terminal](https://github.com/jupyterlite/terminal) and
[Notebook.link](https://notebook.link) projects) but it can be compiled and
used on any POSIX-compliant system.

The Help pages here are generated from the `git2cpp` command and subcommands to show the
functionality that is currently supported. If there are features missing that you would like to use,
please create an issue in the [git2cpp github repository](https://github.com/QuantStack/git2cpp).

The Appendix contains additional information on [Environment variables](env_vars.md) used in
`git2cpp` and about the behaviour of the [WebAssembly build](wasm_build.md).

```{toctree}
:caption: Help pages
:hidden:
created/git2cpp
```

```{toctree}
:caption: Appendix
:hidden:
env_vars
wasm_build
```
