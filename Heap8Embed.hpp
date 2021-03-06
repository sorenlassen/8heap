#pragma once

#include "minpos.h"
#include "v128.h"
#include "align.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <array>
#include <limits>
#include <new>
#include <utility>
#include <vector>

template<class S> class Heap8Embed {
 public:
  typedef std::uint16_t key_type;
  typedef S mapped_type;
  typedef std::pair<key_type, S> entry_type;
  typedef std::size_t size_type;

 private:
  static constexpr key_type kMax = std::numeric_limits<key_type>::max();
  static constexpr size_type kArity = 8;
  static constexpr size_type kSizeMax = align_down(std::numeric_limits<size_type>::max(), kArity);

  static size_type parent(size_type q) { return (q / kArity) - 1; }
  static size_type children(size_type p) { return (p + 1) * kArity; }

  static_assert(sizeof(v128) == kArity * sizeof(key_type));

  struct node {
    typedef std::array<mapped_type, kArity> shadow_vector;
    node() = default;
    node(v128 vals) : values(vals), shadows{0,0,0,0,0,0,0,0} { }
    v128 values;
    shadow_vector shadows;
    minpos_type minpos() const { return ::minpos(values.mm); }
  };
  static_assert(sizeof(node) == kArity * sizeof(entry_type));

 public:
  Heap8Embed() : size_(0) { }
  ~Heap8Embed() = default;
  Heap8Embed(const Heap8Embed&) = delete;
  Heap8Embed& operator=(const Heap8Embed&) = delete;

  size_type size() const { return size_; }

  key_type key(size_type index) const {
    return nod(index)->values.values[index % kArity];
  }

  entry_type entry(size_type index) const {
    node const* n = nod(index);
    size_type i = index % kArity;
    return std::make_pair(n->values.values[i], n->shadows[i]);
  }

  void set_entry(size_type index, entry_type a) {
    node* n = nod(index);
    size_type i = index % kArity;
    n->values.values[i] = a.first;
    n->shadows[i] = a.second;
  }

  void extend(size_type n) {
    if (n > kSizeMax - size_) throw_bad_alloc();
    size_type new_size = size_ + n;
    if (new_size > kArity * nodes_.size()) {
      static_assert(std::numeric_limits<typename nodes_type::size_type>::max() >=
                    std::numeric_limits<size_type>::max() / kArity);
      // Smallest new_nodes_size s.t. size <= kArity * new_nodes_size.
      size_type new_nodes_size = align_up(new_size, kArity) / kArity;
      nodes_.resize(new_nodes_size);
      // Pad values in case new_size < kArity * new_nodes_size
      nodes_.back().values = kV128Max;
    }
    size_ = new_size;
  }

  template<class InputIterator>
  void append_entries(InputIterator begin, InputIterator end) {
    while (begin != end) {
      if (size_ == kArity * nodes_.size()) nodes_.emplace_back(kV128Max);
      node* n = nod(size_);
      size_type i = size_ % kArity;
      n->values.values[i] = begin->first;
      n->shadows[i] = begin->second;
      ++begin;
      ++size_;
    }
  }

  void pull_up(key_type b, mapped_type t, size_type q) {
    assert(q < size_);
    node* n = nod(q);
    size_type j = q % kArity;
    while (q >= kArity) {
      size_type p = parent(q);
      node* m = nod(p);
      size_type i = p % kArity;
      key_type a = m->values.values[i];
      if (a <= b) break;
      n->values.values[j] = a;
      n->shadows[j] = m->shadows[i];
      q = p;
      n = m;
      j = i;
    }
    n->values.values[j] = b;
    n->shadows[j] = t;
  }

  void push_down(key_type a, mapped_type s, size_type p) {
    assert(p < size_);
    node* m = nod(p);
    size_type i = p % kArity;
    while (true) {
      size_type q = children(p);
      if (q >= size_) break;
      node* n = nod(q);
      minpos_type x = n->minpos();
      key_type b = minpos_min(x);
      if (a <= b) break;
      size_type j = minpos_pos(x);
      m->values.values[i] = b;
      m->shadows[i] = n->shadows[j];
      p = q + j;
      m = n;
      i = j;
    }
    m->values.values[i] = a;
    m->shadows[i] = s;
  }

  void heapify() {
    if (size_ <= kArity) return;
    size_type q = align_down(size_ - 1, kArity);

    // The first while loop is an optimization for the bottom level of the heap,
    // inlining the call to heap_push_down which is trivial at the bottom level.
    // Here "bottom level" means the 8-vectors without children.
    size_type r = parent(q);
    while (q > r) {
      node* n = nod(q);
      minpos_type x = n->minpos();
      key_type b = minpos_min(x);
      size_type p = parent(q);
      node* m = nod(p);
      size_type i = p % kArity;
      key_type a = m->values.values[i];
      if (b < a) {
        size_type j = minpos_pos(x);
        mapped_type s = m->shadows[i];
        m->shadows[i] = n->shadows[j];
        m->values.values[i] = b;
        // The next line inlines push_down(a, s, q + j)
        // with the knowledge that children(q + j) >= size_.
        n->values.values[j] = a;
        n->shadows[j] = s;
      }
      q -= kArity;
    }

    while (q > 0) {
      node* n = nod(q);
      minpos_type x = n->minpos();
      key_type b = minpos_min(x);
      size_type p = parent(q);
      node* m = nod(p);
      size_type i = p % kArity;
      key_type a = m->values.values[i];
      if (b < a) {
        size_type j = minpos_pos(x);
        mapped_type s = m->shadows[i];
        m->shadows[i] = n->shadows[j];
        m->values.values[i] = b;
        push_down(a, s, q + j);
      }
      q -= kArity;
    }
  }

  bool is_heap() const {
    if (size_ <= kArity) return true;
    size_type q = align_down(size_ - 1, kArity);
    while (q > 0) {
      minpos_type x = nod(q)->minpos();
      key_type b = minpos_min(x);
      size_type p = parent(q);
      key_type a = key(p);
      if (b < a) return false;
      q -= kArity;
    }
    return true;
  }

  void push_entry(entry_type e) {
    push_entry(e.first, e.second);
  }

  void push_entry(key_type b, mapped_type t) {
    if (size_ == kArity * nodes_.size()) nodes_.emplace_back(kV128Max);
    size_++;
    pull_up(b, t, size_ - 1);
  }

  size_type top_index() const {
    assert(size_ > 0);
    node const* n = nod(0);
    minpos_type x = n->minpos();
    return minpos_pos(x);
  }

  entry_type top_entry() const {
    assert(size_ > 0);
    node const* n = nod(0);
    minpos_type x = n->minpos();
    return std::make_pair(minpos_min(x), n->shadows[minpos_pos(x)]);
  }

  entry_type pop_entry() {
    assert(size_ > 0);
    node* n = nod(0);
    minpos_type x = n->minpos();
    size_type q = minpos_pos(x);
    entry_type e(minpos_min(x), n->shadows[q]);
    size_type p = size_ - 1;
    node* m = nod(p);
    size_type i = p % kArity;
    key_type a = m->values.values[i];
    m->values.values[i] = kMax;
    size_--;
    if (q != size_) {
      mapped_type s = m->shadows[i];
      push_down(a, s, q);
    }
    return e;
  }

  void sort() {
    v128 values = kV128Max;
    size_type x = size_;
    size_type i = x % kArity;
    x -= i;
    if (i != 0) {
      node* n = nod(x);
      do {
        --i;
        entry_type e = pop_entry();
        values.values[i] = e.first;
        n->shadows[i] = e.second;
      } while (i > 0);
      n->values = values;
    }
    while (x > 0) {
      x -= kArity;
      node* n = nod(x);
      for (size_type j = kArity; j > 0; --j) {
        entry_type e = pop_entry();
        values.values[j - 1] = e.first;
        n->shadows[j - 1] = e.second;
      }
      n->values = values;
    }
  }

  bool is_sorted(size_type sz) const {
    if (sz == 0) return true;
    key_type v = key(0);
    for (size_type p = 1; p < sz; ++p) {
      key_type w = key(p);
      if (v < w) return false;
      v = w;
    }
    return true;
  }

  void clear() {
    nodes_.clear();
    nodes_.shrink_to_fit(); // to match heap_clear(heap*)
    size_ = 0;
  }

 private:
  [[noreturn]] static void throw_bad_alloc() {
    std::bad_alloc exception;
    throw exception;
  }

  typedef std::vector<node> nodes_type;

  node* nod(size_type q) { return &nodes_[q / kArity]; }
  node const* nod(size_type q) const { return &nodes_[q / kArity]; }

  nodes_type nodes_;
  size_type size_;
};
