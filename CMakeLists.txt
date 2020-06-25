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

set(DCL_HEADERS
DCL/Communication_Layers/Common/TDCLSocket.h
DCL/Communication_Layers/MacOS_X/at_proto.h
DCL/Communication_Layers/MacOS_X/TDCLIOKitSerialPort.h
DCL/Communication_Layers/MacOS_X/TDCLRendezVous.h
DCL/Communication_Layers/MacOS_X/TDCLRendezVousOT.h
DCL/Communication_Layers/MacOS_X/TDCLXADSP.h
DCL/Communication_Layers/MacOS_X/TDCLXADSPSocket.h
DCL/Communication_Layers/POSIX/TDCLBSDSocket.h
DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h
DCL/Communication_Layers/TDCLBufferedPipe.h
DCL/Communication_Layers/TDCLCommLayer.h
DCL/Communication_Layers/TDCLMNPPipe.h
DCL/Communication_Layers/TDCLHammerPipe.h
DCL/Communication_Layers/TDCLPipe.h
DCL/Communication_Layers/TDCLPipeProxy.h
DCL/Communication_Layers/TDCLSyncCommLayer.h
DCL/Data/Picture/TDCLPicture.h
DCL/Data/Text/TDCLClParagraph.h
DCL/Data/Text/TDCLFontSpecification.h
DCL/Data/Text/TDCLRichText.h
DCL/Data/Text/TDCLStyleRef.h
DCL/Exceptions/Errors/TDCLBadParamError.h
DCL/Exceptions/Errors/TDCLBadStateError.h
DCL/Exceptions/Errors/TDCLLimitReachedError.h
DCL/Exceptions/Errors/TDCLMemError.h
DCL/Exceptions/Errors/TDCLNotImplementedError.h
DCL/Exceptions/Errors/TDCLUnknownError.h
DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h
DCL/Exceptions/IO_Exceptions/TDCLEOFException.h
DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h
DCL/Exceptions/IO_Exceptions/TDCLFullException.h
DCL/Exceptions/IO_Exceptions/TDCLIOException.h
DCL/Exceptions/IO_Exceptions/TDCLPermissionException.h
DCL/Exceptions/IO_Exceptions/TDCLPositionException.h
DCL/Exceptions/IO_Exceptions/TDCLReadOnlyException.h
DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h
DCL/Exceptions/Link_Exceptions/TDCLBadAppCmdException.h
DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h
DCL/Exceptions/Link_Exceptions/TDCLLinkException.h
DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h
DCL/Exceptions/TDCLClassCastException.h
DCL/Exceptions/TDCLException.h
DCL/Exceptions/TDCLNewtonException.h
DCL/Exceptions/TDCLNotAvailableException.h
DCL/Exceptions/TDCLNSException.h
DCL/Exceptions/TDCLPackageException.h
DCL/Exceptions/TDCLUserCancelledException.h
DCL/Exceptions/Thread_Exceptions/TDCLSemaphoreBusyException.h
DCL/Exceptions/Thread_Exceptions/TDCLThreadActiveException.h
DCL/Exceptions/Thread_Exceptions/TDCLThreadDeadLockException.h
DCL/Exceptions/Thread_Exceptions/TDCLThreadException.h
DCL/Exceptions/Thread_Exceptions/TDCLThreadStoppedException.h
DCL/Headers/DCLDebugMacOSPPCPrefix.h
DCL/Headers/DCLDebugOSXPrefix.h
DCL/Headers/DCLDefinitions.h
DCL/Headers/DCLErrorMessages.h
DCL/Headers/DCLFinalMacOSPPCPrefix.h
DCL/Headers/DCLFinalOSXPrefix.h
DCL/Headers/DCLMacErrorMessages.h
DCL/Headers/DCLStrings.h
DCL/Headers/PrecompiledHeader.h
DCL/Interfaces/Common/TDCLLogApplication.h
DCL/Interfaces/IDCLFiles.h
DCL/Interfaces/IDCLThreads.h
DCL/Interfaces/MacOS/TDCLMacDesktopFolder.h
DCL/Interfaces/MacOS/TDCLMacFile.h
DCL/Interfaces/MacOS/TDCLMacFiles.h
DCL/Interfaces/MacOS/TDCLMacFolder.h
DCL/Interfaces/MacOS/TDCLMacGetPutFiles.h
DCL/Interfaces/MacOS/TDCLMacOS7DesktopFolder.h
DCL/Interfaces/MacOS/TDCLMacOS7File.h
DCL/Interfaces/MacOS/TDCLMacOS7Folder.h
DCL/Interfaces/MacOS/TDCLMacThreads.h
DCL/Interfaces/MacOS/UDCLCTBConfig.h
DCL/Interfaces/MacOS/UDCLMacMachineName.h
DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonDesktopFolder.h
DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFile.h
DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFolder.h
DCL/Interfaces/MacOS_X/DCLCocoaApplication.h
DCL/Interfaces/MacOS_X/IDCLObjCApplication.h
DCL/Interfaces/MacOS_X/TDCLObjCApplication.h
DCL/Interfaces/MacOS_X/UDCLMacCarbonUtils.h
DCL/Interfaces/POSIX/Compat/libgen.h
DCL/Interfaces/POSIX/TDCLPOSIXDir.h
DCL/Interfaces/POSIX/TDCLPOSIXDirBase.h
DCL/Interfaces/POSIX/TDCLPOSIXFile.h
DCL/Interfaces/POSIX/TDCLPOSIXFiles.h
DCL/Interfaces/POSIX/TDCLPOSIXRootDir.h
DCL/Interfaces/POSIX/TDCLPThreads.h
DCL/Interfaces/TDCLApplication.h
DCL/Interfaces/TDCLEvent.h
DCL/Interfaces/TDCLFile.h
DCL/Interfaces/TDCLFolder.h
DCL/Interfaces/TDCLFSItem.h
DCL/Interfaces/TDCLFSItemRef.h
DCL/Interfaces/TDCLRootFolder.h
DCL/Interfaces/TDCLThread.h
DCL/Interfaces/UDCLTime.h
DCL/Link/App_Commands/TDCLAppCmdFile.h
DCL/Link/App_Commands/TDCLAppCmdKbdChar.h
DCL/Link/App_Commands/TDCLAppCmdString.h
DCL/Link/App_Commands/TDCLAppCommand.h
DCL/Link/Dock_Commands/TDCLDockCmdArray.h
DCL/Link/Dock_Commands/TDCLDockCmdBackupIDs.h
DCL/Link/Dock_Commands/TDCLDockCmdCString.h
DCL/Link/Dock_Commands/TDCLDockCmdDesktopInfo.h
DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h
DCL/Link/Dock_Commands/TDCLDockCmdLoadPkg.h
DCL/Link/Dock_Commands/TDCLDockCmdNewtonInfo.h
DCL/Link/Dock_Commands/TDCLDockCmdNewtonName.h
DCL/Link/Dock_Commands/TDCLDockCmdNoData.h
DCL/Link/Dock_Commands/TDCLDockCmdNSObject.h
DCL/Link/Dock_Commands/TDCLDockCmdPackageList.h
DCL/Link/Dock_Commands/TDCLDockCmdPassword.h
DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h
DCL/Link/Dock_Commands/TDCLDockCmdTwoNSObjects.h
DCL/Link/Dock_Commands/TDCLDockCommand.h
DCL/Link/Inspector_Commands/TDCLInspectorCommand.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdDeletePkg.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdEnterBreak.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdException.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdExceptionRef.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdExitBreak.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdFuncObj.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdLoadPkg.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdResult.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdStackFrame.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdText.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdCode.h
DCL/Link/Inspector_Commands/TDCLInspectorCmdCodeBlock.h
DCL/Link/Link_Engines/TDCLBrowseEngine.h
DCL/Link/Link_Engines/TDCLFileImportEngine.h
DCL/Link/Link_Engines/TDCLKeyboardEngine.h
DCL/Link/Link_Engines/TDCLLinkEngine.h
DCL/Link/Link_Engines/TDCLLoadPkgEngine.h
DCL/Link/Link_Engines/TDCLSyncEngine.h
DCL/Link/Link_Engines/TDCLSyncSourceEngine.h
DCL/Link/TDCLDockLink.h
DCL/Link/TDCLFullDockLink.h
DCL/Link/TDCLLink.h
DCL/Link/TDCLLoadPackageLink.h
DCL/Link/TDCLInspectorLink.h
DCL/NS_Objects/Exchange/KDCLNSOF.h
DCL/NS_Objects/Exchange/KDCLPkg.h
DCL/NS_Objects/Exchange/TDCLNSCodec.h
DCL/NS_Objects/Exchange/TDCLNSDecoder.h
DCL/NS_Objects/Exchange/TDCLNSEncoder.h
DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h
DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h
DCL/NS_Objects/Exchange/TDCLPkgDecoder.h
DCL/NS_Objects/Exchange/TDCLPkgEncoder.h
DCL/NS_Objects/Exchange/TDCLTextDecoder.h
DCL/NS_Objects/Exchange/TDCLTextEncoder.h
DCL/NS_Objects/Exchange/TDCLXMLDecoder.h
DCL/NS_Objects/Exchange/TDCLXMLEncoder.h
DCL/NS_Objects/KDCLNSErrors.h
DCL/NS_Objects/Objects/IStreamable.h
DCL/NS_Objects/Objects/KDCLSYM.h
DCL/NS_Objects/Objects/TDCLNSArray.h
DCL/NS_Objects/Objects/TDCLNSBinary.h
DCL/NS_Objects/Objects/TDCLNSFrame.h
DCL/NS_Objects/Objects/TDCLNSFunction.h
DCL/NS_Objects/Objects/TDCLNSObject.h
DCL/NS_Objects/Objects/TDCLNSRef.h
DCL/NS_Objects/Objects/TDCLNSString.h
DCL/NS_Objects/Objects/TDCLNSSymbol.h
DCL/NS_Objects/Objects/TDCLNSWeakRef.h
DCL/NS_Objects/Views/ObjC/DCLNSRef.h
DCL/Package/KDCLPackageErrors.h
DCL/Package/TDCLPackage.h
DCL/Package/TDCLPkgNOSPart.h
DCL/Package/TDCLPkgPart.h
DCL/Package/TDCLPkgRelocatablePart.h
DCL/Package/UDCLPaperback.h
DCL/Server/TDCLOneLinkServer.h
DCL/Server/TDCLServer.h
DCL/Server/TDCLSimpleServer.h
DCL/Streams/TDCLMemStream.h
DCL/Streams/TDCLRandomAccessStream.h
DCL/Streams/TDCLStdStream.h
DCL/Streams/TDCLStream.h
K/Crypto/U3DES.h
K/Crypto/UDES.h
K/Defines/KDefinitions.h
K/Defines/UByteSex.h
K/Exceptions/Errors/TError.h
K/Exceptions/Errors/TMemError.h
K/Exceptions/IO/TEOFException.h
K/Exceptions/IO/TIOException.h
K/Exceptions/TException.h
K/Math/TInt64.h
K/Math/TSInt64.h
K/Math/TUInt64.h
K/Math/UTInt64.h
K/Misc/RelocHack.h
K/Misc/TCircleBuffer.h
K/Misc/TDoubleLinkedList.h
K/Misc/TMappedFile.h
K/Misc/TNameServerSingleton.h
K/Misc/TRefCounter.h
K/Misc/TSmartPtr.h
K/Misc/UBase64.h
K/Misc/UPStrings.h
K/Tests/KDebug.h
K/Tests/TFunctionMonitor.h
K/Tests/TMonitorMethodClient.h
K/Tests/TNewtonTraceMonitor.h
K/Tests/TTraceMonitor.h
K/Threads/TCondVar.h
K/Threads/TMutex.h
K/Threads/TThread.h
K/Unicode/TMutableStringRef.h
K/Unicode/TString.h
K/Unicode/TStringRef.h
K/Unicode/UnicodeDefinitions.h
K/Unicode/UUTF16Conv.h
K/Unicode/UUTF16CStr.h
K/XML/UExpatUtils.h
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory (Sample_Code/ToolchainUtils)
add_subdirectory (Sample_Code/NSOFtoXML)
add_subdirectory (Sample_Code/PackageUtils)
add_subdirectory (Tests)

foreach(HEADER ${HEADERS})
   string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
   install(FILES ${HEADER} DESTINATION include/${DIR})
endforeach(HEADER ${HEADERS})

install(TARGETS DCL LIBRARY DESTINATION lib)
