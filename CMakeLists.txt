cmake_minimum_required(VERSION 3.16)

project(mgwObsLog VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS uifiles)

find_package(PkgConfig REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network Charts PrintSupport)

# Find libzip
find_package(libzip CONFIG REQUIRED)
find_package(supernovas CONFIG REQUIRED)

pkg_search_module(Qwt REQUIRED IMPORTED_TARGET Qt6Qwt6)


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
    Qt6::Charts
    Qt6::PrintSupport
    libzip::zip
    supernovas::core
    PkgConfig::Qwt
)

# Set Windows subsystem to Windows (not console)
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
#    add_custom_command(
#        TARGET ${PROJECT_NAME}
#        POST_BUILD
#        COMMAND ${Qt6_DIR}/../../../bin/windeployqt.exe
#        ARGS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} --compiler-runtime
#    )
endif()