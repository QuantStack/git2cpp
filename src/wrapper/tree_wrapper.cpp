#include "../wrapper/tree_wrapper.hpp"

tree_wrapper::tree_wrapper(git_tree* tree)
    : base_type(tree)
{
}

tree_wrapper::~tree_wrapper()
{
    git_tree_free(p_resource);
    p_resource = nullptr;
}
