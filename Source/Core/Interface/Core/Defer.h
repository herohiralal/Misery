#pragma once
#include <__init.h>

namespace DeferInternals
{
    struct Helper
    {
        template <typename TCallable>
        struct Defer
        {
            TCallable func;
            ~Defer() { func(); }
        };

        template <typename TCallable>
        Defer<TCallable> operator+(TCallable&& func)
        {
            return Defer<TCallable>{ std::forward<TCallable>(func) };
        }
    };
}

#define DEFER_CONCAT_IMPL(x, y) x##y
#define DEFER_CONCAT(x, y) DEFER_CONCAT_IMPL(x, y)
#define DEFER auto DEFER_CONCAT(defer_, __COUNTER__) = DeferInternals::Helper() + [&]()
