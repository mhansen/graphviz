#include <boost/algorithm/string.hpp>
#include <catch2/catch.hpp>
#include <fmt/format.h>

#include "svg_analyzer.h"
#include <cgraph++/AGraph.h>
#include <gvc++/GVContext.h>
#include <gvc++/GVLayout.h>
#include <gvc++/GVRenderData.h>

#include "test_utilities.h"

TEST_CASE(
    "SvgAnalyzer",
    "The SvgAnalyzer parses an SVG produced by Graphviz to an internal data "
    "structure, supports retrieval of information about that graph and "
    "recreation of the original SVG from that data structure") {

  const auto shape_char_ptr = GENERATE(from_range(all_node_shapes));
  const std::string shape{shape_char_ptr};
  INFO(fmt::format("Shape: {}", shape));

  auto dot = fmt::format("digraph g1 {{node [shape={}]; a -> b}}", shape);

  auto g = CGraph::AGraph{dot};

  const auto demand_loading = false;
  auto gvc = GVC::GVContext(lt_preloaded_symbols, demand_loading);

  const auto layout = GVC::GVLayout(std::move(gvc), std::move(g), "dot");

  const auto result = layout.render("svg");
  const std::string original_svg{result.string_view()};
  SVGAnalyzer svgAnalyzer{result.c_str()};

  const std::size_t expected_num_graphs = 1;
  const std::size_t expected_num_nodes = 2;
  const std::size_t expected_num_edges = 1;

  {
    const std::size_t expected_num_svgs = expected_num_graphs;
    const std::size_t expected_num_groups =
        expected_num_graphs + expected_num_nodes + expected_num_edges;
    const std::size_t expected_num_circles = 0;
    const std::size_t expected_num_ellipses = [&]() {
      if (shape == "doublecircle") {
        return expected_num_nodes * 2;
      } else if (contains_ellipse_shape(shape)) {
        return expected_num_nodes;
      } else {
        return 0UL;
      }
    }();
    const std::size_t expected_num_lines = 0;
    const std::size_t expected_num_paths =
        expected_num_edges + (shape == "cylinder" ? expected_num_nodes * 2 : 0);
    const std::size_t expected_num_polygons =
        expected_num_graphs + expected_num_edges + [&]() {
          if (shape == "noverhang") {
            return expected_num_nodes * 4;
          } else if (shape == "tripleoctagon") {
            return expected_num_nodes * 3;
          } else if (shape == "doubleoctagon" || shape == "fivepoverhang" ||
                     shape == "threepoverhang" || shape == "assembly") {
            return expected_num_nodes * 2;
          } else if (contains_polygon_shape(shape)) {
            return expected_num_nodes;
          } else {
            return 0UL;
          }
        }();
    const std::size_t expected_num_polylines = [&]() {
      if (shape == "Mdiamond" || shape == "Msquare") {
        return expected_num_nodes * 4;
      } else if (shape == "box3d" || shape == "signature" ||
                 shape == "insulator" || shape == "ribosite" ||
                 shape == "rnastab") {
        return expected_num_nodes * 3;
      } else if (shape == "Mcircle" || shape == "note" ||
                 shape == "component" || shape == "restrictionsite" ||
                 shape == "noverhang" || shape == "assembly" ||
                 shape == "proteasesite" || shape == "proteinstab") {
        return expected_num_nodes * 2;
      } else if (shape == "underline" || shape == "tab" ||
                 shape == "promoter" || shape == "terminator" ||
                 shape == "utr" || shape == "primersite" ||
                 shape == "fivepoverhang" || shape == "threepoverhang") {
        return expected_num_nodes;
      } else {
        return 0UL;
      }
    }();
    const std::size_t expected_num_rects = 0;
    const std::size_t expected_num_titles =
        expected_num_graphs + expected_num_nodes + expected_num_edges;

    CHECK(svgAnalyzer.num_svgs() == expected_num_svgs);
    CHECK(svgAnalyzer.num_groups() == expected_num_groups);
    CHECK(svgAnalyzer.num_circles() == expected_num_circles);
    CHECK(svgAnalyzer.num_ellipses() == expected_num_ellipses);
    CHECK(svgAnalyzer.num_lines() == expected_num_lines);
    CHECK(svgAnalyzer.num_paths() == expected_num_paths);
    CHECK(svgAnalyzer.num_polygons() == expected_num_polygons);
    CHECK(svgAnalyzer.num_polylines() == expected_num_polylines);
    CHECK(svgAnalyzer.num_rects() == expected_num_rects);
    CHECK(svgAnalyzer.num_titles() == expected_num_titles);

    const auto indent_size = 0;
    auto recreated_svg = svgAnalyzer.svg_string(indent_size);

    // compare the initial lines of the recreated SVG that we can fully recreate
    // with the original SVG
    std::vector<std::string> original_svg_lines;
    boost::split(original_svg_lines, original_svg, boost::is_any_of("\n"));
    std::vector<std::string> recreated_svg_lines;
    boost::split(recreated_svg_lines, recreated_svg, boost::is_any_of("\n"));
    for (std::size_t i = 0; i < original_svg_lines.size(); i++) {
      REQUIRE(i < recreated_svg_lines.size());
      if (recreated_svg_lines[i] == "<svg>") {
        // stop comparison here since we do not yet handle the Graphviz version
        // and build date comment and the graph title comment that comes before
        // the 'svg' element
        break;
      }
      REQUIRE(recreated_svg_lines[i] == original_svg_lines[i]);
    }

    // do some sanity checks of the parts of the recreated SVG that we cannot
    // yet compare with the original SVG
    CHECK(recreated_svg.find("<svg>") != std::string::npos);
    CHECK(recreated_svg.find("</svg>") != std::string::npos);
    CHECK(recreated_svg.find("<g>") != std::string::npos);
    CHECK(recreated_svg.find("</g>") != std::string::npos);
    CHECK(recreated_svg.find("<title/>") != std::string::npos);
    CHECK(recreated_svg.find("<polygon/>") != std::string::npos);
    CHECK(recreated_svg.find("<path/>") != std::string::npos);
  }
}
