#
# ensure_version(MIN_VERSION VERSION_FOUND VERSION_OK)
#
# Compares version numbers of the form "x.y.z", will set VERSION_OK to true
# if VERSION_FOUND >= MIN_VERSION.
#
# Example:
#   ensure_version("2.5.31" "flex 2.5.4a" VERSION_OK)
#
# Copyright (C) 2006, David Faure <faure@kde.org>
# Copyright (C) 2007, Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

macro(ensure_version requested_version found_version output_var)
    # Parse the parts of the version string
    string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" req_major_vers "${requested_version}")
    string(REGEX REPLACE "[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1" req_minor_vers "${requested_version}")
    string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" req_patch_vers "${requested_version}")
    string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" found_major_vers "${found_version}")
    string(REGEX REPLACE "[0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" found_minor_vers "${found_version}")
    string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" found_patch_vers "${found_version}")

    # Compute an overall version number which can be compared at once
    math(EXPR req_vers_num "${req_major_vers}*10000 + ${req_minor_vers}*100 + ${req_patch_vers}")
    math(EXPR found_vers_num "${found_major_vers}*10000 + ${found_minor_vers}*100 + ${found_patch_vers}")

    if (found_vers_num LESS req_vers_num)
        set( ${output_var} FALSE )
    else (found_vers_num LESS req_vers_num)
        set( ${output_var} TRUE )
    endif (found_vers_num LESS req_vers_num)
endmacro(ensure_version)
