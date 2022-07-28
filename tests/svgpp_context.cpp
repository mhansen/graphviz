#include <any>
#include <stdexcept>

#include <boost/array.hpp>
#include <fmt/format.h>

#include "svg_analyzer_interface.h"
#include "svgpp_context.h"

static std::string to_color_string(SvgppContext::color_t color) {
  return fmt::format("rgb({},{},{})", (color >> 16) & 0xff, (color >> 8) & 0xff,
                     (color >> 0) & 0xff);
}

SvgppContext::SvgppContext(ISVGAnalyzer *svgAnalyzer)
    : m_svgAnalyzer(svgAnalyzer){};

void SvgppContext::on_enter_element(svgpp::tag::element::svg) {
  m_svgAnalyzer->on_enter_element_svg();
}

void SvgppContext::on_enter_element(svgpp::tag::element::g) {
  m_svgAnalyzer->on_enter_element_g();
}

void SvgppContext::on_enter_element(svgpp::tag::element::circle) {
  m_svgAnalyzer->on_enter_element_circle();
}

void SvgppContext::on_enter_element(svgpp::tag::element::ellipse) {
  m_svgAnalyzer->on_enter_element_ellipse();
}

void SvgppContext::on_enter_element(svgpp::tag::element::line) {
  m_svgAnalyzer->on_enter_element_line();
}

void SvgppContext::on_enter_element(svgpp::tag::element::path) {
  m_svgAnalyzer->on_enter_element_path();
}

void SvgppContext::on_enter_element(svgpp::tag::element::polygon) {
  m_svgAnalyzer->on_enter_element_polygon();
}

void SvgppContext::on_enter_element(svgpp::tag::element::polyline) {
  m_svgAnalyzer->on_enter_element_polyline();
}

void SvgppContext::on_enter_element(svgpp::tag::element::rect) {
  m_svgAnalyzer->on_enter_element_rect();
}

void SvgppContext::on_enter_element(svgpp::tag::element::text) {
  m_svgAnalyzer->on_enter_element_text();
}

void SvgppContext::on_enter_element(svgpp::tag::element::title) {
  m_svgAnalyzer->on_enter_element_title();
}

void SvgppContext::on_exit_element() { m_svgAnalyzer->on_exit_element(); }

void SvgppContext::path_move_to(double x, double y,
                                svgpp::tag::coordinate::absolute c) {
  (void)x;
  (void)y;
  (void)c;
}

void SvgppContext::path_line_to(double x, double y,
                                svgpp::tag::coordinate::absolute c) {
  (void)x;
  (void)y;
  (void)c;
}

void SvgppContext::path_cubic_bezier_to(double x1, double y1, double x2,
                                        double y2, double x, double y,
                                        svgpp::tag::coordinate::absolute c) {
  (void)x1;
  (void)y1;
  (void)x2;
  (void)y2;
  (void)x;
  (void)y;
  (void)c;
}

void SvgppContext::path_quadratic_bezier_to(
    double x1, double y1, double x, double y,
    svgpp::tag::coordinate::absolute c) {
  (void)x1;
  (void)y1;
  (void)x;
  (void)y;
  (void)c;
}

void SvgppContext::path_elliptical_arc_to(double rx, double ry,
                                          double x_axis_rotation,
                                          bool large_arc_flag, bool sweep_flag,
                                          double x, double y,
                                          svgpp::tag::coordinate::absolute c) {
  (void)rx;
  (void)ry;
  (void)x_axis_rotation;
  (void)large_arc_flag;
  (void)sweep_flag;
  (void)x;
  (void)y;
  (void)c;
}

void SvgppContext::path_close_subpath() {}

void SvgppContext::path_exit() {}

void SvgppContext::set(svgpp::tag::attribute::cy, const double v) {
  m_svgAnalyzer->set_cy(v);
}

void SvgppContext::set(svgpp::tag::attribute::cx, const double v) {
  m_svgAnalyzer->set_cx(v);
}

void SvgppContext::set(svgpp::tag::attribute::fill, svgpp::tag::value::none) {
  m_svgAnalyzer->set_fill("none");
}

