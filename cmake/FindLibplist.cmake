find_package(PkgConfig)
pkg_check_modules(PC_LIBPLIST QUIET plist)

find_path(LIBPLIST_INCLUDE_DIR
        NAMES plist/plist.h
        HINTS ${PC_LIBPLIST_INCLUDEDIR} ${PC_LIBPLIST_INCLUDE_DIRS})
find_library(LIBPLIST_LIBRARY
        NAMES plist libplist
        HINTS ${PC_LIBPLIST_LIBDIR} ${PC_LIBPLIST_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBPLIST DEFAULT_MSG
        LIBPLIST_LIBRARY LIBPLIST_INCLUDE_DIR)

mark_as_advanced(LIBPLIST_INCLUDE_DIR LIBPLIST_LIBRARY)

set(LIBPLIST_LIBRARIES ${LIBPLIST_LIBRARY})
set(LIBPLIST_INCLUDE_DIRS ${LIBPLIST_INCLUDE_DIR})