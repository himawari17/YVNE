add_executable(vndeps dependencies_test.c)
target_link_libraries(vndeps PRIVATE yvne::dependencies)

set(YVNE_TEST_TARGETS vndeps)
foreach(component IN ITEMS version logger timer frame_clock input paths manifest vfs stream assets platform engine)
    add_executable(vn${component} ${component}_test.c)
    target_link_libraries(vn${component} PRIVATE yvne SDL3::SDL3)
    list(APPEND YVNE_TEST_TARGETS vn${component})
endforeach()

foreach(target IN LISTS YVNE_TEST_TARGETS)
    target_compile_options(${target} PRIVATE ${VN_WARNING_FLAGS})
    # Tests use assert for checks and setup, including in Release builds.
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )
endforeach()

target_compile_definitions(vnversion PRIVATE YVNE_TEST_VERSION="${PROJECT_VERSION}")
target_link_libraries(vnengine PRIVATE yvne::glad)
target_link_libraries(vnplatform PRIVATE yvne::glad)
target_include_directories(vnassets SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/third_party/stb")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/assets")
target_compile_definitions(vnassets PRIVATE
    YVNE_TEST_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}/assets"
)

file(COPY fixtures/project DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/fixtures")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fixtures/outside.txt" "outside project\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fixtures/project-sibling/outside.txt" "outside project\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fixtures/project/assets/empty.txt" "")
foreach(target IN ITEMS vnmanifest vnvfs vnstream)
    target_compile_definitions(${target} PRIVATE
        YVNE_TEST_PROJECT_DIR="${CMAKE_CURRENT_BINARY_DIR}/fixtures/project"
    )
endforeach()
if(UNIX)
    file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/fixtures/project-sibling/outside.txt"
        "${CMAKE_CURRENT_BINARY_DIR}/fixtures/project/assets/escape.txt" SYMBOLIC)
    file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/fixtures/project/assets/sample.txt"
        "${CMAKE_CURRENT_BINARY_DIR}/fixtures/project/assets/linked.txt" SYMBOLIC)
    target_compile_definitions(vnvfs PRIVATE YVNE_TEST_SYMLINKS=1)
endif()

function(yvne_add_test name target)
    add_test(NAME ${name} COMMAND ${target})
    add_custom_target(check_${name}
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -C "$<CONFIG>" -R "^${name}$"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        DEPENDS vn ${target}
        USES_TERMINAL
    )
endfunction()

yvne_add_test(dependencies_link vndeps)
foreach(component IN ITEMS version logger timer frame_clock input paths manifest vfs stream assets)
    yvne_add_test(${component}_unit vn${component})
endforeach()
yvne_add_test(platform_failure vnplatform)
yvne_add_test(engine_smoke vnengine)

set_tests_properties(dependencies_link version_unit logger_unit PROPERTIES LABELS "M0;unit")
set_tests_properties(timer_unit frame_clock_unit input_unit paths_unit manifest_unit vfs_unit stream_unit
    PROPERTIES LABELS "M1;unit")
set_tests_properties(platform_failure PROPERTIES LABELS "M1;integration")
set_tests_properties(engine_smoke PROPERTIES LABELS "M0;M1;graphics;integration" RUN_SERIAL TRUE TIMEOUT 20)
set_tests_properties(assets_unit PROPERTIES LABELS "M2;unit")
set_tests_properties(logger_unit PROPERTIES
    PASS_REGULAR_EXPRESSION "\\[WARN\\] logger_test.c:42: logger value: 7[\r\n]+[^\r\n]*\\[ERROR\\] logger_test.c:43: error test[\r\n]+[^\r\n]*\\[UNKNOWN\\] logger_test.c:44: unknown level test"
)

add_custom_target(check
    COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -C "$<CONFIG>"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    DEPENDS vn ${YVNE_TEST_TARGETS}
    USES_TERMINAL
)
