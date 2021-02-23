/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#ifndef STLAB_CONCURRENCY_TRAITS_HPP
#define STLAB_CONCURRENCY_TRAITS_HPP

/**************************************************************************************************/

namespace stlab {

/**************************************************************************************************/

inline namespace v1 {
/**************************************************************************************************/

class avoid_ {};

template <typename T>
using avoid = std::conditional_t<std::is_same<void, T>::value, avoid_, T>;

/**************************************************************************************************/

template <bool...>
struct bool_pack;
template <bool... v>
using all_true = std::is_same<bool_pack<true, v...>, bool_pack<v..., true>>;


/**************************************************************************************************/

template <template <typename> class test, typename T>
struct smart_test : test<T> {};

template <typename T>
using smart_is_copy_constructible = smart_test<std::is_copy_constructible, T>;

template <typename T>
constexpr bool smart_is_copy_constructible_v = smart_is_copy_constructible<T>::value;

/**************************************************************************************************/

template <typename T>
using enable_if_copyable = std::enable_if_t<smart_is_copy_constructible_v<T>>;

template <typename T>
using enable_if_not_copyable = std::enable_if_t<!smart_is_copy_constructible_v<T>>;

/**************************************************************************************************/

// the following implements the C++ standard 17 proposal N4502
#if __GNUC__ < 5 && !defined __clang__
// http://stackoverflow.com/a/28967049/1353549
template <typename...>
struct voider {
    using type = void;
};
template <typename... Ts>
using void_t = typename voider<Ts...>::type;
#else
template <typename...>
using void_t = void;
#endif

struct nonesuch {
    nonesuch() = delete;
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    void operator=(nonesuch const&) = delete;
};

// primary template handles all types not supporting the archetypal Op:
template <class Default, class, template <class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

// the specialization recognizes and handles only types supporting Op:
template <class Default, template <class...> class Op, class... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

template <template <class...> class Op, class... Args>
using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template <template <class...> class Op, class... Args>
using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

/**************************************************************************************************/
/**************************************************************************************************/

// Static if original implementation by Vittorio Romero
// Original implementation is here:
// https://github.com/SuperV1234/meetingcpp2015/blob/master/1_StaticIf/p2.cpp

template <bool TX>
using bool_ = std::integral_constant<bool, TX>;

template <bool TX>
constexpr bool_<TX> bool_v{};

template <typename TPredicate>
auto static_if(TPredicate) noexcept;

namespace detail {

template <typename TFunctionToCall>
struct static_if_result;

template <bool TPredicateResult>
struct static_if_impl;

template <>
struct static_if_impl<false> {
    template <typename TF>
    auto& then_(TF&&) {
        return *this;
    }

    template <typename TF>
    auto else_(TF&& f) noexcept {
        return static_if_result<TF>(std::forward<TF>(f));
    }

    template <typename TPredicate>
    auto else_if_(TPredicate) noexcept {
        return static_if(TPredicate{});
    }

    template <typename... Ts>
    auto operator()(Ts&&...) noexcept {}
};

template <>
struct static_if_impl<true> {
    template <typename TF>
    auto& else_(TF&&) noexcept {
        return *this;
    }

    template <typename TF>
    auto then_(TF&& f) noexcept {
        return static_if_result<TF>(std::forward<TF>(f));
    }

    template <typename TPredicate>
    auto& else_if_(TPredicate) noexcept {
        return *this;
    }
};

template <typename TFunctionToCall>
struct static_if_result : TFunctionToCall {
    template <typename TFFwd>
    static_if_result(TFFwd&& f) noexcept : TFunctionToCall(std::forward<TFFwd>(f)) {}

    template <typename TF>
    auto& else_(TF&&) noexcept {
        return *this;
    }

    template <typename TF>
    auto& then_(TF&&) noexcept {
        return *this;
    }

    template <typename TPredicate>
    auto& else_if_(TPredicate) noexcept {
        return *this;
    }

    // Using `operator()` will call the base `TFunctionToCall::operator()`.
};

} // namespace detail


template <typename TPredicate>
auto static_if(TPredicate) noexcept {
  return detail::static_if_impl < TPredicate{} > {};
} 


}// namespace v1

/**************************************************************************************************/

} // namespace stlab

/**************************************************************************************************/

#endif

/**************************************************************************************************/
