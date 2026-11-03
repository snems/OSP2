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

# Silence 7-Zip's multi-line progress output so parallel pak steps do not
# interleave their logs; each pak still prints its own "Packing <file>" line.
# Errors keep flowing to stderr and a failed pack still returns non-zero.
if(NOT DEFINED PK3_ARCHIVE_QUIET)
  set(PK3_ARCHIVE_QUIET -bso0 -bsp0 -bse2)
endif()

message(STATUS
  "Using zip command: ${PK3_ARCHIVE} ${PK3_ARCHIVE_QUIET} <args>."
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

  set(PK3_OUTPUT_FILE ${PK3_OUTPUT_DIR}/${ARG_PAK_NAME})

  if(ARG_VM_DIR)
    set(PK3_FILES_TO_PACK ${ARG_ASSETS_DIR}/* ./vm/)

    set(PK3_DEPENDS
      ${ARG_VM_DIR}/vm/cgame.qvm
      ${ARG_VM_TARGETS}
      qvm_tools)
  else()
    set(PK3_FILES_TO_PACK ${ARG_ASSETS_DIR}/*)

    set(PK3_DEPENDS "")
  endif()

  # We really want to ensure that pk3 always gets latest version of assets. As CMake
  # is unable to track folder contents change, we have to make pk3 every build.
  set(DUMMY_DEPENDENCY ${PK3_OUTPUT_DIR}/dummy.txt)

  add_custom_command(
    OUTPUT ${PK3_OUTPUT_FILE} ${DUMMY_DEPENDENCY}
    COMMAND
      ${PK3_ARCHIVE} ${PK3_ARCHIVE_QUIET} ${PK3_OUTPUT_FILE} ${PK3_FILES_TO_PACK}
    COMMENT "Packing ${PK3_OUTPUT_FILE}"
    DEPENDS ${PK3_DEPENDS}
    WORKING_DIRECTORY ${ARG_VM_DIR}
  )

  add_custom_target(${ARG_PAK_NAME} ALL DEPENDS ${PK3_OUTPUT_FILE} ${DUMMY_DEPENDENCY})

  install(FILES ${PK3_OUTPUT_FILE} DESTINATION ${PK3_OUTPUT_DIR})
endfunction()

# Pack an explicit list of files into a pak. Unlike add_pk3() this does not
# pull in the whole assets tree; it is used for server-side paks that ship only
# selected vm files (e.g. zz-osp-server3b.pk3 with qagame.qvm).
function(add_pk3_files)
  cmake_parse_arguments(ARG ""
    "PAK_NAME;OUTPUT_DIRECTORY;WORKING_DIRECTORY"
    "FILES_TO_PACK;DEPENDS" ${ARGV})

  set(PK3_OUTPUT_DIR ${CMAKE_BINARY_DIR}/$<CONFIG>)

  if(ARG_OUTPUT_DIRECTORY)
    set(PK3_OUTPUT_DIR ${PK3_OUTPUT_DIR}/${ARG_OUTPUT_DIRECTORY})
  endif()

  add_custom_command(
    OUTPUT ${PK3_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PK3_OUTPUT_DIR}
  )

  set(PK3_OUTPUT_FILE ${PK3_OUTPUT_DIR}/${ARG_PAK_NAME})

  # Always repack, mirroring add_pk3: the packed files may be regenerated
  # without CMake noticing. The dummy name is per-pak to avoid clashes.
  set(DUMMY_DEPENDENCY ${PK3_OUTPUT_DIR}/dummy-${ARG_PAK_NAME}.txt)

  add_custom_command(
    OUTPUT ${PK3_OUTPUT_FILE} ${DUMMY_DEPENDENCY}
    COMMAND
      ${PK3_ARCHIVE} ${PK3_ARCHIVE_QUIET} ${PK3_OUTPUT_FILE} ${ARG_FILES_TO_PACK}
    COMMENT "Packing ${PK3_OUTPUT_FILE}"
    DEPENDS ${ARG_DEPENDS}
    WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
  )

  add_custom_target(${ARG_PAK_NAME} ALL DEPENDS ${PK3_OUTPUT_FILE} ${DUMMY_DEPENDENCY})

  install(FILES ${PK3_OUTPUT_FILE} DESTINATION ${PK3_OUTPUT_DIR})
endfunction()