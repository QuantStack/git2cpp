#pragma once

#include <git2.h>

// Apply in-memory config overrides to a repository's config. These have a higher priority than
// the repository's config file but are never written to it. Only used in WebAssembly, where
// core.filemode=false is set as the filesystem does not always support executable file permissions.
void apply_config_overrides(git_repository* repo);
