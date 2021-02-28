/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/future.hpp>
#include <stlab/concurrency/utility.hpp>

#include "future_test_helper.hpp"

using namespace std;
using namespace stlab;
using namespace future_test_helper;

using test_configuration = boost::mpl::list<
  std::pair<detail::immediate_executor_type, future_test_helper::copyable_test_fixture>,
  std::pair<detail::immediate_executor_type, future_test_helper::moveonly_test_fixture>,
  std::pair<detail::immediate_executor_type, future_test_helper::void_test_fixture>,
  std::pair<detail::portable_task_system, future_test_helper::copyable_test_fixture>,
  std::pair<detail::portable_task_system, future_test_helper::moveonly_test_fixture>,
  std::pair<detail::portable_task_system, future_test_helper::void_test_fixture>,
  std::pair<detail::os_default_executor_type, future_test_helper::copyable_test_fixture>,
  std::pair<detail::os_default_executor_type, future_test_helper::moveonly_test_fixture>,
  std::pair<detail::os_default_executor_type, future_test_helper::void_test_fixture>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(
    future_recover_failure_after_recover_initialized_with_same_executor_on_rvalue,
    T,
    test_configuration) {
    BOOST_TEST_MESSAGE(
        "running future recover, failure after recover initialized with same executor on r-value"
        << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type = typename test_fixture_t::value_type;

    using task_t = task<value_type(future<value_type>)>;
    using op_t = future<value_type> (future<value_type>::*)(task_t &&)&&;

    op_t ops[] = {static_cast<op_t>(&future<value_type>::template recover<task_t>),
                  static_cast<op_t>(&future<value_type>::template operator^<task_t>)};

    for (const auto& op : ops) {
        test_fixture_t testFixture{0};
        test_executor_t executor;

        executor_wrapper<test_executor_t> wrappedExecutor{executor};

        auto error_check{false};
        auto sut = (async(std::ref(wrappedExecutor), testFixture.void_to_value_type_failing()).*
                    op)([&](future<value_type> failedFuture) {
            error_check = testFixture.verify_failure(std::move(failedFuture));
            return testFixture.argument();
        });

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(2, wrappedExecutor.counter());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
    future_recover_failure_after_recover_initialized_with_same_executor_on_lvalue,
    T,
    test_configuration) {
    BOOST_TEST_MESSAGE(
        "running future recover, failure after recover initialized with same executor on l-value"
        << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type = typename test_fixture_t::value_type;

    {
        test_fixture_t testFixture{0};
        test_executor_t executor;

        executor_wrapper<test_executor_t> wrappedExecutor{executor};

        auto error_check{false};
        auto l_value = async(std::ref(wrappedExecutor), testFixture.void_to_value_type_failing());
        auto sut =
            test_fixture_t::move_if_moveonly(l_value).recover([&](future<value_type> failedFuture) {
                error_check = testFixture.verify_failure(std::move(failedFuture));
                return testFixture.argument();
            });

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(2, wrappedExecutor.counter());
    }

    {
        test_fixture_t testFixture{0};
        test_executor_t executor;

        executor_wrapper<test_executor_t> wrappedExecutor{executor};

        auto error_check{false};
        auto l_value = async(std::ref(wrappedExecutor), testFixture.void_to_value_type_failing());
        auto sut =
            test_fixture_t::move_if_moveonly(l_value) ^ [&](future<value_type> failedFuture) {
                error_check = testFixture.verify_failure(std::move(failedFuture));
                return testFixture.argument();
            };

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(2, wrappedExecutor.counter());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
    future_recover_failure_after_recover_initialized_with_different_exector_on_rvalue,
    T,
    test_configuration) {
    BOOST_TEST_MESSAGE(
        "running future recover, failure after recover initialized with different executor on r-value"
        << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type = typename test_fixture_t::value_type;

    {
        test_fixture_t testFixture{0};
        test_executor_t executor1, executor2;

        executor_wrapper<test_executor_t> wrappedExecutor1{executor1};
        executor_wrapper<test_executor_t> wrappedExecutor2{executor2};

        auto error_check{false};
        auto sut = async(std::ref(wrappedExecutor1), testFixture.void_to_value_type_failing())
                       .recover(std::ref(wrappedExecutor2), [&](future<value_type> failedFuture) {
                           error_check = testFixture.verify_failure(std::move(failedFuture));
                           return testFixture.argument();
                       });

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_EQUAL(1, wrappedExecutor1.counter());
        BOOST_REQUIRE_GE(2, wrappedExecutor1.counter());
    }

    {
        test_fixture_t testFixture{0};
        test_executor_t executor1, executor2;

        executor_wrapper<test_executor_t> wrappedExecutor1{executor1};
        executor_wrapper<test_executor_t> wrappedExecutor2{executor2};

        auto error_check{false};
        auto sut = async(std::ref(wrappedExecutor1), testFixture.void_to_value_type_failing()) ^
                   (executor{std::ref(wrappedExecutor2)} & [&](future<value_type> failedFuture) {
                       error_check = testFixture.verify_failure(std::move(failedFuture));
                       return testFixture.argument();
                   });

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(1, wrappedExecutor1.counter());
        BOOST_REQUIRE_GE(1, wrappedExecutor1.counter());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
    future_recover_failure_after_recover_initialized_with_different_executor_on_lvalue,
    T,
    test_configuration) {
    BOOST_TEST_MESSAGE(
        "running future recover, failure after recover initialized with different executor on l-value"
        << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type = typename test_fixture_t::value_type;

    {
        test_fixture_t testFixture{0};
        test_executor_t executor1, executor2;

        executor_wrapper<test_executor_t> wrappedExecutor1{executor1};
        executor_wrapper<test_executor_t> wrappedExecutor2{executor2};

        auto error_check{false};
        auto l_value = async(std::ref(wrappedExecutor1), testFixture.void_to_value_type_failing());
        auto sut = test_fixture_t::move_if_moveonly(l_value).recover(
            std::ref(wrappedExecutor2), [&](future<value_type> failedFuture) {
                error_check = testFixture.verify_failure(std::move(failedFuture));
                return testFixture.argument();
            });

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(1, wrappedExecutor1.counter());
        BOOST_REQUIRE_GE(1, wrappedExecutor2.counter());
    }

    {
        test_fixture_t testFixture{0};
        test_executor_t executor1, executor2;

        executor_wrapper<test_executor_t> wrappedExecutor1{executor1};
        executor_wrapper<test_executor_t> wrappedExecutor2{executor2};

        auto error_check{false};
        auto l_value = async(std::ref(wrappedExecutor1), testFixture.void_to_value_type_failing());
        auto sut = test_fixture_t::move_if_moveonly(l_value) ^
                   (executor{std::ref(wrappedExecutor2)} & [&](future<value_type> failedFuture) {
                       error_check = testFixture.verify_failure(std::move(failedFuture));
                       return testFixture.argument();
                   });

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(1, wrappedExecutor1.counter());
        BOOST_REQUIRE_GE(1, wrappedExecutor2.counter());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(future_recover_failure_before_recover_initialized_with_same_executor,
                              T,
                              test_configuration) {
    BOOST_TEST_MESSAGE(
        "running future recover, failure before recover initialized with same executor"
        << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type = typename test_fixture_t::value_type;

    {
        test_fixture_t testFixture{0};
        test_executor_t executor;

        executor_wrapper<test_executor_t> wrappedExecutor{executor};

        auto error_check{false};
        auto interim = async(std::ref(wrappedExecutor), testFixture.void_to_value_type_failing());

        wait_until_future_fails<test_exception>(interim);

        auto sut =
            test_fixture_t::move_if_moveonly(interim).recover([&](future<value_type> failedFuture) {
                error_check = testFixture.verify_failure(std::move(failedFuture));
                return testFixture.argument();
            });

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(2, wrappedExecutor.counter());
    }
    {
        test_fixture_t testFixture{0};
        test_executor_t executor;

        executor_wrapper<test_executor_t> wrappedExecutor{executor};

        auto error_check{false};
        auto interim = async(std::ref(wrappedExecutor), testFixture.void_to_value_type_failing());

        wait_until_future_fails<test_exception>(interim);

        auto sut =
            test_fixture_t::move_if_moveonly(interim) ^ [&](future<value_type> failedFuture) {
                error_check = testFixture.verify_failure(std::move(failedFuture));
                return testFixture.argument();
            };

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(2, wrappedExecutor.counter());
    }
}

/**************************************************************************************************/
/******************************** Immediate Executor Only Tests ***********************************/
/**************************************************************************************************/

using immediate_only_test_configuration = boost::mpl::list<
    std::pair<detail::immediate_executor_type, future_test_helper::copyable_test_fixture>,
    std::pair<detail::immediate_executor_type, future_test_helper::moveonly_test_fixture>,
    std::pair<detail::immediate_executor_type, future_test_helper::void_test_fixture>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(future_recover_with_broken_promise, T, test_configuration) {
    BOOST_TEST_MESSAGE("future recover with broken promise" << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type_t = typename test_fixture_t::value_type;

    using task_t = task<value_type_t(future<value_type_t>)>;
    using op_t = future<value_type_t> (future<value_type_t>::*)(task_t &&)&&;

    op_t ops[] = {static_cast<op_t>(&future<value_type_t>::template recover<task_t>),
                  static_cast<op_t>(&future<value_type_t>::template operator^<task_t>)};

    for (const auto& op : ops) {
        test_executor_t executor;
        executor_wrapper<test_executor_t> wrappedExecutor{executor};

        test_fixture_t testFixture;

        auto check{false};
        auto sut = [&check, &testFixture, &wrappedExecutor, &op](auto&&) {
            auto promise_future = package<value_type_t(value_type_t)>(
                std::ref(wrappedExecutor), testFixture.value_type_to_value_type());

            return (test_fixture_t::move_if_moveonly(promise_future.second).*op)(
                [&check](const auto& f) {
                    check = true;
                    try {
                      return static_if(bool_v<std::is_same_v<value_type_t, void>>)
                        .then_([&](auto&&) { (void) f.get_try(); })
                        .else_([&](auto&&) { return *f.get_try(); })(std::ignore);

                    } catch (const exception&) {
                        throw;
                    }
                });
        }(std::ignore);

        wait_until_future_ready(sut);

        check_failure<future_error>(sut, future_error_codes::broken_promise);
        BOOST_REQUIRE(check);
        BOOST_REQUIRE_GE(1, wrappedExecutor.counter());
    }
}

/**************************************************************************************************/
/******************************** Threaded Only Tests *********************************************/
/**************************************************************************************************/

using threaded_only_test_configuration = boost::mpl::list<
    std::pair<detail::portable_task_system, future_test_helper::copyable_test_fixture>,
    std::pair<detail::portable_task_system, future_test_helper::moveonly_test_fixture>,
    std::pair<detail::portable_task_system, future_test_helper::void_test_fixture>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(future_recover_failure_after_recover_initialized_on_rvalue,
                              T,
                              threaded_only_test_configuration) {
    BOOST_TEST_MESSAGE("future recover, failure after recover initialized on rvalue"
                       << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type = typename test_fixture_t::value_type;

    using task_t = task<value_type(future<value_type>)>;
    using op_t = future<value_type> (future<value_type>::*)(task_t &&)&&;

    op_t ops[] = {static_cast<op_t>(&future<value_type>::template recover<task_t>),
                  static_cast<op_t>(&future<value_type>::template operator^<task_t>)};

    for (const auto& op : ops) {
        test_fixture_t testFixture;
        test_executor_t executor;

        executor_wrapper<test_executor_t> wrappedExecutor(executor);
        auto error_check{false};
        mutex block;
        future<value_type> sut;

        {
            lock_t hold(block);
            sut = (async(std::ref(wrappedExecutor),
                         [&_block = block]() -> value_type {
                             lock_t lock(_block);
                             throw test_exception("failure");
                         }).*
                   op)([&](auto failedFuture) {
                error_check = testFixture.verify_failure(std::move(failedFuture));
                return testFixture.argument();
            });
        }

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(2, wrappedExecutor.counter());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(future_recover_failure_before_recover_initialized_on_lvalue,
                              T,
                              threaded_only_test_configuration) {
    BOOST_TEST_MESSAGE("future recover, failure before recover initialized on lvalue"
                       << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type = typename test_fixture_t::value_type;

    {
        test_fixture_t testFixture;
        test_executor_t executor;

        executor_wrapper<test_executor_t> wrappedExecutor{executor};
        auto error_check{false};
        mutex block;
        future<value_type> sut;

        {
            lock_t hold(block);
            auto lvalue = async(std::ref(wrappedExecutor), [&_block = block]() -> value_type {
                lock_t lock(_block);
                throw test_exception("failure");
            });

            sut = test_fixture_t::move_if_moveonly(lvalue).recover([&](auto failedFuture) {
                error_check = testFixture.verify_failure(std::move(failedFuture));
                return testFixture.argument();
            });
        }

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(2, wrappedExecutor.counter());
    }
    {
        test_fixture_t testFixture;
        test_executor_t executor;

        executor_wrapper<test_executor_t> wrappedExecutor{executor};
        auto error_check{false};
        mutex block;
        future<value_type> sut;

        {
            lock_t hold(block);
            auto lvalue = async(std::ref(wrappedExecutor), [&_block = block]() -> value_type {
                lock_t lock(_block);
                throw test_exception("failure");
            });

            sut = test_fixture_t::move_if_moveonly(lvalue) ^ [&](auto failedFuture) {
                error_check = testFixture.verify_failure(std::move(failedFuture));
                return testFixture.argument();
            };
        }

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_GE(2, wrappedExecutor.counter());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
    future_recover_failure_before_recover_initialized_with_different_executor_on_rvalue,
    T,
    threaded_only_test_configuration) {
    BOOST_TEST_MESSAGE(
        "future recover, failure before recover initialized with different executor on rvalue"
        << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type = typename test_fixture_t::value_type;

    {
        test_fixture_t testFixture;
        test_executor_t executor1, executor2;

        executor_wrapper<test_executor_t> wrappedExecutor1{executor1};
        executor_wrapper<test_executor_t> wrappedExecutor2{executor2};

        auto error_check{false};
        mutex block;
        future<value_type> sut;

        {
            lock_t hold(block);
            sut = async(std::ref(wrappedExecutor1), [&_block = block]() -> value_type {
                      lock_t lock(_block);
                      throw test_exception("failure");
                  }).recover(std::ref(wrappedExecutor2), [&](auto failedFuture) {
                error_check = testFixture.verify_failure(std::move(failedFuture));
                return testFixture.argument();
            });
        }

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_EQUAL(1, wrappedExecutor1.counter());
        BOOST_REQUIRE_EQUAL(1, wrappedExecutor1.counter());
    }
    {
        test_fixture_t testFixture;
        test_executor_t executor1, executor2;

        executor_wrapper<test_executor_t> wrappedExecutor1{executor1};
        executor_wrapper<test_executor_t> wrappedExecutor2{executor2};

        auto error_check{false};
        mutex block;
        future<value_type> sut;

        {
            lock_t hold(block);
            sut = async(std::ref(wrappedExecutor1),
                        [&_block = block]() -> value_type {
                            lock_t lock(_block);
                            throw test_exception("failure");
                        }) ^
                  (executor{std::ref(wrappedExecutor2)} & [&](auto failedFuture) {
                      error_check = testFixture.verify_failure(std::move(failedFuture));
                      return testFixture.argument();
                  });
        }

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_EQUAL(1, wrappedExecutor1.counter());
        BOOST_REQUIRE_EQUAL(1, wrappedExecutor1.counter());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
    future_recover_failure_before_recover_initialized_with_different_executor_on_lvalue,
    T,
    threaded_only_test_configuration) {
    BOOST_TEST_MESSAGE(
        "future recover, failure before recover initialized with different executor on lvalue"
        << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type = typename test_fixture_t::value_type;

    {
        test_fixture_t testFixture;
        test_executor_t executor1, executor2;

        executor_wrapper<test_executor_t> wrappedExecutor1{executor1};
        executor_wrapper<test_executor_t> wrappedExecutor2{executor2};

        auto error_check{false};
        mutex block;
        future<value_type> sut;

        {
            lock_t hold(block);
            auto lvalue = async(std::ref(wrappedExecutor1), [&_block = block]() -> value_type {
                lock_t lock(_block);
                throw test_exception("failure");
            });

            sut = test_fixture_t::move_if_moveonly(lvalue).recover(
                std::ref(wrappedExecutor2), [&](auto failedFuture) {
                    error_check = testFixture.verify_failure(std::move(failedFuture));
                    return testFixture.argument();
                });
        }

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_EQUAL(1, wrappedExecutor1.counter());
        BOOST_REQUIRE_EQUAL(1, wrappedExecutor1.counter());
    }
    {
        test_fixture_t testFixture;
        test_executor_t executor1, executor2;

        executor_wrapper<test_executor_t> wrappedExecutor1{executor1};
        executor_wrapper<test_executor_t> wrappedExecutor2{executor2};

        auto error_check{false};
        mutex block;
        future<value_type> sut;

        {
            lock_t hold(block);
            auto lvalue = async(std::ref(wrappedExecutor1), [&_block = block]() -> value_type {
                lock_t lock(_block);
                throw test_exception("failure");
            });

            sut = test_fixture_t::move_if_moveonly(lvalue) ^
                  (executor{std::ref(wrappedExecutor2)} & [&](auto failedFuture) {
                      error_check = testFixture.verify_failure(std::move(failedFuture));
                      return testFixture.argument();
                  });
        }

        wait_until_future_ready(sut);

        BOOST_REQUIRE(error_check);
        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
        BOOST_REQUIRE_EQUAL(1, wrappedExecutor1.counter());
        BOOST_REQUIRE_EQUAL(1, wrappedExecutor1.counter());
    }
}
