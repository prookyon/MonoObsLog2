set(QWT_ROOT "" CACHE PATH "Path to the Qwt installation root")

if (NOT QWT_ROOT)
    message(FATAL_ERROR
            "QWT_ROOT must point to a manually built Qwt installation. "
            "Example: -DQWT_ROOT=C:/dev/qwt-6.3.0")
endif ()

get_filename_component(_qwt_root "${QWT_ROOT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(_qwt_include_dir "${_qwt_root}/src")
set(_qwt_release_dll "${_qwt_root}/lib/qwt.dll")
set(_qwt_release_lib "${_qwt_root}/lib/qwt.lib")
set(_qwt_debug_dll "${_qwt_root}/lib/qwtd.dll")
set(_qwt_debug_lib "${_qwt_root}/lib/qwtd.lib")

foreach (_qwt_path IN ITEMS
        "${_qwt_include_dir}"
        "${_qwt_release_dll}"
        "${_qwt_release_lib}"
        "${_qwt_debug_dll}"
        "${_qwt_debug_lib}"
)
    if (NOT EXISTS "${_qwt_path}")
        message(FATAL_ERROR
                "QWT_ROOT is set to '${QWT_ROOT}', resolved to '${_qwt_root}', "
                "but required Qwt path does not exist: "
                "${_qwt_path}")
    endif ()
endforeach ()

add_library(Qwt::Qwt SHARED IMPORTED GLOBAL)

set_target_properties(Qwt::Qwt PROPERTIES
        IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
        IMPORTED_LOCATION_RELEASE "${_qwt_release_dll}"
        IMPORTED_IMPLIB_RELEASE "${_qwt_release_lib}"
        IMPORTED_LOCATION_DEBUG "${_qwt_debug_dll}"
        IMPORTED_IMPLIB_DEBUG "${_qwt_debug_lib}"
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
        INTERFACE_INCLUDE_DIRECTORIES "${_qwt_include_dir}"
)

unset(_qwt_root)
unset(_qwt_include_dir)
unset(_qwt_release_dll)
unset(_qwt_release_lib)
unset(_qwt_debug_dll)
unset(_qwt_debug_lib)
