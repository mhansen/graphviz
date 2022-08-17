#include <string_view>

#include <catch2/catch.hpp>
#include <fmt/format.h>

#include "svg_analyzer.h"
#include "test_utilities.h"
#include <cgraph++/AGraph.h>
#include <gvc++/GVContext.h>
#include <gvc++/GVLayout.h>
#include <gvc++/GVRenderData.h>

TEST_CASE("Graph rankdir", "Test that the Graphviz `rankdir` attribute affects "
                           "the relative placement of nodes and edges "
                           "correctly when the 'dot` layout engine is used") {

  const auto rankdir = GENERATE(from_range(all_rank_directions));
  INFO(fmt::format("Rankdir: {}", rankdir));

  const auto shape = GENERATE_COPY(filter(
      [](std::string_view shape) {
        return !node_shapes_without_svg_shape.contains(shape);
      },
      from_range(all_node_shapes)));
  INFO(fmt::format("Shape: {}", shape));

  auto dot = fmt::format(
      "digraph g1 {{rankdir={}; node [shape={} fontname=Courier]; a -> b}}",
      rankdir, shape);
  auto g = CGraph::AGraph{dot};

  const auto demand_loading = false;
  auto gvc = GVC::GVContext{lt_preloaded_symbols, demand_loading};

  const auto layout = GVC::GVLayout(std::move(gvc), std::move(g), "dot");

  const auto result = layout.render("svg");
  SVGAnalyzer svgAnalyzer{result.c_str()};

  REQUIRE(svgAnalyzer.graphs().size() == 1);
  const auto &graph = svgAnalyzer.graphs().back();
  const auto node_a = graph.node("a");
  const auto node_b = graph.node("b");
  const auto edge_ab = graph.edge("a->b");
  if (rankdir == "TB") {
    CHECK(node_a.center().is_higher_than(node_b.center()));
  } else if (rankdir == "BT") {
    CHECK(node_a.center().is_lower_than(node_b.center()));
  } else if (rankdir == "LR") {
    CHECK(node_a.center().is_more_left_than(node_b.center()));
  } else if (rankdir == "RL") {
    CHECK(node_a.center().is_more_right_than(node_b.center()));
  }
}
