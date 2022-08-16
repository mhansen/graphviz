#pragma once

#include <string>
#include <string_view>

#include "svg_element.h"

/**
 * @brief The GraphvizNode class represents a Graphviz node according to the DOT
 * language
 */

class GraphvizNode {
public:
  GraphvizNode() = delete;
  explicit GraphvizNode(SVG::SVGElement &svg_element);

  /// Return the node's bounding box
  SVG::SVGRect bbox() const;
  /// Return the node's `node_id` as defined by the DOT language
  std::string_view node_id() const;
  /// Return a non-mutable reference to the SVG `g` element corresponding to the
  /// node
  const SVG::SVGElement &svg_g_element() const;

private:
  /// The `node_id` according to the DOT language specification. Note that this
  /// is not the same as the `id` attribute of a node
  std::string m_node_id;
  /// The SVG `g` element corresponding to the node
  SVG::SVGElement &m_svg_g_element;
};
