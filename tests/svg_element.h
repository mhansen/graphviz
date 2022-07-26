#pragma once

#include <cmath>
#include <string>
#include <string_view>
#include <vector>

namespace SVG {

struct SVGRect {
  double x;
  double y;
  double width;
  double height;
};

enum class SVGElementType {
  Circle,
  Ellipse,
  Group,
  Line,
  Path,
  Polygon,
  Polyline,
  Rect,
  Svg,
  Text,
  Title,
};

std::string_view tag(SVG::SVGElementType type);

struct SVGAttributes {
  std::string class_;
  double height;
  SVGRect viewBox;
  double width;
};

/**
 * @brief The SVGElement class represents an SVG element
 */

class SVGElement {
public:
  SVGElement() = delete;
  explicit SVGElement(SVG::SVGElementType type);

  std::string to_string(std::size_t indent_size) const;

  SVGAttributes attributes;
  /// The Graphviz build date
  std::string graphviz_build_date;
  std::vector<SVGElement> children;
  /// The `graph_id`, `node_id` or `edgeop` according to the DOT language
  /// specification. Note that this is not the same as the `id` attribute of the
  /// SVG element
  std::string graphviz_id;
  /// The Graphviz release version
  std::string graphviz_version;
  /// The SVG element text node contents. Not to be confused with an SVG `text`
  /// element
  std::string text;
  /// The type of SVG element
  const SVGElementType type;

private:
  void to_string_impl(std::string &output, std::size_t indent_size,
                      std::size_t current_indent) const;
};

} // namespace SVG
