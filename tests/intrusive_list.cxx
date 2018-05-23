/*
 * intrusive_list.cxx
 * Copyright© 2017 rsw0x
 *
 * Distributed under terms of the MIT license.
 */

#include "../intrusive_list.hpp"
#include "doctest.h"
#include <algorithm>
#include <array>
#include <memory>

struct S {
  pep::intrusive_node n;
  pep::intrusive_node n2;

  friend bool operator==(const S& lhs, const S& rhs) { return &lhs == &rhs; }
};

using sl = pep::islist<S, &S::n>;
using sl2 = pep::islist<S, &S::n2>;

struct empty {};

auto constexpr foob(pep::intrusive_node S::*mem_p) {
  union {
    empty e{};
    S s;
  } s{};
  pep::intrusive_node& n = s.s.*mem_p;
  char* c1 = static_cast<char*>(static_cast<void*>(std::addressof(s)));
  char* c2 = static_cast<char*>(static_cast<void*>(std::addressof(n)));
  auto i = c2 - c1;
  return i;
}

template <typename iter>
void reverse(iter first, iter last) {
  while (true)
    if (first == last || first == --last)
      return;
    else {
      std::printf("SWAP BEGIN.\n");
      std::swap(*first, *last);
      std::printf("SWAP END.\n");
      // std::iter_swap(first, last);
      ++first;
    }
}

TEST_CASE("ilist") {
  sl sl_;
  S s;
  constexpr auto _p = pep::details::offset_of(&S::n);
  REQUIRE(sl_.empty());
  std::array<S, 10> arr;
  SUBCASE("1") {
    S s;
    REQUIRE(sl_.empty());
    sl_.push_back(&s);
    REQUIRE(!sl_.empty());
    sl_.pop_front();
    REQUIRE(sl_.empty());
    sl_.push_back(&s);
    REQUIRE(!sl_.empty());
    sl_.pop_front();
    REQUIRE(sl_.empty());
    sl_.push_front(&s);
    REQUIRE(!sl_.empty());
    sl_.pop_front();
    REQUIRE(sl_.empty());
    sl_.push_front(&s);
    auto& _ = sl_.back();
    REQUIRE(&sl_.back() == &sl_.front());
    (void)_;
    // const sl sl2{};
    // (void)sl2;
    //
    constexpr sl sl3{};
    for (auto& v : sl_) {
      (void)v;
    }
    for (auto& v : sl3) {
      (void)v;
    }

    int i = 0;
    for (auto& e : sl_) {
      (void)e;
      ++i;
    }
    REQUIRE(i == 1);
  }
  SUBCASE("2") {
    REQUIRE(std::distance(sl_.begin(), sl_.end()) == 0);
    REQUIRE(std::distance(sl_.cbegin(), sl_.cend()) == 0);
    S s;
    sl_.push_front(&s);
    REQUIRE(std::distance(sl_.begin(), sl_.end()) == 1);
    REQUIRE(std::distance(sl_.cbegin(), sl_.cend()) == 1);
    sl_.pop_front();
    REQUIRE(std::distance(sl_.begin(), sl_.end()) == 0);
    REQUIRE(std::distance(sl_.cbegin(), sl_.cend()) == 0);
    REQUIRE(sl_.empty());
  }
  SUBCASE("3") {
    std::for_each(arr.begin(), arr.end(), [&](S& s) { sl_.push_back(&s); });
    REQUIRE(sl_.begin() == sl_.begin());
    REQUIRE(sl_.begin() != ++sl_.begin());

    REQUIRE(sl_.end() == sl_.end());

    REQUIRE((++sl_.begin()).ptr_ != nullptr);
  }
  SUBCASE("4") {
    std::for_each(arr.begin(), arr.end(), [&](S& s) { sl_.push_back(&s); });
    REQUIRE(std::equal(sl_.begin(), sl_.end(), arr.begin()));
  }
  SUBCASE("5") {
    std::for_each(arr.begin(), arr.end(), [&](S& s) { sl_.push_back(&s); });
    std::for_each(sl_.begin(), sl_.end(), [](S&) {});
    for (auto& s : sl_) {
      (void)s;
    }
  }
}
