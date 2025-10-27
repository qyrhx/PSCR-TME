#pragma once

#include <condition_variable>
#include <mutex>
#include <cstdlib>
#include <cstring>

namespace pr {

// MT safe version of the Queue, non blocking.
// Based on circular buffer.
// Store pointers to T, not T itself ; consumer is responsible for deleting them.
// nullptr is returned by pop if the queue is empty.
// push returns false if the queue is full.
template<typename T>
class Queue {
  T **tab_;
  const size_t allocsize_;
  size_t begin_;
  size_t size_;
  mutable std::mutex m_;
  std::condition_variable cv_prod;
  std::condition_variable cv_cons;
  bool block;

  // fonctions private, sans protection mutex
  bool empty() const { return size_ == 0; }

  bool full() const { return size_ == allocsize_; }

 public:
  Queue(size_t size = 10) : allocsize_(size), begin_(0), size_(0), block(true) {
    tab_ = new T *[size];
    // zero-initialize, not strictly necessary
    memset(tab_, 0, size * sizeof(T *));
  }

  size_t size() const {
    std::unique_lock<std::mutex> lg(m_);
    return size_;
  }

  void setBlock(const bool b) { block = b; }

  T *pop() {
    T *ret;
    {  // C.S
      std::unique_lock<std::mutex> lg(m_);
      while (empty() && block)
        cv_cons.wait(lg);  // Note: when we enter wait, it's a sort of a hole in C.S.
      if (empty() && ! block)
        return nullptr;
      ret = tab_[begin_];
      // cleanup, not strictly necessary
      tab_[begin_] = nullptr;
      size_--;
      begin_ = (begin_ + 1) % allocsize_;
    }
    cv_prod.notify_one();
    return ret;
  }

  bool push(T *elt) {
    {  // C.S
      std::unique_lock<std::mutex> lg(m_);
      while (full() && block)
        cv_prod.wait(lg);
      if (full() && ! block)
        return false;
      tab_[(begin_ + size_) % allocsize_] = elt;
      size_++;
    }
    cv_cons.notify_one();
    return true;
  }

  ~Queue() {
    // ?? lock a priori inutile, ne pas detruire si on travaille encore avec
    for (size_t i = 0; i < size_; i++) {
      auto ind = (begin_ + i) % allocsize_;
      // producer allocated, consumer should delete
      // but we are destroyed with some elements still in the queue
      // we must delete them.
      delete tab_[ind];
    }
    delete[] tab_;
  }
};

} /* namespace pr */
