// Copyright (c) 2019 Bastien Penavayre
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

namespace unconstexpr {
template <auto V>
struct value_t {
  static constexpr auto value = V;
};

template <class Lambda>
struct value_lambda {
  static constexpr auto value = Lambda{}();
};
}  // namespace unconstexpr