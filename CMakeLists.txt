# For platform checks see http://www.cmake.org/Wiki/CMake_HowToDoPlatformChecks
project(befree)

# Where to look first for CMake modules, before ${CMAKE_ROOT}/Modules/ is checked.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Include macros.
include(BeFreeDefaults)
include(EnsureOutOfSourceBuild)
include(ConfigureChecks.cmake)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckTypeSize)

# Disable in-source builds.
ensure_out_of_source_build("BeFree requires an out of source build. Please create a separate build directory and run 'cmake path_to_befree [options]' there.")

# Set version number, An even minor number corresponds to releases.
set(BEFREE_MAJOR_VERSION 0)
set(BEFREE_MINOR_VERSION 1)
set(BEFREE_BUILD_VERSION 0)
set(BEFREE_VERSION
    "${BEFREE_MAJOR_VERSION}.${BEFREE_MINOR_VERSION}.${BEFREE_BUILD_VERSION}")

# Debug option.
option(BEFREE_DEBUG "Build the project using debugging code" ON)

# Fix if CMAKE_SYSTEM_PROCESSOR is unknown
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "unknown")
    exec_program("uname -m" OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR)
endif (CMAKE_SYSTEM_PROCESSOR STREQUAL "unknown")

# Test platform.
if (NOT UNIX)
    message(FATAL_ERROR "BeFree only compiles and run on *NIX platforms.")
