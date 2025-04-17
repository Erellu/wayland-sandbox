find_package(PkgConfig)
pkg_check_modules(PC_PANGOCAIRO QUIET pangocairo)

find_path(PANGOCAIRO_INCLUDE_DIR
          NAMES pangocairo.h
          HINTS ${PC_PANGOCAIRO_INCLUDE_DIRS}
          PATH_SUFFIXES pango-1.0)

find_library(PANGOCAIRO_LIBRARY
             NAMES pangocairo-1.0
             HINTS ${PC_PANGOCAIRO_LIBRARY_DIRS}
)

# message(FATAL_ERROR ${PANGOCAIRO_INCLUDE_DIR})

set(PANGOCAIRO_LIBRARIES ${PANGOCAIRO_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PangoCairo DEFAULT_MSG  PANGOCAIRO_LIBRARIES PANGOCAIRO_INCLUDE_DIR)

mark_as_advanced(PANGOCAIRO_INCLUDE_DIR PANGOCAIRO_LIBRARIES PANGOCAIRO_LIBRARY)
