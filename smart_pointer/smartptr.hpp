#pragma once
#include <iostream>
#include <memory>

namespace control_block {
struct BaseControlBlock {
 public:
  void* object;
  size_t weak_counter = 0;
  size_t shared_counter = 0;

  virtual ~BaseControlBlock() = default;
  virtual void delete_ptr() {}
  virtual void destroy() {}
};

template <typename T, typename Deleter = std::default_delete<T>,
    typename Alloc = std::allocator<T>>
struct ControlBlock : public BaseControlBlock {
  bool constructed_by_allocator = false;
  Alloc alloc;
  Deleter deleter;

  using alloc_traits = std::allocator_traits<Alloc>;
  using control_alloc = std::allocator_traits<control_block::ControlBlock<T>>;
  using block_type = control_block::ControlBlock<T, std::allocator<T>>;
  using block_alloc =
      typename std::allocator_traits<Alloc>::template rebind_alloc<
          control_block::ControlBlock<T, Deleter, Alloc>>;
  using control_block_alloc =
      typename std::allocator_traits<Alloc>::template rebind_alloc<block_type>;
  using block_alloc_traits =
      typename std::allocator_traits<Alloc>::template rebind_traits<
          control_block::ControlBlock<T, Deleter, Alloc>>;

  ControlBlock(std::nullptr_t, Deleter deleter = Deleter())
      : deleter(deleter), alloc(alloc) {
    Alloc alloc = Alloc();
    BaseControlBlock::object = nullptr;
  }
  template <typename Y>
  ControlBlock(Y* ptr, Deleter deleter = Deleter(), Alloc alloc = Alloc())
      : alloc(alloc), deleter(deleter) {
    BaseControlBlock::object = ptr;
  }
  ~ControlBlock() = default;

  void delete_ptr() { deleter(static_cast<T*>(BaseControlBlock::object)); }

  void destroy() {
    block_alloc block_alloc(alloc);
    block_alloc_traits::deallocate(block_alloc, this, 1);
  }
};

template <typename T, typename Alloc = std::allocator<T>>
struct MakeSharedPtr : BaseControlBlock {
  Alloc alloc;
  T object;

  using block_type = control_block::ControlBlock<T, std::allocator<T>>;
  using alloc_traits = std::allocator_traits<Alloc>;
  using control_makes = control_block::MakeSharedPtr<T, Alloc>;
  using block_alloc =
      typename alloc_traits::template rebind_alloc<control_makes>;
  using control_block_alloc = typename std::allocator_traits<
      std::allocator<T>>::template rebind_alloc<block_type>;
  void destroy() {
    block_alloc block_al(alloc);
    alloc_traits::template rebind_traits<control_makes>::deallocate(block_al,
                                                                    this, 1);
  }

  void delete_ptr() {
    alloc_traits::destroy(alloc, static_cast<T*>(BaseControlBlock::object));
  }

  template <typename... Args>
  MakeSharedPtr(Args&&... args)
      : alloc(Alloc()), object(std::forward<Args>(args)...) {
    ++BaseControlBlock::shared_counter;
    BaseControlBlock::object = &object;
  }
};
};  // namespace control_block

template <typename T>
class SharedPtr {
 private:
  control_block::BaseControlBlock* cptr_ = nullptr;
  T* ptr_;

  template <typename Y>
  friend class WeakPtr;

  template <typename Y>
  friend class SharedPtr;

  template <typename Y, typename... Args>
  friend SharedPtr<Y> MakeShared(Args&&... args);

  template <typename Y, typename Alloc, typename... Args>
  friend SharedPtr<Y> AllocateShared(const Alloc& alloc, Args&&... args);

  template <typename Y, typename Alloc>
  SharedPtr(control_block::MakeSharedPtr<Y, Alloc>* control_block)
      : cptr_(control_block) {
    ptr_ = static_cast<T*>(cptr_->object);
  }

 public:
  SharedPtr() : ptr_(nullptr) {}
  SharedPtr(std::nullptr_t) : ptr_(nullptr) {}
  void swap(SharedPtr& other) {
    std::swap(ptr_, other.ptr_);
    std::swap(ptr_, other.ptr);
  }

  template <typename Y, typename Alloc = std::allocator<T>>
  SharedPtr(Y* ptr) {
    using block_type = control_block::ControlBlock<T, std::allocator<T>>;
    using control_block_alloc = typename std::allocator_traits<
        std::allocator<T>>::template rebind_alloc<block_type>;
    using block_alloc =
        typename std::allocator_traits<Alloc>::template rebind_alloc<
            control_block::ControlBlock<T, std::default_delete<T>, Alloc>>;
    using control_alloc = std::allocator_traits<control_block::ControlBlock<T>>;
    block_alloc alloc;
    using control_default =
        control_block::ControlBlock<T, std::default_delete<T>,
    std::allocator<T>>;
    using block_alloc_traits = typename std::allocator_traits<
        Alloc>::template rebind_traits<control_default>;
    cptr_ = block_alloc_traits::allocate(alloc, 1);
    block_alloc_traits::construct(
        alloc, reinterpret_cast<control_block::ControlBlock<T>*>(cptr_), ptr);
    ++cptr_->shared_counter;
    ptr_ = static_cast<T*>(cptr_->object);
  }
  template <typename Y>
  void reset(Y* ptr) {
    SharedPtr<T>(ptr).swap(*this);
  }

