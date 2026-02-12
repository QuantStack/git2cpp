#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"
#include "../wrapper/repository_wrapper.hpp"

class tag_subcommand
{
public:

    explicit tag_subcommand(const libgit2_object&, CLI::App& app);

    void run();

private:

    void list_tags(repository_wrapper& repo);
    void delete_tag(repository_wrapper& repo);
    void create_lightweight_tag(repository_wrapper& repo);
    void create_tag(repository_wrapper& repo);
    std::optional<object_wrapper> get_target_obj(repository_wrapper& repo);
    void handle_error(int error);

    std::string m_delete;
    std::string m_message;
    std::string m_tag_name;
    std::string m_target;
    bool m_list_flag = false;
    bool m_force_flag = false;
    int m_num_lines = 0;
};
