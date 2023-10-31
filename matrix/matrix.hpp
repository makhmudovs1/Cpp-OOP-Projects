#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

template <size_t N, size_t M, typename T = int64_t>
class Matrix;

template <size_t N, size_t M, typename T>
bool operator==(const Matrix<N, M, T>& matrix, const Matrix<N, M, T>& other);

template <size_t N, size_t M, typename T>
class Matrix {
 private:
  std::vector<std::vector<T>> matrix_;

 public:
  Matrix() : matrix_(N, std::vector<T>(M, 0)) {}
  Matrix(T value) : matrix_(N, std::vector<T>(M, value)) {}
  Matrix(std::vector<std::vector<T>>& other) { matrix_ = other; }

  T& operator()(const size_t& index_i, const size_t& index_j) {
    return matrix_[index_i][index_j];
  }

  const T& operator()(const size_t& index_i, const size_t& index_j) const {
    return matrix_[index_i][index_j];
  }

  Matrix<N, M, T>& operator+=(const Matrix<N, M, T>& other) {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        matrix_[i][j] += other(i, j);
      }
    }
    return *this;
  }
  Matrix<N, M, T>& operator+(const Matrix<N, M, T>& other) {
    *this += other;
    return *this;
  }

  Matrix<N, M, T>& operator-=(const Matrix<N, M, T>& other) {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        matrix_[i][j] -= other(i, j);
      }
    }
    return *this;
  }
  Matrix<N, M, T>& operator-(const Matrix<N, M, T>& other) {
    *this -= other;
    return *this;
  }
  Matrix<N, M, T> operator*(const T kElem) {
    Matrix<N, M, T> new_matrix;
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        new_matrix(i, j) = matrix_[i][j] * kElem;
      }
    }
    return new_matrix;
  }

  Matrix<M, N, T> Transposed() const {
    Matrix<M, N, T> new_matrix;
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        new_matrix(i, j) = matrix_[j][i];
      }
    }
    return new_matrix;
  }

  T Trace() const {
    T sum = 0;
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        if (i == j) {
          sum += matrix_[i][j];
        }
      }
    }
    return sum;
  }
};

template <size_t N, size_t M, size_t Ka, typename T = int64_t>
Matrix<N, Ka, T> operator*(const Matrix<N, M, T>& matrix,
                           const Matrix<M, Ka, T>& other) {
  Matrix<N, Ka, T> new_matrix;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < Ka; ++j) {
      for (size_t index = 0; index < M; ++index) {
        new_matrix(i, j) += matrix(i, index) * other(index, j);
      }
    }
  }
  return new_matrix;
}

template <size_t N, size_t M, typename T>
bool operator==(const Matrix<N, M, T>& matrix, const Matrix<N, M, T>& other) {
  return (matrix == other);
}

template <size_t N, size_t M, size_t Mb, size_t Nb, typename T>
Matrix<N, Mb, T> operator*(const Matrix<N, M, T>& lhl,
                           const Matrix<Nb, Mb, T>& rhl) {
  Matrix<N, Mb, T> new_matrix;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < Mb; ++j) {
      for (size_t k = 0; k < N; ++k) {
        new_matrix(i, j) += lhl(i, k) * rhl(k, j);
      }
    }
  }
  return new_matrix;
}

