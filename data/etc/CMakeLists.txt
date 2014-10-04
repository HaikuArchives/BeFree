add_subdirectory(fonts)
add_subdirectory(fortune)

# Install files.
set(files
	directfbrc.app_server
	inputrc
	profile
	teapot.data
	termcap
)
set(install_dir ${ETC_DIR})
install(FILES ${files} DESTINATION ${install_dir})
