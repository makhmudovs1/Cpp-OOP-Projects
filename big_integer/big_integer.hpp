#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

class BigInt {
 private:
  static const int kBase = 1000000000;
  bool sign_ = false;
  std::vector<uint64_t> x_;
  friend BigInt Sum(const BigInt& lhs, const BigInt& rhs);
  friend BigInt Sub(const BigInt& lhs, const BigInt& rhs);
  friend BigInt Div(const BigInt& left, const BigInt& right,
                    const BigInt& b_right);

 public:
  BigInt();
  BigInt(int64_t num);
  BigInt(const std::string& str);
  BigInt operator+() const;
  BigInt operator-() const;
  std::string ToString() const;
  bool operator<(const BigInt& rhs) const;
  bool operator==(const BigInt& rhs) const;
  bool operator!=(const BigInt& rhs) const;
  bool operator>=(const BigInt& rhs) const;
  bool operator>(const BigInt& rhs) const;
  bool operator<=(const BigInt& rhs) const;
  explicit operator bool() const;
  void Clear();
  friend BigInt operator+(const BigInt& lhs, const BigInt& rhs);
  friend BigInt operator-(const BigInt& lhs, const BigInt& rhs);
  friend BigInt operator*(const BigInt& lhs, const BigInt& rhs);
  friend BigInt operator/(const BigInt& left, const BigInt& right);
  BigInt& operator%=(const BigInt& other);
  BigInt operator%(const BigInt& other) const;
  BigInt& operator/=(const BigInt& other);
  BigInt& operator+=(const BigInt& rhs);
  BigInt& operator-=(const BigInt& rhs);
  BigInt& operator*=(const BigInt& rhs);
  BigInt operator++();
  BigInt operator--();
  BigInt operator++(int other);
  BigInt operator--(int other);
  friend std::istream& operator>>(std::istream& input, BigInt& rhs);
  friend std::ostream& operator<<(std::ostream& outstream, const BigInt& rhs);
  void DeleteZeros();
  void ShiftRight();
};
