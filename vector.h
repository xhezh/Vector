#pragma once
#define VECTOR_MEMORY_IMPLEMENTED
#include <initializer_list>
#include <stdexcept>
#include <iterator>
#include <memory>
#include <algorithm>

class ArrayOutOfRange : public std::out_of_range {
public:
  ArrayOutOfRange() : std::out_of_range("VectorOutOfRange") {
  }
};

template <typename T>
class Vector {
public:
  using ValueType = T;
  using Pointer = T*;
  using ConstPointer = const T*;
  using Reference = T&;
  using ConstReference = const T&;
  using Iterator = T*;
  using ConstIterator = const T*;
  using SizeType = size_t;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

private:
  SizeType size_;
  SizeType capacity_;
  Pointer data_;

  Pointer Allocate(SizeType n) {
    if (n == 0) {
      return nullptr;
    }
    return static_cast<Pointer>(operator new(n * sizeof(T)));
  }

  void Deallocate(Pointer p) {
    if (p) {
      operator delete(p);
    }
  }
public:
  Vector() : size_(0), capacity_(0), data_(nullptr) {}

  explicit Vector(SizeType size) : size_(size), capacity_(size), data_(Allocate(size)) {
    try {
      std::uninitialized_default_construct_n(data_, size);
    }
    catch (...) {
      Deallocate(data_);
      throw;
    }
  }

  Vector(SizeType size, const T& value) : size_(size), capacity_(size), data_(Allocate(size)) {
    try {
      std::uninitialized_fill_n(data_, size, value);
    }
    catch (...) {
      Deallocate(data_);
      throw;
    }
  }

  template <class Iterator, class = std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>>>
  Vector(Iterator first, Iterator last) : size_(std::distance(first, last)), capacity_(size_), data_(Allocate(size_)) {
    try {
      std::uninitialized_copy(first, last, data_);
    }
    catch (...) {
      Deallocate(data_);
      throw;
    }
  }

  Vector(std::initializer_list<T> init) : size_(init.size()), capacity_(init.size()), data_(Allocate(size_)) {
    try {
      std::uninitialized_copy(init.begin(), init.end(), data_);
    }
    catch (...) {
      Deallocate(data_);
      throw;
    }
  }

  Vector(const Vector& other) : size_(other.size_), capacity_(other.capacity_), data_(Allocate(other.capacity_)) {
    try {
      std::uninitialized_copy(other.data_, other.data_ + size_, data_);
    }
    catch (...) {
      Deallocate(data_);
      throw;
    }
  }

