// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "cDCL",
    platforms: [
        .macOS(.v10_13),
        .iOS(.v13),
    ],
    products: [
        .library(
            name: "K",
            targets: ["K"]
        ),
    ],
    targets: [
        .target(
            name: "K",
            path: ".",
            exclude: [
                "CMakeLists.txt",
                "COPYING",
                "DCL",
                "Documentation",
                "README.md",
                "Tests",
                "Tools",
                "cmake",
                "K/Misc/TNameServerSingleton.cpp",
                "K/Misc/RelocHack.a",
                "K/Tests",
                "K/XML",
            ],
            sources: [
                "K/Defines/KDefinitions.cpp",
                "K/Exceptions/TException.cpp",
                "K/Exceptions/Errors/TError.cpp",
                "K/Exceptions/Errors/TMemError.cpp",
                "K/Exceptions/IO/TEOFException.cpp",
                "K/Exceptions/IO/TIOException.cpp",
                "K/Misc/CRC32.cpp",
                "K/Misc/TCircleBuffer.cpp",
                "K/Misc/TMappedFile.cpp",
                "K/Misc/TRefCounter.cpp",
                "K/Misc/UBase64.cpp",
                "K/Streams/TFileStream.cpp",
                "K/Streams/TStream.cpp",
                "K/Crypto/UDES.cpp",
                "K/Crypto/UDESTables.cpp",
                "K/Math/TInt64.cpp",
                "K/Math/TSInt64.cpp",
                "K/Math/TUInt64.cpp",
                "K/Math/UTInt64.cpp",
                "K/Unicode/TString.cpp",
                "K/Unicode/TStringRef.cpp",
                "K/Unicode/UUTF16Conv.cpp",
                "K/Unicode/UUTF16CStr.cpp",
                "K/Threads/TCondVar.cpp",
                "K/Threads/TMutex.cpp",
                "K/Threads/TThread.cpp",
            ],
            publicHeadersPath: "."
        ),
    ],
    cxxLanguageStandard: .cxx20
)
