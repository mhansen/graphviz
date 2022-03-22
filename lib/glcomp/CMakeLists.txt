if(with_smyrna)

  find_package(Freetype REQUIRED)
  find_package(GLUT REQUIRED)
  find_package(GTK2 REQUIRED COMPONENTS gtk glade)
  find_package(PANGOCAIRO REQUIRED)
  find_package(PkgConfig REQUIRED)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(XRENDER REQUIRED xrender)
  endif()

  add_library(glcomp
    glcompbutton.h
    glcompdefs.h
    glcompfont.h
    glcompimage.h
    glcomplabel.h
    glcompmouse.h
    glcomppanel.h
    glcompset.h
    glcomptextpng.h
    glcomptexture.h
    glpangofont.h
    glutils.h

    glcompbutton.c
    glcompfont.c
    glcompimage.c
    glcomplabel.c
    glcompmouse.c
    glcomppanel.c
    glcompset.c
    glcomptextpng.c
    glcomptexture.c
    glpangofont.c
    glutils.c
  )

  target_include_directories(glcomp PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${GRAPHVIZ_LIB_DIR}/common
  )

  target_include_directories(glcomp SYSTEM PRIVATE
    ${Freetype_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${GTK2_INCLUDE_DIRS}
    ${PANGOCAIRO_INCLUDE_DIRS}
    ${XRENDER_INCLUDE_DIRS}
  )

  target_link_libraries(glcomp PRIVATE
    ${Freetype_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${GTK2_LIBRARIES}
    ${MATH_LIB}
    ${PANGOCAIRO_LIBRARIES}
    ${XRENDER_LIBRARIES}
  )

endif()
