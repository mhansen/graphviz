#pragma once

#include <cmath>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace SVG {

struct SVGPoint {
  double x;
  double y;
};

struct SVGRect {
  double x;
  double y;
  double width;
  double height;
  void extend(const SVGPoint &point);
  void extend(const SVGRect &other);
};

struct SVGMatrix {
  double a;
  double b;
  double c;
  double d;
  double e;
  double f;
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
  double cx;
  double cy;
  std::string fill;
  std::string font_family;
  double font_size;
  double height;
  std::string id;
  std::vector<SVGPoint> points;
  double rx;
  double ry;
  std::string stroke;
  std::string text_anchor;
  std::optional<SVGMatrix> transform;
  SVGRect viewBox;
  double width;
  double x;
  double y;
};

/**
 * @brief The SVGElement class represents an SVG element
 */

class SVGElement {
public:
  SVGElement() = delete;
  explicit SVGElement(SVG::SVGElementType type);

  /// Return the bounding box of the element and its children. The bounding box
  /// is calculated and stored the first time this function is called and later
  /// calls will return the already calculated value. If this function is called
  /// for an SVG element for which the bounding box is not defined, it will
  /// throw an exception unless the `throw_if_bbox_not_defined` parameter is
  /// `false`.
  SVG::SVGRect bbox(bool throw_if_bbox_not_defined = true);
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
  /// The points given by the `d` attribute of a path element
  std::vector<SVGPoint> path_points;
  /// The SVG element text node contents. Not to be confused with an SVG `text`
  /// element
  std::string text;
  /// The type of SVG element
  const SVGElementType type;

private:
  /// append a string possibly containing an attribute to another string,
  /// handling space separation
  void append_attribute(std::string &output,
                        const std::string &attribute) const;
  std::string id_attribute_to_string() const;
  std::string fill_attribute_to_string() const;
  std::string points_attribute_to_string() const;
  std::string stroke_attribute_to_string() const;
  std::string stroke_to_graphviz_color(const std::string &color) const;
  SVG::SVGRect text_bbox() const;
  void to_string_impl(std::string &output, std::size_t indent_size,
                      std::size_t current_indent) const;

  /// The bounding box of the element and its children. Stored the first time
  /// it's computed
  std::optional<SVG::SVGRect> m_bbox;
};

} // namespace SVG
