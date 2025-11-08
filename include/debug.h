#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <sstream>
#include <ranges>

//#define DEBUGMSG


template <std::ranges::input_range R>
std::string range_to_string(const R& range) {
#ifdef DEBUGMSG
    std::ostringstream oss;
    oss << '(';
    bool first = true;
    for (const auto& x : range) {
        if (!first) oss << ", ";
        first = false;
        oss << x;
    }
    oss << ')';
    return oss.str();
#else
    return "";
#endif
}

template <typename T>
void debug_print_impl(T t)
{
   std::cerr << t << "\n";
}

template<typename T, typename ...Args>
void debug_print_impl(T t, Args... args)
{
   std::cerr << t << " ";
   debug_print_impl(args...);
}

template<typename ...Args>
void dprint(Args... args)
{
#ifdef DEBUGMSG
   std::cerr << "[DEBUG] ";
   debug_print_impl(args...);
#endif
}

#endif // DEBUG_H
