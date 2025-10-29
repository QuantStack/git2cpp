#pragma once

#include <utility>
#include <vector>

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

template <class T>
class list_wrapper : public wrapper_base<typename T::resource_type*>
{
public:

    using base_type = wrapper_base<typename T::resource_type*>;

    explicit list_wrapper(std::vector<T> list)
        : m_list(std::move(list))
    {
        this->p_resource = new base_type::resource_type[m_list.size()];
        for (size_t i=0; i< m_list.size(); ++i)
        {
            this->p_resource[i] = m_list[i];
        }
    }

    ~list_wrapper()
    {
        delete[] this->p_resource;
        this->p_resource = nullptr;
    }

    list_wrapper(list_wrapper&&) noexcept = default;
    list_wrapper& operator=(list_wrapper&&) noexcept = default;

    size_t size() const
    {
        return m_list.size();
    }

    T front()
    {
        // TODO: rework wrapper so they can have references
        // on libgit2 object without taking ownership
        return T(std::move(m_list.front()));
    }

private:

    std::vector<T> m_list;
};
