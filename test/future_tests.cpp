/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#include "future_test_helper.hpp"

#include <boost/mpl/list.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/future.hpp>
#include <stlab/concurrency/immediate_executor.hpp>
#include <stlab/concurrency/utility.hpp>
#include <stlab/test/model.hpp>

#include <deque>

using namespace stlab;
using namespace future_test_helper;

/**************************************************************************************************/

template <class T>
auto promise_future() {
    return package<T(T)>(immediate_executor,
                         [](auto&& x) -> decltype(x) { return std::forward<decltype(x)>(x); });
}

BOOST_AUTO_TEST_CASE(rvalue_through_continuation) {
    BOOST_TEST_MESSAGE("running passing rvalue to continuation");

    annotate_counters counters;

    auto pf = promise_future<annotate>();
    pf.first(annotate(counters));
    (void)pf.second.then([](const annotate&) {}); // copy happens here!

    std::cout << counters;
}

BOOST_AUTO_TEST_CASE(async_lambda_arguments) {
    {
        BOOST_TEST_MESSAGE("running async lambda argument of type rvalue -> value");

        annotate_counters counters;
        (void)async(
            immediate_executor, [](annotate) {}, annotate(counters));
        BOOST_REQUIRE(counters.remaining() == 0);
        BOOST_REQUIRE(counters._copy_ctor == 0);
    }

    {
        BOOST_TEST_MESSAGE("running async lambda argument of type lvalue -> value");

        annotate_counters counters;
        annotate x(counters);
        (void)async(
            immediate_executor, [](annotate) {}, x);
        BOOST_REQUIRE(counters.remaining() == 1);
        BOOST_REQUIRE(counters._copy_ctor == 1);
    }

    {
        BOOST_TEST_MESSAGE("running async lambda argument of type ref -> value");

        annotate_counters counters;
        annotate x(counters);
        (void)async(
            immediate_executor, [](annotate) {}, std::ref(x));
        BOOST_REQUIRE(counters.remaining() == 1);
        BOOST_REQUIRE(counters._copy_ctor == 1);
    }

    {
        BOOST_TEST_MESSAGE("running async lambda argument of type cref -> value");

        annotate_counters counters;
        annotate x(counters);
        (void)async(
            immediate_executor, [](annotate) {}, std::cref(x));
        BOOST_REQUIRE(counters.remaining() == 1);
        BOOST_REQUIRE(counters._copy_ctor == 1);
    }
//-------
#if 0
    {
    // EXPECTED WILL NOT COMPILE
        BOOST_TEST_MESSAGE("running async lambda argument of type rvalue -> &");

        annotate_counters counters;
        async(immediate_executor, [](annotate&){ }, annotate(counters));
        BOOST_REQUIRE(counters.remaining() == 0);
        BOOST_REQUIRE(counters._copy_ctor == 0);
    }

    {
        BOOST_TEST_MESSAGE("running async lambda argument of type lvalue -> &");

        annotate_counters counters;
        annotate x(counters);
        async(immediate_executor, [](annotate&){ }, x);
        BOOST_REQUIRE(counters.remaining() == 1);
        BOOST_REQUIRE(counters._copy_ctor == 1);
    }
#endif

    {
        BOOST_TEST_MESSAGE("running async lambda argument of type ref -> &");

        annotate_counters counters;
        annotate x(counters);
        (void)async(
            immediate_executor, [](annotate&) {}, std::ref(x));
        BOOST_REQUIRE(counters.remaining() == 1);
        BOOST_REQUIRE(counters._copy_ctor == 0);
    }

#if 0
    // EXPECTED WILL NOT COMPILE
    {
    BOOST_TEST_MESSAGE("running async lambda argument of type cref -> &");

    annotate_counters counters;
    annotate x(counters);
    async(immediate_executor, [](annotate&){ }, std::cref(x));
    BOOST_REQUIRE(counters.remaining() == 1);
    BOOST_REQUIRE(counters._copy_ctor == 1);
    }
#endif
    //-------
    {
        BOOST_TEST_MESSAGE("running async lambda argument of type rvalue -> const&");

        annotate_counters counters;
        (void)async(
            immediate_executor, [](const annotate&) {}, annotate(counters));
        BOOST_REQUIRE(counters.remaining() == 0);
        BOOST_REQUIRE(counters._copy_ctor == 0);
    }

    {
        BOOST_TEST_MESSAGE("running async lambda argument of type lvalue -> const&");

        annotate_counters counters;
        annotate x(counters);
        (void)async(
            immediate_executor, [](const annotate&) {}, x);
        BOOST_REQUIRE(counters.remaining() == 1);
        BOOST_REQUIRE(counters._copy_ctor == 1);
    }

    {
        BOOST_TEST_MESSAGE("running async lambda argument of type ref -> const&");

        annotate_counters counters;
        annotate x(counters);
        (void)async(
            immediate_executor, [](const annotate&) {}, std::ref(x));
        BOOST_REQUIRE(counters.remaining() == 1);
        BOOST_REQUIRE(counters._copy_ctor == 0);
    }

    {
        BOOST_TEST_MESSAGE("running async lambda argument of type cref -> const&");

        annotate_counters counters;
        annotate x(counters);
        (void)async(
            immediate_executor, [](const annotate&) {}, std::cref(x));
        BOOST_REQUIRE(counters.remaining() == 1);
        BOOST_REQUIRE(counters._copy_ctor == 0);
    }
    //-------
    {
        BOOST_TEST_MESSAGE("running async lambda argument of type rvalue -> &&");

        annotate_counters counters;
        (void)async(
            immediate_executor, [](annotate&&) {}, annotate(counters));
        BOOST_REQUIRE(counters.remaining() == 0);
        BOOST_REQUIRE(counters._copy_ctor == 0);
    }

    {
        BOOST_TEST_MESSAGE("running async lambda argument of type lvalue -> &&");

        annotate_counters counters;
        annotate x(counters);
        (void)async(
            immediate_executor, [](annotate&&) {}, x);
        BOOST_REQUIRE(counters.remaining() == 1);
        BOOST_REQUIRE(counters._copy_ctor == 1);
    }

#if 0
    // EXPECTED WILL NOT COMPILE
    {
    BOOST_TEST_MESSAGE("running async lambda argument of type ref -> &&");

    annotate_counters counters;
    annotate x(counters);
    async(immediate_executor, [](annotate&&){ }, std::ref(x));
    BOOST_REQUIRE(counters.remaining() == 1);
    BOOST_REQUIRE(counters._copy_ctor == 0);
    }

    {
    BOOST_TEST_MESSAGE("running async lambda argument of type cref -> &&");

    annotate_counters counters;
    annotate x(counters);
    async(immediate_executor, [](annotate&&){ }, std::cref(x));
    BOOST_REQUIRE(counters.remaining() == 1);
    BOOST_REQUIRE(counters._copy_ctor == 0);
    }
#endif
}

