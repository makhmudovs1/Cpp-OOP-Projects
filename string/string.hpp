#pragma once
#include <string.h>

#include <iostream>
#include <vector>

class String {
 private:
  size_t size_;
  char* str_;
  size_t capacity_;

 public:
  String();
  explicit String(size_t size, char character);
  String(const char* str);
  String& operator=(const String& str);
  String(const String& str);
  ~String();
  void Clear();
  void PushBack(char character);
  void PopBack();
  void Resize(size_t new_size, char character = 'o');
  void Reserve(size_t new_cap);
  void ShrinkToFit();
  String operator+(const String& str) const;
  String& operator+=(const String& str);
  bool operator==(const String& str) const;
  bool operator<(const String& str) const;
  bool operator<=(const String& str) const;
  bool operator!=(const String& str) const;
  bool operator>(const String& str) const;
  bool operator>=(const String& str) const;
  friend String operator*(const String& st1, size_t n);
  String& operator*=(int n);
  const char& operator[](size_t index) const;
  char& operator[](size_t index);
  char& Front();
  const char& Front() const;
  char& Back();
  const char& Back() const;
  void Swap(String& other);
  bool Empty() const;
  size_t Size() const;
  size_t Capacity() const;
  const char* Data() const;
  char* Data();
  friend std::istream& operator>>(std::istream& input, String& str);
  friend std::ostream& operator<<(std::ostream& ostream, const String& str);
  String Join(const std::vector<String>& str) const;
  std::vector<String> Split(const String& delim = " ");
};
