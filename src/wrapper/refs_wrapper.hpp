#pragma once

#include <concepts>
#include <string>

#include <git2.h>

#include "../utils/git_exception.hpp"
#include "../wrapper/wrapper_base.hpp"
#include "../wrapper/object_wrapper.hpp"

class reference_wrapper : public wrapper_base<git_reference>
{
public:

    using base_type = wrapper_base<git_reference>;

    ~reference_wrapper();

    reference_wrapper(reference_wrapper&&) noexcept = default;
    reference_wrapper& operator=(reference_wrapper&&) noexcept = default;

    std::string short_name() const;
    bool is_remote() const;
    const git_oid* target() const;
    reference_wrapper write_new_ref(const git_oid target_oid);

    template <class W>
    W peel() const;

private:

    reference_wrapper(git_reference* ref);

    friend class repository_wrapper;
};

class commit_wrapper;
class object_wrapper;

// TODO: add constraints on W
// For now it accepts commit_wrapper and object_wrapper only
template <class W>
W reference_wrapper::peel() const
{
    constexpr git_object_t obj_type = []
    {
        if constexpr (std::same_as<W, commit_wrapper>)
        {
            return GIT_OBJECT_COMMIT;
        }
        else // Default case
        {
            return GIT_OBJECT_ANY;
        }
    }();

    using resource_type = typename W::resource_type;
    git_object* resource = nullptr;
    throw_if_error(git_reference_peel(&resource, this->p_resource, obj_type));
    return W(reinterpret_cast<resource_type*>(resource));
}
