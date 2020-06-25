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

target_link_libraries (ELFtoPKG PUBLIC DCL PRIVATE Threads::Threads)
install(TARGETS ELFtoPKG RUNTIME DESTINATION bin)
