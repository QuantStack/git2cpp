#include "../wrapper/config_wrapper.hpp"
#include "../utils/git_exception.hpp"

config_wrapper::config_wrapper(git_config* cfg)
    : base_type(cfg)
{
}

config_wrapper::~config_wrapper()
{
    git_config_free(p_resource);
    p_resource=nullptr;
}

git_config_entry* config_wrapper::get_entry(std::string name)
{
    git_config_entry* entry;
    throw_if_error(git_config_get_entry(&entry, *this, name.c_str()));
    return entry;
}

void config_wrapper::set_entry(std::string name, std::string value)
{
    throw_if_error(git_config_set_string(*this, name.c_str(), value.c_str()));
}

void config_wrapper::delete_entry(std::string name)
{
     throw_if_error(git_config_delete_entry(*this, name.c_str()));
}
