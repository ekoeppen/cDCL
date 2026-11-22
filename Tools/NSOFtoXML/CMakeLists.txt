include(CheckFunctionExists)
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

add_executable (NSOFtoXML
NSOFtoXML.cpp
)

target_link_libraries (NSOFtoXML PUBLIC DCL)

if(WIN32)
    target_link_options(NSOFtoXML PRIVATE -static)
endif()

if(CDCL_INSTALL)
    install(TARGETS NSOFtoXML RUNTIME DESTINATION bin COMPONENT tools)
endif()
