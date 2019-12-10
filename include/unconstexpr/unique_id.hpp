// Copyright (c) 2019 Bastien Penavayre
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

namespace unconstexpr {
// unique id section
using id_value = const int *;

template <id_value>
struct id_t {};

template <class T>
struct unique_id {
  static constexpr int value = 0;
  constexpr unique_id(T const &) {}
  constexpr operator id_value() const { return &value; }
};

/**
    The following is one of the base of this hack!
    This works because the conversion from unique_id to bool is delayed,
    therefore the lambda is a new one at each instantiation of a template
   depending on that non-type template which leads to 'name' to have a different
   value at each deduction
*/
#define FORCE_UNIQUE(name...) id_value name = unique_id([] {})

}  // namespace unconstexpr