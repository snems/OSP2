# GNU style (GCC/Clang) compiler specific settings

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_C_COMPILER_ID MATCHES "^(Apple)?Clang$")
    return()
endif()

add_compile_options(-Wall -Wimplicit -Wshadow -Werror
    -Wstrict-prototypes -Wformat=2  -Wformat-security
    -Wstrict-aliasing=2 -Wmissing-format-attribute
    -Wdisabled-optimization -Werror-implicit-function-declaration)

add_compile_options(-Wno-format-zero-length -Wno-format-nonliteral)

# lmvec is for vectorized maths
add_compile_options(-lmvec -lm)

# We must have all symbols defined. Without this, successful compilation
# may produce unusable binary, that cannot load because of some missing
# function
add_compile_options(-Wl,--no-undefined)

# There are lots of instances of union based aliasing in the code
# that rely on the compiler not optimising them away, so disable it
add_compile_options(-fno-strict-aliasing)

# This is necessary to hide all symbols unless explicitly exported
# via the Q_EXPORT macro
add_compile_options(-fvisibility=hidden)