endif (NOT UNIX)
if (NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(FATAL_ERROR "BeFree only supports Linux right now.")
endif (NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "i386")
    message(FATAL_ERROR "BeFree requires i486+ processors.")
endif (CMAKE_SYSTEM_PROCESSOR STREQUAL "i386")

# Set common flags.
if (BEFREE_DEBUG)
    set(BEFREE_FLAGS "${BEFREE_FLAGS} -ggdb -DDEBUG=1")
    set(CMAKE_BUILD_TYPE Debug)
endif (BEFREE_DEBUG)
if (NOT BEFREE_DEBUG)
    set(CMAKE_BUILD_TYPE Release)
endif (NOT BEFREE_DEBUG)

# Common compiler flags.
set(BEFREE_FLAGS "${BEFREE_FLAGS} -Wall -Wpointer-arith -Wcast-align -Wsign-compare -Wstrict-aliasing -Wno-multichar")

# Default include directories.
include_directories(${CMAKE_BINARY_DIR}/kits ${CMAKE_SOURCE_DIR}/kits)

# Check for libattr.
check_include_files(sys/xattr.h HAVE_XATTR_H)
check_library_exists(attr lgetxattr "" HAVE_ATTR)
if (NOT HAVE_XATTR_H AND HAVE_ATTR)
	message(FATAL_ERROR "BeFree needs libattr, please install it.")
endif (NOT HAVE_XATTR_H AND HAVE_ATTR)

# Check type size.
check_type_size("float" SIZEOF_FLOAT)
if (NOT HAVE_SIZEOF_FLOAT)
    message(FATAL_ERROR "Type float unknown!")
endif (NOT HAVE_SIZEOF_FLOAT)
check_type_size("double" SIZEOF_DOUBLE)
if (NOT HAVE_SIZEOF_DOUBLE)
    message(FATAL_ERROR "Type double unknown!")
endif (NOT HAVE_SIZEOF_DOUBLE)

# Install directories.
set(DEVELOP_DIR "${CMAKE_INSTALL_PREFIX}/develop")
set(HEADERS_DIR "${DEVELOP_DIR}/headers")
set(CONFIG_DIR "${CMAKE_INSTALL_PREFIX}/common/config")
set(BIN_DIR "${CONFIG_DIR}/bin")
set(ETC_DIR "${CONFIG_DIR}/etc")
set(LIB_DIR "${CONFIG_DIR}/lib")
set(DATA_DIR "${CONFIG_DIR}/share")
set(ROOT_ETC_DIR "${CMAKE_INSTALL_PREFIX}/etc")
set(FONTS_DIR "${ROOT_ETC_DIR}/fonts")
set(PSFONTS_DIR "${FONTS_DIR}/psfonts")
set(TTFONTS_DIR "${FONTS_DIR}/ttfonts")
set(SYSTEM_LIB_DIR "${CMAKE_INSTALL_PREFIX}/beos/system/lib")
set(SYSTEM_SERVERS_DIR "${CMAKE_INSTALL_PREFIX}/beos/system/servers")

# Set flags.
#set(BEFREE_FLAGS "${BEFREE_FLAGS} -DSIZEOF_FLOAT=${SIZEOF_FLOAT} -DSIZEOF_DOUBLE=${SIZEOF_DOUBLE}")
set(CMAKE_C_FLAGS "${BEFREE_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${BEFREE_FLAGS} -Woverloaded-virtual -Wno-ctor-dtor-privacy -Wnon-virtual-dtor -fexceptions -fpermissive")

# Log some information.
message(STATUS "")
message(STATUS "============= Build environment =============")
message(STATUS "CMAKE_SYSTEM:                    ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SYSTEM_NAME:               ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_VERSION:            ${CMAKE_SYSTEM_VERSION}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR:          ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_SIZEOF_VOID_P:             ${CMAKE_SIZEOF_VOID_P}")
message(STATUS "CMAKE_C_FLAGS:                   ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS:                 ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_BUILD_TYPE:                ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_COMPILER:                ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER:              ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_AR:                        ${CMAKE_AR}")
message(STATUS "CMAKE_RANLIB:                    ${CMAKE_RANLIB}")
message(STATUS "=============================================")
message(STATUS "FREETYPE_VERSION:                ${FREETYPE_VERSION}")
message(STATUS "FREETYPE_INCLUDE_DIRS:           ${FREETYPE_INCLUDE_DIRS}")
message(STATUS "FREETYPE_CFLAGS_OTHER:           ${FREETYPE_CFLAGS_OTHER}")
message(STATUS "FREETYPE_LIBRARY_DIRS:           ${FREETYPE_LIBRARY_DIRS}")
message(STATUS "FREETYPE_LIBRARIES:              ${FREETYPE_LIBRARIES}")
message(STATUS "FREETYPE_LDFLAGS_OTHER:          ${FREETYPE_LDFLAGS_OTHER}")
message(STATUS "=============================================")
message(STATUS "DIRECTFB_VERSION:                ${DIRECTFB_VERSION}")
message(STATUS "DIRECTFB_INCLUDE_DIRS:           ${DIRECTFB_INCLUDE_DIRS}")
message(STATUS "DIRECTFB_CFLAGS_OTHER:           ${DIRECTFB_CFLAGS_OTHER}")
message(STATUS "DIRECTFB_LIBRARY_DIRS:           ${DIRECTFB_LIBRARY_DIRS}")
message(STATUS "DIRECTFB_LIBRARIES:              ${DIRECTFB_LIBRARIES}")
message(STATUS "DIRECTFB_LDFLAGS_OTHER:          ${DIRECTFB_LDFLAGS_OTHER}")
message(STATUS "=============================================")
message(STATUS "DIRECTFB_INTERNAL_VERSION:       ${DIRECTFB_INTERNAL_VERSION}")
message(STATUS "DIRECTFB_INTERNAL_INCLUDE_DIRS:  ${DIRECTFB_INTERNAL_INCLUDE_DIRS}")
message(STATUS "DIRECTFB_INTERNAL_CFLAGS_OTHER:  ${DIRECTFB_INTERNAL_CFLAGS_OTHER}")
message(STATUS "DIRECTFB_INTERNAL_LIBRARY_DIRS:  ${DIRECTFB_INTERNAL_LIBRARY_DIRS}")
message(STATUS "DIRECTFB_INTERNAL_LIBRARIES:     ${DIRECTFB_INTERNAL_LIBRARIES}")
message(STATUS "DIRECTFB_INTERNAL_LDFLAGS_OTHER: ${DIRECTFB_INTERNAL_LDFLAGS_OTHER}")
message(STATUS "=============================================")
message(STATUS "FUSION_VERSION:                  ${FUSION_VERSION}")
message(STATUS "FUSION_INCLUDB_DIRS:             ${FUSION_INCLUDE_DIRS}")
message(STATUS "FUSION_CFLAGS_OTHER:             ${FUSION_CFLAGS_OTHER}")
message(STATUS "FUSION_LIBRARY_DIRS:             ${FUSION_LIBRARY_DIRS}")
message(STATUS "FUSION_LIBRARIES:                ${FUSION_LIBRARIES}")
message(STATUS "FUSION_LDFLAGS_OTHER:            ${FUSION_LDFLAGS_OTHER}")
message(STATUS "============= Install folders =============")
message(STATUS "CMAKE_INSTALL_PREFIX:            ${CMAKE_INSTALL_PREFIX}")
message(STATUS "BIN_DIR:                         ${BIN_DIR}")
message(STATUS "ETC_DIR:                         ${ETC_DIR}")
message(STATUS "LIB_DIR:                         ${LIB_DIR}")
message(STATUS "DATA_DIR:                        ${DATA_DIR}")
message(STATUS "ROOT_ETC_DIR:                    ${ROOT_ETC_DIR}")
message(STATUS "FONTS_DIR:                       ${FONTS_DIR}")
message(STATUS "PSFONTS_DIR:                     ${PSFONTS_DIR}")
message(STATUS "TTFONTS_DIR:                     ${TTFONTS_DIR}")
message(STATUS "DEVELOP_DIR:                     ${DEVELOP_DIR}")
message(STATUS "SYSTEM_LIB_DIR:                  ${SYSTEM_LIB_DIR}")
message(STATUS "SYSTEM_SERVERS_DIR:              ${SYSTEM_SERVERS_DIR}")
message(STATUS "")

# Configure file(s).
configure_file("kits/befree-fsh.h.cmake" "${CMAKE_BINARY_DIR}/kits/befree-fsh.h")

# Sub directories.
add_subdirectory(cmake)
add_subdirectory(data)
add_subdirectory(kits)
add_subdirectory(servers)
add_subdirectory(tests)
