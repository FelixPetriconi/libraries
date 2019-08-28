/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#include <boost/test/unit_test.hpp>

#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/future.hpp>
#include <stlab/concurrency/utility.hpp>
#include <stlab/test/model.hpp>

#include "future_test_helper.hpp"

using namespace stlab;
using namespace future_test_helper;

BOOST_FIXTURE_TEST_SUITE(future_when_all_range_void, test_fixture<void>)
BOOST_AUTO_TEST_CASE(future_when_all_void_void_empty_range) {
    BOOST_TEST_MESSAGE("running future when_all void -> void with empty range");
    bool check = {false};
    std::vector<stlab::future<void>> emptyFutures;

    sut = when_all([& _check = check]() { _check = true; } & make_executor<0>(),
                   std::make_pair(emptyFutures.begin(), emptyFutures.end()));

    check_valid_future(sut);
    wait_until_future_completed(sut);

    BOOST_REQUIRE(check);
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
}

BOOST_AUTO_TEST_CASE(future_when_all_void_empty_range) {
    BOOST_TEST_MESSAGE("running future when_all void with empty range");
    size_t p = 0;
    std::vector<stlab::future<int>> emptyFutures;

    sut = when_all([& _p = p](std::vector<int> v) { _p = v.size(); } & make_executor<0>(),
                   std::make_pair(emptyFutures.begin(), emptyFutures.end()));

    check_valid_future(sut);
    wait_until_future_completed(sut);

    BOOST_REQUIRE_EQUAL(size_t(0), p);
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
}

