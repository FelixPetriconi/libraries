/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#ifndef FUTURE_TEST_HELPER_HPP_
#define FUTURE_TEST_HELPER_HPP_

#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/future.hpp>
#include <stlab/test/model.hpp>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <exception>
#include <stack>
#include <string>
#include <thread>

#include <boost/test/unit_test.hpp>

using lock_t = std::unique_lock<std::mutex>;

namespace future_test_helper {

/**************************************************************************************************/

inline auto make_new_expectation() {
    static int ex{42};
    return ++ex;
}

/**************************************************************************************************/

class test_exception : public std::exception {
    std::string _error;

public:
    test_exception() {}

    explicit test_exception(const std::string& error);

    explicit test_exception(const char* error);

    test_exception& operator=(const test_exception&) = default;
    test_exception(const test_exception&) = default;
    test_exception& operator=(test_exception&&) = default;
    test_exception(test_exception&&) = default;

    virtual ~test_exception() {}

    const char* what() const noexcept override;
};

/**************************************************************************************************/

struct copyable_test_fixture {
    using value_type = int;

    std::size_t _expected_operations{};

    value_type _expectation = make_new_expectation();

    void setup() {}

    void tear_down() {}

    auto argument() const { return _expectation; }

    stlab::task<value_type()> void_to_value_type() const {
        return [_val = _expectation] { return _val; };
    }

    stlab::task<value_type()> void_to_value_type_failing() const {
        return []() -> value_type { throw test_exception("failure"); };
    }

    stlab::task<value_type(value_type)> value_type_to_value_type() const {
        return [](auto val) { return val; };
    }

    stlab::task<value_type(value_type)> combine_value_type_with_my_argument() const {
        return [_argument = argument()](auto val) { return val + _argument; };
    }

    bool verify_result(stlab::future<value_type> result) const {
        auto return_value =
            result.is_ready() && !result.exception() && *result.get_try() == _expectation;
        return return_value;
    }

    bool verify_result_with_combined_argument(stlab::future<value_type> result,
                                              const value_type& argument) const {
        auto return_value = result.is_ready() && !result.exception() &&
                            *result.get_try() == _expectation + argument;
        return return_value;
    }

    bool verify_failure(stlab::future<value_type> result) const {
        auto return_value{false};
        try {
            if (result.exception()) {
                std::rethrow_exception(result.exception());
            }
        } catch (const test_exception& ex) {
            return_value = ex.what() == std::string("failure");
        } catch (...) {
        }
        return return_value;
    }

    template <typename F>
    static stlab::task<void(int)> voidContinuation(F&& f) {
        return [_f = std::forward<F>(f)](int val) { _f(val); };
    }

    template <typename T>
    static auto move_if_moveonly(T&& t) {
        return std::forward<T>(t);
    }
};

/**************************************************************************************************/

struct moveonly_test_fixture {
    using value_type = stlab::move_only;

    std::size_t _expected_operations{};

    value_type _expectation = make_new_expectation();

    void setup() {}

    void tear_down() {}

    auto argument() const { return value_type{_expectation.member()}; }

    stlab::task<value_type()> void_to_value_type() const {
        return [_val = _expectation.member()] { return value_type{_val}; };
    }

    stlab::task<value_type()> void_to_value_type_failing() const {
        return []() -> value_type { throw test_exception("failure"); };
    }

    stlab::task<value_type(value_type)> value_type_to_value_type() const {
        return [](auto val) { return val; };
    }

    bool verify_result(stlab::future<value_type> result) const {
        auto return_value = result.is_ready() && !result.exception() &&
                            (*std::move(result).get_try()).member() == _expectation.member();
        return return_value;
    }

    bool verify_failure(stlab::future<value_type> result) const {
        auto return_value{false};
        try {
            if (result.exception()) {
                std::rethrow_exception(result.exception());
            }
        } catch (const test_exception& ex) {
            return_value = ex.what() == std::string("failure");
        } catch (...) {
        }
        return return_value;
    }

    template <typename F>
    static stlab::task<void(value_type)> voidContinuation(F&& f) {
        return {[_f = std::forward<F>(f)](value_type val) mutable { _f(std::move(val)); }};
    }

