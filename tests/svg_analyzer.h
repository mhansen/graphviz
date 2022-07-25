#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "svg_analyzer_interface.h"
#include "svg_element.h"

/**
 * @brief The SVGAnalyzer class analyzes the contents of an SVG document.
 */

class SVGAnalyzer : public ISVGAnalyzer {
public:
  SVGAnalyzer(char *text);
  void on_enter_element_svg() override;
  void on_enter_element_g() override;
  void on_enter_element_circle() override;
  void on_enter_element_ellipse() override;
  void on_enter_element_line() override;
  void on_enter_element_path() override;
  void on_enter_element_polygon() override;
  void on_enter_element_polyline() override;
  void on_enter_element_rect() override;
  void on_enter_element_text() override;
  void on_enter_element_title() override;
  void on_exit_element() override;
  void set_text(std::string_view text) override;
  std::size_t num_svgs() const { return m_num_svgs; };
  std::size_t num_groups() const { return m_num_groups; };
  std::size_t num_circles() const { return m_num_circles; };
  std::size_t num_ellipses() const { return m_num_ellipses; };
  std::size_t num_lines() const { return m_num_lines; };
  std::size_t num_paths() const { return m_num_paths; };
  std::size_t num_polygons() const { return m_num_polygons; };
  std::size_t num_polylines() const { return m_num_polylines; };
  std::size_t num_rects() const { return m_num_rects; };
  std::size_t num_texts() const { return m_num_texts; };
  std::size_t num_titles() const { return m_num_titles; };
  std::string svg_string(std::size_t indent_size = 2) const;

private:
  /// Get the current element being processed by the SVG document traverser
  SVG::SVGElement &current_element();
  /// Enter a new SVG element retrieved by the SVG document traverser into the
  /// list of elements currently being processed
  void enter_element(SVG::SVGElementType type);

  /// A list of pointers to elements currently being processed by the SVG++
  /// document traverser, in hierarchical order. The front element is the top
  /// level SVG and the back element is the current element.
  std::vector<SVG::SVGElement *> m_elements_in_process;
  std::size_t m_num_svgs = 1; // the top level svg is implicit. See
                              // https://github.com/svgpp/svgpp/issues/98
  std::size_t m_num_groups = 0;
  std::size_t m_num_circles = 0;
  std::size_t m_num_ellipses = 0;
  std::size_t m_num_lines = 0;
  std::size_t m_num_paths = 0;
  std::size_t m_num_polygons = 0;
  std::size_t m_num_polylines = 0;
  std::size_t m_num_rects = 0;
  std::size_t m_num_texts = 0;
  std::size_t m_num_titles = 0;
  /// The top level SVG `svg` element corresponding to the Graphviz graph
  SVG::SVGElement m_svg;
};
