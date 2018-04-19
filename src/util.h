#include <memory>

using namespace std;

shared_ptr<int> test(int temp) { return make_shared<int>(temp); }