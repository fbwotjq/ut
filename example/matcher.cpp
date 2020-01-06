//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <boost/ut.hpp>
#include <string_view>
#include <tuple>
#include <type_traits>

struct expr {
  const bool result{};
  const std::string str{};

  constexpr explicit operator bool() const { return result; }
  friend auto operator<<(std::ostream& os, const expr& self) -> std::ostream& {
    return (os << self.str);
  }
};

template <class... Ts>
class any_of {
 public:
  constexpr explicit any_of(Ts... ts) : ts_{ts...} {}

  constexpr auto operator==(std::common_type_t<Ts...> t) const {
    return std::apply([t](auto... args) { return eq(t, args...); }, ts_);
  }

 private:
  template <class T, class U, class... TArgs>
  static constexpr auto eq(const T& t, const U& u, const TArgs&... args) {
    using namespace boost::ut;
    if constexpr (sizeof...(args) > 0) {
      return (that % detail::value{u} == t) or eq(t, args...);
    } else {
      return (that % detail::value{u} == t);
    }
  }

  std::tuple<Ts...> ts_;
};

int main() {
  using namespace boost::ut;
  using namespace std::literals::string_view_literals;

  "matcher"_test = [] {
    constexpr auto is_between = [](auto lhs, auto rhs) {
      return matcher([=](auto value) {
        return that % value >= lhs and that % value <= rhs;
      });
    };

    constexpr auto ends_with = matcher([](const auto& arg, const auto& ext) {
      std::stringstream str{};
      str << '(' << arg << " ends with " << ext << ')';
      if (ext.size() > arg.size()) {
        return expr{{}, str.str()};
      }
      return expr{std::equal(ext.rbegin(), ext.rend(), arg.rbegin()),
                  str.str()};
    });

    auto value = 42;
    auto str = "example.test"sv;

    expect(is_between(1, 100)(value) and ends_with(str, ".test"sv));
    expect(not is_between(1, 100)(0));
    expect(any_of{1, 2, 3} == 2 or any_of{42, 43} == 44);
  };
}
