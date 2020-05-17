#pragma once

#include <functional>
#include <utility>

// Compares key-mapped pair by the first (key) component.
template<class K, class M, class Compare = std::less<K>>
class FirstCompare {
  Compare cmp_;
 public:
  typedef std::pair<K, M> value_type;
  FirstCompare(const Compare& cmp = Compare()) : cmp_(cmp) { }
  ~FirstCompare() = default;
  constexpr bool operator()(const value_type& a, const value_type& b) const {
    return cmp_(a.first, b.first);
  }
};
