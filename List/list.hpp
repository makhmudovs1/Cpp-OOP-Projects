#pragma once

#include "iostream"

template <typename T, typename Allocator = std::allocator<T>>

class List {
 private:
  struct Node {
    Node* prev = nullptr;
    Node* next = nullptr;
    T value = T();
    explicit Node(Node* prev, Node* next) : prev(prev), next(next) {}
    explicit Node(const T& value, Node* prev, Node* next)
        : value(value), prev(prev), next(next) {}
  };

 public:
  using alloc_traits = std::allocator_traits<Allocator>;
  using value_type = T;
  using allocator_type =
      typename std::allocator_traits<Allocator>::allocator_type;
  using node_allocator = typename alloc_traits::template rebind_alloc<Node>;
  using node_alloc_traits = std::allocator_traits<node_allocator>;

  List() {
    alloc_ = static_cast<node_allocator>(Allocator());
    fake_node_ = node_alloc_traits::allocate(alloc_, 1);
    fake_node_->next = fake_node_->prev = fake_node_;
  }

  List(size_t count, const T& value, const Allocator& alloc = Allocator()) {
    alloc_ = static_cast<node_allocator>(alloc);
    fake_node_ = node_alloc_traits::allocate(alloc_, 1);
    fake_node_->next = fake_node_->prev = fake_node_;
    try {
      for (size_t i = 0; i < count; ++i) {
        push_back(value);
      }
    } catch (...) {
      clear();
      throw;
    }
  }

  explicit List(size_t count, const Allocator& alloc = Allocator()) {
    alloc_ = static_cast<node_allocator>(alloc);
    fake_node_ = node_alloc_traits::allocate(alloc_, 1);
    fake_node_->next = fake_node_->prev = fake_node_;
    try {
      for (size_t _ = 0; _ < count; _++) {
        Node* current = node_alloc_traits::allocate(alloc_, 1);
        try {
          node_alloc_traits::construct(alloc_, current, fake_node_->prev,
                                       fake_node_->prev->next);
        } catch (...) {
          node_alloc_traits::deallocate(alloc_, current, 1);
          throw;
        }
        fake_node_->prev->next = current;
        ++size_;
      }
    } catch (...) {
      clear();
      throw;
    }
  }
  List(const List& other) {
    alloc_ = alloc_traits::select_on_container_copy_construction(other.alloc_);
    fake_node_ = node_alloc_traits::allocate(alloc_, 1);
    fake_node_->next = fake_node_->prev = fake_node_;
    Node* current = other.fake_node_->next;
    try {
      for (size_t _ = 0; _ < other.size_; _++) {
        push_back(current->value);
        current = current->next;
      }
    } catch (...) {
      clear();
      throw;
    }
  }
  List(std::initializer_list<T> initializer,
       const Allocator& alloc = Allocator())
      : alloc_(static_cast<node_allocator>(alloc)) {
    fake_node_ = node_alloc_traits::allocate(alloc_, 1);
    fake_node_->next = fake_node_->prev = fake_node_;
    try {
      for (auto it = initializer.begin(); it != initializer.end(); ++it) {
        push_back(*it);
      }
    } catch (...) {
      clear();
      throw;
    }
  }

  ~List() { clear(); }

  List& operator=(const List& other) {
    if (this == &other) {
      return *this;
    }
    List current(other);
    if (node_alloc_traits ::propagate_on_container_copy_assignment::value) {
      current.alloc_ = other.alloc_;
    }
    std::swap(size_, current.size_);
    std::swap(fake_node_, current.fake_node_);
    std::swap(alloc_, current.alloc_);
    return *this;
  }

  T& back() { return fake_node_->prev->value; }
  const T& back() const { return fake_node_->prev->value; }
  bool empty() const { return size_ == 0; }
  T& front() { return fake_node_->next->value; }
  const T& front() const { return fake_node_->next->value; }
  size_t size() const { return size_; }
  node_allocator get_allocator() { return alloc_; }

  void push_back(const T& new_val) {
    Node* node_b = new_node_b(new_val);
    ++size_;
    after(fake_node_->prev, node_b);
  }
  void push_back(T&& new_val) {
    Node* node_b = new_node_b(std::move(new_val));
    ++size_;
    after(fake_node_->prev, node_b);
  }
  void push_front(const T& new_val) {
    Node* new_node_f = new_node_f(new_val);
    ++size_;
    after(fake_node_, new_node_f);
  }
  void push_front(T&& new_val) {
    Node* node_f = new_node_f(std::move(new_val));
    ++size_;
    after(fake_node_, node_f);
  }
  void pop_back() { remove(fake_node_->prev); }
  void pop_front() { remove(fake_node_->next); }

