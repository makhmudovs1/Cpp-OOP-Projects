#pragma once

int* LowerBound(int* first, const int* last, int value) {
  int l = 0;
  int r = last - first;
  while (l < r) {
    int mid = (l + r) >> 1;
    int x = *(first + mid);
    if (x >= value) {
      r = mid;
    } else {
      l = mid + 1;
    }
  }
  return first + r;
}
