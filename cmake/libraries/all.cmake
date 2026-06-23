include(CheckLibraryExists)

find_library(MATH_LIBRARY m)

# Those are for systems with built-in libc
check_library_exists(m cos "" HAVE_LIB_M)

if(MATH_LIBRARY)
  link_libraries(${MATH_LIBRARY})
elseif(HAVE_LIB_M)
  link_libraries(m)
endif()
