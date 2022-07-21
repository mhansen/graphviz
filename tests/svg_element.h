#pragma once

#include <vector>

namespace SVG {

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

/**
 * @brief The SVGElement class represents an SVG element
 */

class SVGElement {
public:
  SVGElement() = delete;
  explicit SVGElement(SVG::SVGElementType type);

  std::vector<SVGElement> children;
  /// The type of SVG element
  const SVGElementType type;
};

} // namespace SVG
