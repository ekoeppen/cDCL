include(CheckFunctionExists)
include(${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest/doctest.cmake)
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

add_library(doctest INTERFACE)
target_include_directories(doctest SYSTEM INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

add_library(doctest_main OBJECT doctest_main.cpp)
target_link_libraries(doctest_main PRIVATE doctest)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Tests)

add_executable(KTests
    Tests/UTestKMappedFile.cpp
    Tests/UTestKStreamsAndBuffers.cpp
    Tests/UTestKThreads.cpp
    $<TARGET_OBJECTS:doctest_main>
)
target_link_libraries(KTests PRIVATE K doctest Threads::Threads)

add_executable(DCLTests
    CommLayerShutdownTests.cpp
    Tests/TTestApplication.cpp
    Tests/TTestDockLink.cpp
    Tests/TTestEmptyFilesIntf.cpp
    Tests/TTestMemoryPipe.cpp
    Tests/TTestServer.cpp
    Tests/UTestEventQueue.cpp
    Tests/UTestWakeableWait.cpp
    Tests/UTestMNPProtocol.cpp
    Tests/UTestMNPPipe.cpp
    Tests/UTestMNPLinkLayer.cpp
    Tests/UTestNS.cpp
    Tests/UTestNSOF.cpp
    Tests/UTestPkg.cpp
    Tests/UTestTime.cpp
    Tests/UTestXML.cpp
    Tests/CommLayerCancelTests.cpp
    Tests/CommLayerStartStopTests.cpp
    Tests/ServerConfigurationTests.cpp
    $<TARGET_OBJECTS:doctest_main>
)
target_link_libraries(DCLTests PUBLIC DCL PRIVATE doctest Threads::Threads)

add_executable(NSReferencesDeathTest
    NSReferencesDeathTest.cpp
)
target_link_libraries(NSReferencesDeathTest PRIVATE DCL)

add_executable(DCLManualTests
    DCLManualTests.cpp
    Tests/TTestApplication.cpp
    Tests/TTestDockLink.cpp
    Tests/TTestEmptyFilesIntf.cpp
    Tests/TTestServer.cpp
    Tests/UTestCommLayers.cpp
)
target_link_libraries(DCLManualTests PRIVATE DCL Threads::Threads)

# Code Coverage Configuration
option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # Add required flags (GCC & LLVM/Clang)
  foreach(_cov_target IN ITEMS DCL DCLTests KTests)
    target_compile_options(${_cov_target} PRIVATE
      -O0        # no optimization
      -g         # generate debug info
      --coverage # sets all required flags
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(${_cov_target} PRIVATE -fprofile-update=atomic)
    endif()
  endforeach()
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(DCL PUBLIC --coverage)
    target_link_options(DCLTests PRIVATE --coverage)
    target_link_options(KTests PRIVATE --coverage)
  else()
    target_link_libraries(DCL PUBLIC --coverage)
    target_link_libraries(DCLTests PRIVATE --coverage)
    target_link_libraries(KTests PRIVATE --coverage)
  endif()
endif(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

if(NOT CMAKE_CROSSCOMPILING OR CMAKE_CROSSCOMPILING_EMULATOR)
    doctest_discover_tests(KTests TEST_PREFIX "KTests.")
    doctest_discover_tests(DCLTests
        TEST_PREFIX "DCL."
        PROPERTIES TIMEOUT 30
    )
else()
    add_test(NAME KTests COMMAND KTests)
    add_test(NAME DCLTests COMMAND DCLTests)
    set_tests_properties(DCLTests PROPERTIES TIMEOUT 30)
endif()

# KASSERT only aborts via SIGINT on POSIX Debug builds.
if(UNIX)
    add_test(NAME NSReferencesDeathTest
             CONFIGURATIONS Debug
             COMMAND NSReferencesDeathTest)
endif()

# Optional documentation check
add_test(NAME doxygen
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts
         COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/scripts/test-doxygen.sh)
set_tests_properties(doxygen PROPERTIES LABELS "optional")
