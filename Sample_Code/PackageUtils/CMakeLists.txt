include(CheckFunctionExists)
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

add_executable (nespackager
NESPackager.cpp
)

target_link_libraries (nespackager LINK_PUBLIC DCL)
install(TARGETS nespackager RUNTIME DESTINATION bin)
install(FILES NESPackager_Template/nespkgtemplate.nsof DESTINATION share/DCL/)
