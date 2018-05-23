/*
 * intrusive_list.hpp Copyright © 2017 rsw0x
 *
 * Distributed under terms of the MIT license.
 */

#pragma once
#include <cassert>
#include <cstdint>
#include <utility>
#ifdef STUPIDLY_STD_COMPLIANT
#  include <iterator>
#else
namespace std {
struct forward_iterator_tag;
}
#endif

namespace pep {

struct intrusive_node;
namespace details {
template <typename T>
constexpr size_t offset_of(intrusive_node T::*mem_p);
}

struct intrusive_node {
private:
  intrusive_node* next_{nullptr};

public:
  intrusive_node* next() { return next_; }
  intrusive_node* next() const { return next_; }
  void next(intrusive_node* n) { next_ = n; }

  template <typename T>
  T* owner(intrusive_node T::*mem_p) {
    char* this_addr = reinterpret_cast<char*>(this);
    size_t this_offset = details::offset_of(mem_p);
    char* owner_addr = this_addr - this_offset;
    assert(owner_addr <= this_addr);
    assert(reinterpret_cast<std::uintptr_t>(owner_addr) % alignof(T) == 0);
    return reinterpret_cast<T*>(owner_addr);
  }
};

namespace details {
struct list_empty_t {};
template <typename T>
constexpr size_t offset_of(intrusive_node T::*mem_p) {
  union {
    char data[sizeof(T)]{};
    T v;
  } u{};

  // workaround for gcc bug
  bool found = false;
  // https://stackoverflow.com/a/49776289/754018
  void const* const desired_addr = static_cast<void const*>(std::addressof(u.v.*mem_p));
  for (size_t i = 0; i != sizeof(T); ++i) {
    void const* const checking_addr = static_cast<void const*>(std::addressof(u.data[i]));
    if (checking_addr == desired_addr) {
      return i;
    }
  }
  if (!found) {
    throw 0;
  }
  return -1u;
}

} // namespace details

template <typename T, intrusive_node T::*node_ptr, bool isConst = false>
class list_iterator {
public:
  using value_type = std::conditional_t<isConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  using node = intrusive_node;
  node* ptr_;

  list_iterator(node* ptr) : ptr_(ptr) {}

  reference operator*() {
    assert(ptr_ != nullptr);
    return *(ptr_->owner<T>(node_ptr));
  }

  pointer operator->() { return ptr_->owner<T>(node_ptr); }

  list_iterator operator++(int) {
    auto tmp = *this;
    assert(ptr_ != nullptr);
    ptr_ = ptr_->next();
    return tmp;
  }

  list_iterator& operator++() {
    assert(ptr_);
    ptr_ = ptr_->next();
    return *this;
  }

  bool operator==(const list_iterator& rhs) const { return ptr_ == rhs.ptr_; }
  bool operator!=(const list_iterator& rhs) const { return !(*this == rhs); }
};

// TODO: move functions that don't depend on node_ptr to base class to reduce
// template instantiations.

template <typename T, intrusive_node T::*node_ptr>
class islist {
public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = T*;
  using difference_type = std::ptrdiff_t;

  using iterator = pep::list_iterator<T, node_ptr>;
  using const_iterator = pep::list_iterator<T, node_ptr, true>;

  constexpr islist() {}

  bool empty() const { return head_.next() == nullptr; }

  const_reference front() {
    assert(!empty());
    return *(head_.next()->template owner<T>(node_ptr));
  }

  const_reference front() const {
    assert(!empty());
    return *(head_.next()->template owner<T>(node_ptr));
  }

  reference back() {
    assert(!empty());
    return *back_->owner<T>(node_ptr);
  }

  const_reference back() const {
    assert(!empty());
    return *back_->owner<T>(node_ptr);
  }

  difference_type size() const { return size_; }

  void push_back(pointer val) {
    assert(val);
    ++size_;
    node* n = &(val->*node_ptr);
    assert(n->next() == nullptr);
    if (back_) {
      assert(head_.next() != nullptr);
      back_->next(n);
    } else {
      assert(head_.next() == nullptr);
      head_.next(n);
    }
    back_ = n;
  }

  void push_front(pointer val) {
    assert(val);
    ++size_;
    node* n = &(val->*node_ptr);
    assert(n->next() == nullptr);
    n->next(head_.next());
    head_.next(n);
    if (!back_) {
      back_ = n;
    }
  }

  void insert_after(const_pointer pos, pointer val) {
    modification_invariant();
    // Insert the node after pos
    // Check if it's an attempt to insert_after the begin() iterator.
    if (pos == &head_) {
      return push_front(val);
    } else if (pos == back_) {
      return push_back(val);
    }
    ++size_;
    node& n = val->*node_ptr;
    assert(n.next() == nullptr);
    node& prev = pos->*node_ptr;
    // Can't be tail, therefore next must not be null.
    assert(prev.next() != nullptr);
    // set the new node's next to the previous node's next
    n.next(prev.next());
    // update the previous node's next pointer to the inserted node.
    prev.next(&n);
  }

  void insert_after(const_iterator pos, pointer val) {
    // can't insert anything after `end`.
    assert(pos != end());
    insert_after(&(*pos), val);
  }

  void pop_front() {
    modification_invariant();
    --size_;
    node* tmp = head_.next();
    head_.next(tmp->next());
    if (size() == 0) {
      assert(tmp == back_);
      assert(head_.next() == nullptr);
      back_ = nullptr;
    }
    tmp->next(nullptr);
  }

  iterator begin() { return {head_.next()}; }
  const_iterator begin() const { return {head_.next()}; }
  const_iterator cbegin() const { return begin(); }
  iterator end() { return {nullptr}; }
  const_iterator end() const { return {nullptr}; }
  const_iterator cend() const { return end(); }

private:
  void modification_invariant() const {
    assert(!empty());
    assert(size() > 0);
    assert(back_ != nullptr);
    assert(head_.next() != nullptr);
  }

  using node = intrusive_node;
  // Sentinel head
  node head_{};
  node* back_{nullptr};
  difference_type size_{0};
};
} // namespace pep
