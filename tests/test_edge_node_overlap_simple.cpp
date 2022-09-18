#include <string>

#include <catch2/catch.hpp>

#include "test_edge_node_overlap_utilities.h"
#include "test_utilities.h"

TEST_CASE(
    "Overlap",
    "[!shouldfail] An edge connected to a node shall not overlap that node") {

  std::string dot =
      "digraph {node[shape=polygon penwidth=2 fontname=Courier] a -> b}";

  const auto filename_base = AUTO_NAME();

  test_edge_node_overlap(dot, {.filename_base = filename_base});
}
