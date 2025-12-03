cmake_minimum_required(VERSION 3.31)

project(MonoObsLog VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# temporarily use git version
# TODO: go back to vcpkg once 1.5.1 is pushed there
add_subdirectory(libraries/SuperNOVAS EXCLUDE_FROM_ALL)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS uifiles)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network)

# Find libzip
find_package(libzip CONFIG REQUIRED)
# temporarily use git version
#find_package(supernovas CONFIG REQUIRED)
find_package(OpenXLSX CONFIG REQUIRED)
find_package(unofficial-qwt CONFIG REQUIRED)



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

include_directories(include ${libzip_INCLUDE_DIRS} ${supernovas_INCLUDE_DIRS})

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/images/resources.rc")


add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_UI}
    resources.qrc
    ${app_icon_resource_windows}
)

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
    libzip::zip
    supernovas::core
    unofficial::qwt::qwt
    OpenXLSX::OpenXLSX
)


if (WIN32)
    # Set Windows subsystem to Windows (not console)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

    # Adds necessary files to run program from build directory
    # --force to overwrite Qt6 dll-s created as part of Qwt build process
    set(WINDEPLOYQT "${QT6_INSTALL_PREFIX}/bin/windeployqt.exe")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${WINDEPLOYQT} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} --compiler-runtime --force)

    # Install commands to create the deploy folder
    if (CMAKE_BUILD_TYPE STREQUAL Release)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../deploy)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zip.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../deploy)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bz2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../deploy)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zlib1.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../deploy)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/core.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../deploy)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qwt.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../deploy)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/OpenXLSX.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../deploy)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/nowide.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../deploy)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pugixml.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../deploy)

        set(WINDEPLOYQT "${Qt6_DIR}/../../../bin/windeployqt.exe")
        set(DEPLOYEDEXE "${CMAKE_CURRENT_BINARY_DIR}/../deploy/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
        install(CODE "
             execute_process(
                COMMAND \"${WINDEPLOYQT}\"
                \"${DEPLOYEDEXE}\"
                --compiler-runtime
                --force
            )
        ")
    endif ()
endif ()