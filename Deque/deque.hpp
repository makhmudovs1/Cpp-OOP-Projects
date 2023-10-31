#include <iostream>
#include <vector>

const int kBatchSize = 24;

template <typename T, typename Allocator = std::allocator<T>>
class Deque {
 private:
  std::vector<T*> deque_;
  int begin_i_ = 0;
  int begin_j_ = 0;
  int length_ = 0;
  Allocator allocator_;

  void clear() {
    int size = 0;
    int elem = begin_j_;
    for (size_t i = begin_i_; i < deque_.size(); ++i) {
      for (int j = elem; j < kBatchSize; ++j) {
        if (size == length_) {
          break;
        }
        ++size;
        traits::destroy(allocator_, deque_[i] + j);
      }
      elem = 0;
    }
    for (size_t i = 0; i < deque_.size(); ++i) {
      traits::deallocate(allocator_, deque_[i], kBatchSize);
    }
  }

  void front_alloc() {
    try {
      std::pair<int, int> end = index_element();
      int size = int(deque_.size());
      for (int i = 0; i < 2 * size; ++i) {
        T* other = traits::allocate(allocator_, kBatchSize);
        deque_.push_back(other);
      }
      int elem = kBatchSize;
      for (int i = size; i < size + end.first - begin_i_ + 1; ++i) {
        std::swap(deque_[i], deque_[begin_i_ + i - size]);
      }
      begin_i_ = size;
    } catch (...) {
      clear();
      throw;
    }
  }

  void back_alloc() {
    try {
      int size = int(deque_.size());
      for (int i = 0; i < 2 * size; ++i) {
        T* other = traits::allocate(allocator_, kBatchSize);
        deque_.push_back(other);
      }
      for (int i = size + begin_i_; i < 2 * size; ++i) {
        std::swap(deque_[i], deque_[begin_i_ + i - size - begin_i_]);
      }
      begin_i_ += size;
    } catch (...) {
      clear();
      throw;
    }
  }

  std::pair<int, int> index_element() const {
    std::pair<int, int> end;
    end.first = begin_i_;
    int len = length_;
    if (len >= kBatchSize - begin_j_) {
      len -= (kBatchSize - begin_j_);
      end.first += 1;
      end.second = len % kBatchSize;
    } else {
      end.second = begin_j_ + len;
    }
    end.first += len / kBatchSize;
    return end;
  }

 public:
  using traits = std::allocator_traits<Allocator>;

  Deque(const Allocator& alloc = Allocator()) : allocator_(alloc), deque_(1) {
    deque_[0] = traits::allocate(allocator_, kBatchSize);
  }

  Deque(const Deque& other)
      : begin_i_(other.begin_i_),
        begin_j_(other.begin_j_),
        length_(other.length_),
        deque_(other.deque_.size()),
        allocator_(
            traits::select_on_container_copy_construction(other.allocator_)) {
    for (size_t i = 0; i < other.deque_.size(); ++i) {
      deque_[i] = traits::allocate(allocator_, kBatchSize);
    }
    int elem = begin_j_;
    int size = 0;
    for (size_t i = begin_i_; i < other.deque_.size(); ++i) {
      if (int(i) > begin_i_) {
        elem = 0;
      }
      if (size == length_) {
        return;
      }
      for (int j = elem; j < kBatchSize; ++j) {
        try {
          if (size == length_) {
            return;
          }
          ++size;
          traits::construct(allocator_, deque_[i] + j, *(other.deque_[i] + j));
        } catch (...) {
          for (int ii = 0; ii < j; ++ii) {
            traits::destroy(allocator_, deque_[i] + ii);
          }
          for (size_t ii = 0; ii < deque_.size(); ++ii) {
            traits::deallocate(allocator_, deque_[ii], kBatchSize);
          }
          throw;
        }
      }
    }
  }

  Deque(size_t count, const Allocator& alloc = Allocator())
      : allocator_(alloc), deque_(count / kBatchSize + 1), length_(count) {
    try {
      int size = 0;
      for (size_t i = 0; i <= count / kBatchSize; ++i) {
        deque_[i] = traits::allocate(allocator_, kBatchSize);
        for (int j = 0; j < kBatchSize; ++j) {
          try {
            ++size;
            traits::construct(allocator_, deque_[i] + j);
            if (size == int(count)) {
              return;
            }
          } catch (...) {
            for (int ii = 0; ii < j; ++ii) {
              traits::destroy(allocator_, deque_[i] + ii);
            }
            for (size_t ii = 0; ii < deque_.size(); ++ii) {
              traits::deallocate(allocator_, deque_[ii], kBatchSize);
            }
            deque_.clear();
            throw;
          }
        }
      }
    } catch (...) {
      clear();
      throw;
    }
  }
  Deque(size_t count, const T& value, const Allocator& alloc = Allocator())
      : allocator_(alloc), length_(count), deque_(count / kBatchSize + 1) {
    try {
      int elem = kBatchSize;
      for (size_t i = 0; i <= count / kBatchSize; ++i) {
        deque_[i] = traits::allocate(allocator_, kBatchSize);
        if (i == count / kBatchSize) {
          elem = count % kBatchSize;
        }
        for (int j = 0; j < elem; ++j) {
          traits::construct(allocator_, deque_[i] + j, value);
        }
      }
    } catch (...) {
      clear();
      throw;
    }
  }

