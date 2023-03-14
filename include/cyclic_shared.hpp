#pragma once

#include "cyclic_shared.fwd.hpp"

#include <utility>

namespace cyclic {

class visitor
{
public:
    template<class T>
    void operator()(const shared_ptr<T>& ptr) const
    {
        untyped_state* state(ptr.untyped_.state_);
        if (state) {
            operator()(state);
        }
    }

protected:
    virtual void operator()(untyped_state* state) const = 0;
};

template<class T>
struct default_trace
{
    void operator()(const T* ptr, const visitor& visitor) const;
};

template<class T>
struct default_delete
{
    void operator()(T* ptr) const
    {
        delete ptr;
    }
};

enum class color {
    Black,  // In use or free
    Gray,   // Possible member of cycle
    White,  // Member of garbage cycle
    Purple, // Possible root of cycle
//  Green,  // Acyclic
//  Red,    // Candidate cycle undergoing Sigma-computation
//  Orange, // Candidate cycle awaiting epoch boundary
};

class untyped_state
{
    friend class roots;

public:
    untyped_state(void* ptr) :
        ptr_(ptr),
        strong_(0),
        weak_(0),
        color_(color::Black)
    {}

    virtual ~untyped_state() = default;

    inline void increment()
    {
        ++strong_;
        color_ = color::Black;
    }

    inline void decrement()
    {
        --strong_;
        if (strong_ == 0) {
            release();
        } else if (color_ != color::Purple) {
            possible_root();
        }
    }

    inline void increment_weak()
    {
        ++weak_;
    }

    inline void decrement_weak()
    {
        --weak_;
        if (weak_ == 0 && strong_ == 0) {
            delete this;
        }
    }

    inline void* get() const
    {
        return ptr_;
    }

    long use_count() const
    {
        return strong_;
    }

protected:
    virtual void trace(const void* ptr, const visitor& visitor) const = 0;

    virtual void free(void* ptr) const = 0;

private:
    template<class Fn>
    void do_trace(void* ptr, Fn fn) const;

    void release();
    void possible_root();
    void mark_gray();
    void scan();
    void scan_black();
    void collect_white();

private:
    void* ptr_;
    long strong_;
    long weak_;
    color color_;
};

struct untyped_shared_ptr
{
    friend class visitor;
    friend class untyped_weak_ptr;
    template<class T> friend class shared_ptr;
    template<class T> friend class weak_ptr;

public:
    untyped_shared_ptr() :
        state_(nullptr)
    {}

    untyped_shared_ptr(std::nullptr_t) :
        untyped_shared_ptr()
    {}

private:
    explicit untyped_shared_ptr(untyped_state* state) :
        state_(state)
    {
        if (state_) {
            state_->increment();
        }
    }

public:
    untyped_shared_ptr(const untyped_shared_ptr& ptr) :
        untyped_shared_ptr(ptr.state_)
    {}

    untyped_shared_ptr(const untyped_weak_ptr& ptr) __attribute__ ((weak)); // FIXME?

    untyped_shared_ptr(untyped_shared_ptr&& ptr) :
        state_(std::exchange(ptr.state_, nullptr))
    {}

    ~untyped_shared_ptr()
    {
        if (state_) {
            state_->decrement();
        }
    }

    untyped_shared_ptr& operator=(const untyped_shared_ptr& ptr)
    {
        return *this = untyped_shared_ptr(ptr);
    }

    untyped_shared_ptr& operator=(untyped_shared_ptr&& ptr)
    {
        std::swap(state_, ptr.state_);
        return *this;
    }

    void* get() const
    {
        return state_ ? state_->get() : nullptr;
    }

    long use_count() const
    {
        return state_ ? state_->use_count() : 0;
    }

    operator bool() const
    {
        return bool(get());
    }

private:
    untyped_state* state_;
};

class untyped_weak_ptr
{
    template<class T> friend class weak_ptr;
    friend class roots;

public:
    untyped_weak_ptr() :
        state_(nullptr)
    {}

    untyped_weak_ptr(std::nullptr_t) :
        untyped_weak_ptr()
    {}

private:
    explicit untyped_weak_ptr(untyped_state* state) :
        state_(state)
    {
        if (state_) {
            state_->increment_weak();
        }
    }

public:
    untyped_weak_ptr(const untyped_shared_ptr& ptr) :
        untyped_weak_ptr(ptr.state_)
    {}

    untyped_weak_ptr(const untyped_weak_ptr& ptr) :
        untyped_weak_ptr(ptr.state_)
    {}

    untyped_weak_ptr(untyped_weak_ptr&& ptr) :
        state_(std::exchange(ptr.state_, nullptr))
    {}

    ~untyped_weak_ptr()
    {
        if (state_) {
            state_->decrement_weak();
        }
    }

    untyped_weak_ptr& operator=(const untyped_weak_ptr& ptr)
    {
        return *this = untyped_weak_ptr(ptr);
    }

    untyped_weak_ptr& operator=(const untyped_shared_ptr& ptr)
    {
        return *this = untyped_weak_ptr(ptr);
    }

