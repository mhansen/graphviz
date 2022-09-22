#include <string>

#include <catch2/catch.hpp>

#include "test_edge_node_overlap_utilities.h"
#include "test_utilities.h"

// FIXME: this test fails for all primitive arrow types except `normal` and
// `inv` which currently are the only ones fixed for penwidth

TEST_CASE("Edge node overlap for all primitive arrow shapes",
          "[!shouldfail] An edge connected to a node shall touch that node and "
          "not overlap it too much, regardless of the primitive arrow shape") {

  std::string filename_base =
      "test_edge_node_overlap_all_primitive_edge_arrows";

  const auto primitive_arrow_shape =
      GENERATE(from_range(all_primitive_arrow_shapes));
  INFO(fmt::format("Edge primitive arrow shape: {}", primitive_arrow_shape));

  INFO(fmt::format("Edge arrow shape: {}", primitive_arrow_shape));
  filename_base += fmt::format("_arrow_shape{}", primitive_arrow_shape);

  const graph_options graph_options = {
      .node_shape = "polygon",
      .node_penwidth = 2,
      .dir = "both",
      .edge_penwidth = 2,
      .primitive_arrowhead_shape = primitive_arrow_shape,
      .primitive_arrowtail_shape = primitive_arrow_shape,
  };

  test_edge_node_overlap(graph_options, {}, {.filename_base = filename_base});
}
