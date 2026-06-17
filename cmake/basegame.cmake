if(NOT BUILD_MOD_LIBRARIES AND NOT BUILD_MOD_QVMS)
    return()
endif()

include(utils/qvm_tools)
include(utils/pk3)
include(utils/set_output_dirs)

set(CGAME_SOURCES
    ${SOURCE_DIR}/cgame/cg_main.c
    ${SOURCE_DIR}/cgame/cg_cvardescriptions.c
    ${SOURCE_DIR}/cgame/cg_crosshair.c
    ${SOURCE_DIR}/cgame/cg_chatfilter.c
    ${SOURCE_DIR}/cgame/cg_consolecmds.c
    ${SOURCE_DIR}/cgame/cg_customloc.c
    ${SOURCE_DIR}/cgame/cg_draw.c
    ${SOURCE_DIR}/cgame/cg_drawtools.c
    ${SOURCE_DIR}/cgame/cg_effects.c
    ${SOURCE_DIR}/cgame/cg_ents.c
    ${SOURCE_DIR}/cgame/cg_event.c
    ${SOURCE_DIR}/cgame/cg_emoji.c
    ${SOURCE_DIR}/cgame/cg_info.c
    ${SOURCE_DIR}/cgame/cg_localents.c
    ${SOURCE_DIR}/cgame/cg_localevents.c
    ${SOURCE_DIR}/cgame/cg_marks.c
    ${SOURCE_DIR}/cgame/cg_players.c
    ${SOURCE_DIR}/cgame/cg_playerstate.c
    ${SOURCE_DIR}/cgame/cg_playerscolors.c
    ${SOURCE_DIR}/cgame/cg_predict.c
    ${SOURCE_DIR}/cgame/cg_scoreboard.c
    ${SOURCE_DIR}/cgame/cg_servercmds.c
    ${SOURCE_DIR}/cgame/cg_snapshot.c
    ${SOURCE_DIR}/cgame/cg_view.c
    ${SOURCE_DIR}/cgame/cg_weapons.c
    ${SOURCE_DIR}/cgame/cg_ospconfig.c
    ${SOURCE_DIR}/cgame/cg_osphud.c
    ${SOURCE_DIR}/cgame/cg_osputil.c
    ${SOURCE_DIR}/cgame/cg_unlagged.c
    ${SOURCE_DIR}/cgame/cg_superhud.c
    ${SOURCE_DIR}/cgame/cg_superhud_configparser.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_ammomessage.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_attacker_icon.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_attacker_name.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_chat.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_flagstatus.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_followmessage.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_fps.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_fragmessage.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_gametime.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_gametype.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_itempickup.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_itempickupicon.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_localtime.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_location.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_name.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_ng.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_ngp.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_obituaries.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_powerup.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_pred.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_rankmessage.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_reward.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_sbab.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_sbac.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_sbai.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_sbamb.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_sbamc.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_sbami.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_sbhb.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_sbhc.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_sbhi.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_score.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_specmessage.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_speed.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_target_name.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_target_status.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_team.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_teamcount.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_vmw.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_warmupinfo.c
    ${SOURCE_DIR}/cgame/cg_superhud_element_weaponlist.c
    ${SOURCE_DIR}/cgame/cg_superhud_private.c
    ${SOURCE_DIR}/cgame/cg_superhud_util.c
    ${SOURCE_DIR}/game/bg_lib.c
    ${SOURCE_DIR}/game/bg_misc.c
    ${SOURCE_DIR}/game/bg_pmove.c
    ${SOURCE_DIR}/game/bg_slidemove.c
)

# Those are tracked only, causing the whole project to recompile
# on content change
set(CGAME_HEADERS
    ${SOURCE_DIR}/cgame/cg_local.h
    ${SOURCE_DIR}/cgame/cg_public.h
    ${SOURCE_DIR}/cgame/cg_superhud_private.h
    ${SOURCE_DIR}/cgame/cg_superhud.h
    ${SOURCE_DIR}/cgame/tr_types.h
    ${SOURCE_DIR}/game/bg_lib.h
    ${SOURCE_DIR}/game/bg_local.h
    ${SOURCE_DIR}/game/bg_public.h
    ${SOURCE_DIR}/qcommon/l_crc.h
    ${SOURCE_DIR}/qcommon/q_platform.h
    ${SOURCE_DIR}/qcommon/q_shared.h
    ${SOURCE_DIR}/qcommon/qcommon.h
    ${SOURCE_DIR}/qcommon/qfiles.h
    ${SOURCE_DIR}/qcommon/surfaceflags.h
)

set(CGAME_BINARY_SOURCES ${SOURCE_DIR}/cgame/cg_syscalls.c)
set(CGAME_QVM_SOURCES ${SOURCE_DIR}/cgame/cg_syscalls.asm)

set(GAME_MODULE_SHARED_SOURCES
    ${SOURCE_DIR}/qcommon/l_crc.c
    ${SOURCE_DIR}/qcommon/common.c
    ${SOURCE_DIR}/qcommon/q_math.c
    ${SOURCE_DIR}/qcommon/q_shared.c
)

set(CGAME_SOURCES_BASEGAME ${CGAME_SOURCES} ${GAME_MODULE_SHARED_SOURCES})

if(BUILD_MOD_LIBRARIES)
    set(CGAME_MODULE_BINARY ${CGAME_MODULE})

    set(CGAME_MODULE_BINARY_BASEGAME ${CGAME_MODULE_BINARY}_${BASEGAME})

    add_library(                ${CGAME_MODULE_BINARY_BASEGAME} SHARED ${CGAME_SOURCES_BASEGAME} ${CGAME_BINARY_SOURCES})
    target_compile_definitions( ${CGAME_MODULE_BINARY_BASEGAME} PRIVATE CGAME)
    target_link_libraries(      ${CGAME_MODULE_BINARY_BASEGAME} PRIVATE ${COMMON_LIBRARIES})
    set_target_properties(      ${CGAME_MODULE_BINARY_BASEGAME} PROPERTIES OUTPUT_NAME ${CGAME_MODULE_BINARY})
    set_output_dirs(            ${CGAME_MODULE_BINARY_BASEGAME} SUBDIRECTORY ${BASEGAME})
endif()

set(VM_DIR "")

if(BUILD_MOD_QVMS)
    set(CGAME_MODULE_QVM_BASEGAME ${CGAME_MODULE}qvm_${BASEGAME})

    add_qvm(${CGAME_MODULE_QVM_BASEGAME}
        DEFINITIONS CGAME
        OUTPUT_NAME ${CGAME_MODULE}
        OUTPUT_DIRECTORY ${BASEGAME}/vm
        SOURCES ${CGAME_SOURCES_BASEGAME} ${CGAME_QVM_SOURCES} ${CGAME_HEADERS})

    set(VM_DIR ${BASEGAME})
endif()

if(BUILD_MOD_PK3)
    add_pk3(
        ASSETS_DIR ${ASSETS_LOCATION}
        VM_DIR ${VM_DIR}
        OUTPUT_DIRECTORY ${BASEGAME}
        PAK_NAME ${OSP_PAKNAME}
    )
endif()
