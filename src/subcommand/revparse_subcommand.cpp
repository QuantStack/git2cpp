#include "revparse_subcommand.hpp"
#include "../utils/git_exception.hpp"
#include "../wrapper/repository_wrapper.hpp"

revparse_subcommand::revparse_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("rev-parse", "Pick out and message parameters");

    auto* bare_opt = sub->add_flag("--is-bare-repository", m_is_bare_repository_flag, "When the repository is bare print \"true\", otherwise \"false\".");
    auto* shallow_opt = sub->add_flag("--is-shallow-repository", m_is_shallow_repository_flag, "When the repository is shallow print \"true\", otherwise \"false\".");
    auto* rev_opt = sub->add_option("<rev>", m_revisions, "Revision(s) to parse (e.g. HEAD, main, HEAD~1, dae86e, ...)");

    sub->parse_complete_callback([this, sub, bare_opt, shallow_opt, rev_opt]() {
        for (CLI::Option* opt : sub->parse_order())
        {
            if (opt == bare_opt)
            {
                m_queries_in_order.push_back("is_bare");
            }
            else if (opt == shallow_opt)
            {
                m_queries_in_order.push_back("is_shallow");
            }
            else if (opt == rev_opt)
            {
                m_queries_in_order.push_back("is_rev");
            }
        }
    });

    sub->callback([this]() { this->run(); });
}

void revparse_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    size_t i = 0;
    if (!m_queries_in_order.empty())
    {
        for (const auto& q : m_queries_in_order)
        {
            if (q == "is_bare")
            {
                std::cout << std::boolalpha << repo.is_bare() << std::endl;
            }
            else if (q == "is_shallow")
            {
                std::cout << std::boolalpha << repo.is_shallow() << std::endl;
            }
            else if (q == "is_rev")
            {
                const auto& rev = m_revisions[i];
                auto obj = repo.revparse_single(rev.c_str());

                if (!obj.has_value())
                {
                    throw git_exception("bad revision '" + rev + "'", git2cpp_error_code::BAD_ARGUMENT);
                }

                auto oid = obj.value().oid();
                std::cout << git_oid_tostr_s(&oid) << std::endl;
                i += 1;
            }
        }
    }
    return;
}
