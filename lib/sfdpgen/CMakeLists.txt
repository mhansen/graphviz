if(with_sfdp)

add_definitions(-DNEATOGEN_EXPORTS)

add_library(sfdpgen STATIC
  # Header files
  Multilevel.h
  post_process.h
  PriorityQueue.h
  sfdp.h
  sfdpinternal.h
  sparse_solve.h
  spring_electrical.h
  stress_model.h
  uniform_stress.h

  # Source files
  Multilevel.c
  post_process.c
  PriorityQueue.c
  sfdpinit.c
  sparse_solve.c
  spring_electrical.c
  stress_model.c
  uniform_stress.c
)

target_include_directories(sfdpgen PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pack
  ../pathplan
)

target_link_libraries(sfdpgen PRIVATE
  gvc
  neatogen
  sparse
)

endif()
