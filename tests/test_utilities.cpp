#include <string_view>
#include <unordered_set>

#include "test_utilities.h"

const std::unordered_set<std::string_view> node_shapes_consisting_of_ellipse = {
    "ellipse",      //
    "oval",         //
    "circle",       //
    "doublecircle", //
    "point",        //
};

const std::unordered_set<std::string_view>
    node_shapes_consisting_of_ellipse_and_polyline = {
        "Mcircle", //
};

const std::unordered_set<std::string_view> node_shapes_consisting_of_path = {
    "cylinder", //
};

const std::unordered_set<std::string_view> node_shapes_consisting_of_polygon = {
    "box",           //
    "polygon",       //
    "egg",           //
    "triangle",      //
    "diamond",       //
    "trapezium",     //
    "parallelogram", //
    "house",         //
    "pentagon",      //
    "hexagon",       //
    "septagon",      //
    "octagon",       //
    "doubleoctagon", //
    "tripleoctagon", //
    "invtriangle",   //
    "invtrapezium",  //
    "invhouse",      //
    "rect",          //
    "rectangle",     //
    "square",        //
    "star",          //
    "cds",           //
    "rpromoter",     //
    "rarrow",        //
    "larrow",        //
    "lpromoter",     //
    "folder",        //
};

const std::unordered_set<std::string_view>
    node_shapes_consisting_of_polygon_and_polyline = {
        "Mdiamond",        //
        "Msquare",         //
        "underline",       //
        "note",            //
        "tab",             //
        "box3d",           //
        "component",       //
        "promoter",        //
        "terminator",      //
        "utr",             //
        "primersite",      //
        "restrictionsite", //
        "fivepoverhang",   //
        "threepoverhang",  //
        "noverhang",       //
        "assembly",        //
        "signature",       //
        "insulator",       //
        "ribosite",        //
        "rnastab",         //
        "proteasesite",    //
        "proteinstab",     //
};

const std::unordered_set<std::string_view> node_shapes_without_svg_shape = {
    "plaintext", //
    "plain",     //
    "none",      //
};

const std::unordered_set<std::string_view> all_node_shapes = {
    "box",             //
    "polygon",         //
    "ellipse",         //
    "oval",            //
    "circle",          //
    "point",           //
    "egg",             //
    "triangle",        //
    "plaintext",       //
    "plain",           //
    "diamond",         //
    "trapezium",       //
    "parallelogram",   //
    "house",           //
    "pentagon",        //
    "hexagon",         //
    "septagon",        //
    "octagon",         //
    "doublecircle",    //
    "doubleoctagon",   //
    "tripleoctagon",   //
    "invtriangle",     //
    "invtrapezium",    //
    "invhouse",        //
    "Mdiamond",        //
    "Msquare",         //
    "Mcircle",         //
    "rect",            //
    "rectangle",       //
    "square",          //
    "star",            //
    "none",            //
    "underline",       //
    "cylinder",        //
    "note",            //
    "tab",             //
    "folder",          //
    "box3d",           //
    "component",       //
    "promoter",        //
    "cds",             //
    "terminator",      //
    "utr",             //
    "primersite",      //
    "restrictionsite", //
    "fivepoverhang",   //
    "threepoverhang",  //
    "noverhang",       //
    "assembly",        //
    "signature",       //
    "insulator",       //
    "ribosite",        //
    "rnastab",         //
    "proteasesite",    //
    "proteinstab",     //
    "rpromoter",       //
    "rarrow",          //
    "larrow",          //
    "lpromoter"        //
};

bool contains_polygon_shape(std::string_view shape) {
  return node_shapes_consisting_of_polygon.contains(shape) ||
         node_shapes_consisting_of_polygon_and_polyline.contains(shape);
}

bool contains_ellipse_shape(std::string_view shape) {
  return node_shapes_consisting_of_ellipse.contains(shape) ||
         node_shapes_consisting_of_ellipse_and_polyline.contains(shape);
}
