# Versions requirements.
set(DIRECTFB_REQUIRED_VERSION "1.1.0")

# Check for pkg-config.
find_package(PkgConfig)
if (NOT PKG_CONFIG_FOUND)
	message(FATAL_ERROR "BeFree needs pkg-config!")
endif (NOT PKG_CONFIG_FOUND)

# Check for FreeType2.
#include(FindFreetype)
pkg_check_modules(FREETYPE "freetype2 >= 9.10.3")
if (NOT FREETYPE_FOUND)
	message(FATAL_ERROR "BeFree needs FreeType!")
endif (NOT FREETYPE_FOUND)

# Check for DirectFB.
pkg_check_modules(DIRECTFB "directfb >= ${DIRECTFB_REQUIRED_VERSION}")
if (NOT DIRECTFB_FOUND)
	message(FATAL_ERROR "BeFree needs directfb >= ${DIRECTFB_REQUIRED_VERSION}!")
endif (NOT DIRECTFB_FOUND)

# Check for DirectFB's libdirect.
pkg_check_modules(DIRECT "direct >= ${DIRECTFB_REQUIRED_VERSION}")
if (NOT DIRECT_FOUND)
	message(FATAL_ERROR "Your DirectFB installation is broken, direct >= ${DIRECTFB_REQUIRED_VERSION} is missing!")
endif (NOT DIRECT_FOUND)

# Check for DirectFB's internal interface and get modules directory.
pkg_check_modules(DIRECTFB_INTERNAL "directfb-internal >= ${DIRECTFB_REQUIRED_VERSION}")
if (NOT DIRECTFB_INTERNAL_FOUND)
	message(FATAL_ERROR "Your DirectFB installation is broken, directfb-internal >= ${DIRECTFB_REQUIRED_VERSION} is missing!")
endif (NOT DIRECTFB_INTERNAL_FOUND)

# Check for DirectFB's Fusion.
pkg_check_modules(FUSION "fusion >= 1.1.0")
if (NOT FUSION_FOUND)
	message(FATAL_ERROR "BeFree needs fusion >= 1.1.0!")
endif (NOT FUSION_FOUND)
