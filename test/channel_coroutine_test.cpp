#include <boost/test/unit_test.hpp>

#include <stlab/concurrency/channel.hpp>
#include <stlab/concurrency/immediate_executor.hpp>

namespace
{
stlab::receiver<int> get_the_int() { co_return 1; }

}

BOOST_AUTO_TEST_CASE(channel_coroutine_int) {
  BOOST_TEST_MESSAGE("channel coroutine int");

  int value{};
  auto w = get_the_int() | [&value](auto v) { value = v; };


  BOOST_REQUIRE(1 == value);
}

BOOST_AUTO_TEST_CASE(channel_coroutine_int_generator) {
  BOOST_TEST_MESSAGE("channel coroutine int generator");

  int value{};
  auto generator = get_the_int();

    for (auto i : generator) 
    {
      value += co_await i;
    }



  BOOST_REQUIRE(1 == value);
}