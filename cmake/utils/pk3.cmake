include_guard(GLOBAL)

if(NOT DEFINED PK3_ARCHIVE)
  if(WIN32)
    set(PK3_ARCHIVE ${CMAKE_SOURCE_DIR}/tools/7za.exe a -tzip)
  elseif(UNIX OR LINUX OR APPLE)
    set(PK3_ARCHIVE 7z a -tzip)
  else()
    message(FATAL_ERROR
      "ZIP compression program was not found for current system. Consider passing -DPK3_ARCHIVE to the Cmake with appropriate command to create .zip file."
    )
  endif()
endif()

message(STATUS
  "Using zip command: ${PK3_ARCHIVE} <args>."
)

function(add_pk3)
  cmake_parse_arguments(ARG "" "" "ASSETS_DIR;VM_DIR;VM_TARGETS;OUTPUT_DIRECTORY;PAK_NAME" ${ARGV})

  set(PK3_OUTPUT_DIR ${CMAKE_BINARY_DIR}/$<CONFIG>)

  if(ARG_OUTPUT_DIRECTORY)
    set(PK3_OUTPUT_DIR ${PK3_OUTPUT_DIR}/${ARG_OUTPUT_DIRECTORY})
  endif()

  add_custom_command(
    OUTPUT ${PK3_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PK3_OUTPUT_DIR}
  )

  set(PK3_OUTPUT_FILE "${PK3_OUTPUT_DIR}/${ARG_PAK_NAME}")

  if(ARG_VM_DIR)
    set(PK3_FILES_TO_PACK ${ARG_ASSETS_DIR}/* ./vm/)

    set(PK3_DEPENDS ${ARG_VM_TARGETS} qvm_tools)
  else()
    set(PK3_FILES_TO_PACK ${ARG_ASSETS_DIR}/*)

    set(PK3_DEPENDS "")
  endif()

  add_custom_command(
    OUTPUT ${PK3_OUTPUT_FILE}
    COMMAND
      ${PK3_ARCHIVE} ${PK3_OUTPUT_FILE} ${PK3_FILES_TO_PACK}
    COMMENT "Packing ${PK3_OUTPUT_FILE}"
    DEPENDS ${PK3_DEPENDS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/${ARG_VM_DIR}
  )

  add_custom_target(${ARG_PAK_NAME} ALL DEPENDS ${PK3_OUTPUT_FILE})

  install(FILES ${PK3_OUTPUT_FILE} DESTINATION ${PK3_OUTPUT_DIR})
endfunction()