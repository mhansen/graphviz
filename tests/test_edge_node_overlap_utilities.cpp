#include <catch2/catch.hpp>
#include <cmath>
#include <fmt/format.h>

#include "svg_analyzer.h"
#include "test_edge_node_overlap_utilities.h"

/// check that edges do not overlap nodes
static bool check_analyzed_svg(SVGAnalyzer &svg_analyzer,
                               const check_options &check_options) {

  REQUIRE(svg_analyzer.graphs().size() == 1);
  auto &recreated_graph = svg_analyzer.graphs().back();

  auto &tail_node = recreated_graph.node("a");
  auto &head_node = recreated_graph.node("b");
  auto &edge = recreated_graph.edge("a->b");

  auto success = true;

// macro for doing the actual check and continue the execution in the same test
// case even if the assertion fails, while still capturing the result to be
// used to decide whether to write SVG files at the end of the test case
#define DO_CHECK(condition)                                                    \
  do {                                                                         \
    CHECK(condition);                                                          \
    success = success && (condition);                                          \
  } while (0)

  const auto edge_bbox = edge.outline_bbox();

  // check head node and edge overlap
  {
    const auto head_node_bbox = head_node.outline_bbox();
    const auto overlap_bbox = edge_bbox.intersection(head_node_bbox);
    INFO("Head node overlap:");
    INFO(fmt::format("  width:  {:.3f}", overlap_bbox.width));
    INFO(fmt::format("  height: {:.3f}", overlap_bbox.height));
    // FIXME: add support for rank direction "LR" and "RL". For now assume
    // "TB" or "BT" and check only in the vertical direction
    const auto head_node_edge_overlap = overlap_bbox.height;

    // check maximum head node and edge overlap
    DO_CHECK(head_node_edge_overlap <=
             check_options.max_node_edge_overlap +
                 check_options.svg_rounding_error * 2);
  }

  // check tail node and edge overlap
  {
    const auto tail_node_bbox = tail_node.outline_bbox();
    const auto overlap_bbox = edge_bbox.intersection(tail_node_bbox);
    INFO("Tail node overlap:");
    INFO(fmt::format("  width:  {:.6f}", overlap_bbox.width));
    INFO(fmt::format("  height: {:.6f}", overlap_bbox.height));
    // FIXME: add support for rank direction "LR" and "RL". For now assume
    // "TB" or "BT" and check only in the vertical direction
    const auto tail_node_edge_overlap = overlap_bbox.height;

    // check maximum tail node and edge overlap
    DO_CHECK(tail_node_edge_overlap <=
             check_options.max_node_edge_overlap +
                 check_options.svg_rounding_error * 2);
  }

  return success;
}

void test_edge_node_overlap(const std::string &dot) {
  auto svg_analyzer = SVGAnalyzer::make_from_dot(dot);

  // The binary search in the bezier_clip function in lib/common/splines.c has a
  // limit for when to consider the boundary found and to be the point inside
  // the boundary. It is the maximum distance between two points on a bezier
  // curve that are on opposite sides of the node boundary (for shape_clip) or
  // on the opposite sides of the boundary of a virtual circle at a specified
  // distance from a given point (for arrow_clip). An margin is needed to
  // account for the error that this limit introduces.
  const double graphviz_bezier_clip_margin = 0.5;
  const int graphviz_num_decimals_in_svg = 2;
  const double graphviz_max_svg_rounding_error =
      std::pow(10, -graphviz_num_decimals_in_svg) / 2;

  const check_options check_options = {
      .max_node_edge_overlap = graphviz_bezier_clip_margin,
      .svg_rounding_error = graphviz_max_svg_rounding_error,
  };

  const auto success = check_analyzed_svg(svg_analyzer, check_options);

  // FIXME: add writing of SVG files for manual inspection when not success
  REQUIRE(success);
}
