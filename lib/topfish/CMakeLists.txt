if(with_smyrna)

  add_library(topfish STATIC
    # Header files
    hierarchy.h

    # Source files
    hierarchy.c
    rescale_layout.c
  )

  target_include_directories(topfish PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${GRAPHVIZ_LIB_DIR}/common
  )

  target_link_libraries(topfish PRIVATE
    gvc
    neatogen
  )

endif()
