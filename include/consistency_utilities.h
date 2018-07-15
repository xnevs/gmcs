#ifndef GMCS_CONSISTENCY_UTILITIES_H_
#define GMCS_CONSISTENCY_UTILITIES_H_

#include "graph_traits.h"
#include "graph_utilities.h"


template <
    typename G,
    typename H,
    typename Map,
    typename Inv,
    typename EdgeEquiv>
bool adjacent_consistency_mono(
    G const & g,
    typename G::index_type u,
    H const & h,
    typename H::index_type v,
    Map const & map,
    Inv const & inv,
    EdgeEquiv const & edge_equiv) {
  auto n = h.num_vertices();
  for (auto oe : edges_or_out_edges(g, u)) {
    auto i = oe.target;
    auto j = map[i];
    if (j != n) {
      if (!h.edge(v, j) || !edge_equiv(g, u, oe, h, v, j)) {
        return false;
      }
    }
  }
  if constexpr (is_directed_v<G>) {
    for (auto ie : g.in_edges(u)) {
      auto i = ie.target;
      auto j = map[i];
      if (j != n) {
        if (!h.edge(j, v) || !edge_equiv(g, ie, u, h, j, v)) {
          return false;
        }
      }
    }
  }
  return true;
}

template <
    typename G,
    typename H,
    typename Map,
    typename Inv,
    typename EdgeEquiv>
bool adjacent_consistency_ind(
    G const & g,
    typename G::index_type u,
    H const & h,
    typename H::index_type v,
    Map const & map,
    Inv const & inv,
    EdgeEquiv const & edge_equiv) {
  if (!adjacent_consistency_mono(g, u, h, v, map, inv, edge_equiv)) {
    return false;
  }

  auto m = g.num_vertices();
  for (auto oe : edges_or_out_edges(h, v)) {
    auto j = oe.target;
    auto i = inv[j];
    if (i != m) {
      if (!g.edge(u, i)) {
        return false;
      }
    }
  }
  if constexpr (is_directed_v<H>) {
    for (auto ie : h.in_edges(v)) {
      auto j = ie.target;
      auto i = inv[j];
      if (i != m) {
        if (!g.edge(i, u)) {
          return false;
        }
      }
    }
  }
  return true;
}

template <typename G, typename H>
bool degree_condition(
    G const & g,
    typename G::index_type u,
    H const & h,
    typename H::index_type v) {
  if constexpr (is_directed_v<G>) {
    return g.out_degree(u) <= h.out_degree(v) && g.in_degree(u) <= h.in_degree(v);
  } else {
    return g.degree(u) <= h.degree(v);
  }
}

#endif  // GMCS_CONSISTENCY_UTILITIES_H_