/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#ifndef STLAB_CONCURRENCY_ROUTER_HPP
#define STLAB_CONCURRENCY_ROUTER_HPP

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <stlab/concurrency/channel.hpp>

/**************************************************************************************************/

namespace stlab {

/**************************************************************************************************/

inline namespace v1 {

/**************************************************************************************************/

template <typename T>
using channel_t = std::pair<sender<T>, receiver<T>>;

template <typename T, typename K, typename E, typename F>
class router_ : public std::enable_shared_from_this<router_<T, K, E, F>> {
    using route_pair_t = std::pair<K, channel_t<T>>;
    E _executor; // of the router function
    F _router_function;
    std::vector<route_pair_t> _routes;
    bool _ready = false;

public:
    router_() = default;
    ~router_() = default;
    router_(router_&&) = default;
    router_(const router_&) = default;
    router_& operator=(router_&&) = default;
    router_& operator=(const router_&) = default;

    router_(E executor, F router_function) :
        _executor{executor}, _router_function(std::move(router_function)) {}

    template <typename... U>
    router_(E executor, F router_function, std::pair<K, channel_t<U>>... route_pairs) :
        _executor{std::move(executor)},
        _router_function(std::move(router_function)), _routes{
                                                          std::make_pair(route_pairs.first,
                                                                         route_pairs.second)...} {
        set_ready();
    }

    void set_ready() {
        std::sort(_routes.begin(), _routes.end(),
                  [](const auto& x, const auto& y) { return x.first < y.first; });
        for (auto& route : _routes)
            route.second.second.set_ready();
        _ready = true;
    }

    stlab::optional<receiver<T>> route(K key) {
        stlab::optional<receiver<T>> result;

        auto find_it = std::lower_bound(_routes.begin(), _routes.end(), key,
                                        [](const auto& p, const auto& k) { return p.first < k; });

        if (find_it == _routes.end() || find_it->first != key) return result;
        result = find_it->second.second;

        return result;
    }

    template <typename Ex>
    receiver<T> add_route(K key, Ex executor) {
        assert(!_ready);
        assert(!route(key));
        channel_t<T> ch = channel<T>(executor);
        auto result = ch.second;
        _routes.emplace_back(key, std::move(ch));
        return result;
    }

    receiver<T> add_route(K key) { return add_route(std::move(key), _executor); }

    void operator()(T arg) {
        assert(_ready);
        _executor([_arg = std::move(arg), this] {
            auto keys = _router_function(_arg);
            auto find_it = std::begin(_routes);
            for (const auto& key : keys) {
                find_it =
                    std::lower_bound(find_it, std::end(_routes), key,
                                     [](const auto& p, const auto& k) { return p.first < k; });
                if (find_it == std::end(_routes)) return;
                if (find_it->first != key) continue;
                find_it->second.first(_arg);
            }
        });
    }
};

template <typename T, typename K, typename E, typename F>
auto router(E executor, F router_function) {
    return router_<T, K, E, F>(std::move(executor), std::move(router_function));
}

/**************************************************************************************************/

} // namespace v1

/**************************************************************************************************/

} // namespace stlab

/**************************************************************************************************/

#endif