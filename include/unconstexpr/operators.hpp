// Copyright (c) 2019 Bastien Penavayre
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "actions.hpp"

namespace unconstexpr {

template <class Meta, HACKESSOR>
constexpr auto operator*(Meta const &) {
  return current_value<Meta, Id>();
}

template <class Meta, class... Opt, HACKESSOR>
constexpr auto operator++(Meta const &, Opt...) {
  constexpr bool is_postincrement = sizeof...(Opt) != 0;
  using inc = value_t<Meta::increment>;
  return increment<Meta, is_postincrement, inc, Id>();
}

template <class Meta, class... Opt, HACKESSOR>
constexpr auto operator--(Meta const &, Opt...) {
  constexpr bool is_postincrement = sizeof...(Opt) != 0;
  using inc = value_t<-1 * Meta::increment>;
  return increment<Meta, is_postincrement, inc, Id>();
}

template <class Meta, auto Value, HACKESSOR>
constexpr auto operator+=(Meta const &c, value_t<Value> const &) {
  static_assert(sizeof(increment<Meta, false, value_t<Value>, Id>()));
  return c;
}

template <class Meta, class Lambda, HACKESSOR>
constexpr auto operator+=(Meta const &c, Lambda const &) {
  static_assert(sizeof(increment<Meta, false, value_lambda<Lambda>, Id>()));
  return c;
}

template <class Meta, class Holder, HACKESSOR>
constexpr auto operator<<(Meta const &c, Holder const &) {
  static_assert(sizeof(set<Meta, Holder, Id>()));
  return c;
}

template <class Meta, auto Value, HACKESSOR>
constexpr auto operator<<(Meta const &c, value_t<Value> const &) {
  static_assert(sizeof(set<Meta, Value, Id>()));
  return c;
}

}  // namespace unconstexpr
