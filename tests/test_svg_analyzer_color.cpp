#include <string_view>

#include <catch2/catch.hpp>
#include <fmt/format.h>

#include "svg_analyzer.h"
#include "test_utilities.h"

TEST_CASE("SvgAnalyzer color",
          "Test that the SvgAnalyzer can recreate the original "
          "SVG with the correct `stroke` and `stroke-opacity` attributes when "
          "the Graphviz `color` attribute is used for nodes and edges ") {

  const auto shape = GENERATE(from_range(all_node_shapes));
  INFO(fmt::format("Shape: {}", shape));

  const std::string_view color =
      GENERATE("", "\"#10204000\"", "\"#10204080\"", "\"#102040ff\"");
  INFO(fmt::format("Color: {}", color));
  const auto color_attr = color.empty() ? "" : fmt::format(" color={}", color);

  // FIXME: Edge arrowheads use `color` also for fill when `fillcolor` is not
  // set. This can result in `fill-opacity' being set which we do not yet
  // support. We therefore temporarily avoid specifying an edge color when
  // opacity is not 0 or 100 %.
  const auto edge_color_attr =
      !color.ends_with("00\"") && !color.ends_with("ff\"")
          ? ""
          : fmt::format(" color={}", color);

  auto dot = fmt::format("digraph g1 {{node [shape={}{}]; edge [{}]; a -> b}}",
                         shape, color_attr, edge_color_attr);

  if (shape == "point" && !color.ends_with("00\"") &&
      !color.ends_with("ff\"")) {
    // FIXME: The `point` shape implicitly uses style="filled" and this in turn
    // causes `color` to be used for fill when `fillcolor` is not set.
    // This can result in `fill-opacity' being set which we do not yet
    // support. We therefore avoid checking the SVG for the `point` shape when
    // the opacity is not 0 or 100 %.
  } else {
    SVGAnalyzer::make_from_dot(dot).re_create_and_verify_svg();
  }
}
