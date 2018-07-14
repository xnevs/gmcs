#ifndef GMCS_VERTEX_ORDER_H_
#define GMCS_VERTEX_ORDER_H_

#include <utility>
#include <algorithm>
#include <numeric>
#include <tuple>
#include <vector>
#include <set>

template <typename G>
std::vector<typename G::index_type> vertex_order_DEG(G const & g) {
  std::vector<typename G::index_type> order(g.num_vertices());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&g](auto u, auto v) {
    return g.degree(u) > g.degree(v);
  });
  return order;
}

template <typename G>
std::vector<typename G::index_type> vertex_order_RDEG(G const & g) {
  using Index = typename G::index_type;

  auto n = g.num_vertices();

  std::vector<Index> vertex_order(n);

  std::vector<bool> avail(n, true);
  for (Index idx=0; idx<n; ++idx) {
    auto bestn = n;
    int bestv = -1;
    for (Index i=0; i<n; ++i) {
      if (avail[i]) {
        auto i_out_edges = g.out_edges(i);
        auto i_in_edges = g.in_edges(i);
        int rdeg =
            std::count_if(std::begin(i_out_edges), std::end(i_out_edges), [&avail](auto oe) {
              return !avail[oe.target];
            })
            +
            std::count_if(std::begin(i_in_edges), std::end(i_in_edges), [&avail](auto ie) {
              return !avail[ie.target];
            });
        if (bestn == n || std::make_pair(rdeg, g.degree(i)) > std::make_pair(bestv, g.degree(bestn))) {
          bestn = i;
          bestv = rdeg;
        }
      }
    }
    avail[bestn] = false;
    vertex_order[idx] = bestn;
  }
  return vertex_order;
}

template <typename G>
std::vector<typename G::index_type> vertex_order_GreatestConstraintFirst(G const & g) {
  using Index = typename G::index_type;

  auto n = g.num_vertices();

  std::vector<Index> vertex_order(n);
  std::iota(std::begin(vertex_order), std::end(vertex_order), 0);

  enum struct Flag {
    vis,
    neigh,
    unv
  };

  std::vector<Flag> flags(n, Flag::unv);
  std::vector<std::tuple<Index,Index,Index,Index>> ranks(n);

  for (Index i=0; i<n; ++i) {
    std::get<2>(ranks[i]) = g.degree(i);
    std::get<3>(ranks[i]) = i;
  }

  for (Index m=0; m<n; ++m) {
    auto u_it = std::max_element(
        std::next(std::begin(vertex_order), m),
        std::end(vertex_order),
        [&ranks](auto u, auto v) {
          return ranks[u] < ranks[v];
        });
    Index u = *u_it;

    if (flags[u] == Flag::unv) {
      for (auto oe : g.out_edges(u)) {
        --std::get<2>(ranks[oe.target]);
      }
      for (auto ie : g.in_edges(u)) {
        --std::get<2>(ranks[ie.target]);
      }
    } else if (flags[u] == Flag::neigh) {
      for (auto oe : g.out_edges(u)) {
        --std::get<1>(ranks[oe.target]);
      }
      for (auto ie : g.in_edges(u)) {
        --std::get<1>(ranks[ie.target]);
      }
    }

    std::swap(vertex_order[m], *u_it);
    flags[u] = Flag::vis;

    for (auto oe : g.out_edges(u)) {
      auto v = oe.target;
      ++std::get<0>(ranks[v]);
      if (flags[v] == Flag::unv) {
        flags[v] = Flag::neigh;
        for (auto v_oe : g.out_edges(v)) {
          ++std::get<1>(ranks[v_oe.target]);
        }
        for (auto v_ie : g.in_edges(v)) {
          ++std::get<1>(ranks[v_ie.target]);
        }
      }
    }
    for (auto ie : g.in_edges(u)) {
      auto v = ie.target;
      ++std::get<0>(ranks[v]);
      if (flags[v] == Flag::unv) {
        flags[v] = Flag::neigh;
        for (auto v_oe : g.out_edges(v)) {
          ++std::get<1>(ranks[v_oe.target]);
        }
        for (auto v_ie : g.in_edges(v)) {
          ++std::get<1>(ranks[v_ie.target]);
        }
      }
    }
  }
  return vertex_order;
}

#endif  // GMCS_VERTEX_ORDER_H_
