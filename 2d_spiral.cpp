int** MakeSpiral(int n) {
  int left_border = 0;
  int right_border = n - 1;
  int** matrix = new int*[n];
  for (int i = 0; i < n; ++i) {
    matrix[i] = new int[n];
  }
  int ans = 0;
  while (ans < n * n) {
    for (int i = left_border; i <= right_border; ++i) {
      matrix[left_border][i] = ++ans;
    }
    if (ans >= n * n) {
      break;
    }
    for (int i = left_border + 1; i <= right_border; ++i) {
      matrix[i][right_border] = ++ans;
    }
    for (int i = right_border - 1; i >= left_border; --i) {
      matrix[right_border][i] = ++ans;
    }
    for (int i = right_border - 1; i > left_border; --i) {
      matrix[i][left_border] = ++ans;
    }
    ++left_border;
    --right_border;
  }
  return matrix;
}
