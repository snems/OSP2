include(CheckLibraryExists)

find_library(MATH_LIBRARY m)

# May be used by compiler on high optimization levels.
# MSVC and MinGW lack supporting this though.
find_library(VECTORIZED_MATH_LIBRARY mvec)

# Those are for systems with built-in libc
check_library_exists(m cos "" HAVE_LIB_M)
check_library_exists(mvec _ZGVdN4v_cos "" HAVE_LIB_MVEC)

if(MATH_LIBRARY)
  link_libraries(${MATH_LIBRARY})
elseif(HAVE_LIB_M)
  link_libraries(m)
endif()

if(VECTORIZED_MATH_LIBRARY)
  link_libraries(${VECTORIZED_MATH_LIBRARY})
elseif(HAVE_LIB_MVEC)
  link_libraries(mvec)
endif()