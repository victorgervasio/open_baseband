if(DATASET_LOG_DIR)
    # explicit log directory supplied by the user (root Makefile)
    set(LATEST_LOG_DIR "${DATASET_LOG_DIR}")
else()
    # no directory supplied: automatically select the latest one
    file(
        GLOB LOG_DIRS
        LIST_DIRECTORIES true
        "${SIM_OUTPUT_DIR}/log/*"
    )

    if(NOT LOG_DIRS)
        message(FATAL_ERROR
            "No simulation log directories found in:\n"
            "  ${SIM_OUTPUT_DIR}/log"
        )
    endif()

    list(SORT LOG_DIRS)

    list(LENGTH LOG_DIRS LOG_COUNT)
    math(EXPR LAST_INDEX "${LOG_COUNT} - 1")
    list(GET LOG_DIRS ${LAST_INDEX} LATEST_LOG_DIR)
endif()

set(LATEST_TERM_LOG
    "${LATEST_LOG_DIR}/term.log"
)

if(NOT EXISTS "${LATEST_TERM_LOG}")
    message(FATAL_ERROR
        "Latest simulation directory does not contain term.log:\n"
        "  ${LATEST_LOG_DIR}"
    )
endif()

# Dataset is stored inside the simulation run directory.
set(DATASET_OUTPUT_DIR
    "${LATEST_LOG_DIR}/dataset"
)

message(STATUS "Latest simulation log:")
message(STATUS "  ${LATEST_TERM_LOG}")

message(STATUS "Dataset output directory:")
message(STATUS "  ${DATASET_OUTPUT_DIR}")

execute_process(
    COMMAND
        "${PYTHON_EXECUTABLE}"
        "${GEN_DATASET_PY}"
        "${LATEST_TERM_LOG}"
        "--output-dir"
        "${DATASET_OUTPUT_DIR}"
    RESULT_VARIABLE RESULT
)

if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR
        "gen_dataset.py failed with return code ${RESULT}"
    )
endif()

message(STATUS "Dataset generated successfully.")
