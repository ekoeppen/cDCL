if(UNIX)
    add_executable(SoupDownloader SoupDownloader.cpp SoupDownloadSession.cpp)
    target_link_libraries(SoupDownloader PRIVATE cDCL::DCL Threads::Threads)
    install(TARGETS SoupDownloader RUNTIME DESTINATION bin)

    if(BUILD_TESTING)
        add_executable(SoupDownloaderTests SoupDownloaderTests.cpp SoupDownloadSession.cpp)
        target_link_libraries(SoupDownloaderTests PRIVATE cDCL::DCL)
        add_test(NAME soup-downloader COMMAND SoupDownloaderTests)

        find_package(Python3 COMPONENTS Interpreter QUIET)
        if(Python3_Interpreter_FOUND)
            add_test(NAME soup-downloader-pty
                COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_pty.py
                        $<TARGET_FILE:SoupDownloader> $<TARGET_FILE:SoupDownloaderTests>)
            set_tests_properties(soup-downloader-pty PROPERTIES TIMEOUT 45)
        endif()
    endif()
endif()
