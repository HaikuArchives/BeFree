# Install CMake files
file(GLOB cmakeFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")
set(modules_install_dir ${DEVELOP_DIR}/cmake/modules)
install(FILES ${cmakeFiles} DESTINATION ${modules_install_dir})
