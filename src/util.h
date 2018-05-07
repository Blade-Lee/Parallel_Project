#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <cstdarg>
#include <cstdio>

#if defined(__APPLE__)
//  #if defined(__x86_64__)
#if 0
#include <sys/sysctl.h>
#else
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif // __x86_64__ or not

#elif _WIN32
#include <time.h>
#include <windows.h>
#else
#include <string.h>
#include <sys/time.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  // fprintf
#include <stdlib.h> // exit

#if TIMING
#define timing_get_start_time(...) get_start_time(__VA_ARGS__)
#define timing_get_end_time(...) get_end_time(__VA_ARGS__)
#define timing_add_record_list(...) add_record_list(__VA_ARGS__)
#define timing_free_record_list(...) free_record_list(__VA_ARGS__)
#define timing_print_record_info(...) print_record_info(__VA_ARGS__)
#else
#define timing_get_start_time(...)
#define timing_get_end_time(...)
#define timing_add_record_list(...)
#define timing_free_record_list(...)
#define timing_print_record_info(...)
#endif

using namespace std;

// Every time a new enum is added, please add corresponding string to method
// map_record_to_string() function in cycletimer.c
typedef enum { READ_FILE = 0, LAST } record_type_t;

typedef struct record {
  double time;
  record_type_t record_type;
  struct record *next;
} record_node_t;

typedef struct {
  double start_time;
  double end_time;
} time_pair_t;

typedef uint64_t SysClock;

static record_node_t *global_record_head = NULL;
static time_pair_t global_time_pair;

//////////
// Return the current CPU time, in terms of clock ticks.
// Time zero is at some arbitrary point in the past.
static SysClock currentTicks() {
//#if defined(__APPLE__) && !defined(__x86_64__)
#if defined(__APPLE__)
  return mach_absolute_time();
#elif defined(_WIN32)
  LARGE_INTEGER qwTime;
  QueryPerformanceCounter(&qwTime);
  return qwTime.QuadPart;
#elif defined(__x86_64__)
  unsigned int a, d;
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
  return ((uint64_t)d << 32) + a;
#elif defined(__ARM_NEON__) && 0 // mrc requires superuser.
  unsigned int val;
  asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(val));
  return val;
#else
  timespec spec;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &spec);
  return (uint64_t)spec.tv_sec * 1000 * 1000 * 1000 + spec.tv_nsec;
#endif
}

//////////
// Return the conversion from ticks to seconds.
static double secondsPerTick() {
  static bool initialized = false;
  static double secondsPerTick_val;
  if (initialized)
    return secondsPerTick_val;
#if defined(__APPLE__)
#if 0
  //  #ifdef __x86_64__
    int args[] = {CTL_HW, HW_CPU_FREQ};
    unsigned int Hz;
    size_t len = sizeof(Hz);
    if (sysctl(args, 2, &Hz, &len, NULL, 0) != 0) {
	fprintf(stderr, "Failed to initialize secondsPerTick_val!\n");
	exit(-1);
    }
    secondsPerTick_val = 1.0 / (double) Hz;
#else
  mach_timebase_info_data_t time_info;
  mach_timebase_info(&time_info);

  // Scales to nanoseconds without 1e-9f
  secondsPerTick_val =
      (1e-9 * (double)time_info.numer) / (double)time_info.denom;
#endif // x86_64 or not
#elif defined(_WIN32)
  LARGE_INTEGER qwTicksPerSec;
  QueryPerformanceFrequency(&qwTicksPerSec);
  secondsPerTick_val = 1.0 / (double)qwTicksPerSec.QuadPart;
#else
  FILE *fp = fopen("/proc/cpuinfo", "r");
  char input[1024];
  if (!fp) {
    fprintf(stderr, "cycletimer failed: couldn't find /proc/cpuinfo.");
    exit(-1);
  }
  // In case we don't find it, e.g. on the N900
  secondsPerTick_val = 1e-9;
  while (!feof(fp) && fgets(input, 1024, fp)) {
    // NOTE(boulos): Because reading cpuinfo depends on dynamic
    // frequency scaling it's better to read the @ sign first
    float GHz, MHz;
    if (strstr(input, "model name")) {
      char *at_sign = strstr(input, "@");
      if (at_sign) {
        char *after_at = at_sign + 1;
        char *GHz_str = strstr(after_at, "GHz");
        char *MHz_str = strstr(after_at, "MHz");
        if (GHz_str) {
          *GHz_str = '\0';
          if (1 == sscanf(after_at, "%f", &GHz)) {
            // printf("GHz = %f\n", GHz);
            secondsPerTick_val = 1e-9f / GHz;
            break;
          }
        } else if (MHz_str) {
          *MHz_str = '\0';
          if (1 == sscanf(after_at, "%f", &MHz)) {
            // printf("MHz = %f\n", MHz);
            secondsPerTick_val = 1e-6f / GHz;
            break;
          }
        }
      }
    } else if (1 == sscanf(input, "cpu MHz : %f", &MHz)) {
      // printf("MHz = %f\n", MHz);
      secondsPerTick_val = 1e-6f / MHz;
      break;
    }
  }
  fclose(fp);
#endif

  initialized = true;
  return secondsPerTick_val;
}

