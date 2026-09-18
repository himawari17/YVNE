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
	src/version.c
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
)

target_include_directories(yvne PUBLIC inc)
target_compile_features(yvne PUBLIC c_std_23)
set_target_properties(yvne PROPERTIES C_EXTENSIONS OFF)

target_compile_definitions(yvne PRIVATE YVNE_VERSION="${PROJECT_VERSION}")
target_compile_definitions(yvne PUBLIC $<$<CONFIG:Debug>:YVNE_DEBUG=1>)
target_link_libraries(yvne PRIVATE SDL3::SDL3 yvne::glad OpenGL::GL)

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
    add_executable(vndeps tests/dependencies_test.c)
    target_link_libraries(vndeps PRIVATE yvne::dependencies)
    target_compile_options(vndeps PRIVATE ${VN_WARNING_FLAGS})
    set_target_properties(vndeps PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )

    add_executable(vnlogger tests/logger_test.c)
    target_link_libraries(vnlogger PRIVATE yvne)
    target_compile_options(vnlogger PRIVATE ${VN_WARNING_FLAGS})
    set_target_properties(vnlogger PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )

    add_executable(vnmanifest tests/manifest_test.c)
    target_link_libraries(vnmanifest PRIVATE yvne)
    target_compile_options(vnmanifest PRIVATE ${VN_WARNING_FLAGS})
    target_compile_definitions(vnmanifest PRIVATE
        YVNE_TEST_PROJECT_DIR="${PROJECT_SOURCE_DIR}/test_project"
    )
    set_target_properties(vnmanifest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )

    add_executable(vnvfs tests/vfs_test.c)
    target_link_libraries(vnvfs PRIVATE yvne)
    target_compile_options(vnvfs PRIVATE ${VN_WARNING_FLAGS})
    target_compile_definitions(vnvfs PRIVATE
        YVNE_TEST_PROJECT_DIR="${PROJECT_SOURCE_DIR}/test_project"
    )
    set_target_properties(vnvfs PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )

    add_test(NAME dependencies_link COMMAND vndeps)
    add_test(NAME logger_unit COMMAND vnlogger)
    add_test(NAME manifest_unit COMMAND vnmanifest)
    add_test(NAME vfs_unit COMMAND vnvfs)
endif()
