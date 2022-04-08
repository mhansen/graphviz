find_package(PANGOCAIRO)
find_package(PkgConfig)
if(PkgConfig_FOUND)
  pkg_check_modules(GLITZ glitz)
endif()

if(GLITZ_FOUND AND PANGOCAIRO_FOUND)

  add_library(gvplugin_glitz SHARED
    gvdevice_glitz.c
    gvplugin_glitz.c
  )

  target_include_directories(gvplugin_glitz PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_glitz SYSTEM PRIVATE
    ${GLITZ_INCLUDE_DIRS}
    ${PANGOCAIRO_INCLUDE_DIRS}
  )

  target_link_libraries(gvplugin_glitz
    ${GLITZ_LIBRARIES}
    ${PANGOCAIRO_LIBRARIES}
  )

  install(
    TARGETS gvplugin_glitz
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )

  set_target_properties(gvplugin_glitz PROPERTIES
    VERSION ${GRAPHVIZ_PLUGIN_VERSION}.0.0
    SOVERSION ${GRAPHVIZ_PLUGIN_VERSION}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_glitz PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_glitz-${GRAPHVIZ_PLUGIN_VERSION}
    )
  endif()

endif()
