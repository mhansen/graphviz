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
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
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
