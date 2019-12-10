// Copyright (c) 2019 Bastien Penavayre
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT


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

namespace increment_hack {
/**
  To circomvent bug in clang:
  Constexpr variables aren't implicitly captured in lambda expressions if we are in a template context.
  As some of the following constexpr variable might be invalid non-type template parameter,
  we defines them here so that we can access them from a default-construstible lambda and pass it around.
*/
template <class Meta, class ValueInc, id_value Id>
struct increment_info {
  static constexpr auto idx = current_index<Meta, Id>();
  static constexpr auto value = Meta::template value<Id, idx>();
  static constexpr auto nvalue = value + ValueInc::value;
};
}  // namespace increment_hack

template <class Meta, bool postincrement = false,
          class ValueInc = value_t<Meta::increment>, HACKESSOR>
constexpr auto increment(Meta const& = {}) {
  using hack = increment_hack::template increment_info<Meta, ValueInc, Id>;
  constexpr auto idx = hack::idx;
  constexpr auto value = hack::value;
  constexpr auto nvalue = hack::nvalue;
  constexpr auto nidx = idx + Meta::increment;
  constexpr auto lambda_next = [] { return hack::nvalue; };
  using value_holder = value_lambda<decltype(lambda_next)>;
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