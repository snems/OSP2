QA_SRC = \
 g_main $(QADIR)/g_syscalls.asm \
 bg_misc bg_lib bg_pmove bg_slidemove \
 q_math q_shared \
 ai_dmnet ai_dmq3 ai_team ai_main ai_chat ai_cmd ai_vcmd \
 g_active g_arenas g_bot g_client g_cmds g_combat g_items g_mem g_misc \
 g_missile g_mover g_rotation g_session g_spawn g_svcmds g_target g_team \
 g_trigger g_unlagged g_utils g_weapon \

CG_SRC = \
 cg_main $(CGDIR)/cg_syscalls.asm \
 cg_consolecmds cg_draw cg_drawtools cg_effects cg_ents cg_event cg_info \
 cg_localents cg_players cg_playerstate cg_predict cg_scoreboard cg_marks \
 cg_servercmds cg_snapshot cg_view cg_weapons \
 bg_slidemove bg_pmove bg_lib bg_misc \
 q_math q_shared cg_ospconfig cg_osputil cg_osphud cg_unlagged cg_playerscolors\
 cg_localevents cg_customloc common\
 cg_superhud cg_superhud_configparser cg_superhud_util cg_superhud_private\
 cg_superhud_element_fps \
 cg_superhud_element_sbhc cg_superhud_element_sbhi cg_superhud_element_sbhb \
 cg_superhud_element_sbac cg_superhud_element_sbai cg_superhud_element_sbab \
 cg_superhud_element_sbamc cg_superhud_element_sbami cg_superhud_element_sbamb \
 cg_superhud_element_target_name cg_superhud_element_target_status \
 cg_superhud_element_vmw \
 cg_superhud_element_attacker_name cg_superhud_element_attacker_icon \
 cg_superhud_element_fragmessage \
 cg_superhud_element_rankmessage \
 cg_superhud_element_ng cg_superhud_element_ngp \
 cg_superhud_element_pred \
 cg_superhud_element_speed \
 cg_superhud_element_localtime cg_superhud_element_gametime \
 cg_superhud_element_ammomessage \
 cg_superhud_element_chat \
 cg_superhud_element_specmessage cg_superhud_element_followmessage \
 cg_superhud_element_itempickup cg_superhud_element_itempickupicon \
 cg_superhud_element_teamcount \
 cg_superhud_element_flagstatus \
 cg_superhud_element_powerup \
 cg_superhud_element_name \
 cg_superhud_element_score \
 cg_superhud_element_reward \
 cg_superhud_element_team \
 cg_superhud_element_weaponlist \
 cg_superhud_element_obituaries \
 cg_superhud_element_warmupinfo cg_superhud_element_gametype cg_superhud_element_location\
 l_crc \
 cg_chatfilter cg_crosshair\
 cg_cvardescriptions\

UI_SRC = \
 ui_main $(UIDIR)/ui_syscalls.asm \
 ui_gameinfo ui_atoms ui_cinematics ui_connect ui_controls2 ui_demo2 \
 ui_mfield ui_credits ui_menu ui_ingame ui_confirm ui_setup ui_options \
 ui_display ui_sound ui_network ui_playermodel ui_players ui_playersettings \
 ui_preferences ui_qmenu ui_serverinfo ui_servers2 ui_sparena \
 ui_specifyserver ui_sppostgame ui_splevel ui_spskill ui_startserver ui_team \
 ui_video ui_addbots ui_removebots ui_teamorders ui_loadconfig ui_saveconfig \
 ui_cdkey ui_mods \
 bg_misc bg_lib \
 q_math q_shared \
