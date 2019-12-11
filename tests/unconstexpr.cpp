#include "unconstexpr/unconstexpr.hpp"

#include <string_view>
#include <type_traits>

using namespace std::literals;

template <class... Args>
struct tuple {
  template <class T>
  constexpr auto operator+(T const &) const {
    return tuple<Args..., T>{};
  }
};

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


  v << [] { return tuple<>{}; };
  v += value_t<42>{};
  v += value_t<nullptr>{};
  v += value_t<42>{};
  using expected = tuple<int, std::nullptr_t, int>;
  static_assert(std::is_same_v<decltype(*v), expected>);
  static_assert(!std::is_same_v<decltype(v), meta_value<>>);
}
