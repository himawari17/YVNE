cmake_minimum_required(VERSION 3.25)

project(yvne VERSION 0.1.0 LANGUAGES C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

option(VN_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(VN_BUILD_MACOS_BUNDLE "Build vn as a macOS .app bundle" OFF)
option(VN_VENDORED_DEPENDENCIES
    "Build SDL, its codecs and GLAD into the executable"
    ON
)

include(cmake/Dependencies.cmake)

add_library(yvne STATIC
    src/core/logger.c
    src/core/engine.c
    src/core/timer.c
	src/core/frame_clock.c
    src/platform/platform.c
    src/platform/window.c
	src/platform/input.c
	src/platform/paths.c
	src/resource/manifest.c
	src/resource/stream.c
	src/resource/directory_mount.c
	src/assets/texture.c
    src/render_gl/texture.c
    src/render_gl/renderer.c
    src/render_gl/viewport.c
)

target_include_directories(yvne PUBLIC inc)
target_include_directories(yvne SYSTEM PRIVATE third_party/stb)
target_compile_features(yvne PUBLIC c_std_23)
set_target_properties(yvne PROPERTIES C_EXTENSIONS OFF)

target_compile_definitions(yvne PRIVATE YVNE_VERSION="${PROJECT_VERSION}")
target_compile_definitions(yvne PUBLIC $<$<CONFIG:Debug>:YVNE_DEBUG=1>)
target_link_libraries(yvne PRIVATE
    SDL3::SDL3
    yvne::glad
    OpenGL::GL
)

add_executable(vn src/main.c)
target_link_libraries(vn PRIVATE yvne yvne::dependencies)

if(APPLE AND VN_BUILD_MACOS_BUNDLE)
    set_target_properties(vn PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "yvne"
        MACOSX_BUNDLE_GUI_IDENTIFIER "dev.yvne.player"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

install(TARGETS vn
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(MSVC)
    set(VN_WARNING_FLAGS /W4)
    if(VN_WARNINGS_AS_ERRORS)
        list(APPEND VN_WARNING_FLAGS /WX)
    endif()
else()
    set(VN_WARNING_FLAGS
        -Wall
        -Wextra
        -Wpedantic
        -Wformat=2
        -Wpointer-arith
        -Wstrict-prototypes
        -Wold-style-definition
        -Wnested-externs
    )
    if(VN_WARNINGS_AS_ERRORS)
        list(APPEND VN_WARNING_FLAGS -Werror)
    endif()
endif()

target_compile_options(yvne PRIVATE ${VN_WARNING_FLAGS})
target_compile_options(vn PRIVATE ${VN_WARNING_FLAGS})

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
