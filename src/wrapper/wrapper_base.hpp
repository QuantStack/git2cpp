#pragma once

#include <utility>

template <class T>
class wrapper_base
{
public:
    using resource_type = T;

    wrapper_base(const wrapper_base&) = delete;
    wrapper_base& operator=(const wrapper_base&) = delete;

    wrapper_base(wrapper_base&& rhs) noexcept
        : p_resource(rhs.p_resource)
    {
        rhs.p_resource = nullptr;
    }
    wrapper_base& operator=(wrapper_base&& rhs) noexcept
    {
        std::swap(p_resource, rhs.p_resource);
        return *this;
    }

    operator resource_type*() const noexcept
    {
        return p_resource;
    }

protected:
    // Allocation and deletion of p_resource must be handled by inheriting class.
    explicit wrapper_base(resource_type* resource = nullptr)
        : p_resource(resource)
    {
    }

    ~wrapper_base() = default;

    resource_type* p_resource = nullptr;
};
