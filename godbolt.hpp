# 1 "include/unconstexpr/unconstexpr.hpp"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "include/unconstexpr/unconstexpr.hpp"





       

# 1 "include/unconstexpr/actions.hpp" 1






       

# 1 "include/unconstexpr/unique_id.hpp" 1





       

namespace unconstexpr {

using id_value = const int *;

template <id_value>
struct id_t {};

template <class T>
struct unique_id {
  static constexpr int value = 0;
  constexpr unique_id(T const &) {}
  constexpr operator id_value() const { return &value; }
};
# 31 "include/unconstexpr/unique_id.hpp"
}
# 10 "include/unconstexpr/actions.hpp" 2
# 1 "include/unconstexpr/utils.hpp" 1





       

namespace unconstexpr {
template <auto V>
struct value_t {
  static constexpr auto value = V;
};

template <class Lambda>
struct value_lambda {
  static constexpr auto value = Lambda{}();
};
}
# 11 "include/unconstexpr/actions.hpp" 2






namespace unconstexpr {

template <class Meta, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
constexpr auto current_index(Meta const& = {}) {
  constexpr auto idx = Meta::template last_index<Id>();
  return idx;
}

template <class Meta, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
constexpr auto current_value(Meta const& = {}) {
  constexpr auto idx = current_index<Meta, Id>();
  return Meta::template value<Id, idx>();
}

namespace increment_hack {






template <class Meta, class ValueInc, id_value Id>
struct increment_info {
  static constexpr auto idx = current_index<Meta, Id>();
  static constexpr auto value = Meta::template value<Id, idx>();
  static constexpr auto nvalue = value + ValueInc::value;
};
}

template <class Meta, bool postincrement = false,
          class ValueInc = value_t<Meta::increment>, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
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

template <class Meta, auto Value, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
constexpr auto set(Meta const& = {}, value_t<Value> const& = {}) {
  constexpr auto idx = current_index<Meta, Id>();
  constexpr auto nidx = idx + Meta::increment;
  using holder = value_t<Value>;
  static_assert(Meta::template set_value<nidx, holder>);
  return Value;
}

template <class Meta, class Lambda, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
constexpr auto set(Meta const& = {}, Lambda const& = {}) {
  constexpr auto idx = current_index<Meta, Id>();
  constexpr auto nidx = idx + Meta::increment;
  using holder = value_lambda<Lambda>;
  static_assert(Meta::template set_value<nidx, holder>);
  return holder::value;
}

}
# 9 "include/unconstexpr/unconstexpr.hpp" 2
# 1 "include/unconstexpr/meta_value.hpp" 1





       




namespace unconstexpr {

template <auto Init = 0, auto Inc = 1, id_value = unique_id([] {})>
struct meta_value {
  template <auto Index>
  struct flagCheck {
    template <id_value Id>
    friend constexpr auto adl(flagCheck, id_t<Id> const &);
  };

  template <auto Index>
  struct flagGet {
    template <id_value Id>
    friend constexpr auto adl(flagGet, id_t<Id> const &);
  };

  template <auto Index, class ValueHolder = value_t<Index>>
  struct writer {
    template <id_value Id>
    friend constexpr auto adl(flagCheck<Index>, id_t<Id> const &) {
      return true;
    }

    template <id_value Id>
    friend constexpr auto adl(flagGet<Index>, id_t<Id> const &) {
      return ValueHolder::value;
    }
  };

  static_assert(sizeof(writer<0, value_t<Init>>));

  template <auto Index, class T, class = decltype(adl(flagCheck<Index>{}, T{}))>
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
    return adl(flagGet<Index>{}, unique_type{});
  }

  static constexpr bool is_meta_var = true;
  static constexpr auto increment = Inc;

  template <auto Index, class ValueHolder>
  static constexpr bool set_value = sizeof(writer<Index, ValueHolder>);
};

}
# 10 "include/unconstexpr/unconstexpr.hpp" 2
# 1 "include/unconstexpr/operators.hpp" 1





       



namespace unconstexpr {

template <class Meta, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
constexpr auto operator*(Meta const &) {
  return current_value<Meta, Id>();
}

template <class Meta, class... Opt, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
constexpr auto operator++(Meta const &, Opt...) {
  constexpr bool is_postincrement = sizeof...(Opt) != 0;
  using inc = value_t<Meta::increment>;
  return increment<Meta, is_postincrement, inc, Id>();
}

template <class Meta, class... Opt, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
constexpr auto operator--(Meta const &, Opt...) {
  constexpr bool is_postincrement = sizeof...(Opt) != 0;
  using inc = value_t<-1 * Meta::increment>;
  return increment<Meta, is_postincrement, inc, Id>();
}

template <class Meta, auto Value, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
constexpr auto operator+=(Meta const &c, value_t<Value> const &) {
  static_assert(sizeof(increment<Meta, false, value_t<Value>, Id>()));
  return c;
}

template <class Meta, class Lambda, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
constexpr auto operator+=(Meta const &c, Lambda const &) {
  static_assert(sizeof(increment<Meta, false, value_lambda<Lambda>, Id>()));
  return c;
}

template <class Meta, class Holder, id_value Id = unique_id([] {}), bool = Meta::is_meta_var>
constexpr auto operator<<(Meta const &c, Holder const &) {
  static_assert(sizeof(set<Meta, Holder, Id>()));
  return c;
}

}
# 10 "include/unconstexpr/unconstexpr.hpp" 2
