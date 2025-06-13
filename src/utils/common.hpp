class noncopiable_nonmovable
{
public:
    noncopiable_nonmovable(const noncopiable_nonmovable&) = delete;
    noncopiable_nonmovable& operator=(const noncopiable_nonmovable&) = delete;
    noncopiable_nonmovable(noncopiable_nonmovable&) = delete;
    noncopiable_nonmovable& operator=(noncopiable_nonmovable&&) = delete;

protected:
    noncopiable_nonmovable() = default;
    ~noncopiable_nonmovable() = default;
};

class libgit2_object : private noncopiable_nonmovable
{
public:

    libgit2_object();
    ~libgit2_object();
};