//////////
// Return the current CPU time, in terms of seconds.
// This is slower than currentTicks().  Time zero is at
// some arbitrary point in the past.
inline double currentSeconds() { return currentTicks() * secondsPerTick(); }

void print_msg(const char *fmt, ...) {
  va_list ap;
  bool got_newline = fmt[strlen(fmt) - 1] == '\n';
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  if (!got_newline)
    fprintf(stderr, "\n");
}

inline void get_start_time() {
  global_time_pair.start_time = currentTicks() * secondsPerTick();
}

inline void get_end_time() {
  global_time_pair.end_time = currentTicks() * secondsPerTick();
}

void add_record_list(record_type_t record_type) {
  record_node_t *new_node = (record_node_t *)malloc(sizeof(record_node_t));
  new_node->time = global_time_pair.end_time - global_time_pair.start_time;
  assert(new_node->time > 0.000000000000001);
  new_node->record_type = record_type;
  new_node->next = NULL;

  if (!(global_record_head)) {
    global_record_head = new_node;
    return;
  }

  new_node->next = global_record_head->next;
  global_record_head->next = new_node;
}

void free_record_list() {
  if (!global_record_head)
    return;

  record_node_t *next = global_record_head->next;

  while (true) {
    free(global_record_head);
    if (!next)
      break;
    global_record_head = next;
    next = global_record_head->next;
  }
}

string map_record_to_string(record_type_t record_type) {
  switch (record_type) {
  case (READ_FILE):
    return "read_graph";
  default:
    return "Unknown record type";
  }
}

void print_record_info() {
  if (!global_record_head) {
    print_msg("No info to be printed!\n");
  }

  int total_enums = LAST;
  double total_time = 0.0;
  double *time_accumulate = (double *)malloc(total_enums * sizeof(double));

  int i;
  for (i = 0; i < total_enums; i++) {
    time_accumulate[i] = 0.0;
  }

  record_node_t *temp = global_record_head;
  while (temp) {
    time_accumulate[temp->record_type] += temp->time;
    total_time += temp->time;
    temp = temp->next;
  }

  print_msg("*******************************************************\n"
            "Time usage distribution:\n"
            "*******************************************************\n");

  for (i = 0; i < total_enums; i++) {
    if (time_accumulate[i] > 0.00000001) {
      print_msg("| %-20s | %10.2fms | %10.2f%% |\n",
                map_record_to_string((record_type_t)i).c_str(),
                time_accumulate[i] * 1000,
                time_accumulate[i] / total_time * 100);
    }
  }

  print_msg("*******************************************************\n"
            "Total time used: %.2fms\n"
            "*******************************************************\n",
            total_time * 1000);

  free_record_list();
}

/**
 * Read lines from a file
 *
 * @param file_name : the input file
 * @return a queue of strings, each of which is a line in file_name, with ending
 * '\n' stripped
 */
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

/**
 * Check whether a string is a number
 *
 * @param s : the input string
 * @return if the string is a number
 */
bool isNumber(const string &s) {
  return !s.empty() &&
         find_if(s.begin(), s.end(), [](char c) { return !isdigit(c); }) ==
             s.end();
}

/**
 * Given an input string (a node), find its corresponding node id
 *
 * @param assign : the assignment map for leaves
 * @param cur_leave : the node id of the next input leave
 * @param input_str : the input ndoe
 * @param output_num : the id of the node in the tree
 */
void assign_number(unordered_map<string, int> &assign, int &cur_leave,
                   const string &input_str, int &output_num) {
  if (isNumber(input_str)) {
    output_num = stoi(input_str);
  } else {
    if (assign.find(input_str) == assign.end()) {
      output_num = cur_leave--;
      assign.emplace(input_str, output_num);
    } else {
      output_num = assign.at(input_str);
    }
  }
}

/**
 * Given one line, extract the neighbors and store the information
 *
 * @param line : the input string line
 * @param assign : the assignment map for leaves
 * @param cur_leave : the node id of the next input leave
 * @return : the pair of node ids in the line
 */
tuple<int, int> getNeighborPair(string line, unordered_map<string, int> &assign,
                                int &cur_leave) {
  string delimiter = "->";
  auto del_idx = line.find(delimiter);
  string first = line.substr(0, del_idx);
  string second = line.substr(del_idx + 2, line.length());
  int first_number, second_number;

  assign_number(assign, cur_leave, first, first_number);
  assign_number(assign, cur_leave, second, second_number);

  return tuple<int, int>(first_number, second_number);
}

/**
 * Given a pair of node ids, store their neighboring information in a map
 *
 * @param neighbors : the map storing neighboring information
 * @param first : first node id
 * @param second : second node id
 */
