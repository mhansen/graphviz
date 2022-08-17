#pragma once

#include "svg_element.h"

/**
 * @brief The GraphvizEdge class represents a Graphviz edge according to the DOT
 * language
 */

class GraphvizEdge {
public:
  GraphvizEdge() = delete;
  explicit GraphvizEdge(SVG::SVGElement &svg_g_element);

  /// Return a non-mutable reference to the SVG `g` element corresponding to the
  /// edge
  const SVG::SVGElement &svg_g_element() const;

private:
  /// The SVG `g` element corresponding to the edge
  SVG::SVGElement &m_svg_g_element;
};
