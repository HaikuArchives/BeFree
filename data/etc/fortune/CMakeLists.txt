# Install files.
set(files
	Art
	Computers
	Education
	Food
	Fortunes
	Goedel
	Haiku
	Humorists
	Kids
	Law
	Linux\ cookies
	Love
	Magic
	Medicine
	Miscellaneous
	News
	One\ Liners
	OS\ Fortunes
	Pets
	Platitudes
	Riddles
	Songs\ &\ Poems
	Sports
	Tech\ Support\ Excuses
)
set(install_dir ${ETC_DIR}/fortune)
install(FILES ${files} DESTINATION ${install_dir})
