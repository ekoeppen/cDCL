include(CheckFunctionExists)
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

add_executable (ELFtoNTK
ELFtoNTK.cpp
TELFFile.cpp
)

target_link_libraries (ELFtoNTK PUBLIC DCL PRIVATE Threads::Threads)

add_executable (ELFtoPKG
ELFtoPKG.cpp
TELFFile.cpp
)

target_link_libraries (ELFtoPKG PUBLIC DCL PRIVATE Threads::Threads)
if (NOT WIN32)
    target_link_libraries (ELFtoPKG PRIVATE pthread)
endif()

add_executable (AIFtoPKG
AIFtoPKG.cpp
TAIFFile.cpp
)

target_link_libraries (AIFtoPKG PUBLIC DCL PRIVATE Threads::Threads)
if (NOT WIN32)
    target_link_libraries (AIFtoPKG PRIVATE pthread)
endif()

add_executable (Rex
Rex.cpp
)

target_link_libraries (Rex PUBLIC DCL PRIVATE Threads::Threads)

if(CDCL_INSTALL)
    install(TARGETS ELFtoNTK RUNTIME DESTINATION bin COMPONENT tools)
    install(TARGETS ELFtoPKG RUNTIME DESTINATION bin COMPONENT tools)
    install(TARGETS AIFtoPKG RUNTIME DESTINATION bin COMPONENT tools)
    install(TARGETS Rex RUNTIME DESTINATION bin COMPONENT tools)
endif()

if(CDCL_BUILD_TESTS AND BUILD_TESTING)
    file(GLOB TEST_SRC_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/Tests/Rex/test-diff-*.src)
    foreach(TEST_SRC ${TEST_SRC_SCRIPTS})
        add_test(NAME ${TEST_SRC}
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Tests/Rex
                 COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/Tests/Rex/test-diff.sh $<TARGET_FILE:Rex> ${TEST_SRC})
    endforeach(TEST_SRC ${TEST_SRC_SCRIPTS})
    add_test(NAME AIFtoPKG.cli
             COMMAND ${CMAKE_COMMAND}
                 -DAIFTOPKG=$<TARGET_FILE:AIFtoPKG>
                 -DREX=$<TARGET_FILE:Rex>
                 -DDUMPPKGDIR=$<TARGET_FILE:DumpPkgDir>
                 -DSAMPLE_AIF=${CMAKE_CURRENT_SOURCE_DIR}/Tests/AIFtoPKG/sample.aif
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/Tests/AIFtoPKG/test-aiftopkg.cmake)
endif()
