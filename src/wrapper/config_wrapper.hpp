#pragma once

#include <string>

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class config_wrapper : public wrapper_base<git_config>
{
public:

    using base_type = wrapper_base<git_config>;

    ~config_wrapper();

    config_wrapper(config_wrapper&&) noexcept = default;
    config_wrapper& operator=(config_wrapper&&) noexcept = default;

    git_config_entry* get_entry(std::string name);
    void set_entry(std::string name, std::string value);
    void delete_entry(std::string name);

private:

    config_wrapper(git_config* cfg);

    friend class repository_wrapper;
};
