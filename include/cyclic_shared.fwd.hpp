#pragma once

namespace cyclic {

class visitor;
class untyped_state;
class untyped_shared_ptr;
class untyped_weak_ptr;

template<class T, class Tracer, class Deleter> class state;
template<class T> class shared_ptr;
template<class T> class weak_ptr;

template<class T> struct default_trace;
template<class T> struct default_delete;

void collect_cycles();

} // namespace cyclic
