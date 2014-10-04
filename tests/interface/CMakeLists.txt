include_directories(${FREETYPE_INCLUDE_DIRS} ${DIRECTFB_INCLUDE_DIRS})
link_directories(${FREETYPE_LIBRARY_DIRS} ${DIRECTFB_LIBRARY_DIRS})

add_executable(button-test button-test.cpp)
target_link_libraries(button-test be)

add_executable(menu menu.cpp)
target_link_libraries(menu be)

add_executable(view-drawing view-drawing.cpp)
target_link_libraries(view-drawing be)

add_executable(window-look-test window-look-test.cpp)
target_link_libraries(window-look-test be)

add_executable(window-wmaction window-wmaction.cpp)
target_link_libraries(window-wmaction be)
