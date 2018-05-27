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
#include <numeric>

struct S {
  int i;
  pep::intrusive_node n;

  friend bool operator==(const S& lhs, const S& rhs) { return &lhs == &rhs; }

  ~S() {}

  S(const S&) = delete;
  S& operator=(const S&) = delete;
  S(S&&) = default;
  S& operator=(S&&) = default;

  S() = default;
};

using sl = pep::intrusive_list<S, &S::n>;

struct empty {};

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

TEST_CASE("types") {
  using C = pep::intrusive_list<S, &S::n>;
  static_assert((std::is_same<C::value_type, S>::value));
  static_assert((std::is_same<C::reference, S&>::value));
  static_assert((std::is_same<C::const_reference, const S&>::value));
  static_assert((std::is_same<C::size_type, std::make_unsigned<C::difference_type>::type>::value));
  static_assert((std::is_same<C::difference_type, std::ptrdiff_t>::value));

  static_assert((std::is_signed<typename C::difference_type>::value));
  static_assert((std::is_unsigned<typename C::size_type>::value));
  static_assert(
    (std::is_same<typename C::difference_type,
                  typename std::iterator_traits<typename C::iterator>::difference_type>::value));
  static_assert(
    (std::is_same<
      typename C::difference_type,
      typename std::iterator_traits<typename C::const_iterator>::difference_type>::value));
}

TEST_CASE("ilist") {
  std::array<S, 10> arr;
  sl sl_;
  REQUIRE(sl_.empty());
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
    REQUIRE(&sl_.back() == &sl_.front());

    sl sl3{};
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

    sl_.pop_back();
    REQUIRE(sl_.empty());
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

TEST_CASE("iterators") {
  S a, b, c;
  sl sl_;
  sl_.push_front(&a);
  sl_.push_front(&b);
  sl_.push_front(&c);
  SUBCASE("prefix") {
    auto it = sl_.begin();
    REQUIRE(&(*it) == &c);
    ++it;
    REQUIRE(&(*it) == &b);
    ++it;
    REQUIRE(&(*it) == &a);
    ++it;
    REQUIRE(it == sl_.end());
    --it;
    REQUIRE(&(*it) == &a);
    --it;
    REQUIRE(&(*it) == &b);
    --it;
    REQUIRE(&(*it) == &c);
  }
  SUBCASE("postfix") {
    auto it = sl_.begin();
    REQUIRE(&(*it) == &c);
    it++;
    REQUIRE(&(*it) == &b);
    it++;
    REQUIRE(&(*it) == &a);
    it++;
    REQUIRE(it == sl_.end());
    it--;
    REQUIRE(&(*it) == &a);
    it--;
    REQUIRE(&(*it) == &b);
    it--;
    REQUIRE(&(*it) == &c);
  }
}

TEST_CASE("non-empty list destructor") {
  S a, b, c;
  {
    sl sl_;
    REQUIRE(sl_.empty());
    sl_.push_front(&a);
    sl_.push_front(&b);
    sl_.push_front(&c);
    REQUIRE(!sl_.empty());
    // REQUIRE(sl_.size() == 3);
    REQUIRE(a.n.is_linked());
    REQUIRE(b.n.is_linked());
    REQUIRE(c.n.is_linked());
  }
  REQUIRE(!a.n.is_linked());
  REQUIRE(!b.n.is_linked());
  REQUIRE(!c.n.is_linked());
}

TEST_CASE("size, iterators") {
  sl sl_;
  const sl& csl = sl_;
  S a{1, {}}, b{2, {}}, c{3, {}};
  REQUIRE(sl_.is_empty());
  // REQUIRE(sl_.size() == 0);
  REQUIRE(std::distance(sl_.begin(), sl_.end()) == 0);
  REQUIRE(std::distance(sl_.cbegin(), sl_.cend()) == 0);
  REQUIRE(sl_.begin() == sl_.end());

  REQUIRE(csl.is_empty());
  // REQUIRE(csl.size() == 0);
  REQUIRE(std::distance(csl.begin(), csl.end()) == 0);
  REQUIRE(std::distance(csl.cbegin(), csl.cend()) == 0);
  REQUIRE(csl.begin() == csl.end());

  sl_.push_front(&a);
  REQUIRE(!sl_.is_empty());
  // REQUIRE(sl_.size() == 1);
  REQUIRE(!csl.is_empty());
  // REQUIRE(csl.size() == 1);
  sl_.push_front(&b);
  REQUIRE(!sl_.is_empty());
  // REQUIRE(sl_.size() == 2);
  REQUIRE(!csl.is_empty());
  // REQUIRE(csl.size() == 2);
  sl_.push_front(&c);
  REQUIRE(!sl_.is_empty());
  // REQUIRE(sl_.size() == 3);
  REQUIRE(!csl.is_empty());
  // REQUIRE(csl.size() == 3);

  REQUIRE(std::distance(sl_.begin(), sl_.end()) == 3);
  REQUIRE(std::distance(sl_.cbegin(), sl_.cend()) == 3);
  REQUIRE(sl_.begin() != sl_.end());
  REQUIRE(std::distance(csl.begin(), csl.end()) == 3);
  REQUIRE(std::distance(csl.cbegin(), csl.cend()) == 3);
  REQUIRE(csl.begin() != csl.end());

  REQUIRE(std::accumulate(sl_.begin(), sl_.end(), 0, [](int a, const S& b) { return a + b.i; }) ==
          6);
  REQUIRE(std::accumulate(csl.begin(), csl.end(), 0, [](int a, const S& b) { return a + b.i; }) ==
          6);

  sl_.pop_front();
  REQUIRE(!sl_.is_empty());
  // REQUIRE(sl_.size() == 2);
  REQUIRE(!csl.is_empty());
  // REQUIRE(csl.size() == 2);
  sl_.pop_front();
  REQUIRE(!sl_.is_empty());
  // REQUIRE(sl_.size() == 1);
  REQUIRE(!csl.is_empty());
  // REQUIRE(csl.size() == 1);
  sl_.pop_front();
  REQUIRE(sl_.is_empty());
  // REQUIRE(sl_.size() == 0);
  REQUIRE(std::distance(sl_.begin(), sl_.end()) == 0);
  REQUIRE(std::distance(sl_.cbegin(), sl_.cend()) == 0);
  REQUIRE(sl_.begin() == sl_.end());

  REQUIRE(csl.is_empty());
  // REQUIRE(csl.size() == 0);
  REQUIRE(std::distance(csl.begin(), csl.end()) == 0);
  REQUIRE(std::distance(csl.cbegin(), csl.cend()) == 0);
  REQUIRE(csl.begin() == csl.end());
  REQUIRE(std::accumulate(sl_.begin(), sl_.end(), 0, [](int a, const S& b) { return a + b.i; }) ==
          0);
  REQUIRE(std::accumulate(csl.begin(), csl.end(), 0, [](int a, const S& b) { return a + b.i; }) ==
          0);
}

TEST_CASE("move ctor") {
  S a{1, {}}, b{2, {}}, c{3, {}};
  sl sl_;
  {
    sl_.push_front(&a);
    sl_.push_front(&b);
    sl_.push_front(&c);
    S d{std::move(a)};
    REQUIRE(!a.n.is_linked());
    REQUIRE(d.n.is_linked());
  }
}
