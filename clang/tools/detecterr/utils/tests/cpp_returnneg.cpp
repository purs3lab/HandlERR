#include <cmath> // for sqrt() function
#include <iostream>

int retneg1() {
  int x = 1;
  // should be included
  if (x == 1) {
    return -1;
  }
}

int retneg2() {
  int x = 1;
  // should be included
  if (x == 1) {
    return static_cast<uintmax_t>(-1);
  }
}

int main() {
  std::cout << "Enter a number: ";
  double x{};
  std::cin >> x;
}
