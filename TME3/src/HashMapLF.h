#pragma once

#include <vector>
#include <forward_list>
#include <utility>
#include <cstddef>
#include "./ListLF.h"

template<typename K, typename V>
class HashMapLF {
 public:
  // Entry stores a const key and a mutable value
  struct Entry {
    const K key;
    V value;

    Entry(const K &k, const V &v) : key(k), value(v) {}
  };

  using Bucket = std::forward_list<Entry>;
  using Table  = std::vector<ListLF>;

  // Construct with a number of buckets (must be >= 1)
  HashMapLF(std::size_t nbuckets = 4096) : buckets_(nbuckets) {}

  // Increment frequency for the given word
  void incrementFrequency(const K &key, V delta = 1) {
    std::size_t idx = std::hash<K>{}(key) % buckets_.size();
    buckets_[idx].incrementCount(key, delta);
  }

  // Convert table contents to a vector of key/value pairs.
  std::vector<std::pair<K, V>> toKeyValuePairs() const {
    std::vector<std::pair<K, V>> out;
    for (const auto &bucket : buckets_) {
      auto kvs = bucket.toKeyValuePairs();
      for (auto [k, v] : kvs) {
        out.emplace_back(k, v);
      }
    }
    return out;
  }

 private:
  Table buckets_;
};