  template <typename Y, typename Deleter>
  void reset(Y* ptr, Deleter del) {
    SharedPtr<Y>(ptr, del).swap(*this);
  }

  template <typename Y, typename Deleter, typename Alloc>
  void reset(Y* ptr, Deleter del, Alloc alloc) {
    SharedPtr<T>(ptr, del, alloc).swap(*this);
  }
  void reset() {
    if (ptr_ != nullptr) {
      cptr_->shared_counter =
          (cptr_->shared_counter != 0 ? cptr_->shared_counter - 1 : 0);
      if (cptr_->shared_counter == 0) {
        cptr_->delete_ptr();
      }
      if (cptr_->shared_counter == 0 && cptr_->weak_counter == 0) {
        cptr_->destroy();
      }
      ptr_ = nullptr;
    }
  }

  SharedPtr(const SharedPtr<T>& other) : cptr_(other.cptr_), ptr_(other.ptr_) {
    if (cptr_ != nullptr) {
      ++cptr_->shared_counter;
    }
  }

  template <typename Y>
  SharedPtr(const SharedPtr<Y>& other) : cptr_(other.cptr_), ptr_(other.ptr_) {
    if (cptr_ != nullptr) {
      ++other.cptr_->shared_counter;
    }
  }

  SharedPtr<T>& operator=(const SharedPtr<T>& new_ptr) {
    SharedPtr<T> ptr = new_ptr;
    std::swap(ptr_, ptr.ptr_);
    std::swap(cptr_, ptr.cptr_);
    return *this;
  }

  template <typename Y>
  SharedPtr(const SharedPtr<Y>&& other) : ptr_(other.ptr_), cptr_(other.cptr_) {
    other.ptr_ = other.cptr_ = nullptr;
  }
  template <typename Y>
  SharedPtr<T>& operator=(const SharedPtr<Y>& new_ptr) {
    SharedPtr<T> ptr = new_ptr;
    std::swap(cptr_, ptr.cptr_);
    std::swap(ptr_, ptr.ptr_);
    return *this;
  }
  SharedPtr(SharedPtr&& other) : cptr_(other.cptr_), ptr_(other.ptr_) {
    other.ptr_ = nullptr;
    other.cptr_ = nullptr;
  }
  SharedPtr<T>& operator=(SharedPtr<T>&& new_ptr) {
    SharedPtr<T> ptr = std::move(new_ptr);
    std::swap(cptr_, ptr.cptr_);
    std::swap(ptr_, ptr.ptr_);
    return *this;
  }
  T* operator->() const { return ptr_; }

  T* get() const {
    if (cptr_ == nullptr) {
      return nullptr;
    }
    return ptr_;
  }
  template <typename Y, typename Deleter>
  SharedPtr(Y* ptr, const Deleter& deleter) {
    using block_type = control_block::ControlBlock<Y, std::allocator<Y>>;
    using alloc_traits = std::allocator_traits<std::allocator<Y>>;
    using control_block_alloc = typename std::allocator_traits<
        std::allocator<Y>>::template rebind_alloc<block_type>;
    using block_alloc = typename alloc_traits::template rebind_alloc<
        control_block::ControlBlock<Y, Deleter, std::allocator<Y>>>;
    block_alloc alloc;
    using control_alloc = std::allocator_traits<control_block::ControlBlock<T>>;
    using block_alloc_traits = typename alloc_traits::template rebind_traits<
        control_block::ControlBlock<Y, Deleter, std::allocator<Y>>>;

    cptr_ = block_alloc_traits::allocate(alloc, 1);
    block_alloc_traits::construct(
        alloc,
        reinterpret_cast<control_block::ControlBlock<Y, Deleter>*>(cptr_), ptr,
        deleter);
    ++cptr_->shared_counter;
    ptr_ = static_cast<Y*>(cptr_->object);
  }

  T& operator*() const { return *ptr_; }

  size_t use_count() const {
    if (ptr_ != nullptr) {
      return cptr_->shared_counter;
    }
    return 0;
  }

