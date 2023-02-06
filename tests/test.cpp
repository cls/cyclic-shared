#include "cyclic_shared.hpp"

#include <iostream>

namespace {

struct foo {
    cyclic::shared_ptr<foo> p;
};

} // namespace

namespace cyclic {

template<>
struct default_trace<foo>
{
    void operator()(const foo* foo, const visitor& visit) const
    {
        visit(foo->p);
    }
};

} // namespace cyclic

int main(int, char**)
{
    cyclic::weak_ptr<foo> weak;

    {
        cyclic::shared_ptr<foo> bar(new foo);
        cyclic::shared_ptr<foo> baz(new foo);

        bar->p = baz;
        baz->p = bar;

        weak = bar;

        std::cerr << bar.get();
        if (bar) {
            std::cerr << " -> " << bar->p.get();
            if (bar->p) {
                std::cerr << " -> " << bar->p->p.get();
            }
        }
        std::cerr << std::endl;
    }

    {
        cyclic::shared_ptr<foo> bar(weak);

        std::cerr << bar.get();
        if (bar) {
            std::cerr << " -> " << bar->p.get();
            if (bar->p) {
                std::cerr << " -> " << bar->p->p.get();
            }
        }
        std::cerr << std::endl;
    }

    std::cerr << "(collect)" << std::endl;
    cyclic::collect_cycles();

    {
        cyclic::shared_ptr<foo> bar(weak);

        std::cerr << bar.get();
        if (bar) {
            std::cerr << " -> " << bar->p.get();
            if (bar->p) {
                std::cerr << " -> " << bar->p->p.get();
            }
        }
        std::cerr << std::endl;
    }

    return 0;
}
