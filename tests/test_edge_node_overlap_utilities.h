#include <string>

#include "svg_analyzer.h"

struct check_options {
  /// maximum allowed overlap between edge and node
  double max_node_edge_overlap;
  /// rounding error caused by limited precision in SVG attribute values
  double svg_rounding_error;
};

struct graph_options {
  std::string_view rankdir = "TB";
  std::string_view node_shape = "polygon";
  double node_penwidth = 1;
  double edge_penwidth = 1;
};

struct write_options {
  std::string filename_base = "test_edge_node_overlap";
  bool write_svg_on_success = false;
  bool write_original_svg = false;
  bool write_recreated_svg = false;
  bool write_annotated_svg = true;
};

/// generate an SVG graph from the `dot` source and check that edges don't
/// overlap nodes
void test_edge_node_overlap(const graph_options &graph_options = {},
                            const write_options &write_options = {});
