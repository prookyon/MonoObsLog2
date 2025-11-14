cmake_minimum_required(VERSION 3.16)

project(mgwObsLog VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS uifiles)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/databasemanager.cpp
    src/objectstab.cpp
    src/sessionstab.cpp
    src/camerastab.cpp
    src/filtertypestab.cpp
    src/filterstab.cpp
    src/telescopestab.cpp
    src/observationstab.cpp
    src/objectstatstab.cpp
    src/monthlystatstab.cpp
    src/settingstab.cpp
    src/abouttab.cpp
)

set(PROJECT_HEADERS
    include/mainwindow.h
    include/databasemanager.h
    include/objectstab.h
    include/sessionstab.h
    include/camerastab.h
    include/filtertypestab.h
    include/filterstab.h
    include/telescopestab.h
    include/observationstab.h
    include/objectstatstab.h
    include/monthlystatstab.h
    include/settingstab.h
    include/abouttab.h
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

include_directories(include)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_UI}
)

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

# Set Windows subsystem to Windows (not console)
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()