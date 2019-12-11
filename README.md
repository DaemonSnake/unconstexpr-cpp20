# Unconstexpr for c++20


## Mission statement

Implement mutable constant expression for __gcc__ and __clang__ (ie: breaking type safety).

It requires at least ___gcc 9.1___ and ___clang 9.0___

What to expect:
* `static_assert` and `sizeof` with side effects
* expression with changing value and type at compile-time
* headaches

Simplest example: 
- constexpr global counter à-la __\_\_COUNTER\_\___
```c++
auto counter = unconstexpr::meta_value{};
static_assert(counter++ == 0);
static_assert(counter++ == 1);
static_assert(*counter == 2);
```

More exciting:
```c++
//include + using namespace std::literals ...

auto var = unconstexpr::meta_var{};

var << []{ return "hello"sv; };
static_assert(*var == "hello"sv);
static_assert(std::is_same_v<decltype(*var), std::string_view>);

var << unconstexpr::value_v<52>{};
static_assert(*var == 52);
static_assert(std::is_same_v<decltype(*var), int>);
```

Weirder:
```c++
using c = unconstexpr::meta_var<>;
static_assert(sizeof(set(c, []{ return "hello"sv; })));
static_assert(get(c) == "hello"sv);

static_assert(std::is_same_v<c, unconstexpr::meta_var<>>, "Will fail!");
```

## Disclaimer

This project probably isn't valid __c++__.
If you have insights about its validity I'd be glad to chat about it.

During the development phase this project caused hard __segfaults__ from __gcc__ and __clang__ alike.
Nothing guarantees that the current behaviour will last.
As both compiler don't have __c++20__ fully implemented yet some features that would greatly simplify
the writing of this library weren't used.

On a lesser note, I've wrote this in a hurry and might contain typos.
If some sections aren't clair or I'm factually incorrect don't hesitate to correct me.
Thanks!

## [bolt is Godbolt is Godbolt is God]

You can play with this project by adding the following include in godbolt:
```c++
#include "https://raw.githubusercontent.com/DaemonSnake/unconstexpr-cpp20/godbolt/godbolt.hpp"
```
It is automatically generated at each push with a github action.

## How to use

TODO, you can check the test folder for now

## Juicy Hacks

This project relies on a bunch of very weird, found-for-the-occasion, hacks.

This section will try to document and explain how they work.

### Unique instantiation template
One of the most important new hack here is the following:
```c++
template<auto X>
struct unique_value {
   static constexpr int value = 0;
};

template<auto X = []{}>
constexpr auto unique_return() {
   return &unique_value<X>::value;
}

static_assert(unique_return() != unique_return());
```
This is the central new hack!
In the project it is combined with __sfiae__ and __friend injection__ (see below).

At each template instantiation of `unique_return()` __X__ will have a new type.

Thanks to this any expression inside `unique_return()` that uses __X__ will need to be reevaluated.

Therefor if the state of the compiler changed since the last instantiation and the we used __X__ in a dependent expression (ex: check that a function is defined) the result of the function might change.

As `lambdas in unevaluated context` isn't implemented yet in clang the following is the work-around used instead

```c++
template<class T>
struct unique_value {
   constexpr unique_value(T const &) {}
   static constexpr int value = 0;
   constexpr operator auto() const { return &value; }
};

template<const int *Id = unique_value([]{})>
constexpr auto unique_return() {
  return Id;
}

static_assert(unique_return() != unique_return());
```

This is a pretty backwards solution that I found by messing around.

From what I can guess the conversion operator to `const int *` of `unique_value<T>` is delayed to when the default parameter is deduced (ie: during the template instantiation).

Thanks to this the lambda seems to be instantiated there and be unique at each instantiation.

Weirdly enough both clang and gcc allow this.

Look at the __WEIRDNESS__ section below for caveats.

### New friend injection

Beforehand, friend injection is weird trick and is also know as the [Barton–Nackman trick](https://en.wikipedia.org/wiki/Barton%E2%80%93Nackman_trick).
If your interested I recommend reading [Filip Roséen original blog]('http://b.atch.se/posts/constexpr-counter/') on how to implement mutable constexpr expressions and how it was used.

It's thanks to this trick that we can control through template instantiation the definition or declaration of a globaly defined function.

The new version looks like this:

```c++
template<int I>
struc flagCheck
{
   template<class Id>
   friend constexpr auto adl(flagCheck, Id);
};

template<int I>
struct flagGet
{
   template<class Id>
   friend constexpr auto adl(flagGet, Id);
};

template<int I, auto V>
struct writer
{
   template<class Id>
   friend constexpr auto adl(flagCheck<I>, Id) { return 0; }

   template<class Id>
   friend constexpr auto adl(flagGet<I>, Id) { return V; }
};
```

There's a lot to unpack here.
New characteristics:
* auto return type
* friend function now templated on an Id
* two flag classes instead of one. Labeled check and get

First the auto return type:

As deducing the return type isn't possible before the definition, this allows the following:
```c++
template<int I, class Id, class = decltype(adl(flagCheck<I>{}, Id{}))>
constexpr bool exist(int) { return true; }

template<int I, class Id>
constexpr bool exist(float) { return false; }

static_assert(exist<0, type_0>(0) == false);
static_assert(sizeof(writer<I, 5>));
static_assert(exist<0, type_1>(0) == true);
```

Then the Id template parameter:

You might have guessed it but Id here is actually a type that is dependent on our __Unique instantiation template__ trick.
Thanks to this each function that uses said trick and transmit it to our `exists()` function will be guaranteed to be actually checked and have a changing result.

```c++
template<const int *> struct id_t {};
template<int I = 0, const int *Id = unique_value([]{})>
constexpr int iterate() {
   if constexpr (exists<I, id_t<Id>>(0)) return iterate<I+1, Id>();
   else return I;
}

```

Finally two flag classes instead of one.:

This is to prevent clang to segfault. Yep...
I will go into further details if people are interested.

### Lambda constexpr encapsulation

```c++
template<class T>
void func(T const &)
{
    constexpr auto value = T{}();
}

void fn()
{
     func([]{ return 3.14; }); //float, double aren't allowed as non-type template parameter
     func([]{ return some_constexpr_fn(); });
     constexpr auto x = some_fn(...);
     func([]{ return x; });
}
```
This trick exploits lambdas' default-constructivity (c++20) and constexpr-ness (c++17).
This allows to hide inside a type a value that wouldn't be allowed as a non-type template parameter but is still constexpr.
For instance: float, class with private member (c++20), user defined class (pre c++20), etc.

## Weirdness

If you look at the source you will find that the main object `meta_value` only has statically defined operators and that the __Unique instantiation template__ isn't used in any nested templated context.

The reason of this is that It doesn't work in a nested template context.
In gcc the deduction fails and in clang it caused a segfault as of clang 9.0.0 but seems to have been fixed in trunk.
You can find an simple example illustrating this [here](https://godbolt.org/z/fYKSEd).
Sadly `lambda in unevaluated context` doesn't seem to fix this as we can see with [this gcc example](https://godbolt.org/z/PqvNMv)


