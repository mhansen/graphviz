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

  /// Add an SVG `rect` element to the edge's corresponding `g` element. The
  /// `rect` is represes the bounding box of the edge.
  void add_bbox();
  /// Add an SVG `rect` element to the edge's corresponding `g` element. The
  /// `rect` represents the outline bounding box of the edge. The outline
  /// bounding box is the bounding box with penwidth taken into account.
  void add_outline_bbox();
  /// Return the bounding box of the edge
  SVG::SVGRect bbox() const;
  /// Return the center of the edge's bounding box
  SVG::SVGPoint center() const;
  /// Return the edge's `color` attribute in RGB hex format if the opacity is
  /// 100 % or in RGBA hex format otherwise.
  std::string color() const;
  /// Return the 'edgeop' according to the DOT language specification. Note that
  /// this is not the same as the 'id' attribute of an edge.
  std::string_view edgeop() const;
  /// Return the edge's `fillcolor` attribute in RGB hex format if the opacity
  /// is 100 % or in RGBA hex format otherwise.
  std::string fillcolor() const;
  /// Return the outline bounding box of the edge. The outline bounding box is
  /// the bounding box with penwidth taken into account.
  SVG::SVGRect outline_bbox() const;
  /// Return the edge's `penwidth` attribute
  double penwidth() const;
  /// Return a non-mutable reference to the SVG `g` element corresponding to the
  /// edge
  const SVG::SVGElement &svg_g_element() const;

private:
  /// The 'edgeop' according to the DOT language specification
  std::string m_edgeop;
  /// The SVG `g` element corresponding to the edge
  SVG::SVGElement &m_svg_g_element;
};