  template <typename Y, typename Deleter, typename Alloc>
  SharedPtr(Y* ptr, const Deleter& deleter, const Alloc& alloc) {
    using alloc_traits = std::allocator_traits<Alloc>;
    using block_type = control_block::ControlBlock<T, Alloc>;
    using block_alloc_traits = typename alloc_traits::template rebind_traits<
        control_block::ControlBlock<T, Deleter, Alloc>>;
    using control_block_alloc = typename std::allocator_traits<
        Alloc>::template rebind_alloc<block_type>;
    using block_alloc = typename alloc_traits::template rebind_alloc<
        control_block::ControlBlock<T, Deleter, Alloc>>;
    block_alloc alloc_block(alloc);
    cptr_ = block_alloc_traits::allocate(alloc_block, 1);
    block_alloc_traits::construct(
        alloc_block,
        reinterpret_cast<control_block::ControlBlock<T, Deleter, Alloc>*>(
            cptr_),
        ptr, deleter, alloc_block);
    ++cptr_->shared_counter;
    ptr_ = static_cast<T*>(cptr_->object);
  }
  ~SharedPtr() {
    if (ptr_ != nullptr) {
      cptr_->shared_counter =
          (cptr_->shared_counter != 0 ? cptr_->shared_counter - 1 : 0);
      if (cptr_->shared_counter == 0) {
        cptr_->delete_ptr();
      }
      if (cptr_->shared_counter == 0 && cptr_->weak_counter == 0) {
        cptr_->destroy();
      }
    }
  }
};

template <typename Y, typename... Args>
SharedPtr<Y> MakeShared(Args&&... args) {
  control_block::MakeSharedPtr<Y>* control_block;
  control_block =
      new control_block::MakeSharedPtr<Y>(std::forward<Args...>(args)...);
  return SharedPtr<Y>(control_block);
}

template <typename T>
class WeakPtr {
 private:
  control_block::BaseControlBlock* cptr_ = nullptr;
  T* ptr_;

 public:
  void swap(WeakPtr& other) {
    std::swap(cptr_, other.cptr);
    std::swap(ptr_, other.ptr);
  }
  WeakPtr(const WeakPtr& ptr) : cptr_(ptr.control_block_), ptr_(ptr.ptr_) {
    if (cptr_ != nullptr) {
      ++cptr_->weak_counter;
    }
  }
  template <typename Y>
  WeakPtr(const SharedPtr<Y>& ptr) : cptr_(ptr.cptr_), ptr_(ptr.ptr_) {
    if (cptr_ != nullptr) {
      ++cptr_->weak_counter;
    }
  }
  template <typename Y>
  WeakPtr(const WeakPtr<Y>& ptr) : cptr_(ptr.control_block_), ptr_(ptr.ptr_) {
    if (cptr_ != nullptr) {
      ++cptr_->weak_counter;
    }
  }

  WeakPtr(const WeakPtr&& ptr) : cptr_(ptr.control_block_), ptr_(ptr.ptr_) {
    ptr.cptr_ = nullptr;
    ptr.ptr_ = nullptr;
  }
  template <typename Y>
  WeakPtr(const WeakPtr&& ptr) : cptr_(ptr.control_block_), ptr_(ptr.ptr_) {
    ptr.cptr_ = nullptr;
    ptr.ptr_ = nullptr;
  }

  WeakPtr& operator=(const WeakPtr& ptr) {
    WeakPtr tmp = ptr;
    swap(tmp);
    return *this;
  }
  template <typename Y>
  WeakPtr& operator=(const WeakPtr<Y>& ptr) {
    WeakPtr tmp = ptr;
    swap(tmp);
    return *this;
  }
  template <typename Y>
  WeakPtr& operator=(const SharedPtr<Y>& ptr) {
    WeakPtr tmp = ptr;
    swap(tmp);
    return *this;
  }
  WeakPtr& operator=(const WeakPtr&& ptr) {
    WeakPtr tmp = std::move(ptr);
    swap(tmp);
    return *this;
  }
  template <typename Y>
  WeakPtr& operator=(const WeakPtr<Y>&& ptr) {
    WeakPtr tmp = std::move(ptr);
    swap(tmp);
    return *this;
  }

  template <typename Y>
  SharedPtr<Y> lock() const {
    return SharedPtr<Y>(ptr_);
  }
  long use_count() const {
    if (cptr_ == 0) {
      return 0;
    }
    return cptr_->shared_counter;
  }

  bool expired() const { return use_count() == 0; }
  ~WeakPtr() {
    if (cptr_ == nullptr) {
      return;
    }
    --cptr_->weak_counter;
    if (cptr_->weak_counter == 0 && cptr_->shared_counter == 0) {
      cptr_->destroy();
    }
  }
};

template <typename Y, typename Alloc, typename... Args>
SharedPtr<Y> AllocateShared(const Alloc& alloc, Args&&... args) {
  using control_block_alloc = typename std::allocator_traits<
      Alloc>::template rebind_alloc<control_block::ControlBlock<Y, Alloc>>;
  using control_makes = control_block::MakeSharedPtr<Y, Alloc>;
  using block_alloc = typename std::allocator_traits<
      Alloc>::template rebind_alloc<control_makes>;
  block_alloc alloc_new(alloc);
  auto block = std::allocator_traits<Alloc>::template rebind_traits<
      control_makes>::allocate(alloc_new, 1);
  std::allocator_traits<Alloc>::template rebind_traits<
      control_makes>::construct(alloc_new, block,
                                std::forward<Args...>(args)...);
  return SharedPtr<Y>(block);
}