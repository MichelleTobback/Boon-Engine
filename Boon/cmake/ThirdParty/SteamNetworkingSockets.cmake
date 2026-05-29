# SteamNetworkingSockets third-party integration for Boon.

if(NOT DEFINED BOON_ENGINE_ROOT)
    message(FATAL_ERROR "BOON_ENGINE_ROOT must be defined before including SteamNetworkingSockets.cmake")
endif()

set(STEAMNET_DIR "${BOON_ENGINE_ROOT}/external/GameNetworkingSockets")
set(STEAMNET_INCLUDE "${STEAMNET_DIR}/include")
set(STEAMNET_BIN_DIR "${STEAMNET_DIR}/bin")

if(NOT TARGET SteamNetworkingSockets)
    add_library(SteamNetworkingSockets STATIC IMPORTED GLOBAL)

    set_target_properties(SteamNetworkingSockets PROPERTIES
        IMPORTED_LOCATION_DEBUG "${STEAMNET_BIN_DIR}/Debug/GameNetworkingSockets.lib"
        IMPORTED_LOCATION_RELEASE "${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.lib"
        IMPORTED_LOCATION_RELWITHDEBINFO "${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.lib"
        IMPORTED_LOCATION_MINSIZEREL "${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.lib"

        INTERFACE_INCLUDE_DIRECTORIES "${STEAMNET_INCLUDE}"
    )

    if(WIN32)
        target_compile_definitions(SteamNetworkingSockets
            INTERFACE
                WIN32_LEAN_AND_MEAN
                NOMINMAX
        )
    endif()
endif()

function(boon_register_runtime_dependency RUNTIME_FILE)
    set_property(GLOBAL APPEND PROPERTY
        BOON_PACKAGE_RUNTIME_DEPENDENCIES
        "${RUNTIME_FILE}"
    )
endfunction()

function(boon_use_SteamNetworkingSockets TARGET_NAME)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
            SteamNetworkingSockets
    )

    if(WIN32)
        boon_register_runtime_dependency(
            "$<IF:$<CONFIG:Debug>,${STEAMNET_BIN_DIR}/Debug/GameNetworkingSockets.dll,${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.dll>"
        )

        boon_register_runtime_dependency(
            "$<IF:$<CONFIG:Debug>,${STEAMNET_BIN_DIR}/Debug/abseil_dll.dll,${STEAMNET_BIN_DIR}/Release/abseil_dll.dll>"
        )

        boon_register_runtime_dependency(
            "$<IF:$<CONFIG:Debug>,${STEAMNET_BIN_DIR}/Debug/libcrypto-3-x64.dll,${STEAMNET_BIN_DIR}/Release/libcrypto-3-x64.dll>"
        )

        boon_register_runtime_dependency(
            "$<IF:$<CONFIG:Debug>,${STEAMNET_BIN_DIR}/Debug/libprotobufd.dll,${STEAMNET_BIN_DIR}/Release/libprotobuf.dll>"
        )
    endif()
endfunction()