/**************************************************************************************************/

using all_test_types = boost::mpl::list<void, int, stlab::move_only>;

BOOST_AUTO_TEST_CASE_TEMPLATE(future_default_constructed, T, all_test_types) {
    BOOST_TEST_MESSAGE("running future default constructed of type " << typeid(T).name());

    auto sut = future<T>();
    BOOST_REQUIRE(sut.valid() == false);
    BOOST_REQUIRE(sut.is_ready() == false);
}

using test_configuration = boost::mpl::list<
    std::pair<detail::immediate_executor_type, future_test_helper::copyable_test_fixture>,
    std::pair<detail::immediate_executor_type, future_test_helper::moveonly_test_fixture>,
    std::pair<detail::immediate_executor_type, future_test_helper::void_test_fixture>,
    std::pair<detail::portable_task_system, future_test_helper::copyable_test_fixture>,
    std::pair<detail::portable_task_system, future_test_helper::moveonly_test_fixture>,
    std::pair<detail::portable_task_system, future_test_helper::void_test_fixture>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(future_constructed_minimal_fn, T, test_configuration) {
  BOOST_TEST_MESSAGE("running future with minimal" << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    test_fixture_t testFixture;
    test_executor_t executor;

    executor_wrapper<test_executor_t> wrappedExecutor{executor};

    test_setup setup;
    {
        auto sut = async(std::ref(wrappedExecutor), testFixture.void_to_value_type());
        BOOST_REQUIRE(sut.valid() == true);
        BOOST_REQUIRE(!sut.exception());

        wrappedExecutor.batch();

        sut.reset();
        BOOST_REQUIRE(sut.valid() == false);
        BOOST_REQUIRE(sut.is_ready() == false);
    }
    BOOST_REQUIRE_EQUAL(1, wrappedExecutor.counter());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(running_future_with_passed_argument, T, test_configuration) {
    BOOST_TEST_MESSAGE("running future with passed argument "
                       << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type_t = typename test_fixture_t::value_type;
    using sut_t = stlab::future<value_type_t>;
    test_fixture_t testFixture{1};
    test_executor_t executor;

    executor_wrapper<test_executor_t> wrappedExecutor{executor};

    test_setup setup;
    {
        sut_t sut;

        static_if(bool_v<std::is_same_v<value_type_t, void>>)
            .then_([&](auto&&) {
                sut = async(std::ref(wrappedExecutor), testFixture.void_to_value_type());
            })
            .else_([&](auto&&) {
                sut = async(std::ref(wrappedExecutor), testFixture.value_type_to_value_type(),
                            testFixture.argument());
            })(std::ignore);

        wait_until_future_ready(sut);

        BOOST_REQUIRE(sut.valid() == true);
        BOOST_REQUIRE(!sut.exception());

        wrappedExecutor.batch();

        BOOST_REQUIRE(testFixture.verify_result(std::move(sut)));
    }
    BOOST_REQUIRE_EQUAL(1, wrappedExecutor.counter());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(future_detach_without_execution, T, test_configuration) {
    BOOST_TEST_MESSAGE("future detach without execution " << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type_t = typename test_fixture_t::value_type;
    using sut_t = stlab::future<value_type_t>;

    test_fixture_t testFixture;
    test_executor_t executor;
    executor_wrapper<test_executor_t> wrappedExecutor{executor};

    annotate_counters counter;
    bool check = true;
    {
        auto promise_future = package<value_type_t(value_type_t)>(
            std::ref(wrappedExecutor), testFixture.value_type_to_value_type());

        static_if(bool_v<std::is_same_v<value_type_t, void>>)
            .then_([&](auto&&) {
                promise_future.second.then(testFixture.value_type_to_value_type()).detach();
            })
            .else_([&](auto&&) {
                test_fixture_t::move_if_moveonly(promise_future.second)
                    .then(testFixture.value_type_to_value_type())
                    .detach();
            })(std::ignore);
    }
    // TODO check should be set to false within future
    BOOST_REQUIRE_EQUAL(1, counter.remaining());
    BOOST_REQUIRE(check);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(future_detach_with_execution, T, test_configuration) {
    BOOST_TEST_MESSAGE("future detach with execution" << type_to_string<T>());

    using test_executor_t = typename T::first_type;
    using test_fixture_t = typename T::second_type;
    using value_type_t = typename test_fixture_t::value_type;
    using sut_t = stlab::future<value_type_t>;
    test_fixture_t testFixture;
    test_executor_t executor;
    executor_wrapper<test_executor_t> wrappedExecutor{executor};

    annotate_counters counter;

    {
        auto promise_future = package<value_type_t(value_type_t)>(
            std::ref(wrappedExecutor), testFixture.value_type_to_value_type());
        // TODO check thread safe result
        static_if(bool_v<std::is_same_v<value_type_t, void>>)
            .then_([&](auto&& p) {
                p.second.then(testFixture.value_type_to_value_type()).detach();
                p.first();
                wrappedExecutor.batch();
                // wait certain time, until check value is set
                // BOOST_REQUIRE_EQUAL(true, check);
            })
            .else_([&](auto&& p) {
                test_fixture_t::move_if_moveonly(p.second)
                    .then(testFixture.value_type_to_value_type())
                    .detach();
                p.first(testFixture.argument());
                wrappedExecutor.batch();
                // BOOST_REQUIRE_EQUAL(testFixture._expectation, check);
            })(promise_future);
    }
    std::cout << counter;

    BOOST_REQUIRE_EQUAL(1, counter.remaining());
}

BOOST_AUTO_TEST_CASE(future_reduction_with_mutable_task) {
    BOOST_TEST_MESSAGE("future reduction with mutable task");

    auto func = [i = int{0}]() mutable {
        i++;
        return i;
    };

    auto result = stlab::async(stlab::default_executor, [func = std::move(func)]() mutable {
        func();

        return stlab::async(stlab::default_executor,
                            [func = std::move(func)]() mutable { return func(); });
    });

    BOOST_REQUIRE_EQUAL(2, *stlab::blocking_get(result).get_try());
}

BOOST_AUTO_TEST_CASE(future_reduction_with_mutable_void_task) {
    BOOST_TEST_MESSAGE("future reduction with mutable task");

    std::atomic_int check{0};
    auto func = [i = int{0}, &check]() mutable {
        i++;
        ++check;
    };

    auto result = stlab::async(stlab::default_executor, [func = std::move(func)]() mutable {
        func();

        return stlab::async(stlab::default_executor,
                            [func = std::move(func)]() mutable { func(); });
    });

    static_cast<void>(stlab::blocking_get(result));

    BOOST_REQUIRE_EQUAL(2, check);
}

BOOST_AUTO_TEST_CASE(future_reduction_with_move_only_mutable_task) {
    BOOST_TEST_MESSAGE("future reduction with move only mutable task");

    auto func = [i = move_only{0}]() mutable {
        i = move_only{i.member() + 1};
        return std::move(i);
    };

    auto result = stlab::async(stlab::default_executor, [func = std::move(func)]() mutable {
        func();

        return stlab::async(stlab::default_executor,
                            [func = std::move(func)]() mutable { return func(); });
    });

    BOOST_REQUIRE_EQUAL(2, (*stlab::blocking_get(std::move(result)).get_try()).member());
}

BOOST_AUTO_TEST_CASE(future_reduction_with_move_only_mutable_void_task) {
    BOOST_TEST_MESSAGE("future reduction with move only mutable void task");

    int check{0};
    auto func = [i = move_only{0}, &check]() mutable {
        i = move_only{i.member() + 1};
        check += i.member();
    };

    auto result = stlab::async(stlab::default_executor, [func = std::move(func)]() mutable {
        func();

        return stlab::async(stlab::default_executor,
                            [func = std::move(func)]() mutable { return func(); });
    });

    static_cast<void>(stlab::blocking_get(std::move(result)));

    BOOST_REQUIRE_EQUAL(3, check);
}

BOOST_AUTO_TEST_CASE(future_equality_tests) {
    BOOST_TEST_MESSAGE("running future equality tests");
    {
        future<int> a;
        future<int> b;
        BOOST_REQUIRE(a == b);
        BOOST_REQUIRE(!(a != b));
    }

    {
        future<void> a;
        future<void> b;
        BOOST_REQUIRE(a == b);
        BOOST_REQUIRE(!(a != b));
    }

    {
        future<move_only> a;
        future<move_only> b;
        BOOST_REQUIRE(a == b);
        BOOST_REQUIRE(!(a != b));
    }

    {
        future<int> a = async(default_executor, [] { return 42; });
        auto b = a;
        BOOST_REQUIRE(a == b);
    }
    {
        future<void> a = async(default_executor, [] {});
        auto b = a;
        BOOST_REQUIRE(a == b);
    }

    {
        future<int> a = async(default_executor, [] { return 42; });
        future<int> b = async(default_executor, [] { return 42; });
        BOOST_REQUIRE(a != b);
    }
    {
        future<void> a = async(default_executor, [] {});
        future<void> b = async(default_executor, [] {});
        BOOST_REQUIRE(a != b);
    }
    {
        future<move_only> a = async(default_executor, [] { return move_only(42); });
        future<move_only> b;
        BOOST_REQUIRE(a != b);
    }
}

BOOST_AUTO_TEST_CASE(future_swap_tests) {
    {
        auto a = package<int(int)>(immediate_executor, [](int a) { return a + 2; });
        auto b = package<int(int)>(immediate_executor, [](int a) { return a + 4; });

        std::swap(a, b);

        a.first(1);
        b.first(2);

        BOOST_REQUIRE_EQUAL(5, *a.second.get_try());
        BOOST_REQUIRE_EQUAL(4, *b.second.get_try());
    }
    {
        int x(0), y(0);
        auto a = package<void(int)>(immediate_executor, [&x](int a) { x = a + 2; });
        auto b = package<void(int)>(immediate_executor, [&y](int a) { y = a + 4; });

        std::swap(a, b);

        a.first(1);
        b.first(2);

        BOOST_REQUIRE_EQUAL(5, y);
        BOOST_REQUIRE_EQUAL(4, x);
    }
    {
        auto a =
            package<move_only(int)>(immediate_executor, [](int a) { return move_only(a + 2); });
        auto b =
            package<move_only(int)>(immediate_executor, [](int a) { return move_only(a + 4); });

        std::swap(a, b);

        a.first(1);
        b.first(2);

        BOOST_REQUIRE_EQUAL(5, a.second.get_try()->member());
        BOOST_REQUIRE_EQUAL(4, b.second.get_try()->member());
    }
}

BOOST_FIXTURE_TEST_SUITE(future_then_void, test_fixture<int>)

BOOST_AUTO_TEST_CASE(future_get_try_refref) {
    BOOST_TEST_MESSAGE("future get_try()&& accessor test");

    sut = async(default_executor, [] {
              return 42;
          }).then([](int) -> int {
                throw test_exception("failure");
            }).recover([](auto&& f) {
        try {
            std::forward<decltype(f)>(f).get_try();
            return 0;
        } catch (const test_exception&) {
            return 42;
        }
    });

    wait_until_future_completed(sut);
    BOOST_REQUIRE_EQUAL(42, *sut.get_try());
}
BOOST_AUTO_TEST_SUITE_END()
