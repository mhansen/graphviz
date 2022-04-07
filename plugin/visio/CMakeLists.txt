add_library(gvplugin_visio SHARED
  gvplugin_visio.c
  gvrender_visio_vdx.cpp
  VisioGraphic.cpp
  VisioGraphic.h
  VisioRender.cpp
  VisioRender.h
  VisioText.cpp
  VisioText.h
)

target_include_directories(gvplugin_visio PRIVATE
  ../../lib
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pathplan
)

target_link_libraries(gvplugin_visio
  cgraph
  gvc
)

install(
  TARGETS gvplugin_visio
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

set_target_properties(gvplugin_visio PROPERTIES
  VERSION ${GRAPHVIZ_PLUGIN_VERSION}.0.0
  SOVERSION ${GRAPHVIZ_PLUGIN_VERSION}
)

if(MINGW)
  # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
  set_target_properties(gvplugin_visio PROPERTIES
    RUNTIME_OUTPUT_NAME gvplugin_visio-${GRAPHVIZ_PLUGIN_VERSION}
  )
endif()
