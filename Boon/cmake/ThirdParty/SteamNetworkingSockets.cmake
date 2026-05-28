# SteamNetworkingSockets third-party integration for Boon.

if(NOT DEFINED BOON_ENGINE_ROOT)
    message(FATAL_ERROR "BOON_ENGINE_ROOT must be defined before including SteamNetworkingSockets.cmake")
endif()

set(STEAMNET_DIR
    "${BOON_ENGINE_ROOT}/external/GameNetworkingSockets"
)

set(STEAMNET_INCLUDE
    "${STEAMNET_DIR}/include"
)

set(STEAMNET_BIN_DIR
    "${STEAMNET_DIR}/bin"
)

if(NOT TARGET SteamNetworkingSockets)
    add_library(SteamNetworkingSockets STATIC IMPORTED GLOBAL)

    set_target_properties(SteamNetworkingSockets PROPERTIES
        IMPORTED_LOCATION_DEBUG
            "${STEAMNET_BIN_DIR}/Debug/GameNetworkingSockets.lib"

        IMPORTED_LOCATION_RELEASE
            "${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.lib"

        IMPORTED_LOCATION_RELWITHDEBINFO
            "${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.lib"

        IMPORTED_LOCATION_MINSIZEREL
            "${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.lib"

        INTERFACE_INCLUDE_DIRECTORIES
            "${STEAMNET_INCLUDE}"
    )

    if(WIN32)
        target_compile_definitions(SteamNetworkingSockets
            INTERFACE
                WIN32_LEAN_AND_MEAN
                NOMINMAX
        )
    endif()
endif()

function(boon_use_SteamNetworkingSockets TARGET_NAME)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
            SteamNetworkingSockets
    )

    if(WIN32)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${STEAMNET_BIN_DIR}/Debug/GameNetworkingSockets.dll,${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.dll>"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>"

            COMMENT "Copying SteamNetworkingSockets.dll for ${TARGET_NAME}"
            VERBATIM
        )
    endif()
endfunction()