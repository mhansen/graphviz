#pragma once

#include <string>

#include "svg_element.h"

/**
 * @brief The GraphvizNode class represents a Graphviz node according to the DOT
 * language
 */

class GraphvizNode {
public:
  GraphvizNode() = delete;
  explicit GraphvizNode(SVG::SVGElement &svg_element);

  /// Return a non-mutable reference to the SVG `g` element corresponding to the
  /// node
  const SVG::SVGElement &svg_g_element() const;

private:
  /// The SVG `g` element corresponding to the node
  SVG::SVGElement &m_svg_g_element;
};
