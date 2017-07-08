/*
 * intrusive_list.hpp Copyright © 2017 rsw0x
 *
 * Distributed under terms of the MIT license.
 */

#pragma once
#include <cassert>
#include <utility>
#ifdef STUPIDLY_STD_COMPLIANT
#include <iterator>
#else
namespace std{
  struct forward_iterator_tag;
}
#endif



namespace unstd {

template <typename T>
struct intrusive_node {
private:
  T* const owner_;
  intrusive_node* next_{nullptr};

public:
  constexpr intrusive_node(T* owner)
    : owner_(owner) {}

  intrusive_node* next() { return next_; }
  intrusive_node* next() const { return next_; }
  void next(intrusive_node* n) { next_ = n; }

  T* owner() { return owner_; }
};

template <typename T, bool isConst = false>
class list_iterator {
public:
  using value_type = std::conditional_t<isConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  using node = intrusive_node<T>;
  node* ptr_;

  // list_iterator(pointer ptr, pointer prev_ptr)
  list_iterator(node* ptr)
    : ptr_(ptr) {}

  reference operator*() {
    assert(ptr_ != nullptr);
    return *(ptr_->owner());
  }

  pointer operator->() { return ptr_->owner(); }

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

template <typename T, intrusive_node<T> T::*node_ptr>
class islist {
public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = T*;
  using difference_type = std::ptrdiff_t;

  using iterator = unstd::list_iterator<T>;
  using const_iterator = unstd::list_iterator<T, true>;
  // using reverse_iterator = std::reverse_iterator<iterator>;
  // using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr islist() {}

  bool empty() const { return head_.next() == nullptr; }

  const_reference front() {
    assert(!empty());
    return *(head_.next()->owner());
  }

  const_reference front() const {
    assert(!empty());
    return *(head_.next()->owner());
  }

  reference back() {
    assert(!empty());
    return *back_->owner();
  }

  const_reference back() const {
    assert(!empty());
    return *back_->owner();
  }

  difference_type size() const { return size_; }

  void push_back(pointer val) {
    assert(val);
    ++size_;
    node* n = &(val->*node_ptr);
    assert(n->next() == nullptr);
    if(back_){
      assert(head_.next() != nullptr);
      back_->next(n);
    } else{
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
    if(!back_){
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
    prev.next(n);
  }

  void insert_after(const_iterator pos, pointer val) {
    // can't insert anything after `end`.
    assert(pos != end());
    insert_after(&(*pos), val);
  }

  // void erase_after(iterator it) {
  // modification_invariant();
  //   assert(!empty());
  //   assert(size() > 0);
  //   assert(head_ != nullptr);
  //   assert(tail_ != nullptr);
  //   pointer p = &(*it);
  //   if (p == head) {
  //     return pop_front();
  //   } else if (p == tail) {
  //     return pop_back();
  //   }
  //
  //   node<T>& n = p->*node_ptr;
  //   --size_;
  //   assert(n.next != nullptr && n.prev != nullptr);
  //   (n.prev->*node_ptr).next = n.next;
  //   (n.next->*node_ptr).prev = n.prev;
  //
  //   n.clear();
  // }

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

  using node = intrusive_node<T>;
  // Sentinel head
  node head_{nullptr};
  node* back_{nullptr};
  difference_type size_{0};
};
} // namespace unstd
