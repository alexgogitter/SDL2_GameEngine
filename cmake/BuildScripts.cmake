# Configure first so newly added or removed script .cpp files are visible to MSBuild.
execute_process(COMMAND "${CMAKE_COMMAND}" -S "${SOURCE_DIR}" -B "${BUILD_DIR}" RESULT_VARIABLE RESULT)
if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "User script configuration failed (${RESULT})")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}" --config "${CONFIG}" --target UserScripts --parallel RESULT_VARIABLE RESULT)
if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "User script compilation failed (${RESULT})")
endif()
