include(FetchContent)

# Unit test framework (decision D48): Catch2 v3, downloaded at configure time and pinned by
# version and hash, so every machine gets the same bytes.
FetchContent_Declare(Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.16.1.tar.gz
    URL_HASH SHA256=d16bd1b8b2364918bd472cb216394acb93bde6784890df0d91bccfb3f28d777d
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# Helper program with predictable behaviour for the process-launcher tests.
add_executable(process_helper tools/process_helper.cpp)
zack_configure_target(process_helper)

# The referee for whole-program tests (decision D49): runs a command and checks its exit
# code, standard output and standard error.
add_executable(expect_runner tools/expect_runner.cpp)
target_link_libraries(expect_runner PRIVATE zack_driver)
zack_configure_target(expect_runner)

# zack_add_expect_test(NAME <name> [EXPECT <options>...] COMMAND <command>...)
# Registers a test that runs COMMAND through the referee.
function(zack_add_expect_test)
    cmake_parse_arguments(ARG "" "NAME" "EXPECT;COMMAND" ${ARGN})
    add_test(NAME ${ARG_NAME}
        COMMAND expect_runner ${ARG_EXPECT} -- ${ARG_COMMAND})
endfunction()

# Unit tests.
add_executable(unit_tests unit/test_backend.cpp unit/test_process.cpp)
target_link_libraries(unit_tests PRIVATE zack_codegen zack_driver Catch2::Catch2WithMain)
add_dependencies(unit_tests process_helper)
zack_configure_target(unit_tests)
catch_discover_tests(unit_tests
    PROPERTIES ENVIRONMENT "ZACK_PROCESS_HELPER=$<TARGET_FILE:process_helper>")

# Golden tests: compare a tool's output with a file stored in the repository.
zack_add_expect_test(NAME golden_driver_help
    EXPECT --stdout-file ${CMAKE_CURRENT_SOURCE_DIR}/golden/driver_help.txt
    COMMAND $<TARGET_FILE:zackc> --help)

zack_add_expect_test(NAME driver_version_reports_pinned_llvm
    EXPECT --stdout-contains "zackc 0.0.0" --stdout-contains "LLVM 20."
    COMMAND $<TARGET_FILE:zackc> --version)

zack_add_expect_test(NAME driver_without_arguments_is_a_usage_error
    EXPECT --exit-code 2 --stderr-contains "Usage: zackc"
    COMMAND $<TARGET_FILE:zackc>)

zack_add_expect_test(NAME driver_unknown_option_is_a_usage_error
    EXPECT --exit-code 2 --stderr-contains "unknown command '--no-such-option'"
    COMMAND $<TARGET_FILE:zackc> --no-such-option)

# The referee itself must be able to fail. Each of these deliberately wrong expectations has
# to make it exit with an error, so WILL_FAIL turns that into a pass.
function(zack_add_referee_must_fail name)
    zack_add_expect_test(NAME referee_detects_${name} ${ARGN})
    set_tests_properties(referee_detects_${name} PROPERTIES WILL_FAIL TRUE)
endfunction()

zack_add_referee_must_fail(wrong_exit_code
    EXPECT --exit-code 7
    COMMAND $<TARGET_FILE:zackc> --version)
zack_add_referee_must_fail(wrong_stdout_file
    EXPECT --stdout-file ${CMAKE_CURRENT_SOURCE_DIR}/golden/driver_help.txt
    COMMAND $<TARGET_FILE:zackc> --version)
zack_add_referee_must_fail(missing_stdout_text
    EXPECT --stdout-contains "this text is not printed"
    COMMAND $<TARGET_FILE:zackc> --version)
zack_add_referee_must_fail(missing_stderr_text
    EXPECT --stderr-contains "this text is not printed"
    COMMAND $<TARGET_FILE:zackc> --version)
zack_add_referee_must_fail(timeout
    EXPECT --timeout 0.3
    COMMAND $<TARGET_FILE:process_helper> sleep 30)
zack_add_referee_must_fail(program_that_cannot_start
    COMMAND zack-no-such-program-exists)

# End-to-end tests of the Phase 0 spike: build a program with the Backend, link it with the
# C runtime, run it, and check what the user can observe. Each mode has a build test that
# produces the executable (fixture setup) and a run test that checks it.
set(ZACK_SPIKE_DIR ${CMAKE_CURRENT_BINARY_DIR}/spike)
file(MAKE_DIRECTORY ${ZACK_SPIKE_DIR})

function(zack_add_spike_test mode)
    cmake_parse_arguments(ARG "" "" "EXPECT" ${ARGN})
    set(exe ${ZACK_SPIKE_DIR}/${mode}${CMAKE_EXECUTABLE_SUFFIX})
    add_test(NAME spike_build_${mode}
        COMMAND backend_spike ${mode} ${exe} $<TARGET_FILE:zackrt>)
    set_tests_properties(spike_build_${mode} PROPERTIES FIXTURES_SETUP spike_${mode})
    zack_add_expect_test(NAME spike_run_${mode}
        EXPECT --stdout-file ${CMAKE_CURRENT_SOURCE_DIR}/e2e/phase0/${mode}.expected ${ARG_EXPECT}
        COMMAND ${exe})
    set_tests_properties(spike_run_${mode} PROPERTIES FIXTURES_REQUIRED spike_${mode})
endfunction()

zack_add_spike_test(exit42 EXPECT --exit-code 42)
zack_add_spike_test(print42 EXPECT --exit-code 0)
zack_add_spike_test(panic EXPECT --exit-code 101 --stderr-contains "panic: boom"
                                 --stderr-contains "spike.zl:7")
