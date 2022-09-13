#include <string>

#include <catch2/catch.hpp>

#include "test_edge_node_overlap_utilities.h"
#include "test_utilities.h"

TEST_CASE(
    "Overlap",
    "[!shouldfail] An edge connected to a node shall not overlap that node") {

  const graph_options graph_options = {
      .node_shape = "polygon",
      .node_penwidth = 2,
      .edge_penwidth = 2,
  };

  const auto filename_base = AUTO_NAME();

  test_edge_node_overlap(graph_options, {}, {.filename_base = filename_base});
}
