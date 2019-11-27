#pragma once

#include "unique_id.hpp"
#include "utils.hpp"

// force each call to be a new template instanciation
// checks that Meta is a meta_var
// simple workaround for lack of concepts
#define HACKESSOR FORCE_UNIQUE(Id), bool = Meta::is_meta_var

namespace unconstexpr {

template <class Meta, HACKESSOR>
constexpr auto current_index(Meta const& = {}) {
  constexpr auto idx = Meta::template last_index<Id>();
  return idx;
}

template <class Meta, HACKESSOR>
constexpr auto current_value(Meta const& = {}) {
  constexpr auto idx = current_index<Meta, Id>();
  return Meta::template value<Id, idx>();
}

template <class Meta, bool postincrement = false,
          class Inc = value_t<Meta::increment>, HACKESSOR>
constexpr auto increment(Meta const& = {}) {
  constexpr auto idx = current_index<Meta, Id>();
  constexpr auto value = Meta::template value<Id, idx>();
  constexpr auto increment = Inc::value;
  constexpr auto nidx = idx + Meta::increment;
  constexpr auto nvalue = value + increment;
  using value_holder = value_t<nvalue>;
  static_assert(Meta::template set_value<nidx, value_holder>);
  if constexpr (postincrement)
    return value;
  else
    return nvalue;
}

template <class Meta, auto Value, HACKESSOR>
constexpr auto set(Meta const& = {}, value_t<Value> const& = {}) {
  constexpr auto idx = current_index<Meta, Id>();
  constexpr auto nidx = idx + Meta::increment;
  using holder = value_t<Value>;
  static_assert(Meta::template set_value<nidx, holder>);
  return Value;
}

template <class Meta, class Lambda, HACKESSOR>
constexpr auto set(Meta const& = {}, Lambda const& = {}) {
  constexpr auto idx = current_index<Meta, Id>();
  constexpr auto nidx = idx + Meta::increment;
  using holder = value_lambda<Lambda>;
  static_assert(Meta::template set_value<nidx, holder>);
  return holder::value;
}

}  // namespace unconstexpr