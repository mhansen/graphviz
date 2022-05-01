if(with_sfdp)

  add_executable(gvmap
    country_graph_coloring.c
    country_graph_coloring.h
    gvmap.c
    make_map.c
    make_map.h
    power.c
    power.h
  )

  target_include_directories(gvmap PRIVATE
    ../../lib
    ../../lib/common
    ../../lib/cgraph
    ../../lib/cdt
  )

  target_include_directories(gvmap SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )

  target_link_libraries(gvmap
    cgraph
    edgepaintlib
    gvc
    ingraphs
    neatogen
    rbtree
    sfdpgen
    sparse
    ${MATH_LIB}
  )

  if(NOT HAVE_GETOPT_H)
    target_link_libraries(gvmap ${GETOPT_LINK_LIBRARIES})
  endif()

  find_program(GZIP gzip)
  if(GZIP)
    add_custom_target(man-gvmap ALL DEPENDS gvmap.1.gz
                      COMMENT "gvmap man page")
    add_custom_command(
      OUTPUT gvmap.1.gz
      COMMAND ${GZIP} -9 --no-name --to-stdout gvmap.1
        >"${CMAKE_CURRENT_BINARY_DIR}/gvmap.1.gz"
      MAIN_DEPENDENCY gvmap.1
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "compress gvmap man page")
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/gvmap.1.gz
      DESTINATION ${MAN_INSTALL_DIR}/man1)
  else()
    install(
      FILES gvmap.1
      DESTINATION ${MAN_INSTALL_DIR}/man1
    )
  endif()

  install(
    TARGETS gvmap
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )

  install(
    FILES gvmap.sh
    DESTINATION ${BINARY_INSTALL_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE
  )

endif()
