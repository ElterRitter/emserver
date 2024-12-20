cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

if(WIN32 AND ((${CMAKE_BUILD_TYPE} MATCHES Release) OR (${CMAKE_BUILD_TYPE} MATCHES RelDebInfo)) AND NUGET_PATH)
    set(PACKAGING_TARGET_NAME "packaging_${PROJECT_NAME}")
    add_custom_target(${PACKAGING_TARGET_NAME} ALL DEPENDS ${PROJECT_NAME})

    string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_UPPER_TYPE)
    set(loc_pef "IMPORTED_LOCATION_${BUILD_UPPER_TYPE}")
    get_target_property(DEP_FILE Boost::system ${loc_pef})
    string(REPLACE ".lib" ".dll" CSERVER_BOOST_DEP_DLL ${DEP_FILE})

    cmake_path(GET CSERVER_BOOST_DEP_DLL FILENAME BOOST_DEP_FILE_NAME)

    set(NUGET_ARTIFACT_FILENAME "${PROJECT_NAME}.${CSERVER_VERSION}.nupkg")
    set(NUGET_TARGETS_FILENAME "cserver.targets")
    add_custom_command(TARGET ${PACKAGING_TARGET_NAME} COMMAND ${CMAKE_COMMAND} ARGS "-DCSERVER_VERSION=${CSERVER_VERSION}"
                                                                                     "-DCONFIG_FILE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
                                                                                     "-DCSERVER_BOOST_DEP_DLL=${CSERVER_BOOST_DEP_DLL}"
                                                                                    -P "${PROJECT_SOURCE_DIR}/package/fillVersion.cmake"
                                                                                BYPRODUCTS ${BOOST_DEP_FILE_NAME} "cserver.nuspec"
                                                                                )
    add_custom_command(TARGET ${PACKAGING_TARGET_NAME} COMMAND ${CMAKE_COMMAND} -E ARGS copy_if_different ${CSERVER_BOOST_DEP_DLL} "${CMAKE_CURRENT_SOURCE_DIR}/${NUGET_TARGETS_FILENAME}" ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET ${PACKAGING_TARGET_NAME} COMMAND "${NUGET_PATH}/nuget.exe" ARGS pack
                                                       BYPRODUCTS ${NUGET_ARTIFACT_FILENAME} ${NUGET_TARGETS_FILENAME})
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${NUGET_ARTIFACT_FILENAME}" DESTINATION "./packages")
elseif(NOT NUGET_PATH AND WIN32)
    message(WARNING "Nuget path is empty. Can't create nuget package")
elseif(WIN32 AND NUGET_PATH)
    message(WARNING "Build configuration ${CMAKE_BUILD_TYPE} not supported for packaging")
elseif(NOT WIN32)
    message(STATUS "Make packaging file structure to project ${PROJECT_NAME} with version ${CnMp_PROJECT_VERSION_FULL}")
    find_program(LSB_RELEASE_EXECUTABLE lsb_release)
    if(LSB_RELEASE_EXECUTABLE)
        execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -r OUTPUT_VARIABLE OS_RELEASE_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(STRIP ${OS_RELEASE_VERSION} OS_RELEASE_VERSION)
        string(FIND ${OS_RELEASE_VERSION} "1.7" OUT_VAR)
        string(TOLOWER ${PROJECT_NAME} LOWER_PROJECT_NAME)
        set(PACKAGING_TARGET_NAME "packaging_${PROJECT_NAME}")
        set(PACK_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/deb_package/usr/lib/$<TARGET_FILE_NAME:${PROJECT_NAME}>)
        if(${OUT_VAR} LESS 0)
            message(STATUS "Building package for astra 1.6 with release ${OS_RELEASE_VERSION}")
            set(PACK_FILE_NAME "${LOWER_PROJECT_NAME}_${CnMp_PROJECT_VERSION_FULL}_astra16")
        else()
            message(STATUS "Building package for astra 1.7 with release ${OS_RELEASE_VERSION}")
            set(PACK_FILE_NAME "${LOWER_PROJECT_NAME}_${CnMp_PROJECT_VERSION_FULL}_astra17")
        endif()
        add_custom_target(${PACKAGING_TARGET_NAME} ALL DEPENDS ${PROJECT_NAME})
        add_custom_command(TARGET ${PACKAGING_TARGET_NAME} COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                                                                  "$<TARGET_FILE:${PROJECT_NAME}>"
                                                                                  "${PACK_BINARY_DIR}"
                                                                   BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/deb_package/usr/lib/*.*")
        add_custom_command(TARGET ${PACKAGING_TARGET_NAME} COMMAND ${CMAKE_COMMAND} ARGS "-DCSERVER_VERSION=${CSERVER_VERSION}"
                                                                                         "-DCONFIG_FILE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
                                                                                         "-DPACKAGE_NAME=${LOWER_PROJECT_NAME}"
                                                                                         "-DPACKAGE_VERSION=${CnMp_PROJECT_VERSION}"
                                                                                         -P "${PROJECT_SOURCE_DIR}/package/fillVersion.cmake"
                                                                                        BYPRODUCTS "deb_package/DEBIAN/control")
        add_custom_command(TARGET ${PACKAGING_TARGET_NAME} COMMAND dpkg-deb ARGS "--build"
                                                                                 "${CMAKE_CURRENT_BINARY_DIR}/deb_package"
                                                                                 "${PACK_FILE_NAME}.deb"
                                                                            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/deb_package/${PACK_FILE_NAME}.deb")
        add_custom_command(TARGET ${PACKAGING_TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/deb_package")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PACK_FILE_NAME}.deb" DESTINATION ".")
    endif()
else()
    message(FATAL_ERROR "Some stange things are here")
endif()

