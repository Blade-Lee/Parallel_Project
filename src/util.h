#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <string>

using namespace std;

typedef struct int_temp {
  int num;
  bool is_zero;

  int_temp(int input) {
    num = input;
    is_zero = input == 0 ? true : false;
  }

} int_t;

queue<string> readLines(string file_name) {
  queue<string> lines;
  ifstream in(file_name.c_str());

  if (!in) {
    return lines;
  }

  string str;
  while (getline(in, str)) {
    lines.emplace(str);
  }

  return lines;
}

void runBaseline(string file_name, string outfile_name) {
  auto lines = readLines(file_name);

  int num_leaves = stoi(lines.front());
  lines.pop();

  while (!lines.empty()) {
    auto line = lines.front();
    lines.pop();

    // TODO: continue to parse the lines and construct arrays
  }
}

shared_ptr<int_t> test(int temp) { return make_shared<int_t>(temp); }