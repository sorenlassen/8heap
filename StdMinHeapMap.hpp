#pragma once

#include "StdMinHeap.hpp"
#include "FirstCompare.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <limits>
#include <new>
#include <utility>
#include <vector>

template<class M, class K = uint16_t, class Compare = std::greater<K>>
class StdMinHeapMap {
 public:
  typedef K key_type;
  typedef M mapped_type;
  typedef std::pair<K, M> entry_type;

 private:
  typedef FirstCompare<K, M, Compare> EntryCompare;
  typedef StdMinHeap<entry_type, EntryCompare> heap_type;

 public:
  typedef typename heap_type::size_type size_type;

  StdMinHeapMap(const Compare& cmp = Compare()) : heap_(cmp) { }
  ~StdMinHeapMap() = default;
  StdMinHeapMap(const StdMinHeapMap&) = delete;
  StdMinHeapMap& operator=(const StdMinHeapMap&) = delete;

  size_type size() const { return heap_.size(); }

  key_type key(size_type index) const { return entry(index).first; }

  entry_type entry(size_type index) const { return heap_[index]; }

  void set_entry(size_type index, entry_type a) { heap_[index] = a; }

  void extend(size_type n) { heap_.extend(n); }

  template<class InputIterator>
  void append_entries(InputIterator begin, InputIterator end) {
    heap_.append(begin, end);
  }

  void pull_up(key_type b, mapped_type t, size_type q) {
    heap_.pull_up(entry_type(b, t), q);
  }

  void push_down(key_type a, mapped_type s, size_type p) {
    heap_.push_down(entry_type(a, s), p);
  }

  void heapify() { heap_.heapify(); }

  bool is_heap() const { return heap_.is_heap(); }

  void push_entry(entry_type e) { heap_.push(e); }

  void push_entry(key_type b, mapped_type t) { heap_.push(entry_type(b, t)); }

  size_type top_index() const { return 0; }

  entry_type top_entry() const { return heap_.top(); }

  entry_type pop_entry() { return heap_.pop(); }

  void sort() { heap_.sort(); }

  bool is_sorted(size_type sz) const { return heap_.is_sorted(sz); }

  void clear() { heap_.clear(); }

 private:
  heap_type heap_;
};
