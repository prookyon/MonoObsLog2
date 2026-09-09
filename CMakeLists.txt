cmake_minimum_required(VERSION 3.28)

if (POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif ()

project(MonoObsLog VERSION 1.1.0 LANGUAGES CXX)

set(PROJECT_DEFAULT_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/deploy")
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT
        OR CMAKE_INSTALL_PREFIX STREQUAL "C:/Program Files (x86)/${PROJECT_NAME}")
    set(CMAKE_INSTALL_PREFIX "${PROJECT_DEFAULT_INSTALL_PREFIX}" CACHE PATH
            "Install path prefix, prepended onto install directories."
            FORCE
    )
endif ()

include(GNUInstallDirs)
include(FetchContent)
FetchContent_Declare(
        supernovas
        GIT_REPOSITORY https://github.com/Smithsonian/SuperNOVAS.git
        GIT_TAG        v1.6.0
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(supernovas)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network)
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

find_package(libzip CONFIG REQUIRED)
find_package(OpenXLSX CONFIG REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/Qwt.cmake")


set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/settingsmanager.cpp
    src/db/databasemanager.cpp
    src/db/databasebackup.cpp
    src/db/objectsrepository.cpp
    src/db/sessionsrepository.cpp
    src/db/camerasrepository.cpp
    src/db/filtertypesrepository.cpp
    src/db/filtersrepository.cpp
    src/db/telescopesrepository.cpp
    src/db/observationsrepository.cpp
    src/simbadquery.cpp
    src/tabs/objectstab.cpp
    src/tabs/sessionstab.cpp
    src/tabs/camerastab.cpp
    src/tabs/filtertypestab.cpp
    src/tabs/filterstab.cpp
    src/tabs/telescopestab.cpp
    src/tabs/observationstab.cpp
    src/tabs/objectstatstab.cpp
    src/tabs/monthlystatstab.cpp
    src/tabs/settingstab.cpp
    src/tabs/abouttab.cpp
    src/astrocalc.cpp
)

set(PROJECT_HEADERS
    include/mainwindow.h
    include/settingsmanager.h
    include/db/databasemanager.h
    include/db/databasebackup.h
    include/db/objectsrepository.h
    include/db/sessionsrepository.h
    include/db/camerasrepository.h
    include/db/filtertypesrepository.h
    include/db/filtersrepository.h
    include/db/telescopesrepository.h
    include/db/observationsrepository.h
    include/simbadquery.h
    include/tabs/objectstab.h
    include/tabs/sessionstab.h
    include/tabs/camerastab.h
    include/tabs/filtertypestab.h
    include/tabs/filterstab.h
    include/tabs/telescopestab.h
    include/tabs/observationstab.h
    include/tabs/objectstatstab.h
    include/tabs/monthlystatstab.h
    include/tabs/settingstab.h
    include/tabs/abouttab.h
    include/numerictablewidgetitem.h
    include/astrocalc.h
    include/ER.h
)

set(PROJECT_UI
    uifiles/mainwindow.ui
    uifiles/objects_tab.ui
    uifiles/sessions_tab.ui
    uifiles/cameras_tab.ui
    uifiles/filtertypes_tab.ui
    uifiles/filters_tab.ui
    uifiles/telescopes_tab.ui
    uifiles/observations_tab.ui
    uifiles/objectstats_tab.ui
    uifiles/monthlystats_tab.ui
    uifiles/settings_tab.ui
    uifiles/about_tab.ui
)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/images/resources.rc")


qt_add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_UI}
    resources.qrc
    ${app_icon_resource_windows}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

set_target_properties(${PROJECT_NAME} PROPERTIES
        AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/uifiles"
)

target_include_directories(${PROJECT_NAME}
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Network
        libzip::zip
        supernovas::core
        OpenXLSX::OpenXLSX
        Qwt::Qwt
)


if (WIN32)
    # Set Windows subsystem to Windows (not console)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND_EXPAND_LISTS
    )

    # Adds necessary files to run program from build directory
    # --force to overwrite Qt6 dll-s created as part of Qwt build process
    set(WINDEPLOYQT "${QT6_INSTALL_PREFIX}/bin/windeployqt.exe")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${WINDEPLOYQT}
            $<TARGET_FILE:${PROJECT_NAME}>
            --compiler-runtime
            --force
    )

    install(TARGETS ${PROJECT_NAME}
            RUNTIME_DEPENDENCY_SET ${PROJECT_NAME}_runtime_dependencies
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(CODE [[
        if (POLICY CMP0207)
            cmake_policy(SET CMP0207 NEW)
        endif ()
    ]])

    install(RUNTIME_DEPENDENCY_SET ${PROJECT_NAME}_runtime_dependencies
            PRE_EXCLUDE_REGEXES
            [[api-ms-.*]]
            [[ext-ms-.*]]
            POST_EXCLUDE_REGEXES
            [[.*[Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\].*]]
            [[.*[Ss][Yy][Ss][Tt][Ee][Mm]32[/\\].*]]
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    qt_generate_deploy_app_script(
            TARGET ${PROJECT_NAME}
            OUTPUT_SCRIPT qt_deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
            DEPLOY_TOOL_OPTIONS
            --compiler-runtime
            --force
    )
    install(SCRIPT ${qt_deploy_script})
endif ()
