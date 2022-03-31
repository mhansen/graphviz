find_package(PkgConfig)
if(PkgConfig_FOUND)
  pkg_check_modules(WEBP libwebp)
endif()

if(WEBP_FOUND)

  add_library(gvplugin_webp SHARED
    gvplugin_webp.c
    gvloadimage_webp.c
    gvdevice_webp.c
  )

  target_include_directories(gvplugin_webp PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pathplan
  )

  target_include_directories(gvplugin_webp SYSTEM PRIVATE
    ${WEBP_INCLUDE_DIRS}
  )

  target_link_libraries(gvplugin_webp
    ${WEBP_LINK_LIBRARIES}
  )

  install(
    TARGETS gvplugin_webp
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )

endif()
