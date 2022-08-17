#include "graphviz_edge.h"

GraphvizEdge::GraphvizEdge(SVG::SVGElement &svg_g_element)
    : m_svg_g_element(svg_g_element) {}

const SVG::SVGElement &GraphvizEdge::svg_g_element() const {
  return m_svg_g_element;
}