void SvgppContext::set(svgpp::tag::attribute::fill,
                       svgpp::tag::value::currentColor) {
  throw std::runtime_error{
      "the 'fill' attribute 'currentColor' value is not yet implemented"};
}

void SvgppContext::set(svgpp::tag::attribute::fill, color_t color,
                       svgpp::tag::skip_icc_color) {
  m_svgAnalyzer->set_fill(to_color_string(color));
}

void SvgppContext::set(svgpp::tag::attribute::stroke, svgpp::tag::value::none) {
  m_svgAnalyzer->set_stroke("none");
}

void SvgppContext::set(svgpp::tag::attribute::stroke,
                       svgpp::tag::value::currentColor) {
  throw std::runtime_error{
      "the 'stroke' attribute 'currentColor' value is not yet implemented"};
}

void SvgppContext::set(svgpp::tag::attribute::stroke,
                       SvgppContext::color_t color,
                       svgpp::tag::skip_icc_color) {
  m_svgAnalyzer->set_stroke(to_color_string(color));
}

void SvgppContext::transform_matrix(const boost::array<double, 6> &matrix) {
  double a = matrix.at(0);
  double b = matrix.at(1);
  double c = matrix.at(2);
  double d = matrix.at(3);
  double e = matrix.at(4);
  double f = matrix.at(5);
  m_svgAnalyzer->set_transform(a, b, c, d, e, f);
}

void SvgppContext::set(svgpp::tag::attribute::r a, const double v) {
  (void)a;
  (void)v;
}

void SvgppContext::set(svgpp::tag::attribute::rx, const double v) {
  m_svgAnalyzer->set_rx(v);
}

void SvgppContext::set(svgpp::tag::attribute::ry, const double v) {
  m_svgAnalyzer->set_ry(v);
}

void SvgppContext::set(svgpp::tag::attribute::x1 a, const double v) {
  (void)a;
  (void)v;
}

void SvgppContext::set(svgpp::tag::attribute::y1 a, const double v) {
  (void)a;
  (void)v;
}

void SvgppContext::set(svgpp::tag::attribute::x2 a, const double v) {
  (void)a;
  (void)v;
}

void SvgppContext::set(svgpp::tag::attribute::y2 a, const double v) {
  (void)a;
  (void)v;
}

void SvgppContext::set(svgpp::tag::attribute::x a, const double v) {
  (void)a;
  (void)v;
}

void SvgppContext::set(svgpp::tag::attribute::y a, const double v) {
  (void)a;
  (void)v;
}

void SvgppContext::set(svgpp::tag::attribute::width, const double v) {
  m_svgAnalyzer->set_width(v);
}

void SvgppContext::set(svgpp::tag::attribute::height, const double v) {
  m_svgAnalyzer->set_height(v);
}

void SvgppContext::set(svgpp::tag::attribute::id,
                       boost::iterator_range<const char *> v) {
  m_svgAnalyzer->set_id({v.begin(), v.end()});
}

void SvgppContext::set(svgpp::tag::attribute::class_,
                       boost::iterator_range<const char *> v) {
  m_svgAnalyzer->set_class({v.begin(), v.end()});
}

void SvgppContext::set(svgpp::tag::attribute::text_anchor,
                       svgpp::tag::value::start) {
  m_svgAnalyzer->set_text_anchor("start");
}

void SvgppContext::set(svgpp::tag::attribute::text_anchor,
                       svgpp::tag::value::middle) {
  m_svgAnalyzer->set_text_anchor("middle");
}

void SvgppContext::set(svgpp::tag::attribute::text_anchor,
                       svgpp::tag::value::end) {
  m_svgAnalyzer->set_text_anchor("end");
}

void SvgppContext::set(svgpp::tag::attribute::viewBox, const double v1,
                       const double v2, const double v3, const double v4) {
  m_svgAnalyzer->set_viewBox(v1, v2, v3, v4);
}

void SvgppContext::set(svgpp::tag::attribute::points,
                       const SvgppContext::PointsRange &range) {
  for (auto &it : range) {
    m_svgAnalyzer->set_point(it);
  }
}

void SvgppContext::set_text(boost::iterator_range<const char *> v) {
  m_svgAnalyzer->set_text({v.begin(), v.end()});
}
