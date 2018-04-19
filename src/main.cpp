#include <iostream>
#include "util.h"

using namespace std;

int main() {
  cout << "Hello World!" << endl;
  auto ptr = test(5);
  cout << "int: " << *ptr << endl;
  return 0;
}