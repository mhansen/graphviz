#pragma once

#include <string_view>
#include <unordered_set>

/// node shapes

extern const std::unordered_set<std::string_view> all_node_shapes;
extern const std::unordered_set<std::string_view>
    node_shapes_consisting_of_ellipse;
extern const std::unordered_set<std::string_view>
    node_shapes_consisting_of_ellipse_and_polyline;
extern const std::unordered_set<std::string_view>
    node_shapes_consisting_of_path;
extern const std::unordered_set<std::string_view>
    node_shapes_consisting_of_polygon;
extern const std::unordered_set<std::string_view>
    node_shapes_consisting_of_polygon_and_polyline;
extern const std::unordered_set<std::string_view> node_shapes_without_svg_shape;

bool contains_ellipse_shape(std::string_view shape);
bool contains_polygon_shape(std::string_view shape);

/// arrow shapes
extern const std::unordered_set<std::string_view>
    primitive_polygon_arrow_shapes;
extern const std::unordered_set<std::string_view>
    primitive_polygon_and_polyline_arrow_shapes;
extern const std::unordered_set<std::string_view> all_primitive_arrow_shapes;

/// rank directions
extern const std::unordered_set<std::string_view> all_rank_directions;
