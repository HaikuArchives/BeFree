#
# ensure_out_of_source_build(ERROR_MESSAGE)
#
# Bails out the error in ERROR_MESSAGE if cmake is running out of
# the build directory.
#
# Example:
#   ensure_out_of_source_build("Out of source build!")
#
# Copyright (C) 2006, Alexander Neundorf <neundorf@kde.org>
# Copyright (C) 2007, Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

macro(ensure_out_of_source_build _errorMessage)
	string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
	if (insource)
		message(FATAL_ERROR "${_errorMessage}")
	endif (insource)
endmacro(ensure_out_of_source_build)
