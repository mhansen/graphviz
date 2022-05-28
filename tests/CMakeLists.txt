cmake_minimum_required (VERSION 3.12 FATAL_ERROR)

find_package(Boost REQUIRED)
find_package(Catch2 REQUIRED)
find_package(RAPIDXML REQUIRED)
find_package(SVGPP REQUIRED)

enable_testing()

# for convenience, make a library that depends on everything so each
# separate test can be as small as possible
add_library(test_common STATIC
            catch2_main.cpp
            svgpp_context.cpp
            svgpp_context.h
            svgpp_document_traverser.cpp
            svgpp_document_traverser.h
            svg_analyzer.cpp
            svg_analyzer.h
            svg_analyzer_interface.h
            ../cmd/dot/dot_builtins.cpp
)
set_target_properties(test_common PROPERTIES CXX_STANDARD 20)
set_target_properties(test_common PROPERTIES CXX_STANDARD_REQUIRED ON)
target_include_directories(test_common PRIVATE
  ../lib
)
target_include_directories(test_common SYSTEM PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${RAPIDXML_INCLUDE_DIRS}
  ${SVGPP_INCLUDE_DIRS}
)
target_link_libraries(test_common PUBLIC Catch2::Catch2)
target_link_libraries(test_common PUBLIC
  gvplugin_core
  gvplugin_dot_layout
  gvplugin_gd
  gvplugin_neato_layout
  cgraph
  cgraph++
  gvc
  gvc++
)

# common steps for creating a test case
macro(CREATE_TEST testname)
  add_executable(test_${testname} test_${testname}.cpp)
  set_target_properties(test_${testname} PROPERTIES CXX_STANDARD 20)
  set_target_properties(test_${testname} PROPERTIES CXX_STANDARD_REQUIRED ON)
  add_test(NAME test_${testname}
           COMMAND test_${testname} --reporter junit --out test_${testname}.xml)
  target_include_directories(test_${testname} PRIVATE
    ../lib
    ../lib/cdt
    ../lib/cgraph
    ../lib/cgraph++
    ../lib/common
    ../lib/gvc
    ../lib/pathplan
  )
  target_link_libraries(test_${testname} PRIVATE
    test_common
  )
endmacro()

CREATE_TEST(simple)
CREATE_TEST(AGraph_construction)
CREATE_TEST(GVContext_construction)
CREATE_TEST(GVLayout_construction)
CREATE_TEST(GVLayout_render)
CREATE_TEST(GVContext_render_svg)
