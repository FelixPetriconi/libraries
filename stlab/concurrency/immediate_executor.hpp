/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#ifndef STLAB_CONCURRENCY_IMMEDIATE_EXECUTOR_HPP
#define STLAB_CONCURRENCY_IMMEDIATE_EXECUTOR_HPP

#include <chrono>

#include <stlab/concurrency/executor_base.hpp>

/**************************************************************************************************/

namespace stlab {

/**************************************************************************************************/

inline namespace v1 {
/**************************************************************************************************/

namespace detail {

/**************************************************************************************************/

struct immediate_executor_type {
    template <typename F>
    void operator()(F&& f) const {
        std::forward<F>(f)();
    }

    template <typename F>
    void operator()(std::chrono::steady_clock::time_point, F&& f) const {
        std::forward<F>(f)();
    }
};

/**************************************************************************************************/

} // namespace detail

/**************************************************************************************************/

constexpr auto immediate_executor = detail::immediate_executor_type{};

namespace detail
{
auto stlab_is_executor [[maybe_unused]] (decltype(immediate_executor)) -> std::true_type;
}


/**************************************************************************************************/

} // namespace v1

/**************************************************************************************************/

} // namespace stlab

/**************************************************************************************************/

#endif

/**************************************************************************************************/
