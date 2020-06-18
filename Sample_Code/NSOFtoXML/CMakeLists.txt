include(CheckFunctionExists)
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

add_executable (NSOFtoXML
NSOFtoXML.cpp
)

target_link_libraries (NSOFtoXML LINK_PUBLIC DCL pthread)
install(TARGETS NSOFtoXML RUNTIME DESTINATION bin)
