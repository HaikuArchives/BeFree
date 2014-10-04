# Install files.
file(GLOB afm_files "${CMAKE_CURRENT_SOURCE_DIR}/*.afm")
file(GLOB pfb_files "${CMAKE_CURRENT_SOURCE_DIR}/*.pfb")
set(install_dir ${PSFONTS_DIR})
install(FILES ${afm_files} ${pfb_files} DESTINATION ${install_dir})