    untyped_weak_ptr& operator=(untyped_weak_ptr&& ptr)
    {
        std::swap(state_, ptr.state_);
        return *this;
    }

    untyped_shared_ptr lock() const
    {
        return state_ && state_->get() ? untyped_shared_ptr(state_) : nullptr;
    }

private:
    untyped_state* state_;
};

untyped_shared_ptr::untyped_shared_ptr(const untyped_weak_ptr& ptr) :
    untyped_shared_ptr(ptr.lock())
{}

template<class T, class Tracer, class Deleter>
class state : public untyped_state
{
public:
    state(T* ptr, Tracer tracer, Deleter deleter) :
        untyped_state(ptr),
        trace_(std::forward<Tracer>(tracer)),
        delete_(std::forward<Deleter>(deleter))
    {}

    ~state() final = default;

    void trace(const void* ptr, const visitor& visitor) const final
    {
        trace_(static_cast<const T*>(ptr), visitor);
    }

    void free(void* ptr) const final
    {
        delete_(static_cast<T*>(ptr));
    }

private:
    Tracer trace_;
    Deleter delete_;
};

template<class T>
class shared_ptr
{
    friend class visitor;
    friend class weak_ptr<T>;

public:
    using weak_type = weak_ptr<T>;

    shared_ptr() :
        ptr_(nullptr)
    {}

    shared_ptr(std::nullptr_t) :
        shared_ptr()
    {}

    template<class Y>
    shared_ptr(Y* ptr) :
        shared_ptr(ptr, default_trace<Y>())
    {}

    template<class Y, class Tracer>
    shared_ptr(Y* ptr, Tracer tracer) :
        shared_ptr(ptr, std::forward<Tracer>(tracer), default_delete<Y>())
    {}

    template<class Y, class Tracer, class Deleter>
    shared_ptr(Y* ptr, Tracer tracer, Deleter deleter) :
        ptr_(ptr),
        untyped_(new state<Y, Tracer, Deleter>(ptr, std::forward<Tracer>(tracer), std::forward<Deleter>(deleter)))
    {}

    template<class Y>
    shared_ptr(const shared_ptr<Y>& ptr) :
        shared_ptr(ptr.untyped_)
    {}

    template<class Y>
    shared_ptr(const weak_ptr<Y>& ptr);

    template<class Y>
    shared_ptr(shared_ptr<Y>&& ptr) :
        ptr_(ptr.ptr_),
        untyped_(std::move(ptr.untyped_))
    {}

    ~shared_ptr() = default;

    template<class Y>
    shared_ptr& operator=(const shared_ptr<Y>& ptr)
    {
        return *this = shared_ptr(ptr);
    }

    template<class Y>
    shared_ptr& operator=(shared_ptr&& ptr)
    {
        ptr_ = ptr.ptr_;
        std::swap(untyped_, ptr.untyped_);
        return *this;
    }

    T* get() const
    {
        return static_cast<T*>(untyped_.get());
    }

    T& operator*() const
    {
        return *get();
    }

    T* operator->() const
    {
        return get();
    }

    long use_count() const
    {
        return untyped_.use_count();
    }

    operator bool() const
    {
        return bool(untyped_);
    }

private:
    T* ptr_;
    untyped_shared_ptr untyped_;
};

template<class T>
class weak_ptr
{
public:
    weak_ptr() :
        ptr_(nullptr)
    {}

    weak_ptr(std::nullptr_t) :
        weak_ptr()
    {}

    template<class Y>
    weak_ptr(const weak_ptr<Y>& ptr) :
        ptr_(ptr.ptr_),
        untyped_(ptr.untyped_)
    {}

    template<class Y>
    weak_ptr(const shared_ptr<Y>& ptr) :
        ptr_(ptr.ptr_),
        untyped_(ptr.untyped_)
    {}

    template<class Y>
    weak_ptr(weak_ptr<Y>&& ptr) :
        ptr_(ptr.ptr_),
        untyped_(std::move(ptr.untyped_))
    {}

    ~weak_ptr() = default;

    template<class Y>
    weak_ptr& operator=(const weak_ptr<Y>& ptr)
    {
        return *this = weak_ptr(ptr);
    }

    template<class Y>
    weak_ptr& operator=(const shared_ptr<Y>& ptr)
    {
        return *this = weak_ptr(ptr);
    }

    template<class Y>
    weak_ptr& operator=(weak_ptr<Y>&& ptr)
    {
        ptr_ = ptr.ptr_;
        std::swap(untyped_, ptr.untyped_);
        return *this;
    }

    shared_ptr<T> lock() const
    {
        shared_ptr<T> ptr;
        untyped_state* state(untyped_.state_);
        if (state) {
            ptr.ptr_ = ptr_;
            ptr.untyped_ = untyped_shared_ptr(state);
        }
        return ptr;
    }

private:
    T* ptr_;
    untyped_weak_ptr untyped_;
};

template<class T>
template<class Y>
shared_ptr<T>::shared_ptr(const weak_ptr<Y>& ptr) :
    shared_ptr(ptr.lock())
{}

} // namespace cyclic
