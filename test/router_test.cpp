/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/


#include <boost/test/unit_test.hpp>

#include <stlab/concurrency/channel.hpp>
#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/router.hpp>
#include <stlab/concurrency/serial_queue.hpp>

#include <iostream>
#include <string>
#include <thread>

using namespace std;
using namespace std::placeholders;
using namespace stlab;

BOOST_AUTO_TEST_CASE(initial_router_test) {
    sender<std::string> send;
    receiver<std::string> receive;

    tie(send, receive) = channel<std::string>(default_executor);

    // must be in sorted order
    // std::array<std::string, 4> keys{ "contains hello", "contains world", "default", "hello world"
    // };

    auto sq = serial_queue_t{default_executor};

    auto router = stlab::make_router<std::string, std::string>(sq.executor(), [](const std::string& t) {
        std::vector<std::string> result;
        if (t.find("hello") != std::string::npos) result.push_back("contains hello");
        if (t.find("hello") != std::string::npos) result.push_back("contains hello");
        if (t.find("world") != std::string::npos) result.push_back("contains world");
        if (t == "hello world") result.push_back("hello world");
        if (result.empty()) result.push_back("default");
        return result;
    });

    std::atomic_int done{8};

    auto default_case = router.add_route("default", default_executor) |
                        [](std::string t) -> std::string { return t + ": default case\n"; };

    auto hello_world = router.add_route("hello world", default_executor) |
                       [](std::string t) -> std::string { return t + ": is hello world\n"; };

    auto hello = router.add_route("contains hello", default_executor) |
                 [](std::string t) -> std::string { return t + ": contains hello\n"; };

    auto world = router.add_route("contains world", default_executor) |
                 [](std::string t) -> std::string { return t + ": contains world\n"; };

    router.set_ready();
    auto hold = receive | std::ref(router);

    auto all_cases = stlab::merge_channel<unordered_t>(
        stlab::default_executor,
        [& _done = done](std::string message) {
            std::cout << message;
            --_done;
        },
        std::move(default_case), std::move(hello), std::move(world), std::move(hello_world));

    // It is necessary to mark the receiver side as ready, when all connections are
    // established
    receive.set_ready();
    hold.set_ready();

    send("bob");
    send("hello");
    send("world");
    send("hello world");

    // Waiting just for illustrational purpose
    while (done.load() != 0) {
        std::this_thread::sleep_for(chrono::milliseconds(1));
    }
}