    template <class T>
    static constexpr std::remove_reference_t<T>&& move_if_moveonly(T&& t) noexcept {
        return static_cast<std::remove_reference_t<T>&&>(t);
    }
};

/**************************************************************************************************/

struct void_test_fixture {
    using value_type = void;

    std::size_t _expected_operations{};

    int _expectation = make_new_expectation();

    mutable std::vector<int> _results{};

    void setup() {}

    void tear_down() {}

    void argument() {}

    stlab::task<value_type()> void_to_value_type() const {
        return [this] { _results.push_back(_expectation); };
    }

    stlab::task<value_type(value_type)> value_type_to_value_type() const {
        return [this] { _results.push_back(_expectation); };
    }

    stlab::task<value_type()> void_to_value_type_failing() const {
        return [] { throw test_exception("failure"); };
    }

    bool verify_result(stlab::future<void> result) const {
        auto return_value =
            result.is_ready() && !result.exception() && _expected_operations == _results.size() &&
            std::find_if_not(_results.cbegin(), _results.cend(),
                             [this](auto val) { return val == _expectation; }) == _results.cend();
        return return_value;
    }

    bool verify_failure(stlab::future<value_type> result) const {
        auto return_value{false};
        try {
            if (result.exception()) {
                std::rethrow_exception(result.exception());
            }
        } catch (const test_exception& ex) {
            return_value = ex.what() == std::string("failure");
        } catch (...) {
        }
        return return_value;
    }

    template <typename F>
    static stlab::task<void()> voidContinuation(F&& f) {
        return [_f = std::forward<F>(f)]() mutable { _f(); };
    }

    template <typename T>
    static auto move_if_moveonly(T&& t) {
        return std::forward<T>(t);
    }
};

/**************************************************************************************************/

template <typename T>
std::string type_to_string() {
    return std::string(" ") + typeid(typename T::first_type).name() + " " +
           typeid(typename T::second_type::value_type).name();
}

/**************************************************************************************************/

template <typename T>
class executor_wrapper {
    T& _executor;
    std::atomic_size_t _counter{};

public:
    executor_wrapper(T& executor) : _executor(executor) {}

    template <typename U>
    void operator()(U&& u) {
        ++_counter;
        _executor(std::forward<U>(u));
    }

    auto counter() const { return _counter.load(); }
};

/**************************************************************************************************/

template <typename T>
class executor_wrapper_stepped {
    using queue_t = std::deque<stlab::task<void()>>;
    T& _executor;
    std::atomic_size_t _counter{};
    std::mutex _mutex;
    queue_t _tasks;

    auto pop_front_unsafe(queue_t& queue) {
        assert(!queue.empty());
        auto result = std::move(queue.front());
        queue.pop_front();
        return result;
    }

public:
    executor_wrapper_stepped(T& executor) : _executor(executor) {}

    template <typename U>
    void operator()(U&& u) {
        std::unique_lock<std::mutex> guard(_mutex);
        _tasks.emplace_back(std::forward<U>(u));
    }

    void step() {
        stlab::task<void()> task;
        {
            std::unique_lock<std::mutex> guard(_mutex);
            task = pop_front_unsafe(_tasks);
        }
        _executor(std::move(task));
        ++_counter;
    }

    void batch() {
        queue_t tmp;
        {
            std::unique_lock<std::mutex> guard(_mutex);
            std::swap(tmp, _tasks);
        }
        auto wasFilled = !tmp.empty();
        while (!tmp.empty()) {
            auto task = pop_front_unsafe(tmp);
            _executor(std::move(task));
            ++_counter;
        }
        if (wasFilled) batch();
    }

