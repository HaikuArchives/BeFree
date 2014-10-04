# - Try to find the freetype library
# Once done this will define
#
#  FREETYPE_FOUND - system has Fontconfig
#  FREETYPE_INCLUDE_DIR - the FONTCONFIG include directory
#  FREETYPE_LIBRARIES - Link these to use FREETYPE
#
# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIR)

  # in cache already
  set(FREETYPE_FOUND TRUE)

else (FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIR)

  FIND_PROGRAM(FREETYPECONFIG_EXECUTABLE NAMES freetype-config PATHS
     /usr/bin
     /usr/local/bin
     /opt/local/bin
  )

  #reset vars
  set(FREETYPE_LIBRARIES)
  set(FREETYPE_INCLUDE_DIR)

  # if freetype-config has been found
  if(FREETYPECONFIG_EXECUTABLE)

    EXEC_PROGRAM(${FREETYPECONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE FREETYPE_LIBRARIES)

    EXEC_PROGRAM(${FREETYPECONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE FREETYPE_INCLUDE_DIR)
    if(FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIR)
      set(FREETYPE_FOUND TRUE)
      #message(STATUS "Found freetype: ${FREETYPE_LIBRARIES}")
    endif(FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIR)

    MARK_AS_ADVANCED(FREETYPE_LIBRARIES FREETYPE_INCLUDE_DIR)

    set( FREETYPE_LIBRARIES ${FREETYPE_LIBRARIES} CACHE INTERNAL "The libraries for freetype" )

  endif(FREETYPECONFIG_EXECUTABLE)


  IF (FREETYPE_FOUND)
    IF (NOT FREETYPE_FIND_QUIETLY)
       MESSAGE(STATUS "Found Freetype: ${FREETYPE_LIBRARIES}")
    ENDIF (NOT FREETYPE_FIND_QUIETLY)
  ELSE (FREETYPE_FOUND)
    IF (FREETYPE_FIND_REQUIRED)
       MESSAGE(FATAL_ERROR "Could not find FreeType library")
    ENDIF (FREETYPE_FIND_REQUIRED)
  ENDIF (FREETYPE_FOUND)

endif (FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIR)