if(NOT BUILD_MOD_LIBRARIES AND NOT BUILD_MOD_QVMS)
    return()
endif()

include(utils/qvm_tools)
include(utils/pk3)
include(utils/set_output_dirs)
include(mod)

include(basegame_sources)

if(BUILD_MOD_LIBRARIES)
    set(CGAME_MODULE_BINARY ${CGAME_MODULE})

    set(CGAME_MODULE_BINARY_BASEGAME ${CGAME_MODULE_BINARY}_${BASEGAME})

    add_library(                ${CGAME_MODULE_BINARY_BASEGAME} SHARED ${CGAME_SOURCES_BASEGAME} ${CGAME_BINARY_SOURCES})
    target_compile_definitions( ${CGAME_MODULE_BINARY_BASEGAME}
        PRIVATE CGAME
        PRIVATE OSP_VERSION=${OSP_VERSION}
        PRIVATE OSP_CLIENT=${OSP_CLIENT}
    )
    target_link_libraries(      ${CGAME_MODULE_BINARY_BASEGAME} PRIVATE ${COMMON_LIBRARIES})
    set_target_properties(      ${CGAME_MODULE_BINARY_BASEGAME} PROPERTIES OUTPUT_NAME ${CGAME_MODULE_BINARY})
    set_output_dirs(            ${CGAME_MODULE_BINARY_BASEGAME} SUBDIRECTORY ${BASEGAME})

    set(GAME_MODULE_BINARY ${GAME_MODULE})

    set(GAME_MODULE_BINARY_BASEGAME ${GAME_MODULE_BINARY}_${BASEGAME})

    add_library(                ${GAME_MODULE_BINARY_BASEGAME} SHARED ${GAME_SOURCES} ${GAME_BINARY_SOURCES})
    target_compile_definitions( ${GAME_MODULE_BINARY_BASEGAME}
        PRIVATE OSP_VERSION=${OSP_VERSION}
        PRIVATE OSP_CLIENT=${OSP_CLIENT}
    )
    target_link_libraries(      ${GAME_MODULE_BINARY_BASEGAME} PRIVATE ${COMMON_LIBRARIES})
    # Legacy OSP game code is not warning-clean; keep warnings but do not fail on them.
    target_compile_options(     ${GAME_MODULE_BINARY_BASEGAME} PRIVATE
        $<$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>:-Wno-error>
        $<$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>:-Wno-error=implicit-function-declaration>)
    set_target_properties(      ${GAME_MODULE_BINARY_BASEGAME} PROPERTIES OUTPUT_NAME ${GAME_MODULE_BINARY})
    set_output_dirs(            ${GAME_MODULE_BINARY_BASEGAME} SUBDIRECTORY ${BASEGAME})

    set(UI_MODULE_BINARY ${UI_MODULE})

    set(UI_MODULE_BINARY_BASEGAME ${UI_MODULE_BINARY}_${BASEGAME})

    add_library(                ${UI_MODULE_BINARY_BASEGAME} SHARED ${UI_SOURCES} ${UI_BINARY_SOURCES})
    target_compile_definitions( ${UI_MODULE_BINARY_BASEGAME}
        PRIVATE Q3UI
    )
    target_link_libraries(      ${UI_MODULE_BINARY_BASEGAME} PRIVATE ${COMMON_LIBRARIES})
    # Legacy OSP ui code is not warning-clean; keep warnings but do not fail on them.
    target_compile_options(     ${UI_MODULE_BINARY_BASEGAME} PRIVATE
        $<$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>:-Wno-error>
        $<$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>:-Wno-error=implicit-function-declaration>)
    set_target_properties(      ${UI_MODULE_BINARY_BASEGAME} PROPERTIES OUTPUT_NAME ${UI_MODULE_BINARY})
    set_output_dirs(            ${UI_MODULE_BINARY_BASEGAME} SUBDIRECTORY ${BASEGAME})
endif()

if(BUILD_MOD_QVMS)
    set(CGAME_MODULE_QVM_BASEGAME ${CGAME_MODULE}qvm_${BASEGAME})

    string(REGEX REPLACE "[^A-Za-z0-9]" "_" CGAME_MODULE_QVM_BASEGAME ${CGAME_MODULE_QVM_BASEGAME})

    add_qvm(${CGAME_MODULE_QVM_BASEGAME}
        DEFINITIONS CGAME "OSP_VERSION=\"\\\"${OSP_VERSION}\\\"\"" "OSP_CLIENT=\"\\\"${OSP_CLIENT}\\\"\""
        OUTPUT_NAME ${CGAME_MODULE}
        OUTPUT_DIRECTORY ${BASEGAME}/vm
        SOURCES ${CGAME_SOURCES_BASEGAME} ${CGAME_QVM_SOURCES} ${CGAME_HEADERS})

    set(GAME_MODULE_QVM_BASEGAME ${GAME_MODULE}qvm_${BASEGAME})

    string(REGEX REPLACE "[^A-Za-z0-9]" "_" GAME_MODULE_QVM_BASEGAME ${GAME_MODULE_QVM_BASEGAME})

    add_qvm(${GAME_MODULE_QVM_BASEGAME}
        DEFINITIONS CGAME "OSP_VERSION=\"\\\"${OSP_VERSION}\\\"\""
        OUTPUT_NAME ${GAME_MODULE}
        OUTPUT_DIRECTORY ${BASEGAME}/vm
        SOURCES ${GAME_SOURCES} ${GAME_QVM_SOURCES} ${GAME_HEADERS})

    set(UI_MODULE_QVM_BASEGAME ${UI_MODULE}qvm_${BASEGAME})

    string(REGEX REPLACE "[^A-Za-z0-9]" "_" UI_MODULE_QVM_BASEGAME ${UI_MODULE_QVM_BASEGAME})

    add_qvm(${UI_MODULE_QVM_BASEGAME}
        DEFINITIONS Q3UI
        DEFINITIONS CGAME "OSP_VERSION=\"\\\"${OSP_VERSION}\\\"\""
        OUTPUT_NAME ${UI_MODULE}
        OUTPUT_DIRECTORY ${BASEGAME}/vm
        SOURCES ${UI_SOURCES} ${UI_QVM_SOURCES} ${UI_HEADERS})

    set(VM_DIR ${CMAKE_BINARY_DIR}/$<CONFIG>/${BASEGAME})
endif()

if(BUILD_MOD_PK3)
    add_pk3(
        ASSETS_DIR ${ASSETS_LOCATION}
        VM_DIR ${VM_DIR}
        VM_TARGETS
            ${CGAME_MODULE_QVM_BASEGAME}
            ${GAME_MODULE_QVM_BASEGAME}
            ${UI_MODULE_QVM_BASEGAME}
        OUTPUT_DIRECTORY ${BASEGAME}
        PAK_NAME ${OSP_PAKNAME}
    )

    if(BUILD_MOD_QVMS)
        add_pk3_files(
            PAK_NAME ${OSP_SERVER_PAKNAME}
            OUTPUT_DIRECTORY ${BASEGAME}
            WORKING_DIRECTORY ${VM_DIR}
            FILES_TO_PACK vm/${GAME_MODULE}.qvm
            DEPENDS
                ${GAME_MODULE_QVM_BASEGAME}
                ${VM_DIR}/vm/${GAME_MODULE}.qvm
                qvm_tools
        )
    endif()
endif()
