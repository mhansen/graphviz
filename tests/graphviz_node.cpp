#include "graphviz_node.h"

GraphvizNode::GraphvizNode(SVG::SVGElement &svg_element)
    : m_svg_g_element(svg_element) {}

const SVG::SVGElement &GraphvizNode::svg_g_element() const {
  return m_svg_g_element;
}
