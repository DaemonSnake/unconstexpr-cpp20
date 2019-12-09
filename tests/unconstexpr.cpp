#include "unconstexpr.hpp"

#include <string_view>
#include <type_traits>

using namespace std::literals;

int main() {
  using namespace unconstexpr;

  constexpr auto v = meta_value{};
  static_assert(*v == 0);

  static_assert(increment(v) == 1);
  static_assert(*v == 1);
  static_assert(++v == 2);
  static_assert(v++ == 2);
  static_assert(*v == 3);
  static_assert(--v == 2);

  v << [] { return 42; };
  static_assert(*v == 42);

  v << [] { return "hello"sv; };
  static_assert(*v == "hello"sv);
}
