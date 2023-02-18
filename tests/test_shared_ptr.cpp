#include "cyclic_shared.hpp"

#include <boost/test/unit_test.hpp>

namespace {

class counter
{
public:
    counter(int* count_ptr) :
        count_ptr_(count_ptr)
    {}

    ~counter()
    {
        ++*count_ptr_;
    }

private:
    int* count_ptr_;
};

} // namespace

namespace cyclic {

template<>
struct default_trace<counter>
{
    void operator()(const counter*, const visitor&) const
    {}
};

} // namespace cyclic

BOOST_AUTO_TEST_SUITE(shared_ptr)

BOOST_AUTO_TEST_CASE(default_ctor)
{
    cyclic::shared_ptr<counter> ptr;
    BOOST_TEST(ptr.get() == nullptr);
}

BOOST_AUTO_TEST_CASE(raw_ctor)
{
    int count(0);
    {
        counter* raw(new counter(&count));
        cyclic::shared_ptr<counter> ptr(raw);
        BOOST_TEST(ptr.get() == raw);
    }
    BOOST_TEST(count == 1);
}

BOOST_AUTO_TEST_CASE(ctor_copy_null)
{
    cyclic::shared_ptr<counter> ptr;
    BOOST_TEST(ptr.get() == nullptr);
    cyclic::shared_ptr<counter> copy(ptr);
    BOOST_TEST(copy.get() == nullptr);
}

BOOST_AUTO_TEST_CASE(ctor_copy)
{
    int count(0);
    {
        counter* raw(new counter(&count));
        cyclic::shared_ptr<counter> ptr(raw);
        {
            cyclic::shared_ptr<counter> copy(ptr);
            BOOST_TEST(copy.get() == ptr.get());
        }
        BOOST_TEST(count == 0);
    }
    BOOST_TEST(count == 1);
}

BOOST_AUTO_TEST_SUITE_END()
