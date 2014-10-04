# Install header files.
file(GLOB headerFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
set(headers_install_dir ${DEVELOP_DIR}/headers/os/add-ons/graphics)
install(FILES ${headerFiles} DESTINATION ${headers_install_dir})
