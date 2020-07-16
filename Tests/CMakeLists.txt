include(CheckFunctionExists)
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/Tests)
add_executable (DCLTests
tests.cpp
Tests/TTestApplication.cpp
Tests/TTestDockLink.cpp
Tests/TTestEmptyFilesIntf.cpp
Tests/TTestServer.cpp
Tests/UTestCommLayers.cpp
Tests/UTestNS.cpp
Tests/UTestNSOF.cpp
Tests/UTestPkg.cpp
Tests/UTestThreads.cpp
Tests/UTestTime.cpp
Tests/UTestXML.cpp
)

# Code Coverage Configuration
option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # Add required flags (GCC & LLVM/Clang)
  target_compile_options(DCLTests PRIVATE
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
  )
  target_compile_options(DCL PRIVATE
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
  )
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(DCL PRIVATE --coverage)
    target_link_options(DCLTests PRIVATE --coverage)
  else()
    target_link_libraries(DCL PRIVATE --coverage)
    target_link_libraries(DCLTests PRIVATE --coverage)
  endif()
endif(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

target_link_libraries (DCLTests PUBLIC DCL PRIVATE Threads::Threads)

file(GLOB TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/test-*.sh)
foreach(TEST_SCRIPT ${TEST_SCRIPTS})
    add_test(NAME ${TEST_SCRIPT}
             CONFIGURATIONS Debug
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts
             COMMAND bash ${TEST_SCRIPT} $<TARGET_FILE:DCLTests>)
endforeach(TEST_SCRIPT ${TEST_SCRIPTS})

