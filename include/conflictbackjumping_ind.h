#ifndef GMCS_CONFLICTBACKJUMPING_IND_H_
#define GMCS_CONFLICTBACKJUMPING_IND_H_

#include <iterator>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "graph_traits.h"
#include "label_equivalence.h"

template <
    typename G,
    typename H,
    typename Callback,
    typename IndexOrderG,
    typename VertexEquiv = default_vertex_label_equiv<G, H>,
    typename EdgeEquiv = default_edge_label_equiv<G, H>>
void conflictbackjumping_ind(
    G const & g,
    H const & h,
    Callback const & callback,
    IndexOrderG const & index_order_g,
    VertexEquiv const & vertex_equiv = VertexEquiv(),
    EdgeEquiv const & edge_equiv = EdgeEquiv()) {

  using IndexG = typename G::index_type;
  using IndexH = typename H::index_type;

  struct explorer {

    G const & g;
    H const & h;
    Callback callback;

    IndexOrderG const & index_order_g;

    vertex_equiv_helper<VertexEquiv> vertex_equiv;
    edge_equiv_helper<EdgeEquiv> edge_equiv;

    IndexG m;
    IndexH n;

    IndexG level;

    std::vector<IndexH> map;

    IndexG backjump_level;
    std::vector<boost::dynamic_bitset<>> conflicts;

    explorer(
        G const & g,
        H const & h,
        Callback const & callback,
        IndexOrderG const & index_order_g,
        VertexEquiv const & vertex_equiv,
        EdgeEquiv const & edge_equiv)
        : g{g},
          h{h},
          callback{callback},
          index_order_g{index_order_g},
          vertex_equiv{vertex_equiv},
          edge_equiv{edge_equiv},

          m{g.num_vertices()},
          n{h.num_vertices()},
          level{0},
          map(m, n),
          backjump_level{m},
          conflicts(m, boost::dynamic_bitset<>(m)) {
    }

    bool explore() {
      if (level == m) {
        backjump_level = m;
        conflicts[m-1].set();
        return callback();
      } else {
        auto x = index_order_g[level];
        conflicts[level].reset();
        backjump_level = level+1;
        bool proceed = true;
        for (IndexH y=0; y<n; ++y) {
          if (consistency(y)) {
            map[x] = y;
            ++level;
            proceed = explore();
            --level;
            map[x] = n;
            if (!proceed || backjump_level <= level) {
              return proceed;
            }
          }
        }
        auto pos = conflicts[level].find_next(m-1-level);
        if (pos != boost::dynamic_bitset<>::npos) {
          backjump_level = m - pos;
        } else {
          backjump_level = 0;
        }

        if (backjump_level > 0) {
          conflicts[backjump_level-1] |= conflicts[level];
        }
        return proceed;
      }
    }

    bool consistency(IndexH y) {
      auto x = index_order_g[level];

      if (!vertex_equiv(g, x, h, y)) {
        return false;
      }

      for (IndexG i=0; i<level; ++i) {
        auto u = index_order_g[i];
        auto v = map[u];
        if (v == y) {
          conflicts[level].set(m-1-i);
          return false;
        }
        auto x_out = g.edge(x, u);
        if (x_out != h.edge(y, v) || (x_out && !edge_equiv(g, x, u, h, y, v))) {
          conflicts[level].set(m-1-i);
          return false;
        }
        if constexpr (is_directed_v<G>) {
          auto x_in = g.edge(u, x);
          if (x_in != h.edge(v, y) || (x_in && !edge_equiv(g, u, x, h, v, y))) {
            conflicts[level].set(m-1-i);
            return false;
          }
        }
      }
      return true;
    }
  } e(g, h, callback, index_order_g, vertex_equiv, edge_equiv);

  e.explore();
}

#endif  // GMCS_CONFLICTBACKJUMPING_IND_H_
