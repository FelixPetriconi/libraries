/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#include <boost/mpl/list.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/future.hpp>
#include <stlab/concurrency/immediate_executor.hpp>
#include <stlab/concurrency/utility.hpp>

#include <stlab/test/model.hpp>

#include "future_test_helper.hpp"
#include <deque>

using namespace stlab;
using namespace future_test_helper;




BOOST_AUTO_TEST_CASE(future_blocking_get_copyable_value) {
  BOOST_TEST_MESSAGE("future blocking_get with copyable value");
  auto answer = [] { return 42; };

  stlab::future<int> f = stlab::async(stlab::default_executor, answer);

  BOOST_REQUIRE_EQUAL(42, stlab::blocking_get(f));
}

BOOST_AUTO_TEST_CASE(future_blocking_get_moveonly_value) {
  BOOST_TEST_MESSAGE("future blocking_get with moveonly value");
  auto answer = [] { return stlab::move_only(42); };

  stlab::future<stlab::move_only> f = stlab::async(stlab::default_executor, answer);

  BOOST_REQUIRE_EQUAL(42, stlab::blocking_get(std::move(f)).member());
}

BOOST_AUTO_TEST_CASE(future_blocking_get_void) {
  BOOST_TEST_MESSAGE("future blocking_get with void");
  int v = 0;
  auto answer = [&] { v = 42; };

  stlab::future<void> f = stlab::async(stlab::default_executor, answer);

  stlab::blocking_get(f);
  BOOST_REQUIRE_EQUAL(42, v);
}

BOOST_AUTO_TEST_CASE(future_blocking_get_void_with_timeout) {
  BOOST_TEST_MESSAGE("future blocking_get with void");
  int v = 0;
  auto answer = [&] { v = 42; };

  stlab::future<void> f = stlab::async(stlab::default_executor, answer);

  auto r = stlab::blocking_get(f, std::chrono::seconds(2));
  BOOST_REQUIRE_EQUAL(42, v);
  BOOST_REQUIRE_EQUAL(true, r);
}

BOOST_AUTO_TEST_CASE(future_blocking_get_void_with_timeout_reached) {
  BOOST_TEST_MESSAGE("future blocking_get with void with a timeout");
  auto answer = [&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    throw test_exception("failure");
  };

  stlab::future<void> f = stlab::async(stlab::default_executor, answer);

  BOOST_REQUIRE_NO_THROW(stlab::blocking_get(f, std::chrono::milliseconds(100)));
}

BOOST_AUTO_TEST_CASE(future_blocking_get_copyable_value_error_case) {
  BOOST_TEST_MESSAGE("future blocking_get with copyable value error case");
  auto answer = []() -> int { throw test_exception("failure"); };

  stlab::future<int> f = stlab::async(stlab::default_executor, answer);

  BOOST_REQUIRE_EXCEPTION(stlab::blocking_get(f), test_exception, ([](const auto& e) {
    return std::string(e.what()) == std::string("failure");
  }));
}

BOOST_AUTO_TEST_CASE(future_blocking_get_moveonly_value_error_case) {
  BOOST_TEST_MESSAGE("future blocking_get with moveonly value");
  auto answer = []() -> stlab::move_only { throw test_exception("failure"); };

  stlab::future<stlab::move_only> f = stlab::async(stlab::default_executor, answer);

  BOOST_REQUIRE_EXCEPTION(stlab::blocking_get(std::move(f)), test_exception, ([](const auto& e) {
    return std::string(e.what()) == std::string("failure");
  }));
}

BOOST_AUTO_TEST_CASE(future_blocking_get_void_error_case) {
  BOOST_TEST_MESSAGE("future blocking_get with void error case");

  auto answer = [] { throw test_exception("failure"); };

  stlab::future<void> f = stlab::async(stlab::default_executor, answer);

  BOOST_REQUIRE_EXCEPTION(stlab::blocking_get(f), test_exception, ([](const auto& e) {
    return std::string(e.what()) == std::string("failure");
  }));
}

BOOST_AUTO_TEST_CASE(future_blocking_get_void_error_case_with_timeout) {
  BOOST_TEST_MESSAGE("future blocking_get with void error case with timout");

  auto answer = [] { throw test_exception("failure"); };

  stlab::future<void> f = stlab::async(stlab::default_executor, answer);

  BOOST_REQUIRE_EXCEPTION(
    stlab::blocking_get(f, std::chrono::seconds(60)), test_exception,
    ([](const auto& e) { return std::string(e.what()) == std::string("failure"); }));
}

BOOST_AUTO_TEST_CASE(future_blocking_get_copyable_value_timeout) {
  BOOST_TEST_MESSAGE("future blocking_get with copyable value with timeout");
  auto answer = [] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 42;
  };

  stlab::future<int> f = stlab::async(stlab::default_executor, answer);

  auto r = stlab::blocking_get(f, std::chrono::milliseconds(500));
  // timeout should have been reached
  BOOST_REQUIRE(!r);
}

BOOST_AUTO_TEST_CASE(future_blocking_get_moveonly_value_and_timeout) {
  BOOST_TEST_MESSAGE("future blocking_get with moveonly value");
  auto answer = [] {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return stlab::move_only(42);
  };

  stlab::future<stlab::move_only> f = stlab::async(stlab::default_executor, answer);
  auto r = stlab::blocking_get(std::move(f), std::chrono::milliseconds(500));
  BOOST_REQUIRE(r);
  BOOST_REQUIRE_EQUAL(42, (*r).member());
}

BOOST_AUTO_TEST_CASE(future_blocking_get_moveonly_value_error_case_and_timeout) {
  BOOST_TEST_MESSAGE("future blocking_get with moveonly value and timeout set");
  auto answer = []()->move_only {
    throw test_exception("failure");
  };

  stlab::future<stlab::move_only> f = stlab::async(stlab::default_executor, answer);

  BOOST_REQUIRE_EXCEPTION(
    stlab::blocking_get(std::move(f), std::chrono::seconds(500)), test_exception,
    ([](const auto& e) { return std::string(e.what()) == std::string("failure"); }));
}