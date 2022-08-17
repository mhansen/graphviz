#pragma once

#include <string>
#include <string_view>

#include "svg_element.h"

/**
 * @brief The GraphvizEdge class represents a Graphviz edge according to the DOT
 * language
 */

class GraphvizEdge {
public:
  GraphvizEdge() = delete;
  explicit GraphvizEdge(SVG::SVGElement &svg_g_element);

  SVG::SVGRect bbox() const;
  SVG::SVGPoint center() const;
  std::string_view edgeop() const;
  /// Return a non-mutable reference to the SVG `g` element corresponding to the
  /// edge
  const SVG::SVGElement &svg_g_element() const;

private:
  /// The 'edgeop' according to the DOT language specification. Note that this
  /// is not the same as the 'id' attribute of an edge
  std::string m_edgeop;
  /// The SVG `g` element corresponding to the edge
  SVG::SVGElement &m_svg_g_element;
};
