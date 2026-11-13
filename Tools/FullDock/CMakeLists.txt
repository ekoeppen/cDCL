if(NOT WIN32)
    add_executable (FullDock
        FullDock.cpp
    )

    target_link_libraries (FullDock PUBLIC DCL PRIVATE Threads::Threads)

    if(CDCL_INSTALL)
        install(TARGETS FullDock RUNTIME DESTINATION bin)
    endif()
endif()
