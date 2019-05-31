/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#include <stlab/concurrency/channel.hpp>
#include <stlab/concurrency/router.hpp>
#include <stlab/concurrency/default_executor.hpp>

using namespace std;
using namespace std::placeholders;
using namespace stlab;

int main() {
    sender<std::string> send;
    receiver<std::string> receive;

    tie(send, receive) = channel<std::string>(default_executor);

    // must be in sorted order
    std::array<std::string, 4 > keys{ "contains hello", "contains world", "default", "hello world" };

#if 0
    stlab::router<std::string, std::string> router{ std::make_pair(begin(route_keys), end(route_keys)), [](const std::string& t) {
        std::set<std::string> keys;
        if (t.find("hello") != std::string::npos) keys.insert("contains hello");
        if (t.find("world") != std::string::npos) keys.insert("contains world");
        if (t == "hello world") keys.insert("hello world");
        if (keys.empty()) keys.insert("default");
        return keys;
    } };
#else
    auto router = stlab::router(stlab::default_executor, [](const std::string& t) {
        std::vector<std::string> result;
        if (t.find("hello") != std::string::npos) result.push_back("contains hello");
        if (t.find("hello") != std::string::npos) result.push_back("contains hello");
        if (t.find("world") != std::string::npos) result.push_back("contains world");
        if (t == "hello world") result.push_back("hello world");
        if (result.empty()) result.push_back("default");
        return result;
        }, std::make_pair(begin(route_keys), end(route_keys))
            );
#endif

    std::atomic_bool done{ false };

    auto default_case = router.get_route("default") | [](std::string t) -> std::string {
        return t + ": default case\n";
    };

    auto hello = router.get_route("contains hello") | [](std::string t) -> std::string {
        return t + ": contains hello\n";
    };

    auto world = router.get_route("contains world") | [](std::string t) -> std::string {
        return t + ": contains world\n";
    };

    auto hello_world = router.get_route("hello world") | [](std::string t) -> std::string {
        return t + ": is hello world\n";
    };

    auto other_hello_world = router.get_route("hello world") | [](std::string t) -> std::string {
        return t + ": is also hello world\n";
    };

    router.set_ready();
    auto hold = receive | std::bind(std::move(router), _1);

    auto all_cases =
        stlab::merge_channel<unordered_t>(stlab::default_executor,
            [&_done = done](std::string message) {
        std::cout << message;
        _done = true;
    },
            std::move(default_case), std::move(hello), std::move(world), std::move(hello_world), std::move(other_hello_world));

    // It is necessary to mark the receiver side as ready, when all connections are
    // established
    receive.set_ready();

    send("bob");
    send("hello");
    send("world");
    send("hello world");

    // Waiting just for illustrational purpose
    while (!done.load()) {
        this_thread::sleep_for(chrono::milliseconds(1));
    }
    return 0;
}