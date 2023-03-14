#include "cyclic_shared.hpp"

#include <boost/test/unit_test.hpp>

namespace {

class counter
{
public:
    explicit counter(int* count_ptr) :
        count_(*count_ptr)
    {}

    ~counter()
    {
        ++count_;
    }

private:
    int& count_;
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
        BOOST_TEST(count == 0);
    }
    BOOST_TEST(count == 1);
}

BOOST_AUTO_TEST_CASE(ctor_copy_null)
{
    cyclic::shared_ptr<counter> ptr;
    BOOST_TEST(ptr.get() == nullptr);
    cyclic::shared_ptr<counter> copy(ptr);
    BOOST_TEST(ptr.get() == nullptr);
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
            BOOST_TEST(ptr.get() == raw);
            BOOST_TEST(copy.get() == raw);
            BOOST_TEST(count == 0);
        }
        BOOST_TEST(count == 0);
    }
    BOOST_TEST(count == 1);
}

BOOST_AUTO_TEST_CASE(ctor_move_null)
{
    cyclic::shared_ptr<counter> ptr;
    BOOST_TEST(ptr.get() == nullptr);
    cyclic::shared_ptr<counter> moved(std::move(ptr));
    BOOST_TEST(ptr.get() == nullptr);
    BOOST_TEST(moved.get() == nullptr);
}

BOOST_AUTO_TEST_CASE(ctor_move)
{
    int count(0);
    {
        counter* raw(new counter(&count));
        cyclic::shared_ptr<counter> ptr(raw);
        {
            cyclic::shared_ptr<counter> moved(std::move(ptr));
            BOOST_TEST(ptr.get() == nullptr);
            BOOST_TEST(moved.get() == raw);
            BOOST_TEST(count == 0);
        }
        BOOST_TEST(count == 1);
    }
    BOOST_TEST(count == 1);
}

BOOST_AUTO_TEST_SUITE_END()
