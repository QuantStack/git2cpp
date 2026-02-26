#include <git2.h>
#include <optional>
#include <termcolor/termcolor.hpp>

#include "../utils/common.hpp"
#include "../utils/git_exception.hpp"
#include "../subcommand/diff_subcommand.hpp"
#include "../wrapper/patch_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"

diff_subcommand::diff_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("diff", "Show changes between commits, commit and working tree, etc");

    sub->add_option("<files>", m_files, "tree-ish objects to compare");

    sub->add_flag("--stat", m_stat_flag, "Generate a diffstat");
    sub->add_flag("--shortstat", m_shortstat_flag, "Output only the last line of --stat");
    sub->add_flag("--numstat", m_numstat_flag, "Machine-friendly --stat");
    sub->add_flag("--summary", m_summary_flag, "Output a condensed summary");
    sub->add_flag("--name-only", m_name_only_flag, "Show only names of changed files");
    sub->add_flag("--name-status", m_name_status_flag, "Show names and status of changed files");
    sub->add_flag("--raw", m_raw_flag, "Generate the diff in raw format");

    sub->add_flag("--cached,--staged", m_cached_flag, "Compare staged changes to HEAD");
    sub->add_flag("--no-index", m_no_index_flag, "Compare two files on filesystem");

    sub->add_flag("-R", m_reverse_flag, "Swap two inputs");
    sub->add_flag("-a,--text", m_text_flag, "Treat all files as text");
    sub->add_flag("--ignore-space-at-eol", m_ignore_space_at_eol_flag, "Ignore changes in whitespace at EOL");
    sub->add_flag("-b,--ignore-space-change", m_ignore_space_change_flag, "Ignore changes in amount of whitespace");
    sub->add_flag("-w,--ignore-all-space", m_ignore_all_space_flag, "Ignore whitespace when comparing lines");
    sub->add_flag("--patience", m_patience_flag, "Generate diff using patience algorithm");
    sub->add_flag("--minimal", m_minimal_flag, "Spend extra time to find smallest diff");

    // TODO: add the following flags after the "move" subcommand has been implemented (needed for the tests)
    // sub->add_option("-M,--find-renames", m_rename_threshold, "Detect renames")
    //     ->expected(0,1)
    //     ->each([this](const std::string&) { m_find_renames_flag = true; });
    // sub->add_option("-C,--find-copies", m_copy_threshold, "Detect copies")
    //     ->expected(0,1)
    //     ->each([this](const std::string&) { m_find_copies_flag = true; });
    // sub->add_flag("--find-copies-harder", m_find_copies_harder_flag, "Detect copies from unmodified files");
    // sub->add_flag("-B,--break-rewrites", m_break_rewrites_flag, "Detect file rewrites");

    sub->add_option("-U,--unified", m_context_lines, "Lines of context");
    sub->add_option("--inter-hunk-context", m_interhunk_lines, "Context between hunks");
    sub->add_option("--abbrev", m_abbrev, "Abbreviation length for object names")
        ->expected(0,1);

    sub->add_flag("--color", m_colour_flag, "Show colored diff");
    sub->add_flag("--no-color", m_no_colour_flag, "Turn off colored diff");

    sub->callback([this]() { this->run(); });
}

void print_stats(const diff_wrapper& diff, bool use_colour, bool stat_flag, bool shortstat_flag, bool numstat_flag, bool summary_flag)
{
    git_diff_stats_format_t format;
    if (stat_flag)
    {
        if (shortstat_flag || numstat_flag || summary_flag)
        {
            throw git_exception("Only one of --stat, --shortstat, --numstat and --summary should be provided.", git2cpp_error_code::BAD_ARGUMENT);
        }
        else
        {
            format = GIT_DIFF_STATS_FULL;
        }
    }
    else if (shortstat_flag)
    {
        if (numstat_flag || summary_flag)
        {
            throw git_exception("Only one of --stat, --shortstat, --numstat and --summary should be provided.", git2cpp_error_code::BAD_ARGUMENT);
        }
        else
        {
            format = GIT_DIFF_STATS_SHORT;
        }
    }
    else if (numstat_flag)
    {
        if (summary_flag)
        {
            throw git_exception("Only one of --stat, --shortstat, --numstat and --summary should be provided.", git2cpp_error_code::BAD_ARGUMENT);
        }
        else
        {
            format = GIT_DIFF_STATS_NUMBER;
        }
    }
    else if (summary_flag)
    {
        format = GIT_DIFF_STATS_INCLUDE_SUMMARY;
    }

    auto stats = diff.get_stats();
    auto buf = stats.to_buf(format, 80);

    if (use_colour && stat_flag)
    {
        // Add colors to + and - characters
        std::string output(buf.ptr);
        bool in_parentheses = false;
        for (char c : output)
        {
            if (c == '(')
            {
                in_parentheses = true;
                std::cout << c;
            }
            else if (c == ')')
            {
                in_parentheses = false;
                std::cout << c;
            }
            else if (c == '+' && !in_parentheses)
            {
                std::cout << termcolor::green << '+' << termcolor::reset;
            }
            else if (c == '-' && !in_parentheses)
            {
                std::cout << termcolor::red << '-' << termcolor::reset;
            }
            else
            {
                std::cout << c;
            }
        }
    }
    else
    {
        std::cout << buf.ptr;
    }

    git_buf_dispose(&buf);
}