  template <bool IsConst>
  class Iterator {
   public:
    using value_type = typename std::conditional<IsConst, const T, T>::type;
    using pointer = typename std::conditional<IsConst, const T*, T*>::type;
    using reference = typename std::conditional<IsConst, const T&, T&>::type;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;

    explicit Iterator(Node* node_ptr) : ptr_(node_ptr) {}
    Iterator& operator++() {
      ptr_ = ptr_->next;
      return *this;
    }
    Iterator& operator--() {
      ptr_ = ptr_->prev;
      return *this;
    }
    Iterator operator++(int) {
      Iterator res = *this;
      ++(*this);
      return res;
    }
    reference operator*() const { return ptr_->value; }
    pointer operator->() const { return &(ptr_->value); }
    Iterator operator--(int) {
      Iterator copy = *this;
      ptr_ = ptr_->prev;
      return copy;
    }
    template <bool TM>
    bool operator==(const Iterator<TM>& iter) const {
      return ptr_ == iter.ptr_;
    }
    template <bool TM>
    bool operator!=(const Iterator<TM>& iter) const {
      return ptr_ != iter.ptr_;
    }
    explicit operator Iterator<true>() { return Iterator<true>(ptr_); }

   private:
    Node* ptr_;
  };
  using iterator = Iterator<false>;
  using reverse_iterator_t = std::reverse_iterator<Iterator<false>>;

  Iterator<false> begin() { return Iterator<false>(fake_node_->next); }
  Iterator<false> end() { return Iterator<false>(fake_node_); }

  Iterator<true> begin() const { return Iterator<true>(fake_node_->next); }
  Iterator<true> end() const { return Iterator<true>(fake_node_); }

  Iterator<true> cbegin() const { return Iterator<true>(fake_node_->next); }
  [[maybe_unused]] Iterator<true> cend() const {
    return Iterator<true>(fake_node_);
  }

  reverse_iterator_t rbegin() { return std::make_reverse_iterator(end()); }
  [[maybe_unused]] reverse_iterator_t rend() {
    return std::make_reverse_iterator(begin());
  }

 private:
  size_t size_ = 0;
  node_allocator alloc_;
  Node* fake_node_;

  void after(Node* first, Node* second) const {
    first->next = second;
    second->next->prev = second;
  }
  Node* new_node_b(const T& temp) {
    Node* pointer = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, pointer, temp, fake_node_->prev,
                                   fake_node_->prev->next);
    } catch (...) {
      node_alloc_traits::deallocate(alloc_, pointer, 1);
      throw;
    }
    return pointer;
  }

  Node* new_node_f(const T& temp) {
    Node* pointer = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, pointer, temp, fake_node_,
                                   fake_node_->next);
    } catch (...) {
      node_alloc_traits::deallocate(alloc_, pointer, 1);
      throw;
    }
    return pointer;
  }

  Node* new_node_b(T&& temp) {
    Node* pointer = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, pointer, std::move(temp),
                                   fake_node_->prev, fake_node_->prev->next);
    } catch (...) {
      node_alloc_traits::deallocate(alloc_, pointer, 1);
      throw;
    }
    return pointer;
  }

  Node* new_node_f(T&& temp) {
    Node* pointer = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, pointer, std::move(temp), fake_node_,
                                   fake_node_->next);
    } catch (...) {
      node_alloc_traits::deallocate(alloc_, pointer, 1);
      throw;
    }
    return pointer;
  }

  void remove(Node* node) {
    if (node == fake_node_) {
      return;
    }
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node_alloc_traits ::destroy(alloc_, node);
    node_alloc_traits ::deallocate(alloc_, node, 1);
    --size_;
  }

  void clear() {
    Node* current = fake_node_->next;
    while (current != fake_node_) {
      Node* copy = current;
      current = current->next;
      node_alloc_traits::destroy(alloc_, copy);
      node_alloc_traits::deallocate(alloc_, copy, 1);
    }
    node_alloc_traits::deallocate(alloc_, fake_node_, 1);
    size_ = 0;
  }
};
