#include <stdexcept>

#include "svg_analyzer.h"
#include "svgpp_context.h"
#include "svgpp_document_traverser.h"

SVGAnalyzer::SVGAnalyzer(char *text)
    : m_svg(SVG::SVGElement(SVG::SVGElementType::Svg)) {
  m_elements_in_process.push_back(&m_svg);
  SvgppContext context{this};
  traverseDocumentWithSvgpp(context, text);
  if (m_elements_in_process.size() != 1) {
    throw std::runtime_error{
        "Wrong number of elements in process after traversing SVG document"};
  }
}

void SVGAnalyzer::on_enter_element_svg() { m_num_svgs++; }

void SVGAnalyzer::on_enter_element_g() {
  enter_element(SVG::SVGElementType::Group);
  m_num_groups++;
}

void SVGAnalyzer::on_enter_element_circle() {
  enter_element(SVG::SVGElementType::Circle);
  m_num_circles++;
}

void SVGAnalyzer::on_enter_element_ellipse() {
  enter_element(SVG::SVGElementType::Ellipse);
  m_num_ellipses++;
}

void SVGAnalyzer::on_enter_element_line() {
  enter_element(SVG::SVGElementType::Line);
  m_num_lines++;
}

void SVGAnalyzer::on_enter_element_path() {
  enter_element(SVG::SVGElementType::Path);
  m_num_paths++;
}

void SVGAnalyzer::on_enter_element_polygon() {
  enter_element(SVG::SVGElementType::Polygon);
  m_num_polygons++;
}

void SVGAnalyzer::on_enter_element_polyline() {
  enter_element(SVG::SVGElementType::Polyline);
  m_num_polylines++;
}

void SVGAnalyzer::on_enter_element_rect() {
  enter_element(SVG::SVGElementType::Rect);
  m_num_rects++;
}

void SVGAnalyzer::on_enter_element_text() {
  enter_element(SVG::SVGElementType::Text);
  m_num_texts++;
}

void SVGAnalyzer::on_enter_element_title() {
  enter_element(SVG::SVGElementType::Title);
  m_num_titles++;
}

void SVGAnalyzer::on_exit_element() { m_elements_in_process.pop_back(); }

SVG::SVGElement &SVGAnalyzer::current_element() {
  if (m_elements_in_process.empty()) {
    throw std::runtime_error{"No current element"};
  }
  return *m_elements_in_process.back();
}

void SVGAnalyzer::enter_element(SVG::SVGElementType type) {
  if (m_elements_in_process.empty()) {
    throw std::runtime_error{
        "No element is currently being processed by the SVG++ document "
        "traverser when entering a new element. Expecting at least the top "
        "level 'svg' to be in process"};
  }
  auto &element = current_element();
  element.children.emplace_back(type);
  m_elements_in_process.push_back(&element.children.back());
}

void SVGAnalyzer::set_text(std::string_view text) {
  auto &element = current_element();
  element.text = text;
}

void SVGAnalyzer::set_graphviz_version(std::string_view version) {
  m_svg.graphviz_version = version;
}

void SVGAnalyzer::set_graphviz_build_date(std::string_view build_date) {
  m_svg.graphviz_build_date = build_date;
}

std::string SVGAnalyzer::svg_string(std::size_t indent_size) const {
  std::string output{};
  output += m_svg.to_string(indent_size);
  return output;
}
