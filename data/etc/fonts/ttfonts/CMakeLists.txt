# Install files.
file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.ttf")
set(install_dir ${TTFONTS_DIR})
install(FILES ${files} DESTINATION ${install_dir})
