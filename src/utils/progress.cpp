#include <iomanip>
#include <iostream>
#include <string_view>

#include "../utils/progress.hpp"

int sideband_progress(const char* str, int len, void*)
{
    printf("remote: %.*s", len, str);
    fflush(stdout);
    return 0;
}

int fetch_progress(const git_indexer_progress* stats, void* payload)
{
    static bool done = false;

    // We need to copy stats into payload even if the fetch is done,
    // because the checkout_progress callback will be called with the
    // same payload and needs the data to be up do date.
    auto* pr = reinterpret_cast<git_indexer_progress*>(payload);
    *pr = *stats;

    if (done)
    {
        return 0;
    }

    int network_percent = pr->total_objects > 0 ?
        (100 * pr->received_objects / pr->total_objects)
        : 0;
    size_t kbytes = pr->received_bytes / 1024;
    size_t mbytes = kbytes / 1024;

    std::cout << "Receiving objects: " << std::setw(4) << network_percent
        << "% (" << pr->received_objects << "/" << pr->total_objects << "), ";
    if (mbytes != 0)
    {
        std::cout << mbytes << " MiB";
    }
    else if  (kbytes != 0)
    {
        std::cout << kbytes << " KiB";
    }
    else
    {
        std::cout << pr->received_bytes << " bytes";
    }
    // TODO: compute speed

    if (pr->received_objects == pr->total_objects)
    {
        std::cout << ", done." << std::endl;
        done = true;
    }
    else
    {
        std::cout << '\r';
    }
    return 0;
}

void checkout_progress(const char* path, size_t cur, size_t tot, void* payload)
{
    static bool done = false;
    if (done)
    {
        return;
    }
    auto* pr = reinterpret_cast<git_indexer_progress*>(payload);
    int deltas_percent = pr->total_deltas > 0 ?
        (100 * pr->indexed_deltas / pr->total_deltas)
        : 0;

    std::cout << "Resolving deltas: " << std::setw(4) << deltas_percent
        << "% (" << pr->indexed_deltas << "/" << pr->total_deltas << ")";
    if (pr->indexed_deltas == pr->total_deltas)
    {
        std::cout << ", done." << std::endl;
        done = true;
    }
    else
    {
        std::cout << '\r';
    }
}

int update_refs(const char* refname, const git_oid* a, const git_oid* b, git_refspec*, void*)
{
    char a_str[GIT_OID_SHA1_HEXSIZE+1], b_str[GIT_OID_SHA1_HEXSIZE+1];

    git_oid_fmt(b_str, b);
    b_str[GIT_OID_SHA1_HEXSIZE] = '\0';

    if (git_oid_is_zero(a))
    {
        std::string n, name, ref;
        const size_t last_slash_idx = std::string_view(refname).find_last_of('/');
        name = std::string_view(refname).substr(last_slash_idx + 1, -1);
        if (std::string_view(refname).find("remote") != std::string::npos)   // maybe will string_view need the size of the string
        {
            n = " * [new branch]      ";
            auto new_refname =  std::string_view(refname).substr(0, last_slash_idx - 1);
            const size_t second_to_last_slash_idx = std::string_view(new_refname).find_last_of('/');
            ref = std::string_view(refname).substr(second_to_last_slash_idx + 1, -1);
        }
        else if (std::string_view(refname).find("tags") != std::string::npos)
        {
            n = " * [new tag]         ";
            ref = name;
        }
        else
        {
            // could it be something else ?
        }
        std::cout << n << name << "\t-> " << ref << std::endl;
    }
    else
    {
        git_oid_fmt(a_str, a);
        a_str[GIT_OID_SHA1_HEXSIZE] = '\0';

        std::cout << "[updated] "
                  << std::string(a_str, 10)
                  << ".."
                  << std::string(b_str, 10)
                  << " " << refname << std::endl;
    }

    return 0;
}

int push_transfer_progress(unsigned int current, unsigned int total, size_t bytes, void*)
{
    if (total > 0)
    {
        int percent = (100 * current) / total;
        std::cout << "Writing objects: " << percent << "% (" << current
            << "/" << total << "), " << bytes << " bytes\r";
    }
    return 0;
}

int push_update_reference(const char* refname, const char* status, void*)
{
    if (status)
    {
        std::cout << "  " << refname << " " << status << std::endl;
    }
    else
    {
        std::cout << "  " << refname << std::endl;
    }
    return 0;
}
