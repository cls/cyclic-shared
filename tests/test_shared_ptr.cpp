#include "cyclic_shared.hpp"

#include <boost/test/unit_test.hpp>

namespace {

class dtor_counter
{
public:
    explicit dtor_counter(int* count_ptr) :
        count_(*count_ptr)
    {}

    ~dtor_counter()
    {
        ++count_;
    }

private:
    int& count_;
};

} // namespace

namespace cyclic {

template<>
struct default_trace<dtor_counter>
{
    void operator()(const dtor_counter*, const visitor&) const
    {}
};

} // namespace cyclic

BOOST_AUTO_TEST_SUITE(shared_ptr)

BOOST_AUTO_TEST_CASE(default_ctor)
{
    cyclic::shared_ptr<dtor_counter> ptr;
    BOOST_TEST(ptr.get() == nullptr);
    BOOST_TEST(ptr.use_count() == 0);
}

BOOST_AUTO_TEST_CASE(raw_ctor)
{
    int dtor_count(0);
    {
        dtor_counter* raw(new dtor_counter(&dtor_count));
        cyclic::shared_ptr<dtor_counter> ptr(raw);
        BOOST_TEST(ptr.get() == raw);
        BOOST_TEST(ptr.use_count() == 1);
        BOOST_TEST(dtor_count == 0);
    }
    BOOST_TEST(dtor_count == 1);
}

BOOST_AUTO_TEST_CASE(ctor_copy_null)
{
    cyclic::shared_ptr<dtor_counter> ptr;
    cyclic::shared_ptr<dtor_counter> copy(ptr);
    BOOST_TEST(ptr.get() == nullptr);
    BOOST_TEST(copy.get() == nullptr);
    BOOST_TEST(ptr.use_count() == 0);
    BOOST_TEST(copy.use_count() == 0);
}

BOOST_AUTO_TEST_CASE(ctor_copy)
{
    int dtor_count(0);
    {
        dtor_counter* raw(new dtor_counter(&dtor_count));
        cyclic::shared_ptr<dtor_counter> ptr(raw);
        {
            cyclic::shared_ptr<dtor_counter> copy(ptr);
            BOOST_TEST(ptr.get() == raw);
            BOOST_TEST(copy.get() == raw);
            BOOST_TEST(ptr.use_count() == 2);
            BOOST_TEST(copy.use_count() == 2);
            BOOST_TEST(dtor_count == 0);
        }
        BOOST_TEST(ptr.use_count() == 1);
        BOOST_TEST(dtor_count == 0);
    }
    BOOST_TEST(dtor_count == 1);
}

BOOST_AUTO_TEST_CASE(ctor_move_null)
{
    cyclic::shared_ptr<dtor_counter> ptr;
    cyclic::shared_ptr<dtor_counter> moved(std::move(ptr));
    BOOST_TEST(ptr.get() == nullptr);
    BOOST_TEST(moved.get() == nullptr);
    BOOST_TEST(ptr.use_count() == 0);
    BOOST_TEST(moved.use_count() == 0);
}

BOOST_AUTO_TEST_CASE(ctor_move)
{
    int dtor_count(0);
    {
        dtor_counter* raw(new dtor_counter(&dtor_count));
        cyclic::shared_ptr<dtor_counter> ptr(raw);
        {
            cyclic::shared_ptr<dtor_counter> moved(std::move(ptr));
            BOOST_TEST(ptr.get() == nullptr);
            BOOST_TEST(moved.get() == raw);
            BOOST_TEST(ptr.use_count() == 0);
            BOOST_TEST(moved.use_count() == 1);
            BOOST_TEST(dtor_count == 0);
        }
        BOOST_TEST(ptr.use_count() == 0);
        BOOST_TEST(dtor_count == 1);
    }
    BOOST_TEST(dtor_count == 1);
}

BOOST_AUTO_TEST_SUITE_END()
