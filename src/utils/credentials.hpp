#pragma once

#include <git2/credential.h>

// Libgit2 callback of type git_credential_acquire_cb to obtain user credentials
// (username and password) to authenticate remote https access.
int user_credentials(
    git_credential** out,
	const char* url,
	const char* username_from_url,
	unsigned int allowed_types,
	void* payload
);
