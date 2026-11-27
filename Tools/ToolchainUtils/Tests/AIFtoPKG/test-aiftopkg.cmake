set(TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/aiftopkg-test")
file(MAKE_DIRECTORY "${TMP_DIR}")

set(OUT_PKG "${TMP_DIR}/out.pkg")
set(OUT_RAW_PKG "${TMP_DIR}/out_raw.pkg")
set(REX_SRC "${TMP_DIR}/out.rex.src")
set(OUT_REX "${TMP_DIR}/out.rex")

file(REMOVE "${OUT_PKG}" "${OUT_RAW_PKG}" "${REX_SRC}" "${OUT_REX}")

# Test -protocol with flags and -notify (including high-bit byte to verify unsigned shift)
execute_process(
    COMMAND "${AIFTOPKG}" -o "${OUT_PKG}" TestPkg -protocol "${SAMPLE_AIF}" -autoLoad -autoRemove -autoCopy -notify "prnt" handler
    RESULT_VARIABLE RES
    OUTPUT_VARIABLE OUT
    ERROR_VARIABLE ERR
)
if(NOT RES EQUAL 0)
    message(FATAL_ERROR "AIFtoPKG -protocol failed (${RES}): ${ERR}")
endif()

execute_process(
    COMMAND "${DUMPPKGDIR}" "${OUT_PKG}"
    RESULT_VARIABLE RES
    OUTPUT_VARIABLE DUMP_OUT
    ERROR_VARIABLE DUMP_ERR
)
if(NOT RES EQUAL 0)
    message(FATAL_ERROR "DumpPkgDir failed (${RES}): ${DUMP_ERR}")
endif()
string(FIND "${DUMP_OUT}" "Offset count: 3" HAS_RELOCS)
if(HAS_RELOCS EQUAL -1)
    message(FATAL_ERROR "Expected 3 relocations in DumpPkgDir output, got:\n${DUMP_OUT}")
endif()

# Test -raw
execute_process(
    COMMAND "${AIFTOPKG}" -o "${OUT_RAW_PKG}" RawPkg -raw "${SAMPLE_AIF}"
    RESULT_VARIABLE RES
    OUTPUT_VARIABLE OUT
    ERROR_VARIABLE ERR
)
if(NOT RES EQUAL 0)
    message(FATAL_ERROR "AIFtoPKG -raw failed (${RES}): ${ERR}")
endif()

# Test Rex integration on the generated package
file(WRITE "${REX_SRC}" "id 2\nversion 1\nmanufacturer 'Eins'\nstart 0x00800000\npackage \"${OUT_PKG}\"\n")
execute_process(
    COMMAND "${REX}" -o "${OUT_REX}" "${REX_SRC}"
    RESULT_VARIABLE RES
    OUTPUT_VARIABLE OUT
    ERROR_VARIABLE ERR
)
if(NOT RES EQUAL 0)
    message(FATAL_ERROR "Rex failed (${RES}): ${ERR}")
endif()

# Verify invalid CLI usage returns non-zero without crashing
execute_process(
    COMMAND "${AIFTOPKG}" -badflag
    RESULT_VARIABLE RES
    OUTPUT_QUIET
    ERROR_QUIET
)
if(RES EQUAL 0)
    message(FATAL_ERROR "Expected non-zero exit status for invalid arguments")
endif()

file(REMOVE_RECURSE "${TMP_DIR}")
