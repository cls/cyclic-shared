#include "cyclic_shared.hpp"

#include <cstdint>
#include <unordered_map>

namespace cyclic {

class roots
{
private:
    roots() = default;

public:
    static roots& get()
    {
        static roots instance;
        return instance;
    }

    void insert(untyped_state* state)
    {
        const auto key(reinterpret_cast<uintptr_t>(state));
        const auto it(map_.find(key));
        if (it == map_.end()) {
            map_.emplace(key, untyped_weak_ptr(state));
        }
    }

    bool contains(const untyped_state* state) const
    {
        const auto key(reinterpret_cast<uintptr_t>(state));
        return bool(map_.count(key));
    }

    void collect_cycles()
    {
        mark();
        scan();
        collect();
    }

private:
    void mark()
    {
        auto it(map_.begin());
        const auto end(map_.end());

        while (it != end) {
            untyped_state& state(*it->second.state_);
            if (state.color_ == color::Purple && state.strong_ > 0) {
                state.mark_gray();
                ++it;
            } else {
                it = map_.erase(it);
            }
        }
    }

    void scan() const
    {
        for (const auto& entry : map_) {
            untyped_state* state(entry.second.state_);
            if (state->color_ == color::Gray) {
                state->scan();
            }
        }
    }

    void collect()
    {
        for (const auto& entry : map_) {
            untyped_state* state(entry.second.state_);
            if (state->color_ == color::White) {
                state->collect_white();
            }
        }
        map_.clear();
    }

private:
    std::unordered_map<std::uintptr_t, untyped_weak_ptr> map_;
};

void collect_cycles()
{
    roots::get().collect_cycles();
}

namespace {

template<class Fn>
class fn_visitor : public visitor
{
public:
    fn_visitor(Fn fn) :
        fn_(std::forward<Fn>(fn))
    {}

    void operator()(untyped_state* state) const final
    {
        fn_(state);
    }

private:
    Fn fn_;
};

} // namespace

template<class Fn>
void untyped_state::do_trace(void* ptr, Fn fn) const
{
    trace(ptr, fn_visitor<Fn>(std::forward<Fn>(fn)));
}

void untyped_state::release()
{
    color_ = color::Black;
    void* const ptr(std::exchange(ptr_, nullptr));
    free(ptr);
    if (weak_ == 0) {
        delete this;
    }
}

void untyped_state::possible_root()
{
    color_ = color::Purple;
    roots::get().insert(this);
}

void untyped_state::mark_gray()
{
    color_ = color::Gray;
    do_trace(ptr_, [](untyped_state* child) {
        --child->strong_;
        ++child->weak_;
        if (child->color_ != color::Gray) {
            child->mark_gray();
        }
    });
}

void untyped_state::scan()
{
    if (strong_ == 0) {
        color_ = color::White;
        do_trace(ptr_, [](untyped_state* child) {
            if (child->color_ == color::Gray) {
                child->scan();
            }
            --child->weak_;
            ++child->strong_;
        });
    } else {
        scan_black();
    }
}

void untyped_state::scan_black()
{
    color_ = color::Black;
    do_trace(ptr_, [](untyped_state* child) {
        if (child->color_ != color::Black) {
            child->scan_black();
        }
        --child->weak_;
        ++child->strong_;
    });
}

void untyped_state::collect_white()
{
    color_ = color::Black;
    void* const ptr(std::exchange(ptr_, nullptr));
    do_trace(ptr, [](untyped_state* child) {
        if (child->color_ == color::White) {
            child->collect_white();
        }
    });
    free(ptr);
}

} // namespace cyclic
