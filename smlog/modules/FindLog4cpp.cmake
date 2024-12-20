
find_library(Log4cpp_LIBRARY "log4cpp")
get_filename_component(Log4cpp_LIBRARY_DIR ${Log4cpp_LIBRARY} DIRECTORY)
string(FIND ${Log4cpp_LIBRARY} "/lib/" Log4cpp_LIBRARY_POS REVERSE)
if (NOT ${Log4cpp_LIBRARY_POS} STREQUAL -1)
    string(SUBSTRING ${Log4cpp_LIBRARY} 0 ${Log4cpp_LIBRARY_POS} Log4cpp_ROOT_FOLDER)
    find_path(Log4cpp_INCLUDE_DIR "include/log4cpp" PATHS ${Log4cpp_ROOT_FOLDER} NO_DEFAULT_PATH)
    set(Log4cpp_INCLUDE_DIR "${Log4cpp_INCLUDE_DIR}/include")
endif ()
mark_as_advanced(Log4cpp_LIBRARY Log4cpp_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Log4cpp" DEFAULT_MSG Log4cpp_LIBRARY Log4cpp_INCLUDE_DIR)

if (Log4cpp_FOUND)
    set(Log4cpp_TARGET "Log4cpp::log4cpp")
    add_library(${Log4cpp_TARGET} UNKNOWN IMPORTED)
    set_target_properties(${Log4cpp_TARGET} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${Log4cpp_INCLUDE_DIR}")
    set_target_properties(${Log4cpp_TARGET} PROPERTIES
            IMPORTED_LOCATION "${Log4cpp_LIBRARY}")
    if (WIN32)
        target_link_libraries(${Log4cpp_TARGET} INTERFACE kernel32 user32 ws2_32 advapi32)
    endif ()
endif ()
