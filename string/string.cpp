#include "string.hpp"

const int kKName = 1000;

String::String() {
  str_ = nullptr;
  capacity_ = 0;
  size_ = 0;
}

String::String(size_t size, char character) : size_(size), capacity_(size) {
  str_ = new char[size + 1];
  str_[size] = '\0';
  memset(str_, character, size);
}

String::String(const char* str) {
  size_ = strlen(str);
  capacity_ = size_;
  str_ = new char[capacity_ + 1];
  strcpy(str_, str);
}

String::String(const String& str) {
  str_ = new char[str.Capacity() + 1];
  strcpy(str_, str.Data());
  size_ = str.Size();
  capacity_ = str.Capacity();
}

String& String::operator=(const String& str) {
  if (&str == this) {
    return *this;
  }
  delete[] str_;
  str_ = new char[str.Capacity() + kKName];
  strcpy(str_, str.Data());
  size_ = str.Size();
  capacity_ = str.Capacity();
  return *this;
}

void String::Clear() { size_ = 0; }

void String::PushBack(char character) {
  if (size_ == capacity_) {
    if (capacity_ == 0) {
      capacity_ = kKName;
    }
    Reserve(capacity_ * 2);
  }
  str_[size_] = character;
  ++size_;
  str_[size_] = '\0';
}

void String::PopBack() {
  if (size_ != 0) {
    --size_;
  }
}

void String::Resize(size_t new_size, char character) {
  if (new_size > capacity_) {
    Reserve(new_size);
    memset(str_, character, new_size);
  }
  size_ = new_size;
}

String::~String() { delete[] str_; }

void String::Reserve(size_t new_cap) {
  if (str_ == nullptr) {
    str_ = new char[new_cap + 1];
    str_[0] = '\0';
    capacity_ = new_cap;
  } else if (new_cap > capacity_) {
    char* str = new char[new_cap + 1];
    if (str_ != nullptr) {
      strcpy(str, str_);
    }
    delete[] str_;
    str_ = str;
    capacity_ = new_cap;
  }
}

void String::ShrinkToFit() {
  if (capacity_ > size_) {
    capacity_ = size_;
    char* str = new char[size_ + 1];
    strcpy(str, str_);
    delete[] str_;
    str_ = str;
  }
}

void String::Swap(String& other) {
  std::swap(other, *this);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

char& String::Front() { return str_[0]; }
const char& String::Front() const { return str_[0]; }
char& String::Back() { return str_[size_ - 1]; }
const char& String::Back() const { return str_[size_ - 1]; }
bool String::Empty() const { return (size_ == 0); }
size_t String::Size() const { return size_; }
size_t String::Capacity() const { return capacity_; }
const char* String::Data() const { return str_; }
char* String::Data() { return str_; }
bool String::operator<(const String& str) const {
  return strcmp(str_, str.Data()) < 0;
}

bool String::operator!=(const String& str) const {
  return strcmp(str_, str.Data()) != 0;
}

bool String::operator>=(const String& str) const { return !(*this < str); }
bool String::operator==(const String& str) const { return !(*this != str); }
bool String::operator<=(const String& str) const {
  return (*this < str) or (*this == str);
}
bool String::operator>(const String& str) const { return !(*this <= str); }

String& String::operator+=(const String& str) {
  if (capacity_ < size_ + str.Size()) {
    Reserve(std::max(size_ + str.Size(), 2 * capacity_));
  }
  if (str_ != nullptr) {
    strcat(str_, str.Data());
  }
  size_ += str.Size();
  return *this;
}

String String::operator+(const String& str) const {
  String temp;
  temp += *this;
  temp += str;
  return temp;
}

String operator*(const String& st1, size_t n) {
  String temp;
  temp.Reserve(n * st1.Size());
  for (size_t i = 0; i < n; ++i) {
    temp += st1;
  }
  return temp;
}
String& String::operator*=(int n) {
  *this = *this * n;
  return *this;
}

const char& String::operator[](size_t index) const { return *(str_ + index); }
char& String::operator[](size_t index) { return *(str_ + index); }

std::istream& operator>>(std::istream& input, String& str) {
  char chr;
  while (input >> chr) {
    if (chr != ' ') {
      str.PushBack(chr);
    }
  }
  return input;
}

std::ostream& operator<<(std::ostream& ostream, const String& str) {
  ostream << str.Data();
  return ostream;
}

std::vector<String> String::Split(const String& delim) {
  std::vector<String> ans;
  char* res = new char[Size()];
  char* str = str_;
  char* count = strstr(str, delim.Data());
  while (count != nullptr) {
    strncpy(res, str, count - str);
    res[count - str] = '\0';
    str = count + delim.Size();
    ans.push_back(res);
    count = strstr(str, delim.Data());
  }
  ans.push_back(str);
  delete[] res;
  return ans;
}
String String::Join(const std::vector<String>& str) const {
  if (str.empty()) {
    return String("");
  }
  String res = str[0];
  for (size_t i = 1; i < str.size(); ++i) {
    res += *this + str[i];
  }
  return res;
}
