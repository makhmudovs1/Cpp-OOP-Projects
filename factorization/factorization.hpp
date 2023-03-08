#pragma once
#include <vector>

std::vector<int> Factorize(int factorize) {
  /* Not Implemented */
  int l = 2;
  std::vector<int> v;
  while (factorize > 1) {
    while (factorize % l == 0) {
      v.push_back(l);
      factorize /= l;
    }
    l++;
  }
  return {v};
}
