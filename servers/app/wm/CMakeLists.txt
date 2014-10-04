# Image files.
set(imgfiles
	${CMAKE_CURRENT_SOURCE_DIR}/data/foo_n.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/foo_ne.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/foo_e.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/foo_se.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/foo_s.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/foo_sw.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/foo_w.png
	${CMAKE_CURRENT_SOURCE_DIR}/data/foo_nw.png
)

# Build foo.h from the images.
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/data/foo.h"
	COMMAND directfb-csource ARGS --name=foo ${imgfiles} > ${CMAKE_CURRENT_SOURCE_DIR}/data/foo.h
	DEPENDS ${imgfiles}
)

include_directories(
	${DIRECTFB_INCLUDE_DIRS}
	${DIRECTFB_INTERNAL_INCLUDE_DIRS}
	${DIRECT_INCLUDB_DIRS}
	${FUSION_INCLUDB_DIRS}
)

# Build library.
set(sources
	classes/foo.c
	classes/frame.c
	classes/root.c
	classes/window.c

	devices/keyboard.c
	devices/pointer.c
	devices/wheel.c

	context.c
	decoration.c
	device.c
	input_channel.c
	input_switch.c
	prophecy.c
	prophecywm.c
	stret.c
	stret_iteration.c
	stret_test.c
	window.c
)
set_source_files_properties(${sources}
	PROPERTIES
	COMPILB_FLAGS "-D_GNU_SOURCE -D_REENTRANT"
	LINK_FLAGS "-avoid-version -module"
	OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/data/foo.h"
)
add_library(directfbwm_prophecy SHARED ${sources})
link_directories(
	${DIRECTFB_LIBRARY_DIRS}
	${DIRECTFB_INTERNAL_LIBRARY_DIRS}
	${DIRECTFB_LIBRARY_DIRS}
	${FUSION_LIBRARY_DIRS}
)
link_libraries(directfbwm_prophecy
	${DIRECTFB_LIBRARIES}
	${DIRECTFB_INTERNAL_LIBRARIES}
	${DIRECT_LIBRARIES}
	${FUSION_LIBRARIES}
)

# Install library.
install(
	TARGETS directfbwm_prophecy
	DESTINATION ${DIRECTFB_INTERNAL_MODULEDIR}/wm
)