BOOST_AUTO_TEST_CASE(future_when_all_void_range_with_one_element) {
    BOOST_TEST_MESSAGE("running future when_all void with range of one element");
    size_t p = 0;
    size_t r = 0;
    std::vector<stlab::future<int>> futures;
    futures.push_back(async([] { return 42; } & make_executor<0>()));

    sut = when_all([& _p = p, &_r = r](std::vector<int> v) {
                       _p = v.size();
                       _r = v[0];
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    check_valid_future(sut);
    wait_until_future_completed(sut);

    BOOST_REQUIRE_EQUAL(size_t(1), p);
    BOOST_REQUIRE_EQUAL(size_t(42), r);
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

BOOST_AUTO_TEST_CASE(future_when_all_void_range_with_many_elements) {
    BOOST_TEST_MESSAGE("running future when_all void with range with many elements");
    size_t p = 0;
    size_t r = 0;
    std::vector<stlab::future<int>> futures;
    futures.push_back(async([] { return 1; } & make_executor<0>()));
    futures.push_back(async([] { return 2; } & make_executor<0>()));
    futures.push_back(async([] { return 3; } & make_executor<0>()));
    futures.push_back(async([] { return 5; } & make_executor<0>()));

    sut = when_all([& _p = p, &_r = r](std::vector<int> v) {
                       _p = v.size();
                       for (auto i : v) {
                           _r += i;
                       }
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    check_valid_future(sut);
    wait_until_future_completed(sut);

    BOOST_REQUIRE_EQUAL(size_t(4), p);
    BOOST_REQUIRE_EQUAL(size_t(1 + 2 + 3 + 5), r);
    BOOST_REQUIRE_LE(4, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

/*
       /  F1  \
      / / F2 \ \
start           sut
      \ \ F3 / /
       \  F4  /
*/
BOOST_AUTO_TEST_CASE(future_when_all_void_range_with_diamond_formation_elements) {
    BOOST_TEST_MESSAGE("running future when_all void with range with diamond formation");
    int v[4] = {0, 0, 0, 0};
    int r = 0;
    auto start = async([] { return 4711; } & make_executor<0>());
    std::vector<stlab::future<void>> futures(4);
    futures[0] = start | [&_p = v[0]](auto x) { _p = x + 1; } & make_executor<0>();
    futures[1] = start | [&_p = v[1]](auto x) { _p = x + 2; } & make_executor<0>();
    futures[2] = start | [&_p = v[2]](auto x) { _p = x + 3; } & make_executor<0>();
    futures[3] = start | [&_p = v[3]](auto x) { _p = x + 5; } & make_executor<0>();

    sut = when_all([& _r = r, &v]() {
                       for (auto i : v) {
                           _r += i;
                       }
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    check_valid_future(sut);
    wait_until_future_completed(sut);

    BOOST_REQUIRE_EQUAL(4711 + 1 + 4711 + 2 + 4711 + 3 + 4711 + 5, r);
    BOOST_REQUIRE_LE(5, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE(future_when_all_range_int, test_fixture<int>)
BOOST_AUTO_TEST_CASE(future_when_all_int_empty_range) {
    BOOST_TEST_MESSAGE("running future when_all int with empty range");

    std::vector<stlab::future<int>> emptyFutures;

    sut = when_all([](std::vector<int> v) { return static_cast<int>(v.size()); } & make_executor<0>(),
                   std::make_pair(emptyFutures.begin(), emptyFutures.end()));
    check_valid_future(sut);
    wait_until_future_completed(sut);

    BOOST_REQUIRE_EQUAL(0, *sut.get_try());
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
}

BOOST_AUTO_TEST_CASE(future_when_all_int_range_with_one_element) {
    BOOST_TEST_MESSAGE("running future when_all int with range of one element");
    size_t p = 0;
    std::vector<stlab::future<int>> futures;
    futures.push_back(async([] { return 42; } & make_executor<0>()));

    sut = when_all([&_p = p](std::vector<int> v) {
                       _p = v.size();
                       return v[0];
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    check_valid_future(sut);
    wait_until_future_completed(sut);

    BOOST_REQUIRE_EQUAL(size_t(1), p);
    BOOST_REQUIRE_EQUAL(42, *sut.get_try());
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

BOOST_AUTO_TEST_CASE(future_when_all_int_range_with_many_elements) {
    BOOST_TEST_MESSAGE("running future when_all int with range with many elements");
    size_t p = 0;
    std::vector<stlab::future<int>> futures;
    futures.push_back(async([] { return 1; } & make_executor<0>()));
    futures.push_back(async([] { return 2; } & make_executor<0>()));
    futures.push_back(async([] { return 3; } & make_executor<0>()));
    futures.push_back(async([] { return 5; } & make_executor<0>()));

    sut = when_all([& _p = p](std::vector<int> v) {
                       _p = v.size();
                       auto r = 0;
                       for (auto i : v) {
                           r += i;
                       }
                       return r;
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    check_valid_future(sut);
    wait_until_future_completed(sut);

    BOOST_REQUIRE_EQUAL(size_t(4), p);
    BOOST_REQUIRE_EQUAL(1 + 2 + 3 + 5, *sut.get_try());
    BOOST_REQUIRE_LE(4, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

/*
       /  F1  \
      / / F2 \ \
start           sut
      \ \ F3 / /
       \  F4  /
*/
BOOST_AUTO_TEST_CASE(future_when_all_int_range_with_diamond_formation_elements) {
    BOOST_TEST_MESSAGE("running future when_all int with range with diamond formation");
    size_t p = 0;
    auto start = async([] { return 4711; } & make_executor<0>());
    std::vector<stlab::future<int>> futures(4);
    futures[0] = start | [](auto x) { return x + 1; } & make_executor<0>();
    futures[1] = start | [](auto x) { return x + 2; } & make_executor<0>();
    futures[2] = start | [](auto x) { return x + 3; } & make_executor<0>();
    futures[3] = start | [](auto x) { return x + 5; } & make_executor<0>();

    sut = when_all([& _p = p](std::vector<int> v) {
                       _p = v.size();
                       auto r = 0;
                       for (auto i : v) {
                           r += i;
                       }
                       return r;
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    check_valid_future(sut);
    wait_until_future_completed(sut);

    BOOST_REQUIRE_EQUAL(size_t(4), p);
    BOOST_REQUIRE_EQUAL(4711 + 1 + 4711 + 2 + 4711 + 3 + 4711 + 5, *sut.get_try());
    BOOST_REQUIRE_LE(4, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_FIXTURE_TEST_SUITE(future_when_all_range_move_only, test_fixture<stlab::move_only>)

BOOST_AUTO_TEST_CASE(future_when_all_move_range_with_many_elements) {
    BOOST_TEST_MESSAGE("running future when_all move_only with range with many elements");
    size_t p = 0;
    std::vector<stlab::future<stlab::move_only>> futures;
    futures.push_back(async([] { return stlab::move_only{1}; } & make_executor<0>()));
    futures.push_back(async([] { return stlab::move_only{2}; } & make_executor<0>()));
    futures.push_back(async([] { return stlab::move_only{3}; } & make_executor<0>()));
    futures.push_back(async([] { return stlab::move_only{5}; } & make_executor<0>()));

    sut = when_all([& _p = p](std::vector<stlab::move_only> v) {
                       _p = v.size();
                       auto r = 0;
                       for (const auto& i : v) {
                           r += i.member();
                       }
                       return stlab::move_only{r};
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    check_valid_future(sut);
    wait_until_future_completed(sut);

    BOOST_REQUIRE_EQUAL(size_t(4), p);
    BOOST_REQUIRE_EQUAL(1 + 2 + 3 + 5, (*sut.get_try()).member() );
    BOOST_REQUIRE_LE(4, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}
BOOST_AUTO_TEST_SUITE_END()

// ----------------------------------------------------------------------------
//                             Error cases
// ----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(future_when_all_range_void_error, test_fixture<void>)
BOOST_AUTO_TEST_CASE(future_when_all_void_range_with_one_element) {
    BOOST_TEST_MESSAGE("running future when_all void with range of one failing element");
    size_t p = 0;
    size_t r = 0;
    std::vector<stlab::future<int>> futures;
    futures.push_back(
        async([]() -> int { throw test_exception("failure"); } & make_executor<0>()));

    sut = when_all([& _p = p, &_r = r](const std::vector<int>& v) {
                       _p = v.size();
                       _r = v[0];
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    wait_until_future_fails<test_exception>(sut);

    check_failure<test_exception>(sut, "failure");
    BOOST_REQUIRE_EQUAL(size_t(0), p);
    BOOST_REQUIRE_EQUAL(size_t(0), r);
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

BOOST_AUTO_TEST_CASE(future_when_all_void_range_with_many_elements_one_failing) {
    BOOST_TEST_MESSAGE(
        "running future when_all void with range with many elements and one failing");
    size_t p = 0;
    size_t r = 0;
    std::vector<stlab::future<int>> futures;
    futures.push_back(
        async([]() -> int { throw test_exception("failure"); } & make_executor<0>()));
    futures.push_back(async([] { return 2; } & make_executor<0>()));
    futures.push_back(async([] { return 3; } & make_executor<0>()));
    futures.push_back(async([] { return 5; } & make_executor<0>()));

    sut = when_all([& _p = p, &_r = r](const std::vector<int>& v) {
                       _p = v.size();
                       for (auto i : v) {
                           _r += i;
                       }
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    wait_until_future_fails<test_exception>(sut);

    check_failure<test_exception>(sut, "failure");
    BOOST_REQUIRE_EQUAL(size_t(0), p);
    BOOST_REQUIRE_EQUAL(size_t(0), r);
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

BOOST_AUTO_TEST_CASE(future_when_all_void_range_with_many_elements_all_failing) {
    BOOST_TEST_MESSAGE(
        "running future when_all void with range with many elements and all failing");
    size_t p = 0;
    size_t r = 0;
    std::vector<stlab::future<int>> futures;
    futures.push_back(
        async([]() -> int { throw test_exception("failure"); } & make_executor<0>()));
    futures.push_back(
        async([]() -> int { throw test_exception("failure"); } & make_executor<0>()));
    futures.push_back(
        async([]() -> int { throw test_exception("failure"); } & make_executor<0>()));
    futures.push_back(
        async([]() -> int { throw test_exception("failure"); } & make_executor<0>()));

    sut = when_all([& _p = p, &_r = r](const std::vector<int>& v) {
                       _p = v.size();
                       for (auto i : v) {
                           _r += i;
                       }
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    wait_until_future_fails<test_exception>(sut);

    check_failure<test_exception>(sut, "failure");
    BOOST_REQUIRE_EQUAL(size_t(0), p);
    BOOST_REQUIRE_EQUAL(size_t(0), r);
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

/*
       /  F1  \
      / / F2 \ \
start           sut
      \ \ F3 / /
       \  F4  /
*/
BOOST_AUTO_TEST_CASE(future_when_all_void_range_with_diamond_formation_elements_start_failing) {
    BOOST_TEST_MESSAGE(
        "running future when_all void with range with diamond formation and start failing");
    int v[4] = {0, 0, 0, 0};
    int r = 0;
    auto start = async([]() -> int { throw test_exception("failure"); } & make_executor<0>());
    std::vector<stlab::future<void>> futures(4);
    futures[0] = start | [& _p = v[0]](auto x) { _p = x + 1; } & make_executor<0>();
    futures[1] = start | [& _p = v[1]](auto x) { _p = x + 2; } & make_executor<0>();
    futures[2] = start | [& _p = v[2]](auto x) { _p = x + 3; } & make_executor<0>();
    futures[3] = start | [& _p = v[3]](auto x) { _p = x + 5; } & make_executor<0>();

    sut = when_all([& _r = r, &v]() {
                       for (auto i : v) {
                           _r += i;
                       }
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    wait_until_future_fails<test_exception>(sut);

    check_failure<test_exception>(sut, "failure");
    BOOST_REQUIRE_EQUAL(0, r);
    for (auto d : v) {
        BOOST_REQUIRE_EQUAL(0, d);
    }
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

BOOST_AUTO_TEST_CASE(
    future_when_all_void_range_with_diamond_formation_elements_one_parallel_failing) {
    BOOST_TEST_MESSAGE(
        "running future when_all void with range with diamond formation and one of the parallel tasks is failing");
    int v[4] = {0, 0, 0, 0};
    int r = 0;
    auto start = async([]() -> int { return 42; } & make_executor<0>());
    std::vector<stlab::future<void>> futures(4);
    futures[0] = start | [& _p = v[0]](auto x) { _p = x + 1; } & make_executor<0>();
    futures[1] = start | [](auto) { throw test_exception("failure"); } & make_executor<0>();
    futures[2] = start | [& _p = v[2]](auto x) { _p = x + 3; } & make_executor<0>();
    futures[3] = start | [& _p = v[3]](auto x) { _p = x + 5; } & make_executor<0>();

    sut = when_all([& _r = r, &v]() {
                       for (auto i : v) {
                           _r += i;
                       }
                   } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    wait_until_future_fails<test_exception>(sut);

    check_failure<test_exception>(sut, "failure");
    BOOST_REQUIRE_EQUAL(0, r);
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

BOOST_AUTO_TEST_CASE(future_when_all_void_range_with_diamond_formation_elements_join_failing) {
    BOOST_TEST_MESSAGE(
        "running future when_all void with range with diamond formation and the joining tasks is failing");
    int v[4] = {0, 0, 0, 0};
    int r = 0;
    auto start = async([]() -> int { return 42; } & make_executor<0>());
    std::vector<stlab::future<void>> futures(4);
    futures[0] = start | [& _p = v[0]](auto x) { _p = x + 1; } & make_executor<0>();
    futures[1] = start | [& _p = v[1]](auto x) { _p = x + 2; } & make_executor<0>();
    futures[2] = start | [& _p = v[2]](auto x) { _p = x + 3; } & make_executor<0>();
    futures[3] = start | [& _p = v[3]](auto x) { _p = x + 5; } & make_executor<0>();

    sut = when_all([]() { throw test_exception("failure"); } & make_executor<1>(),
                   std::make_pair(futures.begin(), futures.end()));

    wait_until_future_fails<test_exception>(sut);

    check_failure<test_exception>(sut, "failure");
    BOOST_REQUIRE_EQUAL(0, r);
    BOOST_REQUIRE_LE(1, custom_scheduler<0>::usage_counter());
    BOOST_REQUIRE_LE(1, custom_scheduler<1>::usage_counter());
}

BOOST_AUTO_TEST_SUITE_END()
