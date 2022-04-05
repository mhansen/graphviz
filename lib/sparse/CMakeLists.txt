add_library(sparse STATIC
  # Header files
  BinaryHeap.h
  clustering.h
  color_palette.h
  colorutil.h
  DotIO.h
  general.h
  IntStack.h
  LinkedList.h
  mq.h
  QuadTree.h
  SparseMatrix.h

  # Source files
  BinaryHeap.c
  clustering.c
  color_palette.c
  colorutil.c
  DotIO.c
  general.c
  IntStack.c
  LinkedList.c
  mq.c
  QuadTree.c
  SparseMatrix.c
)

target_include_directories(sparse PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
)

target_link_libraries(sparse
  ${MATH_LIB}
)
