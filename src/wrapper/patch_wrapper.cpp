#include "../utils/git_exception.hpp"
#include "../wrapper/patch_wrapper.hpp"

patch_wrapper::patch_wrapper(git_patch* patch)
    : base_type(patch)
{
}

patch_wrapper::~patch_wrapper()
{
    git_patch_free(p_resource);
    p_resource = nullptr;
}

git_buf patch_wrapper::to_buf()
{
    git_buf buf = GIT_BUF_INIT;
    throw_if_error(git_patch_to_buf(&buf, *this));
    return buf;
}

patch_wrapper patch_wrapper::patch_from_files(const std::string& path1, const std::string& file1_str, const std::string& path2, const std::string& file2_str, git_diff_options* diffopts)
{
    git_patch* patch;
    throw_if_error(git_patch_from_buffers(&patch, file1_str.c_str(), file1_str.length(), path1.c_str(), file2_str.c_str(), file2_str.length(), path2.c_str(), diffopts));
    return patch_wrapper(patch);
}
