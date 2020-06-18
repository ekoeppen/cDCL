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

target_link_libraries (DCLTests LINK_PUBLIC DCL)

file(GLOB TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/test-*.sh)
foreach(TEST_SCRIPT ${TEST_SCRIPTS})
    add_test(NAME ${TEST_SCRIPT}
             CONFIGURATIONS Debug
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts
             COMMAND sh ${TEST_SCRIPT} $<TARGET_FILE:DCLTests>)
endforeach(TEST_SCRIPT ${TEST_SCRIPTS})