void connectNeighborPair(unordered_map<int, unordered_set<int>> &neighbors,
                         int first, int second) {
  if (neighbors.find(first) != neighbors.end()) {
    neighbors[first].insert(second);
  } else {
    neighbors[first] = {second};
  }

  if (neighbors.find(second) != neighbors.end()) {
    neighbors[second].insert(first);
  } else {
    neighbors[second] = {first};
  }
}

/**
 * Given a neighboring map, generate two arrays for the neighboring
 * relationships.
 *
 * There are N nodes and N - 1 edges in the tree, thus there are 2 * (N - 1)
 * neighboring relationships
 *
 * @param neighbors : the map storing neighboring information
 * @param undirected_idx : undirected_idx[a] == b means that the neighbors of a
 * are stored at neighbor_arr[b]
 * @param neighbor_arr : self-explained
 */
void convertNeighborsToUndirectedArr(
    const unordered_map<int, unordered_set<int>> &neighbors,
    shared_ptr<int> &undirected_idx, shared_ptr<int> &neighbor_arr) {
  int next_neighbor = 0;
  auto temp_idx = undirected_idx.get();
  auto temp_arr = neighbor_arr.get();

  for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
    auto node_num = it->first;
    auto temp_start = next_neighbor;
    temp_idx[node_num] = temp_start;
    for (auto sit = (it->second).begin(); sit != (it->second).end(); ++sit) {
      temp_arr[temp_start++] = *sit;
    }
    next_neighbor = temp_start;
  }
}

/**
 * Given an unrooted undirected tree (N nodes, N - 1 edges, 2 * (N - 1)
 * neighboring relationships), generate a rooted directed tree (N + 1 nodes, N
 * edges, N directed relationships)
 *
 * @param num_undirected_nodes : N
 * @param num_leaves : number of leaves
 * @param undirected_idx : undirected_idx[a] == b means that the neighbors of a
 * are stored at neighbor_arr[b]
 * @param neighbor_arr : self-explained
 * @param directed_idx : directed_idx[a] == b means that the children of a are
 * stored at children_arr[b]
 * @param children_arr : self-explained
 */
void covertUndirectedToDirected(const int num_undirected_nodes,
                                const int num_leaves,
                                shared_ptr<int> &undirected_idx,
                                shared_ptr<int> &neighbor_arr,
                                shared_ptr<int> &directed_idx,
                                shared_ptr<int> &children_arr) {
  auto tmp_undirected_idx = undirected_idx.get();
  auto tmp_neighbor_arr = neighbor_arr.get();
  auto tmp_directed_idx = directed_idx.get();
  auto tmp_children_arr = children_arr.get();

  int root = num_undirected_nodes;
  int left = num_undirected_nodes - 1;
  int right = tmp_neighbor_arr[tmp_undirected_idx[left]];
  int next_children = 0;

  for (int i = 0; i < num_undirected_nodes + 1; ++i) {
    tmp_directed_idx[i] = -1;
  }

  auto temp_start = next_children;
  tmp_directed_idx[root] = temp_start;
  tmp_children_arr[temp_start++] = left;
  tmp_children_arr[temp_start++] = right;
  next_children = temp_start;

  queue<int> q;
  q.emplace(left);
  q.emplace(right);
  unordered_set<int> visited;
  visited.insert(root);
  visited.insert(left);
  visited.insert(right);

  while (!q.empty()) {
    auto cur_node = q.front();
    q.pop();

    if (cur_node < num_leaves) {
      continue;
    }

    auto undirected_start_pos = tmp_undirected_idx[cur_node];
    auto directed_start_pos = next_children;
    tmp_directed_idx[cur_node] = directed_start_pos;
    for (int i = undirected_start_pos; i < undirected_start_pos + 3; ++i) {
      auto neighbor = tmp_neighbor_arr[i];
      if (visited.find(neighbor) == visited.end()) {
        tmp_children_arr[directed_start_pos++] = neighbor;
        q.emplace(neighbor);
        visited.insert(neighbor);
      }
    }
    next_children = directed_start_pos;
  }
}

/**
 * Given a assignment map, populate the chars to a char list
 * @param char_list : length: (str_len) * (N + 1)
 * @param assign : the assignment map for leaves
 * @param num_char_trees : str_len
 * @param num_directed_nodes : N + 1
 */
void initializeCharList(shared_ptr<char> &char_list,
                        unordered_map<string, int> &assign, int num_char_trees,
                        int num_directed_nodes) {
  auto tmp_char_list = char_list.get();
  for (auto it = assign.begin(); it != assign.end(); ++it) {
    const char *chars = (it->first).c_str();
    int node = it->second;
    for (int tree = 0; tree < num_char_trees; ++tree) {
      auto char_pos = tree * num_directed_nodes + node;
      tmp_char_list[char_pos] = chars[tree];
    }
  }
}