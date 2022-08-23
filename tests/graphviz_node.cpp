#include "graphviz_node.h"

GraphvizNode::GraphvizNode(SVG::SVGElement &svg_element)
    : m_node_id(svg_element.graphviz_id), m_svg_g_element(svg_element) {}

SVG::SVGPoint GraphvizNode::center() const { return bbox().center(); }

std::string_view GraphvizNode::node_id() const { return m_node_id; }

SVG::SVGRect GraphvizNode::bbox() const { return m_svg_g_element.bbox(); }

const SVG::SVGElement &GraphvizNode::svg_g_element() const {
  return m_svg_g_element;
}
