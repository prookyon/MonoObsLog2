cmake_minimum_required(VERSION 3.16)

project(mgwObsLog VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS uifiles)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network Charts PrintSupport)

# Find libzip
find_package(libzip CONFIG REQUIRED)
find_package(supernovas CONFIG REQUIRED)

#to work around linker issue with too large debug info
set_source_files_properties(
        lib/QCustomPlot/qcustomplot.cpp
        PROPERTIES COMPILE_FLAGS "-g0"
)

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

add_library(customplot SHARED
        lib/QCustomPlot/qcustomplot.cpp
        lib/QCustomPlot/qcustomplot.h
)
target_compile_definitions(customplot PRIVATE QCUSTOMPLOT_COMPILE_LIBRARY)

target_link_libraries(customplot
        Qt6::Core
        Qt6::Widgets
        Qt6::PrintSupport
)


add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_UI}
    resources.qrc
    ${app_icon_resource_windows}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE QCUSTOMPLOT_USE_LIBRARY)

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
    Qt6::Charts
    Qt6::PrintSupport
    libzip::zip
    supernovas::core
    customplot
)

# Set Windows subsystem to Windows (not console)
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${Qt6_DIR}/../../../bin/windeployqt.exe
        ARGS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} --compiler-runtime
    )
endif()

# set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install CACHE PATH "Install path" FORCE)

# install(TARGETS ${PROJECT_NAME}
#         RUNTIME DESTINATION .
#         LIBRARY DESTINATION .
#         ARCHIVE DESTINATION .
# )

# install(CODE "
#     message(\"windeployqt\")
#     execute_process(COMMAND ${Qt6_DIR}/../../../bin/windeployqt.exe ${CMAKE_INSTALL_PREFIX}${PROJECT_NAME}.exe --compiler-runtime)
# ")