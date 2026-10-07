#include "config_overrides.hpp"

#ifdef EMSCRIPTEN
#    include <iterator>

#    include <git2/sys/config.h>

#    include "git_exception.hpp"
#endif

void apply_config_overrides([[maybe_unused]] git_repository* repo)
{
#ifdef EMSCRIPTEN
    // Config values to override, each in "name=value" form. core.filemode is disabled because
    // the emscripten filesystem does support executable file permissions but some JupyterLite
    // drive implementations do not.
    const char* values[] = {"core.filemode=false"};

    // Create an in-memory config backend holding these values. This backend is read-only, so
    // any config writes (e.g. `git config` or values set by clone/init) skip it and go to the
    // highest-priority writable backend instead, which is the repository's own config file.
    // The overrides are therefore never persisted to disk. backend_type is a label that
    // identifies where these entries came from when inspecting a config entry's origin.
    git_config_backend* backend = nullptr;
    git_config_backend_memory_options opts = GIT_CONFIG_BACKEND_MEMORY_OPTIONS_INIT;
    opts.backend_type = "git2cpp-overrides";
    throw_if_error(git_config_backend_from_values(&backend, values, std::size(values), &opts));

    // git_repository_config returns the repository's cached, shared config object rather than a
    // copy, so adding a backend to it affects all later config lookups through this repository.
    // GIT_CONFIG_LEVEL_APP is the highest priority level, above the local, global and system
    // config files, so these values win over any existing setting. The repo is passed so that
    // conditional includes can be evaluated, and force=0 means fail rather than replace if a
    // backend already exists at this level.
    git_config* cfg = nullptr;
    int error = git_repository_config(&cfg, repo);
    if (error == 0)
    {
        error = git_config_add_backend(cfg, backend, GIT_CONFIG_LEVEL_APP, repo, 0);
        // Only releases our reference; the repository still holds the config and its backends.
        git_config_free(cfg);
    }
    if (error < 0)
    {
        // On success ownership of the backend passes to the config, but on failure it is still
        // ours to free.
        backend->free(backend);
        throw_if_error(error);
    }
#endif
}
