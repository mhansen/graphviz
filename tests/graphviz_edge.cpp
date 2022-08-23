#include "graphviz_edge.h"

GraphvizEdge::GraphvizEdge(SVG::SVGElement &svg_g_element)
    : m_edgeop(svg_g_element.graphviz_id), m_svg_g_element(svg_g_element) {}

std::string_view GraphvizEdge::edgeop() const { return m_edgeop; }

const SVG::SVGElement &GraphvizEdge::svg_g_element() const {
  return m_svg_g_element;
}

SVG::SVGRect GraphvizEdge::bbox() const { return m_svg_g_element.bbox(); }

SVG::SVGPoint GraphvizEdge::center() const { return bbox().center(); }

double GraphvizEdge::penwidth() const {
  return m_svg_g_element.attribute_from_subtree<double>(
      &SVG::SVGAttributes::stroke_width, &SVG::SVGElement::is_shape_element, 1);
}
