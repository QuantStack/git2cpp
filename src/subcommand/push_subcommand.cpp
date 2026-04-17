#include "../subcommand/push_subcommand.hpp"

#include <iostream>
#include <unordered_map>
#include <utility>

#include <git2/remote.h>

#include "../utils/ansi_code.hpp"
#include "../utils/common.hpp"
#include "../utils/credentials.hpp"
#include "../utils/progress.hpp"
#include "../wasm/scope.hpp"

push_subcommand::push_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("push", "Update remote refs along with associated objects");

    sub->add_option("<remote>", m_remote_name, "The remote to push to")->default_val("origin");
    sub->add_option("<refspec>", m_refspecs, "The refspec(s) to push")->expected(0, -1);
    sub->add_flag(
        "--all,--branches",
        m_branches_flag,
        "Push all branches (i.e. refs under " + ansi_code::bold + "refs/heads/" + ansi_code::reset
            + "); cannot be used with other <refspec>."
    );

    sub->callback(
        [this]()
        {
            this->run();
        }
    );
}

void push_subcommand::fill_refspec(repository_wrapper& repo)
{
    const std::string prefix = std::string("refs/heads/");
    if (m_branches_flag)
    {
        auto iter = repo.iterate_branches(GIT_BRANCH_LOCAL);
        auto br = iter.next();
        while (br)
        {
            std::string refspec = std::string(br->name());
            if (refspec.starts_with(prefix))
            {
                refspec = refspec.substr(prefix.size());
            }
            m_refspecs.push_back(refspec);
            br = iter.next();
        }
    }
    else if (m_refspecs.empty())
    {
        std::string branch;
        try
        {
            auto head_ref = repo.head();
            branch = head_ref.short_name();
        }
        catch (...)
        {
            std::cerr << "Could not determine current branch to push." << std::endl;
            return;
        }
        m_refspecs.push_back(branch);
    }
    else
    {
        for (auto& refspec : m_refspecs)
        {
            if (refspec.starts_with(prefix))
            {
                refspec = refspec.substr(prefix.size());
            }
        }
    }
}

std::unordered_map<std::string, git_oid> get_remotes(repository_wrapper& repo, std::string remote_name)
{
    std::vector<std::string> repo_refs = repo.refs_list();
    std::unordered_map<std::string, git_oid> remotes_oids;
    const std::string prefix = std::string("refs/remotes/") + remote_name + "/";
    for (const auto& r : repo_refs)
    {
        if (r.size() > prefix.size() && r.compare(0, prefix.size(), prefix) == 0)
        {
            // r is like "refs/remotes/origin/main"
            std::string short_name = r.substr(prefix.size());  // "main" or "feature/x"

            git_oid oid = repo.ref_name_to_id(r);
            remotes_oids.emplace(short_name, oid);
        }
    }
    return remotes_oids;
}

std::unordered_map<std::string, git_oid> diff_branches(
    std::unordered_map<std::string, git_oid> remotes_before_push,
    std::unordered_map<std::string, git_oid> remotes_after_push
)
{
    std::unordered_map<std::string, git_oid> new_branches;
    for (const auto& br : remotes_after_push)
    {
        const std::string name = br.first;
        const git_oid& oid = br.second;
        if (remotes_before_push.find(name) == remotes_before_push.end())
        {
            new_branches.emplace(name, oid);
        }
    }
    return new_branches;
}

std::pair<std::vector<std::string>, std::vector<std::string>>
split_refspecs(std::vector<std::string> refspecs, std::unordered_map<std::string, git_oid> new_branches)
{
    std::vector<std::string> new_pushed_refspecs;
    std::vector<std::string> existing_refspecs;

    for (const auto refspec : refspecs)
    {
        if (new_branches.find(refspec) == new_branches.end())
        {
            existing_refspecs.push_back(refspec);
        }
        else
        {
            new_pushed_refspecs.push_back(refspec);
        }
    }

    return std::make_pair(new_pushed_refspecs, existing_refspecs);
}

std::pair<std::string, std::string>
get_branch_names(repository_wrapper& repo, std::string remote_name, std::string refspec)
{
    std::optional<std::string> upstream_opt = repo.branch_upstream_name(refspec);
    std::string remote_branch = refspec;
    if (upstream_opt.has_value())
    {
        const std::string up_name = upstream_opt.value();
        auto pos = up_name.find('/');
        if (pos != std::string::npos && pos + 1 < up_name.size())
        {
            std::string up_remote = up_name.substr(0, pos);
            std::string up_branch = up_name.substr(pos + 1);
            if (up_remote == remote_name)
            {
                remote_branch = up_branch;
            }
        }
    }
    return std::make_pair(refspec, remote_branch);
}

void push_subcommand::run()
{
    wasm_http_transport_scope transport;  // Enables wasm http(s) transport.

    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    std::string remote_name = m_remote_name.empty() ? "origin" : m_remote_name;
    auto remote = repo.find_remote(remote_name);

    git_push_options push_opts = GIT_PUSH_OPTIONS_INIT;
    push_opts.callbacks.credentials = user_credentials;
    push_opts.callbacks.push_transfer_progress = push_transfer_progress;
    push_opts.callbacks.push_update_reference = push_update_reference;

    fill_refspec(repo);
    std::vector<std::string> refspecs_push;
    for (auto refspec : m_refspecs)
    {
        refspecs_push.push_back("refs/heads/" + refspec);
    }
    git_strarray_wrapper refspecs_wrapper(refspecs_push);
    git_strarray* refspecs_ptr = refspecs_wrapper;

    auto remotes_before_push = get_remotes(repo, remote_name);

    remote.push(refspecs_ptr, &push_opts);

    auto remotes_after_push = get_remotes(repo, remote_name);

    auto new_branches = diff_branches(remotes_before_push, remotes_after_push);
    auto [new_pushed_refspecs, existing_refspecs] = split_refspecs(m_refspecs, new_branches);

    std::cout << "To " << remote.url() << std::endl;
    for (const auto& refspec : new_pushed_refspecs)
    {
        auto [local_short_name, remote_branch] = get_branch_names(repo, remote_name, refspec);

        std::cout << " * [new branch]      " << local_short_name << " -> " << remote_branch << std::endl;
    }

    for (const auto& refspec : existing_refspecs)
    {
        auto [local_short_name, remote_branch] = get_branch_names(repo, remote_name, refspec);

        git_oid remote_oid = remotes_before_push[refspec];

        git_oid local_oid;
        if (auto ref_opt = repo.find_reference_dwim(("refs/heads/" + local_short_name)))
        {
            const git_oid* target = ref_opt->target();
            local_oid = *target;
        }

        if (!git_oid_equal(&remote_oid, &local_oid))
        {
            std::string old_hex = oid_to_hex(remote_oid);
            std::string new_hex = oid_to_hex(local_oid);
            // TODO: check order of hex codes
            std::cout << "   " << old_hex.substr(0, 7) << ".." << new_hex.substr(0, 7) << "  "
                      << local_short_name << " -> " << remote_branch << std::endl;
        }
    }
}
