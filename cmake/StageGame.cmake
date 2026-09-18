cmake_minimum_required(VERSION 3.25)
if(NOT EXISTS "${SOURCE_DIR}/game.json")
    message(FATAL_ERROR "Missing game.json startup manifest")
endif()
file(MAKE_DIRECTORY "${GAME_DIR}/scripts" "${GAME_DIR}/licenses")
file(REMOVE
    "${GAME_DIR}/PhysXGpu_64.dll"
    "${GAME_DIR}/PhysXDevice64.dll"
)
file(COPY "${RUNTIME}" "${IMGUI}" DESTINATION "${GAME_DIR}")
file(COPY "${SCRIPTS}" DESTINATION "${GAME_DIR}/scripts")
# Copy the CPU runtime dependencies actually used by the player. Do not ship
# PhysX GPU binaries: the engine creates CPU scenes and those optional DLLs add
# hundreds of megabytes to a shareable game without being loaded.
foreach(PATTERN
    "SDL2*.dll"
    "freetype.dll"
    "libpng*.dll"
    "z.dll"
    "bz2.dll"
    "brotli*.dll"
    "PhysX_64.dll"
    "PhysXCommon_64.dll"
    "PhysXFoundation_64.dll"
    "PhysXCooking_64.dll"
)
    file(GLOB RUNTIME_DLLS "${VCPKG_BIN}/${PATTERN}")
    if(RUNTIME_DLLS)
        file(COPY ${RUNTIME_DLLS} DESTINATION "${GAME_DIR}")
    endif()
endforeach()
# Remove individual assets staged by a previous run when their sources disappear.
file(GLOB_RECURSE CONTENT_FILES LIST_DIRECTORIES false RELATIVE "${SOURCE_DIR}" "${SOURCE_DIR}/res/*")
if(EXISTS "${GAME_DIR}/.content-manifest")
    file(STRINGS "${GAME_DIR}/.content-manifest" PREVIOUS_CONTENT)
    foreach(OLD IN LISTS PREVIOUS_CONTENT)
        if(OLD MATCHES "^res/" AND NOT OLD MATCHES "(^|/)\\.\\.(/|$)" AND NOT OLD IN_LIST CONTENT_FILES)
            file(REMOVE "${GAME_DIR}/${OLD}")
        endif()
    endforeach()
endif()
string(REPLACE ";" "\n" CONTENT_MANIFEST "${CONTENT_FILES}")
file(WRITE "${GAME_DIR}/.content-manifest" "${CONTENT_MANIFEST}\n")
file(COPY "${SOURCE_DIR}/res" "${SOURCE_DIR}/game.json" DESTINATION "${GAME_DIR}")
file(GLOB LICENSES "${VCPKG_SHARE}/*/copyright")
foreach(LICENSE IN LISTS LICENSES)
    get_filename_component(PACKAGE_DIR "${LICENSE}" DIRECTORY)
    get_filename_component(PACKAGE "${PACKAGE_DIR}" NAME)
    configure_file("${LICENSE}" "${GAME_DIR}/licenses/${PACKAGE}.txt" COPYONLY)
endforeach()
foreach(PAIR "imgui/LICENSE.txt" "glm/copying.txt" "box2d/LICENSE")
    if(EXISTS "${SOURCE_DIR}/external/${PAIR}")
        string(REPLACE "/" "-" NAME "${PAIR}")
        configure_file("${SOURCE_DIR}/external/${PAIR}" "${GAME_DIR}/licenses/${NAME}" COPYONLY)
    endif()
endforeach()
message(STATUS "Game staged at ${GAME_DIR}")

# Use the newest installed redistributable: vcpkg dependencies may use a newer
# toolset than the engine itself. Ship Release CRT DLLs app-local.
if(WIN32 AND CONFIG STREQUAL "Release")
    file(GLOB CRT_DIRS
        "C:/Program Files/Microsoft Visual Studio/*/*/VC/Redist/MSVC/14.*/x64/Microsoft.VC*.CRT")
    set(CRT_VERSION "0")
    foreach(DIR IN LISTS CRT_DIRS)
        string(REGEX MATCH "MSVC/(14\\.[0-9]+\\.[0-9]+)" MATCHED "${DIR}")
        if(CMAKE_MATCH_1 VERSION_GREATER CRT_VERSION)
            set(CRT_VERSION "${CMAKE_MATCH_1}")
            set(CRT_DIR "${DIR}")
        endif()
    endforeach()
    if(NOT CRT_DIR)
        message(FATAL_ERROR "MSVC redistributable DLLs were not found")
    endif()
    file(GLOB CRT_DLLS "${CRT_DIR}/*.dll")
    file(COPY ${CRT_DLLS} DESTINATION "${GAME_DIR}")
endif()
