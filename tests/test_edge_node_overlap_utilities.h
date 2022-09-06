#include <string>

#include "svg_analyzer.h"

struct check_options {
  /// maximum allowed overlap between edge and node
  double max_node_edge_overlap;
  /// rounding error caused by limited precision in SVG attribute values
  double svg_rounding_error;
};

/// generate an SVG graph from the `dot` source and check that edges don't
/// overlap nodes
void test_edge_node_overlap(const std::string &dot);
