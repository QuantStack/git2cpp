# Environment variables


## In all builds

The following four environment variables should be set for `git2cpp commit` and `git2cpp merge`
subcommands. The use of `git2cpp config` instead is partially supported and will be improved in
time.

`GIT_AUTHOR_EMAIL`
: The email for the "author" field.

`GIT_AUTHOR_NAME`
: The human-readable name in the "author" field.

`GIT_COMMITTER_EMAIL`
: The email for the "committer" field.

`GIT_COMMITTER_NAME`
: The human-readable name for the "committer" field.

`GIT_CREDENTIAL_CALLBACK`
: By default, `git2cpp` will prompt the user to enter a username and password if they are required
  for remote authentication, using a
  [libgit2 credential callback](https://libgit2.org/docs/reference/main/credential/git_credential_acquire_cb.html).
  To disable the callback use `export GIT_CREDENTIAL_CALLBACK=0`.


## In WebAssembly build only

(git_cors_proxy)=
`GIT_CORS_PROXY`
: In-browser remote `git2cpp` operations such as `clone`, `fetch` and `push` usually require use of
  a [CORS proxy server](cors_proxy). Use this environment variable to specify how the target URL is
  encoded into the CORS proxy URL, details of which depend on how the CORS proxy server is
  implemented.

  The `GIT_CORS_PROXY` should contain the URL of the CORS proxy itself, followed by a number of
  substitutions which are denoted by curly braces. To illustrate the substitutions, assume that the
  `git2cpp` command is for the repository at `https://github.com/organisation/repository`.

  Substitutions:

  - `{host}` is replaced by `github.com`
  - `{path}` is replaced by `/organisation/repository/` followed by extra information that depends
    on details of the `git2cpp` operation being performed
  - `{protocol}` is replaced by `https:`
  - `{url}` is equivalent to `{protocol}//{host}{path}`
  - `{api_key}` is replaced by the value of environment variable `GIT_CORS_PROXY_API_KEY` if it is
    set.

  If no substitutions are specified then `{url}` is appended.

  All of the substitutions except `{api_key}` have an `:encode` variant such as `{url:encode}`
  that passes the argument through the
  [encodeURIComponent](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURIComponent)
  JavaScript function, which some CORS proxies require.

  You can verify the actual URL used for requests in the Network tab of your browser's Developer
  Tools for debugging purposes.

  See [CORS proxy server](cors_proxy) for usage examples.

`GIT_CORS_PROXY_API_KEY`
: This value is used to replace the `{api_key}` in `GIT_CORS_PROXY` and is intended for use with a
  CORS proxy that requires an API key. Alternatively the API key could be put directly in the
  `GIT_CORS_PROXY` instead.

(git_http_timeout)=
`GIT_HTTP_TIMEOUT`
: In the WebAssembly build, all http(s) requests are limited by a timeout which has a default of 10
  seconds. To use a different timeout set the `GIT_HTTP_TIMEOUT` environment variable. For example,
  to set a timeout of 20 seconds use:

  ```bash
  export GIT_HTTP_TIMEOUT=20
  ```
