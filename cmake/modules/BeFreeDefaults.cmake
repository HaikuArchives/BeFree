# Always include srcdir and builddir in include path.
# This saves from typing ${CMAKE_CURRENT_SOURCE_DIR} and ${CMAKE_CURRENT_BINARY}
# in about every subdir.
# Since CMake 2.4.0
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Put the include dirs which are in the source or build tree before all
# other include dirs, so the headers in the sources are prefered over
# the already installed ones.
# Since CMake 2.4.1
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# Use colored output, I really like colors!!
# Since CMake 2.4.0
set(CMAKE_COLOR_MAKEFILE ON)
