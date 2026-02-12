#include <git2.h>

#include "../subcommand/tag_subcommand.hpp"
#include "../wrapper/commit_wrapper.hpp"
#include "../wrapper/tag_wrapper.hpp"

tag_subcommand::tag_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("tag", "Create, list, delete or verify tags");

    sub->add_flag("-l,--list", m_list_flag, "List tags. With optional <pattern>.");
    sub->add_flag("-f,--force", m_force_flag, "Replace an existing tag with the given name (instead of failing)");
    sub->add_option("-d,--delete", m_delete, "Delete existing tags with the given names.");
    sub->add_option("-n", m_num_lines, "<num> specifies how many lines from the annotation, if any, are printed when using -l. Implies --list.");
    sub->add_option("-m,--message", m_message, "Tag message for annotated tags");
    sub->add_option("<tagname>", m_tag_name, "Tag name");
    sub->add_option("<commit>", m_target, "Target commit (defaults to HEAD)");

    sub->callback([this]() { this->run(); });
}

// Tag listing: Print individual message lines
void print_list_lines(const std::string& message, int num_lines)
{
    if (message.empty())
    {
        return;
    }

    auto lines = split_input_at_newlines(message);

    // header
    std::cout << lines[0];

    // other lines
    if (num_lines <= 1 || lines.size() <= 2)
    {
        std::cout << std::endl;
    }
    else
    {
        for (size_t i = 1; i < lines.size() ; i++)
        {
            if (i < num_lines)
            {
                std::cout << "\n\t\t" << lines[i];
            }
        }
    }
}

// Tag listing: Print an actual tag object
void print_tag(git_tag* tag, int num_lines)
{
	std::cout << std::left << std::setw(16) << git_tag_name(tag);

	if (num_lines)
	{
		std::string msg = git_tag_message(tag);
		if (!msg.empty())
		{
		    print_list_lines(msg, num_lines);
		}
		else
		{
			std::cout << std::endl;
		}
	}
	else
	{
		std::cout << std::endl;
	}
}

// Tag listing: Print a commit (target of a lightweight tag)
void print_commit(git_commit* commit, std::string name, int num_lines)
{
	std::cout << std::left << std::setw(16) << name;

	if (num_lines)
	{
		std::string msg = git_commit_message(commit);
		if (!msg.empty())
		{
		    print_list_lines(msg, num_lines);
		}
		else
		{
			std::cout <<std::endl;
		}
	}
	else
	{
		std::cout <<std::endl;
	}
}

// Tag listing: Lookup tags based on ref name and dispatch to print
void each_tag(repository_wrapper& repo, const std::string& name, int num_lines)
{
	auto obj = repo.revparse_single(name);

	if (obj.has_value())
	{
	    switch (git_object_type(obj.value()))
    	{
    		case GIT_OBJECT_TAG:
    			print_tag(obj.value(), num_lines);
    			break;
    		case GIT_OBJECT_COMMIT:
    			print_commit(obj.value(), name, num_lines);
    			break;
    		default:
    			std::cout << name << std::endl;
    	}
	}
	else
	{
	    std::cout << name << std::endl;
	}
}

void tag_subcommand::list_tags(repository_wrapper& repo)
{
    std::string pattern = m_tag_name.empty() ? "*" : m_tag_name;
    auto tag_names = repo.tag_list_match(pattern);

    for (const auto& tag_name: tag_names)
    {
        each_tag(repo, tag_name, m_num_lines);
    }
}

void tag_subcommand::delete_tag(repository_wrapper& repo)
{
    if (m_delete.empty())
    {
        throw git_exception("Name required for tag deletion.", git2cpp_error_code::GENERIC_ERROR);
    }

    auto obj = repo.revparse_single(m_delete);
    if (!obj.has_value())
    {
        throw git_exception("error: tag '" + m_delete + "' not found.", git2cpp_error_code::GENERIC_ERROR);
    }

    git_buf abbrev_oid = GIT_BUF_INIT;
    throw_if_error(git_object_short_id(&abbrev_oid, obj.value()));

    std::string oid_str(abbrev_oid.ptr);
    git_buf_dispose(&abbrev_oid);

    throw_if_error(git_tag_delete(repo, m_delete.c_str()));
    std::cout << "Deleted tag '" << m_delete << "' (was " << oid_str << ")" << std::endl;
}

std::optional<object_wrapper> tag_subcommand::get_target_obj(repository_wrapper& repo)
{
    if (m_tag_name.empty())
    {
        throw git_exception("Tag name required", git2cpp_error_code::GENERIC_ERROR);
    }

    std::string target = m_target.empty() ? "HEAD" : m_target;

    auto target_obj = repo.revparse_single(target);
    if (!target_obj.has_value())
    {
        throw git_exception("Unable to resolve target: " + target, git2cpp_error_code::GENERIC_ERROR);
    }

    return target_obj;
}

void tag_subcommand::handle_error(int error)
{
    if (error < 0)
    {
        if (error == GIT_EEXISTS)
        {
            throw git_exception("tag '" + m_tag_name + "' already exists", git2cpp_error_code::FILESYSTEM_ERROR);
        }
        throw git_exception("Unable to create annotated tag", error);
    }
}

void tag_subcommand::create_lightweight_tag(repository_wrapper& repo)
{
    auto target_obj = tag_subcommand::get_target_obj(repo);

    git_oid oid;
    size_t force = m_force_flag ? 1 : 0;
    int error = git_tag_create_lightweight(&oid, repo, m_tag_name.c_str(), target_obj.value(), force);

    handle_error(error);
}

void tag_subcommand::create_tag(repository_wrapper& repo)
{
    auto target_obj = tag_subcommand::get_target_obj(repo);

    auto tagger = signature_wrapper::get_default_signature_from_env(repo);

    git_oid oid;
    size_t force = m_force_flag ? 1 : 0;
    int error = git_tag_create(&oid, repo, m_tag_name.c_str(), target_obj.value(), tagger.first, m_message.c_str(), force);

    handle_error(error);
}

void tag_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    if (!m_delete.empty())
    {
        delete_tag(repo);
    }
    else if (m_list_flag || (m_tag_name.empty() && m_message.empty()))
    {
        list_tags(repo);
    }
    else if (!m_message.empty())
    {
        create_tag(repo);
    }
    else if (!m_tag_name.empty())
    {
        create_lightweight_tag(repo);
    }
    else
    {
        list_tags(repo);
    }

}
