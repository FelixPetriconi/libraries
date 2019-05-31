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

template<typename T>
using channel_t = std::pair<sender<T>, receiver<T>>;

template <typename E, typename Arg, typename F, typename K>
class router_ : std::enable_shared_from_this<router_<E, Arg, F, K>>
{
    struct concept_t;
    using route_pair_t = std::pair<const K, std::unique_ptr<concept_t>>;
    E _executor;
    F _router_function;
    std::vector<route_pair_t> _routes;

    struct concept_t 
    {
        virtual void set_ready() = 0;
        virtual void route(Arg arg) = 0;
        virtual ~concept_t() = default;
    };

    template <typename T>
    struct model : concept_t
    {
        model(channel_t<T> c)
            : _channel(std::move(c))
        {}

        void set_ready() override {
            _channel.second.set_ready();
        }

        void route(Arg arg) override {
            _channel.first(std::move(arg));
        }

        channel_t _channel;
    };

public:
    template <typename... T>
    router_(E executor, F router_function, route_pair_t<K, channel_t<T>>... route_pairs) 
    : _executor{std::move(executor)}
    , _router_function(std::move(router_function))
    , _routes{ std::make_pair(route_pairs.first,
               std::make_unique<model<T>>(std::move(route_pairs.second)))... }
    {}
    // TODO FP ensure order by key

    void set_ready() {
        for (auto& route : _routes)
            route.second->set_ready();
    }

    void operator()(Arg arg) {
        _executor([_weak_this = std::weak_from_this(), _arg = std::move(arg)] {
            auto smart_this = _weak_this.lock();
            if (smart_this) {
                auto keys = smart_this->_router_function(std::move(_arg));
                auto find_it = std::begin(smart_this->_routes);
                for (const auto& key : keys) {
                    find_it =
                        std::lower_bound(find_it, std::end(smart_this->_routes), key,
                                         [](const auto& a, const auto& k) { return a.first < k; });
                    if (find_it == std::end(smart_this->_routes)) return;
                    if (find_it->first != key) continue;
                    find_it->second->route(_arg);
                }
            }
        });
    }
};

template <typename Arg, typename E, typename F, typename K, typename...T>
auto router(E executor, F router_function, std::pair<const K, channel_t<T>>... route_pairs) {
    return router_<E, F, Arg, K>(
               std::move(executor), std::move(router_function), std::move(route_pairs)...);
}

template <typename E, typename F, typename I>
auto router(E executor, F router_function, std::pair<I, I> keys_range) {
    using route_pair_t = typename std::iterator_traits<I>::value_type;
    using K = std::remove_const_t<typename route_pair_t::first_type>;
    using T = typename route_pair_t::second_type;
    using R = std::vector<route_pair_t>;

    R routes(std::distance(keys_range.first, keys_range.second));
    std::transform(
        keys_range.first, keys_range.second, std::begin(routes),
        [_executor = executor](auto key) { return std::make_pair(key, channel<T>(_executor)); });

    return router_<E, F, K, T, R>(std::move(executor), std::move(router_function),
                                  std::move(routes));
}




/**************************************************************************************************/

} // namespace v1

/**************************************************************************************************/

} // namespace stlab

/**************************************************************************************************/

#endif