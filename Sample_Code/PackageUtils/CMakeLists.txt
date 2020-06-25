include(CheckFunctionExists)
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

add_executable (nespackager
NESPackager.cpp
)
add_executable (DumpPkgDir
DumpPkgDir.cpp
)
add_executable (DumpPkgPart
DumpPkgPart.cpp
)

target_link_libraries (nespackager PUBLIC DCL PRIVATE Threads::Threads)
install(TARGETS nespackager RUNTIME DESTINATION bin)
install(FILES NESPackager_Template/nespkgtemplate.nsof DESTINATION share/DCL/)

target_link_libraries (DumpPkgDir PUBLIC DCL PRIVATE Threads::Threads)
install(TARGETS DumpPkgDir RUNTIME DESTINATION bin)

target_link_libraries (DumpPkgPart PUBLIC DCL PRIVATE Threads::Threads)
install(TARGETS DumpPkgPart RUNTIME DESTINATION bin)
