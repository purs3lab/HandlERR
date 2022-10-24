#include <cmath> // for sqrt() function
#include <iostream>

int throw_exception() {
  int x = 1;
  // should be included
  if (x == 1) {
    throw "Can not take sqrt of negative number"; // throw exception of type const char*
  }
}

int main() {
  std::cout << "Enter a number: ";
  double x{};
  std::cin >> x;

  throw_exception();
}
