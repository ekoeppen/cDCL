include(CheckFunctionExists)
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

add_executable (nespackager
NESPackager.cpp
)
add_executable (pbbookmaker
PBBookMaker.cpp
)
add_executable (DumpPkgDir
DumpPkgDir.cpp
)
add_executable (DumpPkgPart
DumpPkgPart.cpp
)
add_executable (WatsonEnabler
WatsonEnabler.cpp
)

target_link_libraries (nespackager PUBLIC DCL PRIVATE Threads::Threads)
target_link_libraries (pbbookmaker PUBLIC DCL PRIVATE Threads::Threads)
target_link_libraries (DumpPkgDir PUBLIC DCL PRIVATE Threads::Threads)
target_link_libraries (DumpPkgPart PUBLIC DCL PRIVATE Threads::Threads)
target_link_libraries (WatsonEnabler PUBLIC DCL PRIVATE Threads::Threads)

if(CDCL_INSTALL)
    install(TARGETS nespackager RUNTIME DESTINATION bin COMPONENT tools)
    install(FILES NESPackager_Template/nespkgtemplate.nsof DESTINATION share/DCL/ COMPONENT tools)
    install(TARGETS pbbookmaker RUNTIME DESTINATION bin COMPONENT tools)
    install(FILES PBBookMaker_Templates/Reader_2.0/pbbooktemplate.nsof DESTINATION share/DCL/ COMPONENT tools)
    install(DIRECTORY PBBookMaker_Templates/ DESTINATION share/DCL/ COMPONENT tools FILES_MATCHING PATTERN *.nsof)
    install(TARGETS DumpPkgDir RUNTIME DESTINATION bin COMPONENT tools)
    install(TARGETS DumpPkgPart RUNTIME DESTINATION bin COMPONENT tools)
    install(TARGETS WatsonEnabler RUNTIME DESTINATION bin COMPONENT tools)
endif()

if(CDCL_BUILD_TESTS AND BUILD_TESTING)
    file(GLOB TEST_PKGS ${CMAKE_CURRENT_SOURCE_DIR}/Tests/*.pkg)
    foreach(TEST_PKG ${TEST_PKGS})
        add_test(NAME DumpPkgDir-${TEST_PKG}
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Tests
                 COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/Tests/test-DumpPkgDir.sh $<TARGET_FILE:DumpPkgDir> ${TEST_PKG})
        add_test(NAME DumpPkgPart-${TEST_PKG}
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Tests
                 COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/Tests/test-DumpPkgPart.sh $<TARGET_FILE:DumpPkgPart> ${TEST_PKG})
    endforeach(TEST_PKG ${TEST_PKGS})
endif()
