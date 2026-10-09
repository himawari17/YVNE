find_package(OpenGL REQUIRED)

add_library(yvne_glad STATIC
    "${PROJECT_SOURCE_DIR}/third_party/glad/src/gl.c"
)
target_include_directories(yvne_glad PUBLIC
    "${PROJECT_SOURCE_DIR}/third_party/glad/include"
)
target_compile_features(yvne_glad PRIVATE c_std_99)
add_library(yvne::glad ALIAS yvne_glad)

if(VN_VENDORED_DEPENDENCIES)
    include(FetchContent)

    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "Build SDL shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build SDL static library" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL test library" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "Build SDL tests" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "Build SDL examples" FORCE)
    set(SDL_DISABLE_INSTALL ON CACHE BOOL "Disable SDL install target" FORCE)

    FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG f87239e71e42da91ca317a12eefb82cfbf3393eb
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(SDL3)

    set(SDLTTF_VENDORED ON CACHE BOOL "Vendor SDL_ttf dependencies" FORCE)
    set(SDLTTF_INSTALL OFF CACHE BOOL "Install SDL_ttf" FORCE)
    set(SDLTTF_SAMPLES OFF CACHE BOOL "Build SDL_ttf samples" FORCE)
    set(SDLTTF_TESTS OFF CACHE BOOL "Build SDL_ttf tests" FORCE)
    set(SDLTTF_PLUTOSVG OFF CACHE BOOL "Enable color emoji SVG support" FORCE)
    FetchContent_Declare(SDL3_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG a1ce3670aec736ecbf0936c43f2f0cc53aa61e5b
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        GIT_SUBMODULES external/freetype external/harfbuzz
        GIT_SUBMODULES_RECURSE TRUE
    )
    FetchContent_MakeAvailable(SDL3_ttf)

    set(SDLMIXER_VENDORED ON CACHE BOOL "Vendor SDL_mixer dependencies" FORCE)
    set(SDLMIXER_DEPS_SHARED OFF CACHE BOOL "Dynamically load audio codecs" FORCE)
    set(SDLMIXER_INSTALL OFF CACHE BOOL "Install SDL_mixer" FORCE)
    set(SDLMIXER_TESTS OFF CACHE BOOL "Build SDL_mixer tests" FORCE)
    set(SDLMIXER_EXAMPLES OFF CACHE BOOL "Build SDL_mixer examples" FORCE)
    set(SDLMIXER_FLAC_LIBFLAC OFF CACHE BOOL "Use libFLAC" FORCE)
    set(SDLMIXER_GME OFF CACHE BOOL "Enable game music formats" FORCE)
    set(SDLMIXER_MOD OFF CACHE BOOL "Enable module formats" FORCE)
    set(SDLMIXER_MP3_MPG123 OFF CACHE BOOL "Use mpg123" FORCE)
    set(SDLMIXER_MIDI OFF CACHE BOOL "Enable MIDI" FORCE)
    set(SDLMIXER_VORBIS_VORBISFILE OFF CACHE BOOL "Use libvorbisfile" FORCE)
    set(SDLMIXER_WAVPACK OFF CACHE BOOL "Enable WavPack" FORCE)
    FetchContent_Declare(SDL3_mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG 72a81869b45e249e8e67102db4e98dd2441f05a1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        GIT_SUBMODULES external/ogg external/opus external/opusfile
        GIT_SUBMODULES_RECURSE TRUE
    )
    FetchContent_MakeAvailable(SDL3_mixer)
else()
    find_package(SDL3 3.4 REQUIRED CONFIG)
    find_package(SDL3_ttf 3.2 REQUIRED CONFIG)
    find_package(SDL3_mixer 3.2 REQUIRED CONFIG)
endif()

add_library(yvne_dependencies INTERFACE)
target_link_libraries(yvne_dependencies INTERFACE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
    yvne::glad
    OpenGL::GL
)
add_library(yvne::dependencies ALIAS yvne_dependencies)