static int colour_printer([[maybe_unused]] const git_diff_delta* delta, [[maybe_unused]] const git_diff_hunk* hunk, const git_diff_line* line, void* payload)
{
	bool use_colour = *reinterpret_cast<bool*>(payload);

	// Only print origin for context/addition/deletion lines
    // For other line types, content already includes everything
    bool print_origin = (line->origin == GIT_DIFF_LINE_CONTEXT ||
                        line->origin == GIT_DIFF_LINE_ADDITION ||
                        line->origin == GIT_DIFF_LINE_DELETION);

	if (use_colour)
	{
		switch (line->origin) {
		case GIT_DIFF_LINE_ADDITION:  std::cout << termcolor::green; break;
		case GIT_DIFF_LINE_DELETION:  std::cout << termcolor::red; break;
		case GIT_DIFF_LINE_ADD_EOFNL: std::cout << termcolor::green; break;
		case GIT_DIFF_LINE_DEL_EOFNL: std::cout << termcolor::red; break;
		case GIT_DIFF_LINE_FILE_HDR:  std::cout << termcolor::bold; break;
		case GIT_DIFF_LINE_HUNK_HDR:  std::cout << termcolor::cyan; break;
		default: break;
		}
	}

	if (print_origin)
	{
	    std::cout << line->origin;
	}

	std::cout << std::string_view(line->content, line->content_len);

	if (use_colour)
	{
	    std::cout << termcolor::reset;
	}

	return 0;
}

void diff_subcommand::print_diff(diff_wrapper& diff, bool use_colour)
{
    if (m_stat_flag || m_shortstat_flag || m_numstat_flag || m_summary_flag)
    {
        print_stats(diff, use_colour, m_stat_flag, m_shortstat_flag, m_numstat_flag, m_summary_flag);
        return;
    }

    // TODO: add the following flags after the "move" subcommand has been implemented (needed for the tests)
    // if (m_find_renames_flag || m_find_copies_flag || m_find_copies_harder_flag || m_break_rewrites_flag)
    // {
    //     git_diff_find_options find_opts;
    //     git_diff_find_options_init(&find_opts, GIT_DIFF_FIND_OPTIONS_VERSION);

    //     if (m_find_renames_flag)
    //     {
    //         find_opts.flags |= GIT_DIFF_FIND_RENAMES;
    //         find_opts.rename_threshold = m_rename_threshold;
    //     }
    //     if (m_find_copies_flag)
    //     {
    //         find_opts.flags |= GIT_DIFF_FIND_COPIES;
    //         find_opts.copy_threshold = m_copy_threshold;
    //     }
    //     if (m_find_copies_harder_flag)
    //     {
    //         find_opts.flags |= GIT_DIFF_FIND_COPIES_FROM_UNMODIFIED;
    //     }
    //     if (m_break_rewrites_flag)
    //     {
    //         find_opts.flags |= GIT_DIFF_FIND_REWRITES;
    //     }

    //     diff.find_similar(&find_opts);
    // }

    git_diff_format_t format = GIT_DIFF_FORMAT_PATCH;
    if (m_name_only_flag)
    {
        format = GIT_DIFF_FORMAT_NAME_ONLY;
    }
    else if (m_name_status_flag)
    {
        format = GIT_DIFF_FORMAT_NAME_STATUS;
    }
    else if (m_raw_flag)
    {
        format = GIT_DIFF_FORMAT_RAW;
    }

    diff.print(format, colour_printer, &use_colour);
}

