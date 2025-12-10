#pragma once

#include <git2.h>

int sideband_progress(const char* str, int len, void*);
int fetch_progress(const git_indexer_progress* stats, void* payload);
void checkout_progress(const char* path, size_t cur, size_t tot, void* payload);
int update_refs(const char* refname, const git_oid* a, const git_oid* b, git_refspec*, void*);
int push_transfer_progress(unsigned int current, unsigned int total, size_t bytes, void*);
int push_update_reference(const char* refname, const char* status, void*);
