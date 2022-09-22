#include <iostream>
#include <string>

#include <catch2/catch.hpp>
#include <fmt/format.h>

#include "test_edge_node_overlap_utilities.h"
#include "test_utilities.h"

TEST_CASE("Overlap ellipse node shapes",
          "[!shouldfail] Test that an edge connected to an ellipse based node "
          "touches that node and does not overlap it too much, regardless of "
          "the node shape") {

  const auto shape = GENERATE_COPY(
      filter([](const std::string_view shape) { return shape != "point"; },
             from_range(node_shapes_consisting_of_ellipse)));

  INFO(fmt::format("Node shape: {}", shape));

  const auto rankdir = GENERATE(from_range(all_rank_directions));
  INFO(fmt::format("Rank direction: {}", rankdir));

  const graph_options graph_options = {
      .rankdir = rankdir,
      .node_shape = shape,
      .node_penwidth = 2,
      .edge_penwidth = 2,
  };

  const auto filename_base = fmt::format("{}_{}", AUTO_NAME(), shape);

  test_edge_node_overlap(graph_options, {}, {.filename_base = filename_base});
}
