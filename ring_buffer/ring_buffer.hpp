#pragma once

#include <cstddef>
#include <vector>

class RingBuffer {
 private:
  size_t capacity_;
  size_t begin_;
  size_t sz_;
  size_t end_;
  std::vector<int> buffer_;

 public:
  explicit RingBuffer(size_t capacity)
      : capacity_(capacity), begin_(0), sz_(0), end_(0), buffer_(capacity, 0) {}
  bool Empty() const { return sz_ == 0; }

  bool TryPush(int element) {
    if (sz_ == capacity_) {
      return false;
    }
    buffer_[end_] = element;
    ++sz_;
    ++end_;
    end_ %= capacity_;
    return true;
  }

  size_t Size() const { return sz_; }

  bool TryPop(int* element) {
    if (Empty()) {
      return false;
    }
    *element = buffer_[begin_];
    ++begin_;
    if (sz_ != 0) {
      --sz_;
    }
    begin_ %= capacity_;
    return true;
  }
};
