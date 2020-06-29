include(CheckFunctionExists)
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

add_executable (ELFtoNTK
ELFtoNTK.cpp
TELFFile.cpp
)

target_link_libraries (ELFtoNTK PUBLIC DCL PRIVATE Threads::Threads)
install(TARGETS ELFtoNTK RUNTIME DESTINATION bin)

add_executable (ELFtoPKG
ELFtoPKG.cpp
TELFFile.cpp
)

target_link_libraries (ELFtoPKG LINK_PUBLIC DCL pthread)
target_link_libraries (ELFtoPKG PUBLIC DCL PRIVATE Threads::Threads)
install(TARGETS ELFtoPKG RUNTIME DESTINATION bin)

add_executable (Rex
Rex.cpp
)

target_link_libraries (Rex PUBLIC DCL PRIVATE Threads::Threads)
install(TARGETS ELFtoPKG RUNTIME DESTINATION bin)

file(GLOB TEST_SRC_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/Tests/Rex/test-diff-*.src)
foreach(TEST_SRC ${TEST_SRC_SCRIPTS})
    add_test(NAME ${TEST_SRC}
             CONFIGURATIONS Debug
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Tests/Rex
             COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/Tests/Rex/test-diff.sh $<TARGET_FILE:Rex> ${TEST_SRC})
endforeach(TEST_SRC ${TEST_SRC_SCRIPTS})