  Vector(Vector&& other) noexcept : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
    other.size_ = 0;
    other.capacity_ = 0;
    other.data_ = nullptr;
  }

  Vector& operator=(const Vector& other) {
    if (this != &other) {
      Vector tmp(other);
      Swap(tmp);
    }
    return *this;
  }

  Vector& operator=(Vector&& other) noexcept {
    if (this != &other) {
      Clear();
      Deallocate(data_);
      size_ = other.size_;
      capacity_ = other.capacity_;
      data_ = other.data_;
      other.size_ = 0;
      other.capacity_ = 0;
      other.data_ = nullptr;
    }
    return *this;
  }

  ~Vector() {
    Clear();
    Deallocate(data_);
  }

  SizeType Size() const {
    return size_;
  }

  SizeType Capacity() const {
    return capacity_;
  }

  bool Empty() const {
    return size_ == 0;
  }

  Reference operator[](SizeType index) {
    return data_[index];
  }

  ConstReference operator[](SizeType index) const {
    return data_[index];
  }

  Reference At(SizeType index) {
    if (index >= size_) {
      throw ArrayOutOfRange{};
    }
    return data_[index];
  }

  ConstReference At(SizeType index) const {
    if (index >= size_) {
      throw ArrayOutOfRange{};
    }
    return data_[index];
  }

  Reference Front() {
    return data_[0];
  }

  ConstReference Front() const {
    return data_[0];
  }

  Reference Back() {
    return data_[size_ - 1];
  }

  ConstReference Back() const {
    return data_[size_ - 1];
  }

  Pointer Data() {
    return data_;
  }

  ConstPointer Data() const {
    return data_;
  }

  void Swap(Vector& other) {
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    std::swap(data_, other.data_);
  }

  void Resize(SizeType new_size) {
    if (new_size > capacity_) {
      Pointer new_data = Allocate(new_size);
      try {
        std::uninitialized_move(data_, data_ + size_, new_data);
        std::uninitialized_default_construct_n(new_data + size_, new_size - size_);
      }
      catch (...) {
        std::destroy(new_data, new_data + size_);
        Deallocate(new_data);
        throw;
      }
      std::destroy(data_, data_ + size_);
      Deallocate(data_);
      data_ = new_data;
      capacity_ = new_size;
    }
    else if (new_size < size_) {
      std::destroy(data_ + new_size, data_ + size_);
    }
    else {
      std::uninitialized_default_construct_n(data_ + size_, new_size - size_);
    }
    size_ = new_size;
  }

  void Resize(SizeType new_size, const T& value) {
    if (new_size > capacity_) {
      Pointer new_data = Allocate(new_size);
      try {
        std::uninitialized_move(data_, data_ + size_, new_data);
        std::uninitialized_fill_n(new_data + size_, new_size - size_, value);
      }
      catch (...) {
        std::destroy(new_data, new_data + size_);
        Deallocate(new_data);
        throw;
      }
      std::destroy(data_, data_ + size_);
      Deallocate(data_);
      data_ = new_data;
      capacity_ = new_size;
    }
    else if (new_size < size_) {
      std::destroy(data_ + new_size, data_ + size_);
    }
    else {
      std::uninitialized_fill_n(data_ + size_, new_size - size_, value);
    }
    size_ = new_size;
  }

  void Reserve(SizeType new_cap) {
    if (new_cap > capacity_) {
      Pointer new_data = Allocate(new_cap);
      try {
        std::uninitialized_move(data_, data_ + size_, new_data);
      }
      catch (...) {
        std::destroy(new_data, new_data + size_);
        Deallocate(new_data);
        throw;
      }
      std::destroy(data_, data_ + size_);
      Deallocate(data_);
      data_ = new_data;
      capacity_ = new_cap;
    }
  }

  void ShrinkToFit() {
    if (size_ == 0) {
      Deallocate(data_);
      data_ = nullptr;
      capacity_ = 0;
    }
    else if (size_ < capacity_) {
      Pointer new_data = Allocate(size_);
      try {
        std::uninitialized_move(data_, data_ + size_, new_data);
      }
      catch (...) {
        std::destroy(new_data, new_data + size_);
        Deallocate(new_data);
        throw;
      }
      std::destroy(data_, data_ + size_);
      Deallocate(data_);
      data_ = new_data;
      capacity_ = size_;
    }
  }

  void Clear() noexcept {
    if (data_ != nullptr) {
      std::destroy(data_, data_ + size_);
      size_ = 0;
    }
  }

  void PushBack(const T& value) {
    if (size_ == capacity_) {
      SizeType new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
      Pointer new_data = Allocate(new_capacity);
      try {
        std::uninitialized_move(data_, data_ + size_, new_data);
        new (new_data + size_) T(value);
      }
      catch (...) {
        std::destroy(new_data, new_data + size_);
        Deallocate(new_data);
        throw;
      }
      std::destroy(data_, data_ + size_);
      Deallocate(data_);
      data_ = new_data;
      capacity_ = new_capacity;
    }
    else {
      new (data_ + size_) T(value);
    }
    ++size_;
  }

  void PushBack(T&& value) {
    if (size_ == capacity_) {
      SizeType new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
      Pointer new_data = Allocate(new_capacity);
      try {
        std::uninitialized_move(data_, data_ + size_, new_data);
        new (new_data + size_) T(std::move(value));
      }
      catch (...) {
        std::destroy(new_data, new_data + size_);
        Deallocate(new_data);
        throw;
      }
      std::destroy(data_, data_ + size_);
      Deallocate(data_);
      data_ = new_data;
      capacity_ = new_capacity;
    }
    else {
      new (data_ + size_) T(std::move(value));
    }
    ++size_;
  }

  void PopBack() { 
    if (size_ > 0) {
      std::destroy_at(data_ + size_ - 1);
      --size_;
    }
  }

  template <typename... Args>
  void EmplaceBack(Args&&... args) {
    if (size_ == capacity_) {
      SizeType new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
      Pointer new_data = Allocate(new_capacity);
      try {
        std::uninitialized_move(data_, data_ + size_, new_data);
        new (new_data + size_) T(std::forward<Args>(args)...);
      }
      catch (...) {
        std::destroy(new_data, new_data + size_);
        Deallocate(new_data);
        throw;
      }
      std::destroy(data_, data_ + size_);
      Deallocate(data_);
      data_ = new_data;
      capacity_ = new_capacity;
    }
    else {
      new (data_ + size_) T(std::forward<Args>(args)...);
    }
    ++size_;
  }

  bool operator==(const Vector& other) const {
    return size_ == other.size_ && std::equal(data_, data_ + size_, other.data_);
  }

  bool operator!=(const Vector& other) const {
    return !(*this == other);
  }

  bool operator<(const Vector& other) const {
    return std::lexicographical_compare(data_, data_ + size_, other.data_, other.data_ + other.size_);
  }

  bool operator>(const Vector& other) const {
    return other < *this;
  }

  bool operator<=(const Vector& other) const {
    return !(other < *this);
  }

  bool operator>=(const Vector& other) const {
    return !(*this < other);
  }

  Iterator begin() { // NOLINT
    return data_;
  }

  ConstIterator begin() const { // NOLINT
    return data_;
  }

  ConstIterator cbegin() const { // NOLINT
    return data_;
  }

  Iterator end() { // NOLINT
    return data_ + size_;
  }

  ConstIterator end() const { // NOLINT
    return data_ + size_;
  }

  ConstIterator cend() const { // NOLINT
    return data_ + size_;
  }

  ReverseIterator rbegin() { // NOLINT
    return std::reverse_iterator<Iterator>(end());
  }

  ConstReverseIterator rbegin() const { // NOLINT
    return std::reverse_iterator<ConstIterator>(end());
  }

  ConstReverseIterator crbegin() const { // NOLINT
    return std::reverse_iterator<ConstIterator>(cend());
  }

  ReverseIterator rend() { // NOLINT
    return std::reverse_iterator<Iterator>(begin());
  }

  ConstReverseIterator rend() const { // NOLINT
    return std::reverse_iterator<ConstIterator>(begin());
  }

  ConstReverseIterator crend() const { // NOLINT
    return std::reverse_iterator<ConstIterator>(cbegin());
  }
};
