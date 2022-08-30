#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <catch2/catch.hpp>
#include <fmt/format.h>

#include "svg_analyzer.h"
#include <cgraph++/AGraph.h>
#include <gvc++/GVContext.h>
#include <gvc++/GVLayout.h>
#include <gvc++/GVRenderData.h>

#include "test_utilities.h"

TEST_CASE("SvgAnalyzer",
          "Test that the SvgAnalyzer can parse an SVG produced by Graphviz to "
          "an internal data structure and re-create the original SVG exactly "
          "from that data structure") {

  const auto shape = GENERATE(from_range(all_node_shapes));
  INFO(fmt::format("Shape: {}", shape));

  auto dot = fmt::format("digraph g1 {{node [shape={}]; a -> b}}", shape);

  auto g = CGraph::AGraph{dot};

  const auto demand_loading = false;
  auto gvc = GVC::GVContext{lt_preloaded_symbols, demand_loading};
  const auto graphviz_version = gvc.version();
  const auto graphviz_build_date = gvc.buildDate();

  const auto layout = GVC::GVLayout(std::move(gvc), std::move(g), "dot");

  const auto result = layout.render("svg");
  const std::string original_svg{result.string_view()};
  SVGAnalyzer svg_analyzer{result.c_str()};
  svg_analyzer.set_graphviz_version(graphviz_version);
  svg_analyzer.set_graphviz_build_date(graphviz_build_date);

  const std::size_t expected_num_graphs = 1;
  const std::size_t expected_num_nodes = 2;
  const std::size_t expected_num_edges = 1;

  CHECK(svg_analyzer.graphs().size() == expected_num_graphs);
  for (const auto &graph : svg_analyzer.graphs()) {
    CHECK(graph.svg_g_element().type == SVG::SVGElementType::Group);
    CHECK(graph.svg_g_element().attributes.class_ == "graph");
    CHECK(graph.svg_g_element().graphviz_id == "g1");

    CHECK(graph.nodes().size() == expected_num_nodes);
    for (const auto &node_it : graph.nodes() | boost::adaptors::indexed(0)) {
      const auto node = node_it.value();
      const auto i = node_it.index();
      CHECK(node.svg_g_element().type == SVG::SVGElementType::Group);
      CHECK(node.svg_g_element().attributes.class_ == "node");
      const auto node_id = i == 0 ? "a" : "b";
      CHECK(node.svg_g_element().graphviz_id == node_id);
    }

    CHECK(graph.edges().size() == expected_num_edges);
    for (const auto &edge : graph.edges()) {
      CHECK(edge.svg_g_element().type == SVG::SVGElementType::Group);
      CHECK(edge.svg_g_element().attributes.class_ == "edge");
      CHECK(edge.svg_g_element().graphviz_id == "a->b");
    }

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
    const std::size_t expected_num_texts =
        shape == "point" ? 0 : expected_num_nodes;

    CHECK(svg_analyzer.num_svgs() == expected_num_svgs);
    CHECK(svg_analyzer.num_groups() == expected_num_groups);
    CHECK(svg_analyzer.num_circles() == expected_num_circles);
    CHECK(svg_analyzer.num_ellipses() == expected_num_ellipses);
    CHECK(svg_analyzer.num_lines() == expected_num_lines);
    CHECK(svg_analyzer.num_paths() == expected_num_paths);
    CHECK(svg_analyzer.num_polygons() == expected_num_polygons);
    CHECK(svg_analyzer.num_polylines() == expected_num_polylines);
    CHECK(svg_analyzer.num_rects() == expected_num_rects);
    CHECK(svg_analyzer.num_titles() == expected_num_titles);
    CHECK(svg_analyzer.num_texts() == expected_num_texts);

    const auto indent_size = 0;
    auto recreated_svg = svg_analyzer.svg_string(indent_size);

    // compare the recreated SVG with the original SVG
    if (recreated_svg != original_svg) {
      std::vector<std::string> original_svg_lines;
      boost::split(original_svg_lines, original_svg, boost::is_any_of("\n"));
      std::vector<std::string> recreated_svg_lines;
      boost::split(recreated_svg_lines, recreated_svg, boost::is_any_of("\n"));
      for (std::size_t i = 0; i < original_svg_lines.size(); i++) {
        REQUIRE(i < recreated_svg_lines.size());
        REQUIRE(recreated_svg_lines[i] == original_svg_lines[i]);
      }
      REQUIRE(recreated_svg_lines.size() == original_svg_lines.size());
    }
  }
}
