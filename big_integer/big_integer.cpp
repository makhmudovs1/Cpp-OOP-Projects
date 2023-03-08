#include "big_integer.hpp"

const int kName = 10;

BigInt::BigInt() {}

BigInt::BigInt(int64_t num) {
  if (num == 0) {
    x_.push_back(0);
  }
  if (num < 0) {
    sign_ = true;
  }
  if (sign_) {
    int res = std::abs(std::abs(num + 1) % kName);
    ++res;
    if (res == kName) {
      res = 0;
    }
    x_.push_back(res);
    num /= kName;
  }
  num = std::abs(num);
  while (num != 0) {
    int res = num % kName;
    x_.push_back(res);
    num /= kName;
  }
  reverse(x_.begin(), x_.end());
}

BigInt::BigInt(const std::string& str) {
  for (auto i = static_cast<size_t>(sign_ = str[0] == '-'); i < str.size();
       ++i) {
    x_.push_back(str[i] - '0');
  }
  if (sign_ && str[1] == '0') {
    x_.clear();
    sign_ = false;
  }
  if (x_.empty()) {
    x_.push_back(0), sign_ = false;
  }
}

BigInt BigInt::operator+() const {
  BigInt rhs = *this;
  return rhs;
}

BigInt BigInt::operator-() const {
  BigInt rhs = *this;
  rhs.sign_ ^= 1;
  if (rhs.x_[0] == 0) {
    rhs.sign_ = false;
  }
  return rhs;
}

std::string BigInt::ToString() const {
  std::string str;
  if (sign_ && (x_.size() > 1 || (x_[0] != 0))) {
    str = "-";
  }
  for (ssize_t i = x_.size() - 1; i >= 0; --i) {
    str += (char)(x_[i] + '0');
  }
  return str;
}

bool BigInt::operator<(const BigInt& rhs) const {
  if (sign_ != rhs.sign_) {
    return !(static_cast<int>(sign_) < static_cast<int>(rhs.sign_));
  }
  if (x_.size() != rhs.x_.size()) {
    if (sign_) {
      return x_.size() > rhs.x_.size();
    }
    return x_.size() < rhs.x_.size();
  }
  for (size_t i = 0; i < x_.size(); ++i) {
    if (x_[i] != rhs.x_[i]) {
      if (sign_) {
        return x_[i] > rhs.x_[i];
      }
      return x_[i] < rhs.x_[i];
    }
  }
  return false;
}

bool BigInt::operator>(const BigInt& rhs) const { return rhs < *this; }
bool BigInt::operator<=(const BigInt& rhs) const { return !(rhs < *this); }
bool BigInt::operator>=(const BigInt& rhs) const { return !(*this < rhs); }
bool BigInt::operator==(const BigInt& rhs) const {
  return sign_ == rhs.sign_ && x_ == rhs.x_;
}
bool BigInt::operator!=(const BigInt& rhs) const { return !(*this == rhs); }

BigInt::operator bool() const { return *this != 0; }

BigInt Sum(const BigInt& lhs, const BigInt& rhs) {
  BigInt right = lhs;
  BigInt rhs1 = rhs;
  reverse(right.x_.begin(), right.x_.end());
  reverse(rhs1.x_.begin(), rhs1.x_.end());
  right.x_.resize(std::max(lhs.x_.size(), rhs.x_.size()) + 1);
  for (size_t i = 0; i < rhs1.x_.size(); i++) {
    right.x_[i] += rhs1.x_[i];
    if (right.x_[i] >= kName) {
      right.x_[i] -= kName;
      ++right.x_[i + 1];
    }
  }
  for (size_t i = rhs1.x_.size(); i < right.x_.size(); ++i) {
    if (right.x_[i] >= kName) {
      right.x_[i] -= kName;
      ++right.x_[i + 1];
    } else {
      break;
    }
  }
  while (right.x_.size() > 1 && (right.x_.back() == 0)) {
    right.x_.pop_back();
  }
  reverse(right.x_.begin(), right.x_.end());
  return right;
}

BigInt operator+(const BigInt& lhs, const BigInt& rhs) {
  if (lhs.sign_ && !rhs.sign_) {
    return rhs - -lhs;
  }
  if (!lhs.sign_ && rhs.sign_) {
    return lhs - -rhs;
  }
  if (lhs.sign_ && rhs.sign_) {
    return -(-rhs + -lhs);
  }
  BigInt res = Sum(lhs, rhs);
  return res;
}

BigInt Sub(const BigInt& lhs, const BigInt& rhs) {
  BigInt right = lhs;
  BigInt rhs1 = rhs;
  reverse(right.x_.begin(), right.x_.end());
  reverse(rhs1.x_.begin(), rhs1.x_.end());
  for (size_t i = 0; i < rhs1.x_.size(); ++i) {
    if (right.x_[i] < rhs1.x_[i]) {
      right.x_[i] += kName;
      size_t ind = i + 1;
      while (right.x_[ind] == 0) {
        right.x_[ind] = kName - 1;
        ++ind;
      }
      --right.x_[ind];
    }
    right.x_[i] -= rhs1.x_[i];
  }
  while (right.x_.size() > 1 && right.x_.back() == 0) {
    right.x_.pop_back();
  }
  reverse(right.x_.begin(), right.x_.end());
  return right;
}

