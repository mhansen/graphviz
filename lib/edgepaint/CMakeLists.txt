add_definitions(-DLAB_GAMUT_EXPORTS)

add_library(edgepaintlib STATIC
  edge_distinct_coloring.h
  furtherest_point.h
  intersection.h
  lab.h
  lab_gamut.h
  node_distinct_coloring.h

  edge_distinct_coloring.c
  furtherest_point.c
  intersection.c
  lab.c
  lab_gamut.c
  node_distinct_coloring.c
)

target_include_directories(edgepaintlib PRIVATE
  ${GRAPHVIZ_LIB_DIR}
  ${GRAPHVIZ_LIB_DIR}/cdt
  ${GRAPHVIZ_LIB_DIR}/cgraph
  ${GRAPHVIZ_LIB_DIR}/common
  ${GRAPHVIZ_LIB_DIR}/gvc
  ${GRAPHVIZ_LIB_DIR}/pathplan
)

# work around needed for an object library
# https://gitlab.com/graphviz/graphviz/-/issues/1613
if(APPLE)
  target_compile_options(edgepaintlib PRIVATE "-fno-common")
endif()
