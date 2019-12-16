// Copyright (c) 2019 Bastien Penavayre
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "unique_id.hpp"
#include "utils.hpp"

namespace unconstexpr {

template <auto Init = 0, auto Inc = 1, FORCE_UNIQUE()>
struct meta_value {
  template <auto Index>
  struct flagCheck {
    template <id_value Id>
    friend constexpr auto unconstexpr_adl(flagCheck, id_t<Id> const &);
  };

  template <auto Index>
  struct flagGet {
    template <id_value Id>
    friend constexpr auto unconstexpr_adl(flagGet, id_t<Id> const &);
  };

  template <auto Index, class ValueHolder = value_t<Index>>
  struct writer {
    template <id_value Id>
    friend constexpr auto unconstexpr_adl(flagCheck<Index>, id_t<Id> const &) {
      return true;
    }

    template <id_value Id>
    friend constexpr auto unconstexpr_adl(flagGet<Index>, id_t<Id> const &) {
      return ValueHolder::value;
    }
  };

  static_assert(sizeof(writer<0, value_t<Init>>));

  template <auto Index, class T, class = decltype(unconstexpr_adl(flagCheck<Index>{}, T{}))>
  static constexpr bool exists(int) {
    return true;
  }

  template <int, class>
  static constexpr bool exists(float) {
    return false;
  }

  template <id_value Id, auto Index = 0>
  static constexpr auto last_index() {
    using unique_type = id_t<Id>;
    constexpr bool ok = exists<Index + Inc, unique_type>(int{});
    if constexpr (ok)
      return last_index<Id, Index + Inc>();
    else
      return Index;
  }

  template <id_value Id, auto Index>
  static constexpr auto value() {
    using unique_type = id_t<Id>;
    return unconstexpr_adl(flagGet<Index>{}, unique_type{});
  }

  static constexpr bool is_meta_var = true;
  static constexpr auto increment = Inc;

  template <auto Index, class ValueHolder>
  static constexpr bool set_value = sizeof(writer<Index, ValueHolder>);
};

}  // namespace unconstexpr