    auto counter() const { return _counter.load(); }
};

/**************************************************************************************************/

namespace impl {

template <typename E, typename F>
void wait_until_this_future_fails(F& f) {
    try {
        while (!f.get_try()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    } catch (const E&) {
    }
}

} // namespace impl

template <typename E, typename... F>
void wait_until_future_fails(F&... f) {
    (void)std::initializer_list<int>{(impl::wait_until_this_future_fails<E>(f), 0)...};
}

/**************************************************************************************************/

template <typename F>
void wait_until_future_ready(F& f) {
    while (!f.is_ready()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

/**************************************************************************************************/

template <typename... F>
void wait_until_future_completed(F&... f) {
    (void)std::initializer_list<int>{(wait_until_future_ready(f), 0)...};
}

/**************************************************************************************************/

template <typename E, typename F>
void check_failure(F& f, const char* message) {
    BOOST_REQUIRE_EXCEPTION(f.get_try(), E, ([_m = message](const auto& e) {
                                return std::string(_m) == std::string(e.what());
                            }));
}

template <typename E, typename F>
void check_failure(F& f, stlab::future_error_codes error_code) {
  BOOST_REQUIRE_EXCEPTION(f.get_try(), E, 
    [_ec = error_code](const auto& e) {
    return e.code() == _ec;
  } );
}


struct test_setup {
    test_setup() {
        // custom_scheduler<0>::reset();
        // custom_scheduler<1>::reset();
    }
};

template <typename T>
struct test_fixture {
    test_fixture() : _task_counter{0} {
        // custom_scheduler<0>::reset();
        // custom_scheduler<1>::reset();
    }

    ~test_fixture() {}

    stlab::future<T> sut;

    template <typename F>
    auto wait_until_future_r_completed(F& f) {
        auto result = f.get_try();
        while (!result) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            result = f.get_try();
        }
        return result;
    }

    void check_valid_future() {}

    void check_valid_future(const stlab::future<T>& f) {
        BOOST_REQUIRE(f.valid() == true);
        BOOST_REQUIRE(!f.exception());
    }

    template <typename F, typename... FS>
    void check_valid_future(const F& f, const FS&... fs) {
        BOOST_REQUIRE(f.valid() == true);
        BOOST_REQUIRE(!f.exception());
        check_valid_future(fs...);
    }

    void wait_until_all_tasks_completed() {
        while (_task_counter.load() != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    std::atomic_int _task_counter{0};
};

struct thread_block_context {
    std::shared_ptr<std::mutex> _mutex;
    std::condition_variable _thread_block;
    std::atomic_bool _go{false};
    std::atomic_bool _may_proceed{false};

    thread_block_context() : _mutex(std::make_shared<std::mutex>()) {}
};

class scoped_decrementer {
    std::atomic_int& _v;

public:
    explicit scoped_decrementer(std::atomic_int& v) : _v(v) {}

    ~scoped_decrementer() { --_v; }
};

template <typename F, typename P>
class test_functor_base : public P {
    F _f;
    std::atomic_int& _task_counter;

public:
    test_functor_base(F f, std::atomic_int& task_counter) :
        _f(std::move(f)), _task_counter(task_counter) {}

    ~test_functor_base() {}

    test_functor_base(const test_functor_base&) = default;
    test_functor_base& operator=(const test_functor_base&) = default;
    test_functor_base(test_functor_base&&) = default;
    test_functor_base& operator=(test_functor_base&&) = default;

    template <typename... Args>
    auto operator()(Args&&... args) const {
        ++_task_counter;
        scoped_decrementer d(_task_counter);
        P::action();
        return _f(std::forward<Args>(args)...);
    }
};

struct null_policy {
    void action() const {}
};

class blocking_policy {
    thread_block_context* _context{nullptr};

public:
    void set_context(thread_block_context* context) { _context = context; }

    void action() const {
        lock_t lock(*_context->_mutex);

        while (!_context->_go || !_context->_may_proceed) {
            _context->_thread_block.wait(lock);
        }
    }
};

class failing_policy {
public:
    void action() const { throw test_exception("failure"); }
};

template <typename F>
auto make_non_blocking_functor(F&& f, std::atomic_int& task_counter) {
    return test_functor_base<F, null_policy>(std::forward<F>(f), task_counter);
}

template <typename F>
auto make_blocking_functor(F&& f, std::atomic_int& task_counter, thread_block_context& context) {
    auto result = test_functor_base<F, blocking_policy>(std::forward<F>(f), task_counter);
    result.set_context(&context);
    return result;
}

template <typename F>
auto make_failing_functor(F&& f, std::atomic_int& task_counter) {
    return test_functor_base<F, failing_policy>(std::forward<F>(f), task_counter);
}
} // namespace future_test_helper

#endif