  Deque(Deque&& other)
      : begin_i_(other.begin_i_),
        begin_j_(other.begin_j_),
        length_(other.length_),
        allocator_(other.allocator_),
        deque_(std::move(other.deque_)) {
    other.begin_i_ = 0;
    other.begin_j_ = 0;
    other.length_ = 0;
  }

  Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator())
      : allocator_(alloc), deque_(init.size() / kBatchSize + 1) {
    for (size_t i = 0; i <= init.size() / kBatchSize; ++i) {
      deque_[i] = traits::allocate(allocator_, kBatchSize);
    }
    for (auto i = init.begin(); i < init.end(); ++i) {
      std::pair<int, int> end = index_element();
      traits::construct(allocator_, deque_[end.first] + end.second, *(i));
      ++length_;
    }
  }

  Deque& operator=(const Deque& other) {
    Deque other1(other);
    if (traits::propagate_on_container_copy_assignment::value) {
      other1.allocator_ = other.allocator_;
    }
    std::swap(allocator_, other1.allocator_);
    std::swap(deque_, other1.deque_);
    std::swap(length_, other1.length_);
    std::swap(begin_i_, other1.begin_i_);
    std::swap(begin_j_, other1.begin_j_);
    return *this;
  }

  Deque& operator=(Deque&& other) noexcept {
    Deque other1(std::move(other));
    if (traits::propagate_on_container_copy_assignment::value) {
      other1.allocator_ = other.allocator_;
    }
    std::swap(allocator_, other1.allocator_);
    std::swap(deque_, other1.deque_);
    std::swap(length_, other1.length_);
    std::swap(begin_i_, other1.begin_i_);
    std::swap(begin_j_, other1.begin_j_);
    return *this;
  }

  size_t size() const { return length_; }

  bool empty() { return length_ == 0; }

  T& operator[](int elem) {
    CommonIterator<false> index(begin_i_, begin_j_);
    index += elem;
    return *(deque_[index.beg_i] + index.beg_j);
  }

  const T& operator[](int elem) const {
    CommonIterator<false> index(begin_i_, begin_j_);
    index += elem;
    return *(deque_[index.beg_i] + index.beg_j);
  }

  T& at(size_t elem) {
    if (elem >= size()) {
      throw std::out_of_range("...");
    }
    CommonIterator<false> index(begin_i_, begin_j_);
    index += elem;
    return *(deque_[index.beg_i] + index.beg_j);
  }

  const T& at(size_t elem) const {
    if (size_t(elem) >= (*this).size()) {
      throw std::out_of_range("...");
    }
    CommonIterator<false> index(begin_i_, begin_j_);
    index += elem;
    return *(deque_[index.beg_i] + index.beg_j);
  }

  Allocator get_allocator() { return allocator_; }

  void push_back(const T& value) { emplace_back(value); }

  void push_back(T&& value) { emplace_back(std::forward<T>(value)); }

  template <class... Args>
  void emplace_back(Args&&... args) {
    std::pair<int, int> end = index_element();
    traits::construct(allocator_, deque_[end.first] + end.second,
                      std::forward<Args>(args)...);
    if (end.second == kBatchSize - 1) {
      if (end.first == int(deque_.size() - 1)) {
        back_alloc();
      }
    }
    ++length_;
  }

  void pop_back() {
    std::pair<int, int> end = index_element();
    end.first -= static_cast<int>(end.second == 0);
    end.second -= static_cast<int>(end.second != 0);
    if (end.second == 0) {
      end.second = kBatchSize - 1;
    }
    traits::destroy(allocator_, deque_[end.first] + end.second);
    --length_;
  }

  void push_front(const T& value) { emplace_front(value); }

  void push_front(T&& value) { emplace_front(std::forward<T>(value)); }

  template <class... Args>
  void emplace_front(Args&&... args) {
    if (begin_j_ != 0) {
      --begin_j_;
      traits::construct(allocator_, deque_[begin_i_] + begin_j_,
                        std::forward<Args>(args)...);
      ++length_;
      return;
    }
    if (begin_i_ == 0) {
      front_alloc();
    }
    --begin_i_;
    begin_j_ = kBatchSize - 1;
    traits::construct(allocator_, deque_[begin_i_] + begin_j_,
                      std::forward<Args>(args)...);
    ++length_;
  }

  void pop_front() {
    traits::destroy(allocator_, deque_[begin_i_] + begin_j_);
    begin_i_ += static_cast<int>(begin_j_ == kBatchSize - 1);
    begin_j_ += static_cast<int>(begin_j_ != kBatchSize - 1);
    if (begin_j_ == kBatchSize - 1) {
      begin_j_ = 0;
    }
    --length_;
  }

  template <bool IsConst>
  struct CommonIterator {
    operator CommonIterator<true>() const {
      return CommonIterator<true>(beg_i, beg_j);
    }
    using difference_type = std::ptrdiff_t;
    using value_type = typename std::conditional<IsConst, const T, T>::type;
    using pointer = typename std::conditional<IsConst, const T*, T*>::type;
    using reference = typename std::conditional<IsConst, const T&, T&>::type;
    using iterator_category = std::random_access_iterator_tag;

   private:
    std::conditional_t<IsConst, const std::vector<T*>, std::vector<T*>> vec_;

   public:
    int beg_i = 0;
    int beg_j = 0;

    CommonIterator(int begini, int beginj) {
      beg_i = begini;
      beg_j = beginj;
    }
    CommonIterator(std::vector<T*> vec, int begini, int beginj) {
      vec_ = vec;
      beg_i = begini;
      beg_j = beginj;
    }

    CommonIterator operator++(int) {
      CommonIterator other = *this;
      this->operator++();
      return other;
    }

    CommonIterator operator--(int) {
      CommonIterator other = *this;
      this->operator--();
      return other;
    }

    CommonIterator& operator++() {
      ++beg_j;
      beg_i += (beg_j / kBatchSize);
      beg_j = beg_j % kBatchSize;
      return *this;
    }

    CommonIterator& operator--() {
      beg_i -= static_cast<int>(beg_j == 0);
      beg_j == 0 ? beg_j = kBatchSize - 1 : --beg_j;
      return *this;
    }

    CommonIterator operator+(int value) const {
      CommonIterator other = *this;
      other += value;
      return other;
    }

    CommonIterator& operator+=(int value) {
      beg_i += value / kBatchSize;
      beg_i += static_cast<int>(beg_j + value % kBatchSize > kBatchSize - 1);
      beg_j = (beg_j + value % kBatchSize);
      beg_j %= kBatchSize;
      return *this;
    }

    CommonIterator operator-(int value) const {
      CommonIterator other = *this;
      other -= value;
      return other;
    }

    CommonIterator& operator-=(int value) {
      beg_i -= value / kBatchSize;
      beg_i -= static_cast<int>(beg_j - value % kBatchSize < 0);
      if (beg_j - value % kBatchSize < 0) {
        beg_j += kBatchSize;
      }
      beg_j -= (value % kBatchSize);
      return *this;
    }

    bool operator<(const CommonIterator& targ) const {
      return !(*this >= targ);
    }

    bool operator>(const CommonIterator& targ) const {
      if (beg_i == targ.beg_i) {
        return beg_j > targ.beg_j;
      }
      return beg_i > targ.beg_i;
    }

    bool operator<=(const CommonIterator& targ) const {
      return !(*this > targ);
    }

    bool operator>=(const CommonIterator& targ) const {
      return (*this > targ || *this == targ);
    }

    bool operator==(const CommonIterator& targ) const {
      return (beg_i == targ.beg_i && beg_j == targ.beg_j);
    }

    bool operator!=(const CommonIterator& targ) const {
      return !(*this == targ);
    }

    difference_type operator-(const CommonIterator& targ) const {
      return beg_j - targ.beg_j + beg_i * kBatchSize - targ.beg_i * kBatchSize;
    }

    reference operator*() { return *(vec_[beg_i] + beg_j); }

    pointer operator->() const { return vec_[beg_i] + beg_j; }
  };

  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  iterator begin() {
    iterator vec(deque_, begin_i_, begin_j_);
    return vec;
  }

  const_iterator cbegin() const {
    iterator vec(deque_, begin_i_, begin_j_);
    return vec;
  }

  iterator end() {
    std::pair<int, int> end;
    end = index_element();
    iterator vec(deque_, end.first, end.second);
    return vec;
  }

  const_iterator cend() const {
    std::pair<int, int> end;
    end = index_element();
    iterator vec(deque_, end.first, end.second);
    return vec;
  }

  reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }

  const_reverse_iterator crbegin() const {
    return std::make_reverse_iterator(cend());
  }

  reverse_iterator rend() { return std::make_reverse_iterator(begin()); }

  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(cbegin());
  }

  void insert(iterator targ, const T& value) {
    try {
      if (targ == end()) {
        push_back(value);
        return;
      }
      push_back(*(end() - 1));
      for (iterator j = end() - 2; j > targ; --j) {
        j = std::move(j - 1);
      }
      *targ = value;
    } catch (...) {
      clear();
      throw;
    }
  }

  void emplace(iterator targ, T&& value) {
    try {
      if (targ == begin()) {
        emplace_front(value);
        return;
      }
      emplace_back(*(end() - 1));
      for (iterator j = end() - 2; j > targ; --j) {
        j = std::move(j - 1);
      }
      *targ = value;
    } catch (...) {
      clear();
      throw;
    }
  }

  void erase(iterator targ) {
    try {
      if (targ == begin()) {
        pop_front();
        return;
      }
      for (iterator j = targ + 1; j < end(); ++j) {
        (j - 1) = std::move(j);
      }
      pop_back();
    } catch (...) {
      clear();
      throw;
    }
  }

  ~Deque() { clear(); }
};
