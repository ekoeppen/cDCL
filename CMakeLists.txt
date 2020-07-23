cmake_minimum_required (VERSION 3.1)
project (dcl)

set(CMAKE_CXX_STANDARD 11)

# build type -- https://blog.kitware.com/cmake-and-the-default-build-type/
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

add_definitions (-DTARGET_OS_POSIX -DHAS_C99_LONGLONG -DTARGET_RT_BIG_ENDIAN=0 -DTARGET_RT_LITTLE_ENDIAN=1 -Wno-multichar)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNDEBUG)
endif()

include_directories (${CMAKE_SOURCE_DIR}/K ${CMAKE_SOURCE_DIR}/DCL ${CMAKE_SOURCE_DIR})
include(CTest)

SET(CMAKE_MACOSX_RPATH 1)
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

add_library (DCL SHARED
DCL/Communication_Layers/TDCLBufferedPipe.cpp
DCL/Communication_Layers/TDCLCommLayer.cpp
DCL/Communication_Layers/TDCLMNPPipe.cpp
DCL/Communication_Layers/TDCLHammerPipe.cpp
DCL/Communication_Layers/TDCLPipe.cpp
DCL/Communication_Layers/TDCLPipeProxy.cpp
DCL/Communication_Layers/TDCLSyncCommLayer.cpp
DCL/Data/Text/TDCLStyleRef.cpp
DCL/Data/Text/TDCLFontSpecification.cpp
DCL/Data/Text/TDCLClParagraph.cpp
DCL/Exceptions/Errors/TDCLBadParamError.cpp
DCL/Exceptions/Errors/TDCLBadStateError.cpp
DCL/Exceptions/Errors/TDCLLimitReachedError.cpp
DCL/Exceptions/Errors/TDCLMemError.cpp
DCL/Exceptions/Errors/TDCLNotImplementedError.cpp
DCL/Exceptions/Errors/TDCLUnknownError.cpp
DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.cpp
DCL/Exceptions/IO_Exceptions/TDCLEOFException.cpp
DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.cpp
DCL/Exceptions/IO_Exceptions/TDCLFullException.cpp
DCL/Exceptions/IO_Exceptions/TDCLIOException.cpp
DCL/Exceptions/IO_Exceptions/TDCLPermissionException.cpp
DCL/Exceptions/IO_Exceptions/TDCLPositionException.cpp
DCL/Exceptions/IO_Exceptions/TDCLReadOnlyException.cpp
DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.cpp
DCL/Exceptions/Link_Exceptions/TDCLBadAppCmdException.cpp
DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.cpp
DCL/Exceptions/Link_Exceptions/TDCLLinkException.cpp
DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.cpp
DCL/Exceptions/TDCLClassCastException.cpp
DCL/Exceptions/TDCLException.cpp
DCL/Exceptions/TDCLNewtonException.cpp
DCL/Exceptions/TDCLNotAvailableException.cpp
DCL/Exceptions/TDCLNSException.cpp
DCL/Exceptions/TDCLPackageException.cpp
DCL/Exceptions/TDCLUserCancelledException.cpp
DCL/Exceptions/Thread_Exceptions/TDCLSemaphoreBusyException.cpp
DCL/Exceptions/Thread_Exceptions/TDCLThreadActiveException.cpp
DCL/Exceptions/Thread_Exceptions/TDCLThreadDeadLockException.cpp
DCL/Exceptions/Thread_Exceptions/TDCLThreadException.cpp
DCL/Exceptions/Thread_Exceptions/TDCLThreadStoppedException.cpp
DCL/Interfaces/Common/TDCLLogApplication.cpp
DCL/Interfaces/TDCLApplication.cpp
DCL/Interfaces/TDCLEvent.cpp
DCL/Interfaces/TDCLFile.cpp
DCL/Interfaces/TDCLFolder.cpp
DCL/Interfaces/TDCLFSItem.cpp
DCL/Interfaces/TDCLFSItemRef.cpp
DCL/Interfaces/TDCLRootFolder.cpp
DCL/Interfaces/TDCLThread.cpp
DCL/Interfaces/UDCLTime.cpp
DCL/Link/App_Commands/TDCLAppCmdFile.cpp
DCL/Link/App_Commands/TDCLAppCmdKbdChar.cpp
DCL/Link/App_Commands/TDCLAppCmdString.cpp
DCL/Link/App_Commands/TDCLAppCommand.cpp
DCL/Link/Dock_Commands/TDCLDockCmdArray.cpp
DCL/Link/Dock_Commands/TDCLDockCmdCString.cpp
DCL/Link/Dock_Commands/TDCLDockCmdDesktopInfo.cpp
DCL/Link/Dock_Commands/TDCLDockCmdGeneric.cpp
DCL/Link/Dock_Commands/TDCLDockCmdLoadPkg.cpp
DCL/Link/Dock_Commands/TDCLDockCmdNewtonInfo.cpp
DCL/Link/Dock_Commands/TDCLDockCmdNewtonName.cpp
DCL/Link/Dock_Commands/TDCLDockCmdNoData.cpp
DCL/Link/Dock_Commands/TDCLDockCmdNSObject.cpp
DCL/Link/Dock_Commands/TDCLDockCmdPackageList.cpp
DCL/Link/Dock_Commands/TDCLDockCmdPassword.cpp
DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.cpp
DCL/Link/Dock_Commands/TDCLDockCmdTwoNSObjects.cpp
DCL/Link/Dock_Commands/TDCLDockCommand.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCommand.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdDeletePkg.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdEnterBreak.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdException.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdExceptionRef.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdExitBreak.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdFuncObj.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdLoadPkg.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdResult.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdStackFrame.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdText.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdCode.cpp
DCL/Link/Inspector_Commands/TDCLInspectorCmdCodeBlock.cpp
DCL/Link/Link_Engines/TDCLBrowseEngine.cpp
DCL/Link/Link_Engines/TDCLFileImportEngine.cpp
DCL/Link/Link_Engines/TDCLKeyboardEngine.cpp
DCL/Link/Link_Engines/TDCLLinkEngine.cpp
DCL/Link/Link_Engines/TDCLLoadPkgEngine.cpp
DCL/Link/Link_Engines/TDCLSyncEngine.cpp
DCL/Link/TDCLDockLink.cpp
DCL/Link/TDCLFullDockLink.cpp
DCL/Link/TDCLLink.cpp
DCL/Link/TDCLLoadPackageLink.cpp
DCL/Link/TDCLInspectorLink.cpp
DCL/NS_Objects/Exchange/TDCLNSCodec.cpp
DCL/NS_Objects/Exchange/TDCLNSDecoder.cpp
DCL/NS_Objects/Exchange/TDCLNSEncoder.cpp
DCL/NS_Objects/Exchange/TDCLNSOFDecoder.cpp
DCL/NS_Objects/Exchange/TDCLNSOFEncoder.cpp
DCL/NS_Objects/Exchange/TDCLPkgDecoder.cpp
DCL/NS_Objects/Exchange/TDCLPkgEncoder.cpp
DCL/NS_Objects/Exchange/TDCLTextDecoder.cpp
DCL/NS_Objects/Exchange/TDCLTextEncoder.cpp
DCL/NS_Objects/Exchange/TDCLXMLDecoder.cpp
DCL/NS_Objects/Exchange/TDCLXMLEncoder.cpp
DCL/NS_Objects/Objects/KDCLSYM.cpp
DCL/NS_Objects/Objects/TDCLNSArray.cpp
DCL/NS_Objects/Objects/TDCLNSBinary.cpp
DCL/NS_Objects/Objects/TDCLNSFrame.cpp
DCL/NS_Objects/Objects/TDCLNSFunction.cpp
DCL/NS_Objects/Objects/TDCLNSObject.cpp
DCL/NS_Objects/Objects/TDCLNSRef.cpp
DCL/NS_Objects/Objects/TDCLNSString.cpp
DCL/NS_Objects/Objects/TDCLNSSymbol.cpp
DCL/NS_Objects/Objects/TDCLNSWeakRef.cpp
DCL/Package/TDCLPackage.cpp
DCL/Package/TDCLPkgNOSPart.cpp
DCL/Package/TDCLPkgPart.cpp
DCL/Package/TDCLPkgRelocatablePart.cpp
DCL/Package/UDCLPaperback.cpp
DCL/Server/TDCLOneLinkServer.cpp
DCL/Server/TDCLServer.cpp
DCL/Server/TDCLSimpleServer.cpp
DCL/Streams/TDCLMemStream.cpp
DCL/Streams/TDCLStdStream.cpp
DCL/Streams/TDCLStream.cpp
DCL/Communication_Layers/Common/TDCLSocket.cpp
DCL/Communication_Layers/POSIX/TDCLBSDSocket.cpp
DCL/Communication_Layers/POSIX/TDCLFDSerialPort.cpp
DCL/Interfaces/POSIX/TDCLPOSIXDir.cpp
DCL/Interfaces/POSIX/TDCLPOSIXDirBase.cpp
DCL/Interfaces/POSIX/TDCLPOSIXFile.cpp
DCL/Interfaces/POSIX/TDCLPOSIXFiles.cpp
DCL/Interfaces/POSIX/TDCLPOSIXRootDir.cpp
DCL/Interfaces/POSIX/TDCLPThreads.cpp
K/Crypto/UDES.cpp
K/Crypto/UDESTables.cpp
K/Math/TInt64.cpp
K/Math/TSInt64.cpp
K/Math/TUInt64.cpp
K/Math/UTInt64.cpp
K/Misc/UBase64.cpp
K/Misc/TRefCounter.cpp
K/Unicode/TString.cpp
K/Unicode/TStringRef.cpp
K/Unicode/UUTF16Conv.cpp
K/Unicode/UUTF16CStr.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory (Sample_Code/ToolchainUtils)
add_subdirectory (Sample_Code/NSOFtoXML)
add_subdirectory (Sample_Code/PackageUtils)
add_subdirectory (Tests)

install(TARGETS DCL LIBRARY DESTINATION lib)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/" DESTINATION "include" FILES_MATCHING PATTERN "*.h")
