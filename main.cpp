#include <cstdint>

#include <fstream>
#include <iostream>

#include "include/read_amalfi.h"

#include "include/simple_adjacency_list.h"
#include "include/adjacency_listmat.h"

#include "include/vertex_order.h"

#include "include/backtracking_ind.h"
#include "include/backtracking_degreeprune_ind.h"
#include "include/backtracking_adjacentconsistency_ind.h"
#include "include/backtracking_degreeprune_adjacentconsistency_ind.h"
#include "include/backtracking_adjacentconsistency_backwardcount_ind.h"
#include "include/backtracking_degreeprune_adjacentconsistency_backwardcount_ind.h"
#include "include/backtracking_adjacentconsistency_forwardcount_ind.h"
#include "include/backtracking_degreeprune_adjacentconsistency_forwardcount_ind.h"
#include "include/backtracking_parent_ind.h"
#include "include/backtracking_forwardparent_ind.h"

int main(int argc, char * argv[]) {
  char const * g_filename = argv[1];
  char const * h_filename = argv[2];

  std::ifstream in{g_filename,std::ios::in|std::ios::binary};
  auto g_ = read_amalfi<simple_adjacency_list<uint16_t>>(in);
  in.close();
  in.open(h_filename,std::ios::in|std::ios::binary);
  auto h_ = read_amalfi<simple_adjacency_list<uint16_t>>(in);
  in.close();
  
  adjacency_listmat<decltype(g_)::index_type> g{g_};
  adjacency_listmat<decltype(h_)::index_type> h{h_};

  auto index_order_g = vertex_order_GreatestConstraintFirst(g);
  
  int count = 0;
  backtracking_parent_ind(
      g,
      h,
      [](auto x, auto y) {return true;},
      [](auto x0, auto x1, auto y0, auto y1) {return true;},
      index_order_g,
      [&count]() {++count; return true;});
  
  std::cout << count << std::endl;
}
