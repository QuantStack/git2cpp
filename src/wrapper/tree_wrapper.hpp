#pragma once

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class tree_wrapper : public wrapper_base<git_tree>
{
public:

    using base_type = wrapper_base<git_tree>;

    ~tree_wrapper();

    tree_wrapper(tree_wrapper&&) noexcept = default;
    tree_wrapper& operator=(tree_wrapper&&) noexcept = default;

private:

     tree_wrapper(git_tree* tree);

     friend class repository_wrapper;
};
