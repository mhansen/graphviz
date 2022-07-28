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

SVG::SVGElement &SVGAnalyzer::grandparent_element() {
  if (m_elements_in_process.empty()) {
    throw std::runtime_error{"No current element to get grandparent of"};
  }
  if (m_elements_in_process.size() == 1) {
    throw std::runtime_error{"No parent element"};
  }
  if (m_elements_in_process.size() == 2) {
    throw std::runtime_error{"No grandparent element"};
  }
  return *m_elements_in_process.end()[-3];
}

SVG::SVGElement &SVGAnalyzer::parent_element() {
  if (m_elements_in_process.empty()) {
    throw std::runtime_error{"No current element to get parent of"};
  }
  if (m_elements_in_process.size() == 1) {
    throw std::runtime_error{"No parent element"};
  }
  return *m_elements_in_process.end()[-2];
}

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

void SVGAnalyzer::set_class(std::string_view class_) {
  current_element().attributes.class_ = class_;
}

void SVGAnalyzer::set_cx(double cx) { current_element().attributes.cx = cx; }

void SVGAnalyzer::set_cy(double cy) { current_element().attributes.cy = cy; }

void SVGAnalyzer::set_fill(std::string_view fill) {
  current_element().attributes.fill = fill;
}

void SVGAnalyzer::set_height(double height) {
  current_element().attributes.height = height;
}

void SVGAnalyzer::set_stroke(std::string_view stroke) {
  current_element().attributes.stroke = stroke;
}

void SVGAnalyzer::set_id(std::string_view id) {
  current_element().attributes.id = id;
}

void SVGAnalyzer::set_rx(double rx) { current_element().attributes.rx = rx; }

void SVGAnalyzer::set_ry(double ry) { current_element().attributes.ry = ry; }

void SVGAnalyzer::set_point(std::pair<double, double> point) {
  current_element().attributes.points.emplace_back(point.first, point.second);
}

void SVGAnalyzer::set_text(std::string_view text) {
  auto &element = current_element();
  element.text = text;

  if (element.type == SVG::SVGElementType::Title) {
    // The title text is normally the 'graph_id', 'node_id' or 'edgeop'
    // according to the DOT language specification. Save it on the parent 'g'
    // element to avoid having to look it up later.
    if (parent_element().type != SVG::SVGElementType::Group) {
      throw std::runtime_error{"Unexpected parent element of 'title' element"};
    }
    parent_element().graphviz_id = text;
    // If the 'g' element corresponds to the graph, set the Graphviz ID also on
    // the top level 'svg' element.
    if (grandparent_element().type == SVG::SVGElementType::Svg) {
      grandparent_element().graphviz_id = text;
    }
  }
}

void SVGAnalyzer::set_text_anchor(std::string_view text_anchor) {
  current_element().attributes.text_anchor = text_anchor;
}

void SVGAnalyzer::set_width(double width) {
  current_element().attributes.width = width;
}

void SVGAnalyzer::set_x(double x) { current_element().attributes.x = x; }

void SVGAnalyzer::set_y(double y) { current_element().attributes.y = y; }

void SVGAnalyzer::set_transform(double a, double b, double c, double d,
                                double e, double f) {
  current_element().attributes.transform = {a, b, c, d, e, f};
}

void SVGAnalyzer::set_viewBox(double x, double y, double width, double height) {
  current_element().attributes.viewBox = {x, y, width, height};
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