diff_wrapper compute_diff_no_index(std::vector<std::string> files, git_diff_options& diffopts) //std::pair<buf_wrapper, diff_wrapper>
{
	if (files.size() != 2)
    {
        throw git_exception("usage: git diff --no-index [<options>] <path> <path> [<pathspec>...]", git2cpp_error_code::BAD_ARGUMENT);
    }

    git_diff_options_init(&diffopts, GIT_DIFF_OPTIONS_VERSION);

    std::string file1_str = read_file(files[0]);
    std::string file2_str = read_file(files[1]);

    if (file1_str.empty())
    {
        throw git_exception("Cannot read file: " + files[0], git2cpp_error_code::GENERIC_ERROR);   //TODO: check error code with git
    }
    if (file2_str.empty())
    {
        throw git_exception("Cannot read file: " + files[1], git2cpp_error_code::GENERIC_ERROR);   //TODO: check error code with git
    }

    auto patch = patch_wrapper::patch_from_files(files[0], file1_str, files[1], file2_str, &diffopts);
	auto buf = patch.to_buf();
	auto diff = diff_wrapper::diff_from_buffer(buf);

	git_buf_dispose(&buf);

	return diff;
}

void diff_subcommand::run()
{
    git_diff_options diffopts;
    git_diff_options_init(&diffopts, GIT_DIFF_OPTIONS_VERSION);

    bool use_colour = false;
    if (m_no_colour_flag)
    {
        if (m_colour_flag)
        {
            throw git_exception("Only one of --color and --no-color should be provided.", git2cpp_error_code::BAD_ARGUMENT);
        }
        else
        {
            use_colour = false;
        }
    }
    else if (m_colour_flag)
    {
        use_colour = true;
    }

    if (m_no_index_flag)
    {
        auto diff = compute_diff_no_index(m_files, diffopts);
        diff_subcommand::print_diff(diff, use_colour);
    }
    else
    {
        auto directory = get_current_git_path();
        auto repo = repository_wrapper::open(directory);

        diffopts.context_lines = m_context_lines;
        diffopts.interhunk_lines = m_interhunk_lines;
        diffopts.id_abbrev = m_abbrev;

        if (m_reverse_flag) { diffopts.flags |= GIT_DIFF_REVERSE; }
        if (m_text_flag) { diffopts.flags |= GIT_DIFF_FORCE_TEXT; }
        if (m_ignore_space_at_eol_flag) { diffopts.flags |= GIT_DIFF_IGNORE_WHITESPACE_EOL; }
        if (m_ignore_space_change_flag) { diffopts.flags |= GIT_DIFF_IGNORE_WHITESPACE_CHANGE; }
        if (m_ignore_all_space_flag) { diffopts.flags |= GIT_DIFF_IGNORE_WHITESPACE; }
        if (m_untracked_flag) { diffopts.flags |= GIT_DIFF_INCLUDE_UNTRACKED; }
        if (m_patience_flag) { diffopts.flags |= GIT_DIFF_PATIENCE; }
        if (m_minimal_flag) { diffopts.flags |= GIT_DIFF_MINIMAL; }

        std::optional<tree_wrapper> tree1;
        std::optional<tree_wrapper> tree2;

        // TODO: throw error if m_files.size() > 2
        if (m_files.size() >= 1)
        {
            tree1 = repo.treeish_to_tree(m_files[0]);
        }
        if (m_files.size() ==2)
        {
            tree2 = repo.treeish_to_tree(m_files[1]);
        }

        auto diff = [&repo, &tree1, &tree2, &diffopts, this]()
        {
            if (tree1.has_value() && tree2.has_value())
            {
                return repo.diff_tree_to_tree(tree1.value(), tree2.value(), &diffopts);
            }
            else if (m_cached_flag)
            {
                if (m_cached_flag || !tree1)
                {
                    tree1 = repo.treeish_to_tree("HEAD");
                }
                return repo.diff_tree_to_index(tree1.value(), std::nullopt, &diffopts);
            }
            else if (tree1)
            {
                return repo.diff_tree_to_workdir_with_index(tree1.value(), &diffopts);
            }
            else
            {
                return repo.diff_index_to_workdir(std::nullopt, &diffopts);
            }
        }();

        diff_subcommand::print_diff(diff, use_colour);
    }
}
