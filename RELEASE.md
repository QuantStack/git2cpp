# Making a new release of git2cpp

This covers making a new github release in the `git2cpp` repository, and propagating that release through to Emscripten-forge so that the new WebAssembly package is available in the Jupyterlite terminal.

## Github release

1. Submit and merge a `git2cpp` PR bumping the version number in `version.hpp`.
2. Fetch the latest changes to your local `git2cpp` repo.
3. Tag the new release using something like `git tag -a 0.0.2 -m "Version 0.0.2"`. Ideally sign the release by adding the `-s` flag.
4. Push the new tag to the github repo using `git push upstream --tags`.
5. Create a github release for the tag. In https://github.com/QuantStack/git2cpp:

    - Click on `Releases`.
    - Click on `Draft a new release`.
    - Select the new tag in the `Choose a tag` dropdown.
    - Click on `Generate release notes`. This will fill out the release notes based on the PRs merged since the last release.
    - Edit the release notes if desired.
    - Click on `Publish release`.

## Update Emscripten-forge recipe

The Emscripten-forge recipe at https://github.com/emscripten-forge/recipes needs to be updated with the new version number and SHA checksum. An Emscripten-forge bot runs once a day and will identify the new github release and create a PR to update the recipe. Wait for this to happen, and if the tests pass and no further changes are required, the PR can be approved and merged.

After the PR is merged to `main`, the recipe will be rebuilt and uploaded to https://prefix.dev/channels/emscripten-forge-4x/packages/git2cpp, which should only take a few minutes.

Any subsequent `cockle` or JupyterLite `terminal` deployments that are rebuilt will download and use the latest `git2cpp` WebAssembly package.