BigInt operator-(const BigInt& lhs, const BigInt& rhs) {
  if (lhs.sign_ && !rhs.sign_) {
    return -(-lhs + rhs);
  }
  if (!lhs.sign_ && rhs.sign_) {
    return lhs + -rhs;
  }
  if (lhs.sign_ && rhs.sign_) {
    return -rhs - -lhs;
  }
  if (lhs < rhs) {
    return -(rhs - lhs);
  }
  BigInt res = Sub(lhs, rhs);
  return res;
}

BigInt& BigInt::operator+=(const BigInt& rhs) { return *this = *this + rhs; }
BigInt& BigInt::operator-=(const BigInt& rhs) { return *this = *this - rhs; }
BigInt& BigInt::operator*=(const BigInt& rhs) { return *this = *this * rhs; }
BigInt BigInt::operator++(int other) {
  BigInt res = *this;
  ++other;
  *this = *this + 1;
  return res;
}
BigInt BigInt::operator--(int other) {
  BigInt res = *this;
  --other;
  *this = *this - 1;
  return res;
}
BigInt BigInt::operator++() { return *this += 1; }
BigInt BigInt::operator--() { return *this -= 1; }

void BigInt::DeleteZeros() {
  while (this->x_.size() > 1 && this->x_.back() == 0) {
    this->x_.pop_back();
  }
  if (this->x_.size() == 1 && this->x_[0] == 0) {
    this->sign_ = false;
  }
}

void BigInt::Clear() {
  std::vector<uint64_t> copy;
  copy.push_back(0);
  x_ = copy;
  sign_ = false;
}

void BigInt::ShiftRight() {
  if (this->x_.empty()) {
    this->x_.push_back(0);
    return;
  }
  this->x_.push_back(this->x_[this->x_.size() - 1]);
  for (size_t i = this->x_.size() - 2; i > 0; --i) {
    this->x_[i] = this->x_[i - 1];
  }
  this->x_[0] = 0;
}

BigInt Div(const BigInt& left, const BigInt& right, const BigInt& b_right) {
  BigInt res = b_right;
  res.sign_ = left.sign_ != right.sign_;
  while (res.x_.size() > 1) {
    if (res.x_[0] == 0) {
      res.x_.erase(res.x_.begin());
    } else {
      break;
    }
  }
  if (res.x_[0] == 0) {
    res.sign_ = false;
  }
  return res;
}

BigInt operator/(const BigInt& left, const BigInt& right) {
  if (right == 0) {
    std::cout << "-1!\n";
    return 0;
  }
  BigInt b_right = right;
  b_right.sign_ = false;
  BigInt res;
  BigInt cur;
  for (size_t i = 0; i < left.x_.size(); ++i) {
    cur.x_.push_back(left.x_[i]);
    int64_t elem = 0;
    int64_t left_pointer = 0;
    int64_t right_pointer = kName;
    while (left_pointer <= right_pointer) {
      int64_t mid = (left_pointer + right_pointer) / 2;
      if (b_right * mid <= cur) {
        elem = mid;
        left_pointer = mid + 1;
      } else {
        right_pointer = mid - 1;
      }
    }
    res.x_.push_back(elem);
    cur = cur - b_right * BigInt(elem);
    if (cur.x_[0] == 0) {
      cur.x_.clear();
    }
  }
  return Div(left, right, res);
}

BigInt& BigInt::operator/=(const BigInt& other) {
  BigInt& result = *this;
  result = result / other;
  return result;
}

BigInt& BigInt::operator%=(const BigInt& other) {
  *this = *this - (*this / other) * other;
  return *this;
}

BigInt BigInt::operator%(const BigInt& other) const {
  BigInt copy = *this;
  copy %= other;
  return copy;
}

BigInt operator*(const BigInt& lhs, const BigInt& rhs) {
  BigInt right = BigInt(0);
  BigInt lhs1 = lhs;
  BigInt rhs1 = rhs;
  reverse(lhs1.x_.begin(), lhs1.x_.end());
  reverse(rhs1.x_.begin(), rhs1.x_.end());
  right.x_.resize(lhs.x_.size() + rhs.x_.size());
  for (size_t i = 0; i < lhs.x_.size(); i++) {
    for (size_t j = 0; j < rhs.x_.size(); j++) {
      right.x_[i + j] += lhs1.x_[i] * rhs1.x_[j];
    }
  }
  for (size_t i = 0; i < right.x_.size(); i++) {
    if (right.x_[i] >= kName) {
      right.x_[i + 1] += right.x_[i] / kName;
      right.x_[i] %= kName;
    }
  }
  while (right.x_.size() > 1 && right.x_.back() == 0) {
    right.x_.pop_back();
  }
  right.sign_ = lhs.sign_ ^ rhs.sign_;
  reverse(right.x_.begin(), right.x_.end());
  if (right.x_[0] == 0) {
    right.sign_ = false;
  }
  return right;
}

std::istream& operator>>(std::istream& input, BigInt& rhs) {
  std::string str;
  input >> str;
  rhs = BigInt(str);
  return input;
}
std::ostream& operator<<(std::ostream& outstream, const BigInt& rhs) {
  if (rhs.sign_) {
    outstream << '-';
  }
  for (size_t i = 0; i < rhs.x_.size(); ++i) {
    outstream << rhs.x_[i];
  }
  return outstream;
}
