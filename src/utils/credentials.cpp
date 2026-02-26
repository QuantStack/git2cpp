#include <git2/credential.h>
#include <iostream>

#include "credentials.hpp"
#include "input_output.hpp"

// git_credential_acquire_cb
int user_credentials(
    git_credential** out,
	const char* url,
	const char* username_from_url,
	unsigned int allowed_types,
	void* payload)
{
    // Check for cached credentials here, if desired.
    // It might be necessary to make this function stateful to avoid repeating unnecessary checks.

    *out = nullptr;

    if (allowed_types & GIT_CREDENTIAL_USERPASS_PLAINTEXT) {
        std::string username = username_from_url ? username_from_url : "";
        if (username.empty()) {
            username = prompt_input("Username: ");
        }
        if (username.empty()) {
            giterr_set_str(GIT_ERROR_HTTP, "No username specified");
            return GIT_EAUTH;
        }

        std::string password = prompt_input("Password: ", false);
        if (password.empty()) {
            giterr_set_str(GIT_ERROR_HTTP, "No password specified");
            return GIT_EAUTH;
        }

        // If successful, this will create and return a git_credential* in the out argument.
        return git_credential_userpass_plaintext_new(out, username.c_str(), password.c_str());
    }

    giterr_set_str(GIT_ERROR_HTTP, "Unexpected credentials request");
    return GIT_ERROR;
}
