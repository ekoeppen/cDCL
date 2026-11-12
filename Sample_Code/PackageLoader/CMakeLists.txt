add_executable (PackageLoader
    PackageLoader.cpp
    TPackageLoaderApp.cpp
    TPackageLoaderLink.cpp
    TDCLClientSocket.cpp
)

target_link_libraries (PackageLoader PUBLIC DCL PRIVATE Threads::Threads)
if(WIN32)
    target_link_libraries(PackageLoader PRIVATE ws2_32)
endif()

install(TARGETS PackageLoader RUNTIME DESTINATION bin)
