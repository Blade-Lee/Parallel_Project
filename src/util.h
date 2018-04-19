#include <memory>

using namespace std;

typedef struct int_temp {
  int num;
  bool is_zero;

  int_temp(int input) {
    num = input;
    is_zero = input == 0 ? true : false;
  }

} int_t;

shared_ptr<int_t> test(int temp) { return make_shared<int_t>(temp); }