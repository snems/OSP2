/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#include "../qcommon/q_shared.h"
#include "g_local.h"

level_locals_t  level;

typedef struct {
	vmCvar_t*    vmCvar;
	char*        cvarName;
	char*        defaultString;
	int         cvarFlags;
	int         modificationCount;  // for tracking changes
	qboolean    trackChange;        // track this variable, and announce if changed
	qboolean    osp_setDefault;     // OSP: OSP_ResetCvarDefaults resets
	// every cvar with this flag to defaultString
	qboolean teamShader;        // track and if changed, update shader state
} cvarTable_t;

gentity_t       g_entities[MAX_GENTITIES];
gclient_t       g_clients[MAX_CLIENTS];

// OSP's bss order. lcc emits the uninitialised globals of a file in
// reverse order of their first declaration, so the cvars declared extern in g_local.h
// (in OSP's order, see there) sit above g_clients, and the ones below are only declared
// here.
vmCvar_t    g_gametype;
vmCvar_t    g_dedicated;
vmCvar_t    g_cheats;
vmCvar_t    g_maxclients;
vmCvar_t    g_maxGameClients;
vmCvar_t    g_restarted;
vmCvar_t    g_allowVote;
vmCvar_t    g_banIPs;
vmCvar_t    g_blood;
vmCvar_t    g_capturelimit;
vmCvar_t    g_debugAlloc;
vmCvar_t    g_debugDamage;
vmCvar_t    g_debugMove;
vmCvar_t    g_dmflags;
vmCvar_t    g_doWarmup;
vmCvar_t    g_filterBan;
vmCvar_t    g_forcerespawn;
vmCvar_t    g_fraglimit;
vmCvar_t    g_friendlyFire;
vmCvar_t    g_gravity;
vmCvar_t    g_knockback;
vmCvar_t    g_inactivity;
vmCvar_t    g_motd;
vmCvar_t    g_needpass;
vmCvar_t    g_password;
vmCvar_t    g_quadfactor;
vmCvar_t    g_spectatorInactivity;
vmCvar_t    g_speed;
vmCvar_t    g_synchronousClients;
vmCvar_t    g_teamAutoJoin;
vmCvar_t    g_teamForceBalance;
vmCvar_t    g_timelimit;
vmCvar_t    g_warmup;
vmCvar_t    g_weaponRespawn;
vmCvar_t    g_weaponTeamRespawn;
vmCvar_t    g_smoothClients;
vmCvar_t    pmove_fixed;
vmCvar_t    pmove_msec;
vmCvar_t    g_rankings;

vmCvar_t    g_log;
vmCvar_t    g_logSync;
vmCvar_t    g_podiumDist;
vmCvar_t    g_podiumDrop;
vmCvar_t    g_listEntity;
vmCvar_t    osp_gamename;
vmCvar_t    sv_hostname;
vmCvar_t    sv_dcount;
int         osp_unk_1c054;      // first bss object: "ca_session" (G_InitWorldSession, CheckTournament)

// OSP 1.03a gameCvarTable: 221 entries
cvarTable_t     gameCvarTable[] = { // OSP: not static (OSP_Cmd_Settings_f)
	{ &g_cheats, "sv_cheats", "", 0, 0, qfalse, qfalse, qfalse },
	{ &osp_gamename, "osp_gamename", "1", 0, 0, qfalse, qfalse, qfalse },
	{ NULL, "gamename", "osp", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse, qfalse, qfalse },
	{ NULL, "gamedate", __DATE__, CVAR_ROM, 0, qfalse, qfalse, qfalse },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse, qfalse, qfalse },
	{ NULL, "sv_mapname", "", CVAR_ROM, 0, qfalse, qfalse, qfalse },
	{ &g_gametype, "g_gametype", "0", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse, qfalse, qfalse },
	{ &g_maxclients, "sv_maxclients", "8", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse, qfalse, qfalse },
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse, qfalse, qfalse },
	{ NULL, "gameversion", "OSP v1.03a", CVAR_SERVERINFO | CVAR_INIT | CVAR_ROM, 0, qfalse, qfalse, qfalse },
	{ &mapsessionauto, "mapsessionauto", "-1", 0, 0, qfalse, qfalse, qfalse },
	{ &mapsessioncount, "mapsessioncount", "none", 0, 0, qfalse, qfalse, qfalse },
	{ NULL, "ca_session", "0 0 0", 0, 0, qfalse, qtrue, qfalse },
	{ NULL, "Players_Red", "", CVAR_SERVERINFO, 0, qfalse, qtrue, qfalse },
	{ NULL, "Players_Blue", "", CVAR_SERVERINFO, 0, qfalse, qtrue, qfalse },
	{ NULL, "Players_Active", "", CVAR_SERVERINFO, 0, qfalse, qtrue, qfalse },
	{ NULL, "Score_Red", "", CVAR_SERVERINFO, 0, qfalse, qtrue, qfalse },
	{ NULL, "Score_Blue", "", CVAR_SERVERINFO, 0, qfalse, qtrue, qfalse },
	{ &default_change, "default_change", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &default_settings, "default_settings", "none", 0, 0, qfalse, qtrue, qfalse },
	{ &Score_Time, "Score_Time", "", CVAR_SERVERINFO, 0, qfalse, qtrue, qfalse },
	{ &server_promode, "server_promode", "0", CVAR_SERVERINFO, 0, qfalse, qfalse, qfalse },
	{ &server_cq3, "server_cq3", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &g_allowVote, "g_allowVote", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse, qfalse, qfalse },
	{ &g_blood, "g_blood", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &g_capturelimit, "capturelimit", "8", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_NORESTART, 0, qfalse, qtrue, qfalse },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &g_debugDamage, "g_debugDamage", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &g_debugMove, "g_debugMove", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &g_dedicated, "dedicated", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &g_dmflags, "dmflags", "0", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qfalse, qtrue, qfalse },
	{ &g_doWarmup, "g_doWarmup", "1", CVAR_ARCHIVE, 0, qtrue, qtrue, qfalse },
	{ &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse, qfalse, qfalse },
	{ &g_forcerespawn, "g_forcerespawn", "20", 0, 0, qfalse, qtrue, qfalse },
	{ &g_friendlyFire, "g_friendlyFire", "1", CVAR_ARCHIVE, 0, qfalse, qtrue, qfalse },
	{ &g_fraglimit, "fraglimit", "20", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_NORESTART, 0, qfalse, qtrue, qfalse },
	{ &g_gravity, "g_gravity", "800", 0, 0, qtrue, qtrue, qfalse },
	{ &g_inactivity, "g_inactivity", "0", 0, 0, qtrue, qfalse, qfalse },
	{ &g_knockback, "g_knockback", "1000", 0, 0, qtrue, qfalse, qfalse },
	{ &g_listEntity, "g_listEntity", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &g_log, "g_log", "games.log", CVAR_ARCHIVE, 0, qfalse, qfalse, qfalse },
	{ &g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0, qfalse, qfalse, qfalse },
	{ &g_motd, "g_motd", "", 0, 0, qfalse, qfalse, qfalse },
	{ &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse, qfalse, qfalse },
	{ &g_password, "g_password", "none", CVAR_USERINFO, 0, qfalse, qfalse, qfalse },
	{ &g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse, qfalse, qfalse },
	{ &g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse, qfalse, qfalse },
	{ &g_quadfactor, "g_quadfactor", "3", 0, 0, qfalse, qtrue, qfalse },
	{ &g_rankings, "g_rankings", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &g_spectatorInactivity, "g_spectatorInactivity", "0", 0, 0, qtrue, qfalse, qfalse },
	{ &g_speed, "g_speed", "320", 0, 0, qtrue, qfalse, qfalse },
	{ &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse, qfalse, qfalse },
	{ &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE, 0, qfalse, qfalse, qfalse },
	{ &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE, 0, qfalse, qfalse, qfalse },
	{ &g_timelimit, "timelimit", "0", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_NORESTART, 0, qfalse, qtrue, qfalse },
	{ &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, qtrue, qtrue, qfalse },
	{ &g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, qfalse, qtrue, qfalse },
	{ &g_weaponTeamRespawn, "g_weaponteamrespawn", "30", 0, 0, qfalse, qtrue, qfalse },
	{ &g_smoothClients, "g_smoothClients", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, qfalse, qtrue, qfalse },
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qfalse, qtrue, qfalse },
	{ &osp_restart, "osp_restart", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &roundlimit, "roundlimit", "5", 0, 0, qfalse, qtrue, qfalse },
	{ &sv_dcount, "sv_dcount", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &sv_hostname, "sv_hostname", "noname", 0, 0, qfalse, qfalse, qfalse },
	{ &admin_log, "admin_log", "none", 0, 0, qfalse, qfalse, qfalse },
	{ &armor_q2style, "armor_q2style", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &bot_checktime, "bot_checktime", "10", 0, 0, qfalse, qfalse, qfalse },
	{ &bot_autoload, "bot_autoload", "[none]", 0, 0, qfalse, qfalse, qfalse },
	{ &decal_directory, "decal_directory", "decals", 0, 0, qfalse, qfalse, qfalse },
	{ &filter_file, "filter_file", "filters.txt", 0, 0, qfalse, qfalse, qfalse },
	{ &freezetag_latejoin, "freezetag_latejoin", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &freezetag_respawn, "freezetag_respawn", "8.0", 0, 0, qfalse, qtrue, qfalse },
	{ &gauntlet_damage, "gauntlet_damage", "50", 0, 0, qfalse, qtrue, qfalse },
	{ &gauntlet_disable, "gauntlet_disable", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &hook_delaytime, "hook_delaytime", "750", 0, 0, qfalse, qfalse, qfalse },
	{ &hook_enable, "hook_enable", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &hook_holdtime, "hook_holdtime", "3", 0, 0, qfalse, qfalse, qfalse },
	{ &hook_sky, "hook_sky", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &hook_speed, "hook_speed", "1200", 0, 0, qfalse, qfalse, qfalse },
	{ &hook_speedpull, "hook_speedpull", "950", 0, 0, qfalse, qfalse, qfalse },
	{ &instagib_reload, "instagib_reload", "1500", 0, 0, qtrue, qfalse, qfalse },
	{ &item_disable, "item_disable", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &log_default, "log_default", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &log_stat, "log_stat", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &map_autoload, "map_autoload", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &map_debug, "map_debug", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &map_file, "map_file", "maps.txt", 0, 0, qfalse, qfalse, qfalse },
	{ &map_halt, "map_halt", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &map_nocount, "map_nocount", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &map_once, "map_once", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &map_queue, "map_queue", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &map_random, "map_random", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &map_rotate, "map_rotate", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &match_carespawn, "match_carespawn", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &match_cawarmupfire, "match_cawarmupfire", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &match_dropitems, "match_dropitems", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &match_hurtself, "match_hurtself", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &match_instagib, "match_instagib", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &match_latejoin, "match_latejoin", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &match_maxsuicides, "match_maxsuicides", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &match_minplayers, "match_minplayers", "2", 0, 0, qfalse, qtrue, qfalse },
	{ &match_mutespecs, "match_mutespecs", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &match_overtime, "match_overtime", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &match_overtimecount, "match_overtimecount", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &match_overtimelength, "match_overtimelength", "5", 0, 0, qfalse, qtrue, qfalse },
	{ &match_poweruprespawn, "match_poweruprespawn", "60", 0, 0, qfalse, qfalse, qfalse },
	{ &match_readypercent, "match_readypercent", "51", 0, 0, qfalse, qtrue, qfalse },
	{ &match_roundtimer, "match_roundtimer", "10", 0, 0, qfalse, qtrue, qfalse },
	{ &match_timeoutlength, "match_timeoutlength", "60", 0, 0, qfalse, qtrue, qfalse },
	{ &match_timeoutcount, "match_timeoutcount", "3", 0, 0, qfalse, qtrue, qfalse },
	{ &match_winbyscore, "match_winbyscore", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &mg_damage, "mg_damage", "7", 0, 0, qfalse, qtrue, qfalse },
	{ &mg_damageTeamplay, "mg_damageTeamplay", "5", 0, 0, qfalse, qtrue, qfalse },
	{ &z_m_current, "z_m_current", "-1", 0, 0, qfalse, qfalse, qfalse },
	{ &mode_customfile, "mode_customfile", "none", 0, 0, qfalse, qfalse, qfalse },
	{ &z_m_default, "z_m_default", "-1", 0, 0, qfalse, qfalse, qfalse },
	{ &mode_start, "mode_start", "-1", 0, 0, qfalse, qfalse, qfalse },
	{ &player_always, "player_always", "", 0, 0, qfalse, qfalse, qfalse },
	{ &player_file, "player_file", "none", 0, 0, qfalse, qfalse, qfalse },
	{ &powerup_disable, "powerup_disable", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &ref_password, "ref_password", "none", 0, 0, qfalse, qfalse, qfalse },
	{ &railgun_damage, "railgun_damage", "100", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_machinegun, "replace_machinegun", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_shotgun, "replace_shotgun", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_glauncher, "replace_glauncher", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_rlauncher, "replace_rlauncher", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_lightning, "replace_lightning", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_railgun, "replace_railgun", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_plasmagun, "replace_plasmagun", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_bfg, "replace_bfg", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_invis, "replace_invis", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &replace_quad, "replace_quad", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_battlesuit, "replace_battlesuit", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_haste, "replace_haste", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_regen, "replace_regen", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &replace_flight, "replace_flight", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &server_availmodes, "server_availmodes", "127", 0, 0, qfalse, qfalse, qfalse },
	{ &server_batcher, "server_batcher", "none", 0, 0, qfalse, qfalse, qfalse },
	{ &server_customclient, "server_customclient", "15", 0, 0, qfalse, qfalse, qfalse },
	{ &server_fastrail, "server_fastrail", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &server_freezetag, "server_freezetag", "0", CVAR_SERVERINFO, 0, qfalse, qtrue, qfalse },
	{ &server_gfxspecfile, "server_gfxspecfile", "none", 0, 0, qfalse, qfalse, qfalse },
	{ &server_lgcooldown, "server_lgcooldown", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &server_maxpacketsmin, "server_maxpacketsmin", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &server_maxpacketsmax, "server_maxpacketsmax", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &server_motdfile, "server_motdfile", "none", 0, 0, qfalse, qfalse, qfalse },


	{ &server_motd1, "server_motd1", " ^B^3*** Welcome to OSP2 Tourney DM/CA/CTF *** ", 0, 0, qfalse, qfalse, qfalse },
	{ &server_motd2, "server_motd2", "  ", 0, 0, qfalse, qfalse, qfalse },
	{ &server_motd3, "server_motd3", "        https://github.com/snems/OSP2", 0, 0, qfalse, qfalse, qfalse },
	{ &server_motd4, "server_motd4", "", 0, 0, qfalse, qfalse, qfalse },
	{ &server_motd5, "server_motd5", "", 0, 0, qfalse, qfalse, qfalse },
	{ &server_motd6, "server_motd6", "", 0, 0, qfalse, qfalse, qfalse },
	{ &server_motd7, "server_motd7", "", 0, 0, qfalse, qfalse, qfalse },
	{ &server_motd8, "server_motd8", "", 0, 0, qfalse, qfalse, qfalse },
	{ &server_ospauth, "server_ospauth", "0", CVAR_SERVERINFO, 0, qfalse, qfalse, qfalse },
	{ &server_realping, "server_realping", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &server_record, "server_record", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &server_thrufloors, "server_thrufloors", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &server_timenudgemax, "server_timenudgemax", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &server_timenudgemin, "server_timenudgemin", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &start_armor, "start_armor", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &start_bfg, "start_bfg", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &start_bullets, "start_bullets", "100", 0, 0, qfalse, qtrue, qfalse },
	{ &start_cells, "start_cells", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &start_grenades, "start_grenades", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &start_health, "start_health", "125", 0, 0, qfalse, qtrue, qfalse },
	{ &start_lightning, "start_lightning", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &start_rockets, "start_rockets", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &start_shells, "start_shells", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &start_slugs, "start_slugs", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &team_allcaptain, "team_allcaptain", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &team_armordamage, "team_armordamage", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &team_maxplayers, "team_maxplayers", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &team_nocontrols, "team_nocontrols", "0", 0, 0, qfalse, qtrue, qfalse },
	{ &team_noteamready, "team_noteamready", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &viewcam_directory, "viewcam_directory", "cfg-viewcam", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_allcaptain, "vote_allow_allcaptain", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_altgraphics, "vote_allow_altgraphics", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_armordamage, "vote_allow_armordamage", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_capfps, "vote_allow_capfps", "0", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_capturelimit, "vote_allow_capturelimit", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_ca_startrespawn, "vote_allow_ca_startrespawn", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_ca_warmupfire, "vote_allow_ca_warmupfire", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_cq3, "vote_allow_cq3", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_dropitems, "vote_allow_dropitems", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_falling, "vote_allow_falling", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_fastrail, "vote_allow_fastrail", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_fraglimit, "vote_allow_fraglimit", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_hook, "vote_allow_hook", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_items, "vote_allow_items", "1023", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_instagib, "vote_allow_instagib", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_kick, "vote_allow_kick", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_lgcooldown, "vote_allow_lgcooldown", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_map, "vote_allow_map", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_maxpackets, "vote_allow_maxpackets", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_machinegun, "vote_allow_machinegun", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_mutespecs, "vote_allow_mutespecs", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_notimer, "vote_allow_notimer", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_osphud, "vote_allow_osphud", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_overtime, "vote_allow_overtime", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_pmove, "vote_allow_pmove", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_poweruprespawn, "vote_allow_poweruprespawn", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_powerups, "vote_allow_powerups", "255", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_promode, "vote_allow_promode", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_referee, "vote_allow_referee", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_reload, "vote_allow_reload", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_respawn, "vote_allow_respawn", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_roundlimit, "vote_allow_roundlimit", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_teamdamage, "vote_allow_teamdamage", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_thrufloors, "vote_allow_thrufloors", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_allow_timenudge, "vote_allow_timenudge", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_timelimit, "vote_allow_timelimit", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_allow_weaponrespawn, "vote_allow_weaponrespawn", "1", 0, 0, qfalse, qfalse, qfalse },
	{ &vote_limit, "vote_limit", "5", 0, 0, qfalse, qtrue, qfalse },
	{ &vote_percent, "vote_percent", "50", 0, 0, qfalse, qtrue, qfalse },
	{ &warmup_armor, "warmup_armor", "100", 0, 0, qfalse, qtrue, qfalse },
	{ &weapon_deaddrop, "weapon_deaddrop", "1", 0, 0, qfalse, qtrue, qfalse },
	{ &weapon_have, "weapon_have", "257", 0, 0, qfalse, qtrue, qfalse },
	{ &weapon_start, "weapon_start", "1", 0, 0, qfalse, qtrue, qfalse },
};

// bk001129 - made static to avoid aliasing
int gameCvarTableSize = sizeof(gameCvarTable) / sizeof(gameCvarTable[0]);


void G_InitGame(int levelTime, int randomSeed, int restart);
void G_RunFrame(int levelTime);
void G_ShutdownGame(int restart);
void CheckExitRules(void);


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
Q_EXPORT
int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	switch (command) {
		case GAME_INIT:
			G_InitGame(arg0, arg1, arg2);
			return 0;
		case GAME_SHUTDOWN:
			G_ShutdownGame(arg0);
			return 0;
		case GAME_CLIENT_CONNECT:
			return (size_t)ClientConnect(arg0, arg1, arg2);
		case GAME_CLIENT_THINK:
			ClientThink(arg0);
			return 0;
		case GAME_CLIENT_USERINFO_CHANGED:
			ClientUserinfoChanged(arg0);
			return 0;
		case GAME_CLIENT_DISCONNECT:
			ClientDisconnect(arg0);
			return 0;
		case GAME_CLIENT_BEGIN:
			ClientBegin(arg0);
			return 0;
		case GAME_CLIENT_COMMAND:
			ClientCommand(arg0);
			return 0;
		case GAME_RUN_FRAME:
			G_RunFrame(arg0);
			return 0;
		case GAME_CONSOLE_COMMAND:
			return ConsoleCommand();
		case BOTAI_START_FRAME:
			return BotAIStartFrame(arg0);
	}

	return -1;
}


void QDECL G_Printf(const char* fmt, ...) {
	va_list     argptr;
	char        text[1024];

	va_start (argptr, fmt);
	vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	trap_Printf(text);
}

void QDECL G_Error(const char* fmt, ...) {
	va_list     argptr;
	char        text[1024];

	va_start (argptr, fmt);
	vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	trap_Error(text);
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams(void) {
	gentity_t*   e, *e2;
	int     i, j;
	int     c, c2;

	c = 0;
	c2 = 0;
	for (i=1, e=g_entities+i ; i < level.num_entities ; i++,e++) {
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < level.num_entities ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
				e2->flags |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if (e2->targetname) {
					e->targetname = e2->targetname;
					e2->targetname = NULL;
				}
			}
		}
	}

	G_Printf ("%i teams with %i entities\n", c, c2);
}

void G_RemapTeamShaders(void) {
#ifdef MISSIONPACK
	char string[1024];
	float f = level.time * 0.001;
	Com_sprintf(string, sizeof(string), "team_icon/%s_red", g_redteam.string);
	AddRemap("textures/ctf2/redteam01", string, f);
	AddRemap("textures/ctf2/redteam02", string, f);
	Com_sprintf(string, sizeof(string), "team_icon/%s_blue", g_blueteam.string);
	AddRemap("textures/ctf2/blueteam01", string, f);
	AddRemap("textures/ctf2/blueteam02", string, f);
	trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
#endif
}


/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars(void) {
	int         i;
	cvarTable_t* cv;
	qboolean remapped = qfalse;

	for (i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++) {
		// OSP: the score / player list serverinfo cvars only exist where they make sense
		if (g_gametype.integer < GT_TEAM && strstr(cv->cvarName, "Score_") == cv->cvarName &&
		        (g_gametype.integer != GT_TOURNAMENT || Q_stricmp(cv->cvarName, "Score_Time"))) {
			continue;
		}
		if (g_gametype.integer < GT_TEAM &&
		        (!Q_stricmp(cv->cvarName, "Players_Red") || !Q_stricmp(cv->cvarName, "Players_Blue"))) {
			continue;
		}
		if (g_gametype.integer >= GT_TEAM && !Q_stricmp(cv->cvarName, "Players_Active")) {
			continue;
		}

		// OSP: per-mode defaults
		if (!Q_stricmp(cv->cvarName, "gamename") && !osp_gamename.integer) {
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, "baseq3", cv->cvarFlags);
		} else if (cv->vmCvar == &start_health && (server_promode.integer > 0 || server_cq3.integer > 0)) {
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, "100", cv->cvarFlags);
		} else if (cv->vmCvar == &g_weaponRespawn && (server_promode.integer > 0 || server_cq3.integer > 0)) {
			char* value = "5";

			switch (g_gametype.integer) {
				case GT_TOURNAMENT:
					value = "15";
					break;
				case GT_TEAM:
					value = "30";
					break;
				case GT_CTF:
					value = "10";
					break;
				default:
					value = "5";
					break;
			}
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, value, cv->cvarFlags);
		} else if (g_gametype.integer == GT_TEAM && cv->vmCvar == &start_bullets) {
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, "50", cv->cvarFlags);
		} else if (g_gametype.integer == GT_CTF && cv->vmCvar == &match_poweruprespawn) {
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, "120", cv->cvarFlags);
		} else if (cv->vmCvar == &g_forcerespawn) {
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, (g_gametype.integer == GT_TOURNAMENT) ? "10" : "20", cv->cvarFlags);
		} else if (g_gametype.integer == GT_TOURNAMENT && cv->vmCvar == &match_mutespecs) {
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, "1", cv->cvarFlags);
		} else if (g_gametype.integer == 5 && cv->vmCvar == &start_armor) {
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, "200", cv->cvarFlags);
		} else if (OSP_IsRoundBased(g_gametype.integer) && cv->vmCvar == &roundlimit) {
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags);
			trap_Cvar_Set("fraglimit", roundlimit.string);
		} else if (cv->vmCvar == &match_overtime) {
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, (g_gametype.integer == GT_TEAM) ? "1" : "0", cv->cvarFlags);
		} else {
			trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags);
		}

		if (cv->vmCvar)
			cv->modificationCount = cv->vmCvar->modificationCount;

		if (cv->teamShader) {
			remapped = qtrue;
		}
	}

	if (remapped) {
		G_RemapTeamShaders();
	}

	// OSP: physics mode and instagib follow their cvars
	trap_Cvar_Update(&server_promode);
	trap_Cvar_Update(&server_cq3);
	OSP_SetPhysicsMode();
	OSP_SetInstagib(0);

	// check some things
	if (g_gametype.integer < 0 || g_gametype.integer >= GT_MAX_GAME_TYPE) {
		G_Printf("g_gametype %i is out of range, defaulting to 0\n", g_gametype.integer);
		trap_Cvar_Set("g_gametype", "0");
	}

	level.warmupModificationCount = g_warmup.modificationCount;
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars(void) {
	int         i;
	cvarTable_t* cv;
	qboolean remapped = qfalse;
	qboolean updateFlags = qfalse;

	if (level.warmupTime) {
		OSP_HidePlayersInServerinfo(0);
	}

	for (i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++) {
		if (cv->vmCvar) {
			trap_Cvar_Update(cv->vmCvar);

			if (cv->modificationCount != cv->vmCvar->modificationCount) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				// OSP: change handlers (id's "Server: %s changed to %s" print is gone)
				if (cv->vmCvar == &roundlimit) {
					trap_Cvar_Set("fraglimit", va("%d", roundlimit.integer));
				} else if (!Q_stricmp(cv->cvarName, "g_password") || !Q_stricmp(cv->cvarName, "sv_maxclients")) {
					level.mapReloadNeeded = 1;
				} else if ((server_record.integer & 16) && !Q_stricmp(cv->cvarName, "sv_hostname")) {
					trap_Cvar_Set("sv_dcount", "0");
				} else if (cv->vmCvar == &admin_log) {
					osp_adminLogDisabled = qfalse;
				} else if (cv->vmCvar == &instagib_reload) {
					if (instagib_reload.integer < 100 || instagib_reload.integer > 5000) {
						trap_Cvar_Set("instagib_reload", "1500");
					}
				} else if (cv->vmCvar == &match_poweruprespawn) {
					int respawn = match_poweruprespawn.integer;

					respawn = (respawn <= 60) ? 60 : ((respawn > 60 && respawn < 120) ? 90 : 120);
					trap_Cvar_Set("match_poweruprespawn", va("%d", respawn));
					if (!(level.physicsMode & 1)) {
						modeRespawnPowerup = 120;
					} else if (g_gametype.integer != GT_CTF) {
						modeRespawnPowerup = respawn;
					} else {
						modeRespawnPowerupCTF = respawn;
					}
				} else if (cv->vmCvar == &match_instagib) {
					OSP_SetInstagib(1);
				} else if (cv->vmCvar == &server_promode || cv->vmCvar == &server_cq3 ||
				           cv->vmCvar == &server_fastrail || cv->vmCvar == &server_lgcooldown) {
					OSP_SetPhysicsMode();
				} else if (cv->vmCvar == &server_maxpacketsmin) {
					trap_SetConfigstring(CS_OSP_MAXPACKETSMIN, va("%d", (server_maxpacketsmin.integer > 15) ? server_maxpacketsmin.integer : 15));
				} else if (cv->vmCvar == &server_maxpacketsmax) {
					trap_SetConfigstring(CS_OSP_MAXPACKETSMAX, va("%d", (server_maxpacketsmax.integer < 125) ? server_maxpacketsmax.integer : 125));
				} else if (cv->vmCvar == &server_timenudgemin) {
					trap_SetConfigstring(CS_OSP_TIMENUDGEMIN, va("%d", (server_timenudgemin.integer > -200) ? server_timenudgemin.integer : -200));
				} else if (cv->vmCvar == &server_timenudgemax) {
					trap_SetConfigstring(CS_OSP_TIMENUDGEMAX, va("%d", (server_timenudgemax.integer < 200) ? server_timenudgemax.integer : 200));
				} else if (cv->vmCvar == &server_customclient) {
					trap_SetConfigstring(CS_OSP_CUSTOMCLIENT, va("%d", server_customclient.integer));
				} else if (cv->vmCvar == &pmove_fixed) {
					trap_SetConfigstring(CS_OSP_PMOVEFIXED, va("%d", pmove_fixed.integer));
				} else if (cv->vmCvar == &server_ospauth) {
					trap_SetConfigstring(CS_OSP_AUTH, va("%d", server_ospauth.integer));
				} else if (cv->vmCvar == &server_freezetag) {
					trap_SetConfigstring(CS_OSP_FREEZETAG, va("%d", (server_freezetag.integer == 0) ? 0 : ((server_freezetag.integer == 2) ? 2 : 1)));
				} else if (level.engineCheck == 2 && (cv->vmCvar == &g_fraglimit || cv->vmCvar == &g_capturelimit ||
				                                      cv->vmCvar == &g_timelimit || cv->vmCvar == &g_dmflags || cv->vmCvar == &g_gametype)) {
					updateFlags = qtrue;
				}

				if (cv->teamShader) {
					remapped = qtrue;
				}
			}
		}
	}

	if (remapped) {
		G_RemapTeamShaders();
	}

	if (updateFlags || level.warmupTime) {
		OSP_HidePlayersInServerinfo(0);
	}
}

/*
============
G_InitGame

============
*/
void G_InitGame(int levelTime, int randomSeed, int restart) {
	int                 i;

	G_Printf ("------- Game Initialization -------\n");

	srand(randomSeed);

	G_RegisterCvars();

	G_ProcessIPBans();

	G_InitMemory();

	// OSP: version and client settings for the OSP cgame
	trap_SetConfigstring(CS_OSP_VERSION, va("OSP2 Tourney DM/CA/CTF v(%s)", XSTRINGIFY(OSP_VERSION)));
	trap_SetConfigstring(CS_OSP_PMOVEFIXED, va("%d", pmove_fixed.integer));
	trap_SetConfigstring(CS_OSP_CUSTOMCLIENT, va("%d", server_customclient.integer));
	trap_SetConfigstring(CS_OSP_AUTH, va("%d", server_ospauth.integer));
	trap_SetConfigstring(CS_OSP_MAXPACKETSMIN, va("%d", (server_maxpacketsmin.integer > 15) ? server_maxpacketsmin.integer : 15));
	trap_SetConfigstring(CS_OSP_MAXPACKETSMAX, va("%d", (server_maxpacketsmax.integer < 125) ? server_maxpacketsmax.integer : 125));
	trap_SetConfigstring(CS_OSP_TIMENUDGEMIN, va("%d", (server_timenudgemin.integer > -200) ? server_timenudgemin.integer : -200));
	trap_SetConfigstring(CS_OSP_TIMENUDGEMAX, va("%d", (server_timenudgemax.integer < 200) ? server_timenudgemax.integer : 200));
	trap_SetConfigstring(CS_OSP_CUSTOMMODE, va("%d", (z_m_current.integer == 2) ? 1 : 0));
	trap_SetConfigstring(CS_OSP_FREEZETAG, va("%d", (server_freezetag.integer == 0) ? 0 : ((server_freezetag.integer == 2) ? 2 : 1)));

	G_Printf ("gamename: %s\n", (osp_gamename.integer) ? "osp" : "baseq3 -- osp");
	G_Printf ("gamedate: %s\n", __DATE__);

	trap_SetConfigstring(CS_INTERMISSION, "0");

	// set some level globals
	memset(&level, 0, sizeof(level));
	OSP_SetPhysicsMode();
	level.time = levelTime;
	level.startTime = levelTime;
	level.clientChangeTime = levelTime;
	level.timeLeftTime = 0;
	level.timeLeftInterval = 7900;

	level.snd_fry = G_SoundIndex("sound/player/fry.wav");   // FIXME standing in lava / slime

	if (g_gametype.integer != GT_SINGLE_PLAYER && g_log.string[0]) {
		trap_FS_FOpenFile(g_log.string, &level.logFile, (g_logSync.integer) ? FS_APPEND_SYNC : FS_APPEND);
		if (!level.logFile) {
			G_Printf("WARNING: Couldn't open logfile: %s\n", g_log.string);
		} else {
			qtime_t ct;
			char*    months[12] = {
				"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
			};
			char    serverinfo[MAX_INFO_STRING];

			trap_GetServerinfo(serverinfo, sizeof(serverinfo));

			G_LogPrintf("------------------------------------------------------------\n");
			G_LogPrintf("InitGame: %s\n", serverinfo);

			// OSP: server time stamp
			trap_RealTime(&ct);
			G_LogPrintf("ServerTime:\t%d%02d%02d%02d%02d%02d\t%02d:%02d:%02d (%02d %s %d)\n",
			            ct.tm_year + 1900, ct.tm_mon + 1, ct.tm_mday, ct.tm_hour, ct.tm_min, ct.tm_sec,
			            ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday, months[ct.tm_mon], ct.tm_year + 1900);
		}
	} else {
		G_Printf("Not logging to disk.\n");
	}

	G_InitWorldSession();

	// initialize all entities for this game
	memset(g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]));
	level.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset(g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]));
	level.clients = g_clients;

	// set client fields on player ents
	for (i=0 ; i<level.maxclients ; i++) {
		g_entities[i].client = level.clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	// let the server system know where the entites are
	trap_LocateGameData(level.gentities, level.num_entities, sizeof(gentity_t),
	                    &level.clients[0].ps, sizeof(level.clients[0]));

	// reserve some spots for dead player bodies
	InitBodyQue();

	ClearRegisteredItems();
	OSP_InitItemNames();

	// OSP: map fixes for ospdm4 / ospdm5
	level.osp_unk_3834 |= (!Q_stricmp(BotMapTitle(), "ospdm4")) ? 1 : 0;
	level.osp_unk_3834 |= (!Q_stricmp(BotMapTitle(), "ospdm5")) ? 2 : 0;

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString();

	// general initialization
	G_FindTeams();

	// make sure we have flags for CTF, etc
	if (g_gametype.integer >= GT_TEAM) {
		G_CheckTeamItems();
	}

	SaveRegisteredItems();

	G_Printf ("-----------------------------------\n");

	if (g_gametype.integer == GT_SINGLE_PLAYER || trap_Cvar_VariableIntegerValue("com_buildScript")) {
		G_ModelIndex(SP_PODIUM_MODEL);
		G_SoundIndex("sound/player/gurp1.wav");
		G_SoundIndex("sound/player/gurp2.wav");
	}

	if (trap_Cvar_VariableIntegerValue("bot_enable")) {
		BotAISetup(restart);
		BotAILoadMap(restart);
		G_InitBots(restart);
	}

	G_RemapTeamShaders();

	OSP_InitGame();
	G_UpdateCvars();
}



/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame(int restart) {
	G_Printf ("==== ShutdownGame ====\n");

	if (level.logFile) {
		G_LogPrintf("ShutdownGame:\n");
		G_LogPrintf("------------------------------------------------------------\n");
		trap_FS_FCloseFile(level.logFile);
	}

	// write all the client session data so we can get it back
	G_WriteSessionData();

	if (trap_Cvar_VariableIntegerValue("bot_enable")) {
		BotAIShutdown(restart);
	}
}



//===================================================================

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link

void QDECL Com_Error (int level_, const char* error, ...) {
	va_list     argptr;
	char        text[1024];

	va_start (argptr, error);
	vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	while (qtrue) G_Error("%s", text);
}

void QDECL Com_Printf(const char* msg, ...) {
	va_list     argptr;
	char        text[1024];

	va_start (argptr, msg);
	vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	G_Printf ("%s", text);
}

#endif

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
AddTournamentPlayer

If there are less than two tournament players, put a
spectator in the game and restart
=============
*/
void AddTournamentPlayer(void) {
	int         i;
	gclient_t*   client;
	gclient_t*   nextInLine = NULL;

	// never change during intermission
	if (level.numNonSpectatorClients >= 2 || level.intermissiontime) {
		return;
	}

	for (i = 0 ; i < level.maxclients ; i++) {
		client = &level.clients[i];
		// OSP: never select players the name filters force to spectate, the dedicated follow
		// or scoreboard clients
		if (client->pers.connected != CON_CONNECTED || client->sess.sessionTeam != TEAM_SPECTATOR ||
		        OSP_CheckClientFilters(client->pers.netname, i, 1) == 2 ||
		        client->sess.spectatorState == SPECTATOR_SCOREBOARD || client->sess.spectatorClient < 0) {
			continue;
		}

		if (!nextInLine || client->sess.spectatorTime < nextInLine->sess.spectatorTime) {
			nextInLine = client;
		}
	}

	if (!nextInLine) {
		return;
	}

	level.warmupTime = -1;

	// set them to free-for-all team
	SetTeam(&g_entities[ nextInLine - level.clients ], "f");
	nextInLine->pers.timeouts = match_timeoutcount.integer;
}

/*
=======================
RemoveTournamentLoser

Make the loser a spectator at the back of the line
=======================
*/
void RemoveTournamentLoser(void) {
	int         clientNum;

	if (level.numPlayingClients != 2 || osp_mapChangeReason) {
		return;
	}

	clientNum = level.sortedClients[1];

	if (level.clients[ clientNum ].pers.connected != CON_CONNECTED) {
		return;
	}

	// make them a spectator
	SetTeam(&g_entities[ clientNum ], "s");
}

/*
=======================
AdjustTournamentScores
=======================
*/
void AdjustTournamentScores(void) {
	int         clientNum;

	if (!osp_mapChangeReason) {
		clientNum = level.sortedClients[0];
		if (level.clients[ clientNum ].pers.connected == CON_CONNECTED) {
			level.clients[ clientNum ].sess.wins++;
			ClientUserinfoChanged(clientNum);
		}

		clientNum = level.sortedClients[1];
		if (level.clients[ clientNum ].pers.connected == CON_CONNECTED) {
			level.clients[ clientNum ].sess.losses++;
			ClientUserinfoChanged(clientNum);
		}
	}
}



/*
=============
SortRanks

=============
*/
int QDECL SortRanks(const void* a, const void* b) {
	gclient_t*   ca, *cb;
	int         teamA, teamB;

	ca = &level.clients[*(int*)a];
	cb = &level.clients[*(int*)b];
	// OSP: coaches sort with the team they coach
	teamA = OSP_ClientTeam(&g_entities[*(int*)a]);
	teamB = OSP_ClientTeam(&g_entities[*(int*)b]);

	// sort special clients last
	if (ca->sess.spectatorState == SPECTATOR_SCOREBOARD || ca->sess.spectatorClient < 0) {
		return 1;
	}
	if (cb->sess.spectatorState == SPECTATOR_SCOREBOARD || cb->sess.spectatorClient < 0) {
		return -1;
	}

	// then connecting clients
	if (ca->pers.connected == CON_CONNECTING) {
		return 1;
	}
	if (cb->pers.connected == CON_CONNECTING) {
		return -1;
	}


	// then spectators
	if (teamA == TEAM_SPECTATOR && teamB == TEAM_SPECTATOR) {
		if (!OSP_IsRoundBased(g_gametype.integer)) {
			if (ca->sess.spectatorTime < cb->sess.spectatorTime) {
				return -1;
			}
			if (ca->sess.spectatorTime > cb->sess.spectatorTime) {
				return 1;
			}
			return 0;
		}

		// OSP: in round-based modes the dead players are spectators, keep their score order
		if (ca->ps.persistant[PERS_SCORE] > cb->ps.persistant[PERS_SCORE]) {
			return -1;
		}
		if (ca->ps.persistant[PERS_SCORE] < cb->ps.persistant[PERS_SCORE]) {
			return 1;
		}
		if (ca->sess.deaths < cb->sess.deaths) {
			return -1;
		}
		if (cb->sess.deaths < ca->sess.deaths) {
			return 1;
		}
		if (ca->sess.damageGiven / 100 > cb->sess.damageGiven / 100) {
			return -1;
		}
		if (cb->sess.damageGiven / 100 > ca->sess.damageGiven / 100) {
			return 1;
		}
		return 0;
	}
	if (teamA == TEAM_SPECTATOR) {
		return 1;
	}
	if (teamB == TEAM_SPECTATOR) {
		return -1;
	}

	// then sort by score
	if (ca->ps.persistant[PERS_SCORE]
	        > cb->ps.persistant[PERS_SCORE]) {
		return -1;
	}
	if (ca->ps.persistant[PERS_SCORE]
	        < cb->ps.persistant[PERS_SCORE]) {
		return 1;
	}
	// OSP: then fewer deaths, fewer suicides, more damage given, less damage received
	if (ca->sess.deaths < cb->sess.deaths) {
		return -1;
	}
	if (cb->sess.deaths < ca->sess.deaths) {
		return 1;
	}
	if (ca->sess.suicides < cb->sess.suicides) {
		return -1;
	}
	if (cb->sess.suicides < ca->sess.suicides) {
		return 1;
	}
	if (ca->sess.damageGiven > cb->sess.damageGiven) {
		return -1;
	}
	if (cb->sess.damageGiven > ca->sess.damageGiven) {
		return 1;
	}
	if (cb->sess.damageReceived < ca->sess.damageReceived) {
		return 1;
	}
	return -1;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks(int checkExit) {
	int     i;
	int     rank;
	int     score;
	int     newScore;
	gclient_t*   cl;
	char    players[TEAM_NUM_TEAMS][128];   // OSP: Players_Active / Players_Red / Players_Blue

	level.follow1 = -1;
	level.follow2 = -1;
	osp_numViewcamClients = 0;
	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;     // don't count bots
	for (i = 0; i < TEAM_NUM_TEAMS; i++) {
		players[i][0] = 0;
		level.numteamVotingClients[i] = 0;
	}
	for (i = 0 ; i < level.maxclients ; i++) {
		cl = &level.clients[i];
		if (cl->pers.connected != CON_DISCONNECTED) {
			level.sortedClients[level.numConnectedClients] = i;
			level.numConnectedClients++;

			if (OSP_ClientTeam(&g_entities[i]) != TEAM_SPECTATOR) {
				int team = OSP_ClientTeam(&g_entities[i]);

				level.numNonSpectatorClients++;
				Q_strcat(players[team], sizeof(players[team]) - 1, va("%d ", level.numConnectedClients));

				// decide if this should be auto-followed
				if (cl->pers.connected == CON_CONNECTED) {
					level.numPlayingClients++;
					// OSP: numteamVotingClients counts the living players of each team
					if (!is_spectator(cl) && !g_entities[i].frozen) {
						if (team == TEAM_RED)
							level.numteamVotingClients[0]++;
						if (team == TEAM_BLUE)
							level.numteamVotingClients[1]++;
					}
					if (!(g_entities[i].r.svFlags & SVF_BOT)) {
						level.numVotingClients++;
					}
					if (level.follow1 == -1) {
						level.follow1 = i;
					} else if (level.follow2 == -1) {
						level.follow2 = i;
					}
				}
			} else if (cl->sess.spectatorState == 4) {
				osp_numViewcamClients++;
			}
		}
	}

	for (i = 0; i < TEAM_NUM_TEAMS; i++) {
		if (!players[i][0]) {
			Q_strncpyz(players[i], "(None)", sizeof(players[i]));
		}
	}

	qsort(level.sortedClients, level.numConnectedClients,
	      sizeof(level.sortedClients[0]), SortRanks);

	// set the rank value for all clients that are connected and not spectators
	if (g_gametype.integer >= GT_TEAM) {
		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for (i = 0;  i < level.numConnectedClients; i++) {
			cl = &level.clients[ level.sortedClients[i] ];
			if (level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE]) {
				cl->ps.persistant[PERS_RANK] = 2;
			} else if (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE]) {
				cl->ps.persistant[PERS_RANK] = 0;
			} else {
				cl->ps.persistant[PERS_RANK] = 1;
			}
		}
	} else {
		rank = -1;
		score = 0;
		for (i = 0;  i < level.numPlayingClients; i++) {
			cl = &level.clients[ level.sortedClients[i] ];
			newScore = cl->ps.persistant[PERS_SCORE];
			if (i == 0 || newScore != score) {
				rank = i;
				// assume we aren't tied until the next client is checked
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank;
			} else {
				// we are tied with the previous client
				level.clients[ level.sortedClients[i-1] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			score = newScore;
			if (g_gametype.integer == GT_SINGLE_PLAYER && level.numPlayingClients == 1) {
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
		}
	}

	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if (g_gametype.integer >= GT_TEAM) {
		// OSP: freeze tag and round-based modes also send the living players of each team
		if (IS_FREEZETAG || OSP_IsRoundBased(g_gametype.integer)) {
			trap_SetConfigstring(CS_SCORES1, va("%i %i", level.teamScores[TEAM_RED], level.numteamVotingClients[0]));
			trap_SetConfigstring(CS_SCORES2, va("%i %i", level.teamScores[TEAM_BLUE], level.numteamVotingClients[1]));
		} else {
			trap_SetConfigstring(CS_SCORES1, va("%i", level.teamScores[TEAM_RED]));
			trap_SetConfigstring(CS_SCORES2, va("%i", level.teamScores[TEAM_BLUE]));
		}
	} else {
		if (level.numConnectedClients == 0) {
			trap_SetConfigstring(CS_SCORES1, va("%i", SCORE_NOT_PRESENT));
			trap_SetConfigstring(CS_SCORES2, va("%i", SCORE_NOT_PRESENT));
		} else if (level.numConnectedClients == 1) {
			trap_SetConfigstring(CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE]));
			trap_SetConfigstring(CS_SCORES2, va("%i", SCORE_NOT_PRESENT));
		} else {
			trap_SetConfigstring(CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE]));
			trap_SetConfigstring(CS_SCORES2, va("%i", level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE]));
		}
	}

	OSP_HidePlayersInServerinfo(1);
	trap_Cvar_Set("Players_Red", players[TEAM_RED]);
	trap_Cvar_Set("Players_Blue", players[TEAM_BLUE]);
	trap_Cvar_Set("Players_Active", players[TEAM_FREE]);

	// OSP: callers can skip the exit check
	if (checkExit) {
		// see if it is time to end the level
		CheckExitRules();

		// if we are at the intermission, send the new info to everyone
		if (level.intermissiontime) {
			SendScoreboardMessageToAllClients();
		}
	}
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients(void) {
	int     i;

	for (i = 0 ; i < level.maxclients ; i++) {
		if (level.clients[ i ].pers.connected == CON_CONNECTED) {
			DeathmatchScoreboardMessage(g_entities + i);
		}
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission(gentity_t* ent) {
	// take out of follow mode if needed
	if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW) {
		StopFollowing(ent);
	}


	// move to the spot
	VectorCopy(level.intermission_origin, ent->s.origin);
	VectorCopy(level.intermission_origin, ent->client->ps.origin);
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pm_type = PM_INTERMISSION;

	// OSP: a flag carrier's hold time counts up to the intermission
	if (g_gametype.integer == GT_CTF && (ent->client->ps.powerups[PW_REDFLAG] || ent->client->ps.powerups[PW_BLUEFLAG])) {
		ent->client->pers.teamState.flagtime += level.time - ent->client->pers.teamState.flagsince;
	}

	// clean up powerup info
	memset(ent->client->ps.powerups, 0, sizeof(ent->client->ps.powerups));

	ent->client->ps.eFlags |= EF_TELEPORT_BIT;
	ent->s.eFlags = 0;
	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.event = 0;
	ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint(void) {
	gentity_t*   ent, *target;
	vec3_t      dir;

	// OSP: a viewcam position of the map config
	if (OSP_ViewcamRandomSpot(0)) {
		return;
	}

	// find the intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent) {      // the map creator forgot to put in an intermission point...
		SelectSpawnPoint (vec3_origin, level.intermission_origin, level.intermission_angle);
	} else {
		VectorCopy (ent->s.origin, level.intermission_origin);
		VectorCopy (ent->s.angles, level.intermission_angle);
		// if it has a target, look towards it
		if (ent->target) {
			target = G_PickTarget(ent->target);
			if (target) {
				VectorSubtract(target->s.origin, level.intermission_origin, dir);
				vectoangles(dir, level.intermission_angle);
			}
		}
	}

}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission(void) {
	int         i;
	gentity_t*   client;

	if (level.intermissiontime) {
		return;     // already active
	}

	level.intermissiontime = level.time;
	FindIntermissionPoint();

	// if single player game
	if (g_gametype.integer == GT_SINGLE_PLAYER) {
		UpdateTournamentInfo();
		SpawnModelsOnVictoryPads();
	}

	OSP_HidePlayersInServerinfo(0);

	// move all clients to the intermission point
	for (i=0 ; i< level.maxclients ; i++) {
		client = g_entities + i;
		if (!client->inuse)
			continue;
		// respawn if dead
		if (client->health <= 0) {
			respawn(client);
		}
		client->client->readyToExit = qfalse;
		if (client->client->pers.mvCount) {
			OSP_MV_RemoveAll(client);
		}
		MoveClientToIntermission(client);
	}

	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();

	// OSP: the next map, or the mode voted for
	if (!level.nextMode || (level.nextMode > 0 && level.voteTime)) {
		OSP_NextMap(0);
	} else if (level.nextMode > 0) {
		trap_SetConfigstring(CS_WARMUP, "0");
		trap_SendServerCommand(-1, va("print \"\nNext mode: ^3%s^7\n\"", OSP_ModeName(level.nextMode - 1)));
		OSP_SetGametype(level.nextMode - 1);
	}
}


/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar

=============
*/
void ExitLevel (void) {
	int     i;
	char*    s;
	char    nextmap[MAX_STRING_CHARS];
	int     mode;
	char    mapname[MAX_STRING_CHARS];
	char    serverinfo[MAX_STRING_CHARS];

	// OSP: a mode vote decides what comes next (custom modes exec their config)
	mode = level.nextMode - 1;
	if (mode >= osp_numModes && !level.voteTime) {
		if (OSP_CustomModeConfig(mode)[0]) {
			OSP_ResetCvarDefaults();
			OSP_SetMapSessionCount(0);
			trap_SendConsoleCommand(EXEC_APPEND, va("exec %s\n", OSP_CustomModeConfig(mode)));
			return;
		}
		// not a loaded custom mode: forget it and change map normally instead of waiting forever
		G_Printf("ExitLevel: mode %d is not a loaded custom mode, ignored.\n", mode);
		level.nextMode = 0;
	}

	if (level.nextMode > 0 && mode < osp_numModes) {
		trap_Cvar_Set("g_gametype", va("%d", osp_modes[mode].gametype));
		level.nextMode = 0;
	} else if (OSP_IsRoundBased(g_gametype.integer) && !osp_mapChangeReason &&
	           ((level.teamScores[TEAM_RED] < roundlimit.integer && level.teamScores[TEAM_BLUE] < roundlimit.integer) ||
	            !map_rotate.integer)) {
		// round-based match not over yet: next round on the same map
		OSP_ExitLevelSession();
		return;
	}

	if (osp_mapChangeReason || OSP_WinningTeam() || !g_doWarmup.integer) {
		level.osp_unk_3808 = (osp_mapChangeReason) ? 1 : 0;
		osp_roundsPlayed = 0;
		osp_matchResultPrint[0] = 0;
		// reset all the scores so we don't enter the intermission again
		level.teamScores[TEAM_RED] = 0;
		level.teamScores[TEAM_BLUE] = 0;
		level.osp_unk_1e00 = 1;
		OSP_UpdateScoreCvars("EndOfMatch");
	}

	s = NULL;
	trap_Cvar_VariableStringBuffer("nextmap", nextmap, sizeof(nextmap) - 1);
	nextmap[sizeof(nextmap) - 1] = 0;
	if ((s = strchr(nextmap, ' '))) {
		*s = 0;
		s++;
	}

	trap_GetServerinfo(serverinfo, sizeof(serverinfo));
	Q_strncpyz(mapname, Info_ValueForKey(serverinfo, "mapname"), sizeof(mapname));
	G_Printf("Current map: \"%s\"\n", mapname);
	G_Printf("Next map: %s\n", (!Q_stricmp(s, "0")) ? "SAME MAP" : va("\"%s\"", s));

	// same map: restart it
	if (!s || !Q_stricmp(mapname, s) || !Q_stricmp(s, "0")) {
		G_WriteSessionData();
		if (!level.restarted) {
			// if we are running a tournement map, kick the loser to spectator status,
			// which will automatically grab the next spectator and restart
			if (g_gametype.integer == GT_TOURNAMENT && !osp_mapChangeReason) {
				RemoveTournamentLoser();
			}
			level.restarted = qtrue;
			level.changemap = NULL;
			level.osp_unk_1dac = 9999;
			level.intermissiontime = 0;
			OSP_MapRestart();
		}
		return;
	}

	if (g_gametype.integer == GT_TOURNAMENT && !osp_mapChangeReason) {
		RemoveTournamentLoser();
	}

	// we need to do this here before chaning to CON_CONNECTING
	G_WriteSessionData();

	level.changemap = NULL;
	level.osp_unk_1dac = 9999;
	level.intermissiontime = 0;

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for (i = 0 ; i < level.maxclients ; i++) {
		if (level.clients[i].pers.connected == CON_CONNECTED) {
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}

	OSP_ExitLevelHelper();
	OSP_AdminLogPrintf(va("NextMap:\tnextmap\t(%s)", s));

	if (level.map_config[0]) {
		G_Printf("Execing %s from level.map_config\n", level.map_config);
		trap_SendConsoleCommand(EXEC_NOW, va("exec %s\n", level.map_config));
		G_UpdateCvars();
		trap_SendConsoleCommand(EXEC_APPEND, "wait; wait; vstr nextmap\n");
	} else {
		trap_SendConsoleCommand(EXEC_APPEND, "vstr nextmap\n");
	}
}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf(const char* fmt, ...) {
	va_list     argptr;
	char        string[1024];
	int         min, tens, sec;
	char*        line;

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	va_start(argptr, fmt);
	vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	// OSP: player stats lines ("PS:") only reach the console with log_stat > 1
	if (g_dedicated.integer) {
		if (strstr(string, "PS:") != string || log_stat.integer > 1) {
			G_Printf("%s", string);
		}
	}

	if (level.logFile) {
		// OSP: log_default 0 stamps the lines with seconds instead of min:sec
		if (log_default.integer) {
			line = va("%3i:%i%i %s", min, tens, sec, string);
		} else {
			line = va("%.1f %s", level.time / 1000.0f, string);
		}
		trap_FS_Write(line, strlen(line), level.logFile);
	}
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit(const char* string) {
	int             i, numSorted;
	gclient_t*       cl;
	char            reason[MAX_STRING_CHARS];
	char            serverinfo[MAX_INFO_STRING];

	// OSP: Game_End lines for the log and the admin log
	Q_strncpyz(reason, string, sizeof(reason));
	if (strstr(reason, "Timelimit")) {
		G_LogPrintf("Game_End: Timelimit\n");
		if (server_record.integer & 16) {
			trap_Cvar_Set("sv_dcount", va("%d", trap_Cvar_VariableIntegerValue("sv_dcount") + 1));
		}
	} else if (strstr(reason, "Fraglimit")) {
		G_LogPrintf("Game_End: Fraglimit\n");
		if (server_record.integer & 16) {
			trap_Cvar_Set("sv_dcount", va("%d", trap_Cvar_VariableIntegerValue("sv_dcount") + 1));
		}
	} else if (strstr(reason, "Capturelimit")) {
		G_LogPrintf("Game_End: Capturelimit\n");
		if (server_record.integer & 16) {
			trap_Cvar_Set("sv_dcount", va("%d", trap_Cvar_VariableIntegerValue("sv_dcount") + 1));
		}
	} else if (strstr(reason, "Scorelimit")) {
		G_LogPrintf("Game_End: Scorelimit\n");
		if (server_record.integer & 16) {
			trap_Cvar_Set("sv_dcount", va("%d", trap_Cvar_VariableIntegerValue("sv_dcount") + 1));
		}
	}

	trap_GetServerinfo(serverinfo, sizeof(serverinfo));
	OSP_AdminLogPrintf(va("Game_End\t%s\t%s\t%s", osp_modes[g_gametype.integer].name,
	                      Info_ValueForKey(serverinfo, "mapname"), reason));

	G_LogPrintf("Exit: %s\n", reason);

	level.intermissionQueued = level.time;

	// OSP: 1v1 wins / losses are counted here, unless the match ends with a map change
	if (g_gametype.integer == GT_TOURNAMENT && !strstr(reason, "map")) {
		AdjustTournamentScores();
	}

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	trap_SetConfigstring(CS_INTERMISSION, "1");

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numConnectedClients;
	if (numSorted > 32) {
		numSorted = 32;
	}

	if (g_gametype.integer >= GT_TEAM) {
		G_LogPrintf("red:%i  blue:%i\n",
		            level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]);
	}

	for (i=0 ; i < numSorted ; i++) {
		cl = &level.clients[level.sortedClients[i]];

		if (cl->sess.sessionTeam == TEAM_SPECTATOR) {
			continue;
		}
		if (cl->pers.connected == CON_CONNECTING) {
			continue;
		}

		G_LogPrintf("score: %i  ping: %i  client: %i %s\n", cl->ps.persistant[PERS_SCORE],
		            (cl->ps.ping < 999) ? cl->ps.ping : 999, level.sortedClients[i], cl->pers.netname);
	}
}


/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit(void) {
	// OSP's intermission actions, each done once
	static int  matchEndDone = 0;       // 1 lead sound, 2 match end stats, 4 final stats
	static int  intermissionDone = 0;   // 1 autoscreenshot, 2 clients cleared, 4 autorecord stopped
	int         ready, notReady;
	int         i;
	gclient_t*   cl;
	int         readyMask;
	int         numClients;

	if (osp_numViewcams && osp_viewcamRandomTime < level.time) {
		OSP_ViewcamRandomSpot(1);
	}

	if (g_gametype.integer >= GT_TEAM && !matchEndDone && level.time - level.intermissiontime > 550) {
		if (!osp_mapChangeReason) {
			if (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE]) {
				OSP_GlobalSound("sound/feedback/redleads.wav");
			} else if (level.teamScores[TEAM_RED] < level.teamScores[TEAM_BLUE]) {
				OSP_GlobalSound("sound/feedback/blueleads.wav");
			} else {
				OSP_GlobalSound("sound/feedback/teamstied.wav");
			}
		}
		matchEndDone = 1;
	}

	if (!(intermissionDone & 1) && level.time - level.intermissiontime > 800) {
		intermissionDone |= 1;
		OSP_AutoScreenshot();
	}
	if (!(intermissionDone & 2) && level.time - level.intermissiontime > 1000) {
		intermissionDone |= 2;
		OSP_ClearClients();
	}
	if (!(intermissionDone & 4) && level.time - level.intermissiontime > 6000) {
		intermissionDone |= 4;
		OSP_AutoRecordStop();
	}

	if (!(matchEndDone & 2) && level.time - level.intermissiontime > 1200) {
		if (OSP_WinningTeam()) {
			OSP_MatchEndStats(0, 1);
		}
		matchEndDone |= 2;
	}
	if (!(matchEndDone & 4) && (level.time - level.intermissiontime > 1800 ||
	                            (OSP_IsRoundBased(g_gametype.integer) && !OSP_WinningTeam()))) {
		OSP_MatchEndStats(1, 1);
		matchEndDone |= 4;
	}

	if (g_gametype.integer == GT_SINGLE_PLAYER) {
		return;
	}
	if (map_halt.integer && !osp_mapChangeReason) {
		return;
	}

	// see which players are ready
	ready = 0;
	notReady = 0;
	readyMask = 0;
	numClients = 0;
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if (cl->pers.connected != CON_CONNECTED) {
			continue;
		}
		numClients++;
		if (g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) {
			continue;
		}

		if (cl->readyToExit) {
			ready++;
			if (i < 16) {
				readyMask |= 1 << i;
			}
		} else {
			notReady++;
		}
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if (cl->pers.connected != CON_CONNECTED) {
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
	}

	// never exit in less than five seconds, nor during a vote
	if (level.time < level.intermissiontime + 5000 || level.voteTime) {
		return;
	}

	// clan arena: go after seven seconds
	if (g_gametype.integer == 5 && level.time > level.intermissiontime + 7000) {
		ExitLevel();
		return;
	}

	// if nobody wants to go, clear timer
	if (!ready && notReady) {
		level.readyToExit = qfalse;
		return;
	}

	// if everyone (or more than half of the players) wants to go, go now
	if (!notReady || (numClients && 100 * notReady / numClients < 50)) {
		level.osp_unk_1dac = 9999;
		ExitLevel();
		return;
	}

	// the first person to ready starts the seven second timeout
	if (!level.readyToExit) {
		level.readyToExit = qtrue;
		level.exitTime = level.time;
	}

	// if we have waited seven seconds since at least one player
	// wanted to exit, go ahead
	if (level.time < level.exitTime + 7000) {
		return;
	}

	level.osp_unk_1dac = 9999;
	ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied(void) {
	int     a, b;

	if (level.numPlayingClients < 2) {
		return qfalse;
	}

	if (g_gametype.integer >= GT_TEAM) {
		return level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE];
	}

	if (g_gametype.integer == GT_FFA) {
		return qfalse;
	}

	a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
	b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];

	return a == b;
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules(void) {
	int         i;
	gclient_t*   cl;

	if (level.osp_unk_1dac) {
		return;
	}

	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if (level.intermissiontime) {
		CheckIntermissionExit ();
		return;
	}

	if (level.intermissionQueued) {
		if (level.time - level.intermissionQueued >= INTERMISSION_DELAY_TIME) {
			level.intermissionQueued = 0;
			BeginIntermission();
			OSP_ClearCaptains();
		}
		return;
	}

	// OSP: empty server for ten minutes: back to the default settings / mode
	if (!level.numConnectedClients && level.time - level.clientChangeTime > 600000) {
		if (default_change.integer) {
			OSP_RestoreDefaultSettings();
			G_LogPrintf("Resetting back to default settings\n");
		}
		if (z_m_default.integer >= 0 && z_m_default.integer != z_m_current.integer) {
			G_Printf("RotateMode: Current: %s, Default: %s\n", OSP_ModeName(z_m_current.integer), OSP_ModeName(z_m_default.integer));
			level.nextMode = z_m_default.integer;
			if (level.nextMode >= osp_numModes) {
				trap_Cvar_Set("z_m_current", va("%d", level.nextMode));
				trap_Cvar_Update(&z_m_current);
				OSP_HidePlayersInServerinfo(0);
				OSP_SetMapSessionCount(0);
				if (OSP_CustomModeConfig(level.nextMode)[0]) {
					G_Printf(">> Execing default mode config: '%s'\n", OSP_CustomModeConfig(level.nextMode));
					trap_SendConsoleCommand(EXEC_APPEND, va("exec %s\n", OSP_CustomModeConfig(level.nextMode)));
				} else {
					G_Printf(">> Default mode %d is not a loaded custom mode, ignored.\n", level.nextMode);
				}
				return;
			} else {
				level.nextMode++;
				OSP_SetGametype(z_m_default.integer);
				OSP_LoadModeMaplist(z_m_default.integer, 0);
				OSP_SetMapIndex(0);
				OSP_NextMap(3);
				LogExit(va("Inactive default-config rotate: %s", OSP_ModeName(z_m_default.integer)));
				return;
			}
		}
	}

	if (OSP_IsRoundBased(g_gametype.integer)) {
		OSP_CA_RoundEnd();
		return;
	}

	if (!level.warmupTime) {
		if (IS_FREEZETAG) {
			FT_CheckRoundEnd();
		}

		if (g_timelimit.integer && osp_matchTime - level.startTime >= 60000 * (g_timelimit.integer + level.overtimeMinutes)) {
			CalculateRanks(0);
			// check for sudden death / overtime
			if (match_overtime.integer != 2 && ScoreIsTied()) {
				if ((match_overtime.integer == 1 || match_overtime.integer == 3) &&
				        (level.overtimeCount < match_overtimecount.integer || !match_overtimecount.integer)) {
					int     length;

					length = match_overtimelength.integer;
					level.osp_unk_1e00 = 1;
					OSP_UpdateScoreCvars("Overtime");
					if (level.overtimeCount) {
						OSP_GlobalSound("sound/world/klaxon2.wav");
					}
					length = (length < 1) ? 1 : length;
					level.overtimeMinutes += length;
					level.overtimeCount++;
					level.suddenDeath = 0;
					trap_SendServerCommand(-1, va("cp \"^3Overtime! ^5%d minut%s added\n\"", length, (length == 1) ? "e" : "es"));
					return;
				}
				if (match_overtime.integer != 3) {
					if (!level.suddenDeath) {
						level.osp_unk_1e00 = 1;
						level.suddenDeath = 1;
						if (level.overtimeCount) {
							OSP_GlobalSound("sound/world/tim_weird2.wav");
						}
						OSP_UpdateScoreCvars("Sudden Death");
						trap_SendServerCommand(-1, "cp \"^1Sudden Death Overtime!\n\"");
					}
					return;
				}
			}
			trap_SendServerCommand(-1, "print \"Timelimit hit.\n\"");
			LogExit("Timelimit hit.");
			level.osp_unk_1e00 = 1;
			OSP_UpdateScoreCvars("Timelimit");
			OSP_BuzzerSound();
			return;
		}

		if (g_gametype.integer == GT_CTF && g_capturelimit.integer) {
			if (level.teamScores[TEAM_RED] >= g_capturelimit.integer) {
				trap_SendServerCommand(-1, "print \"Red hit the capturelimit.\n\"");
				LogExit("Capturelimit hit.");
				level.osp_unk_1e00 = 1;
				OSP_UpdateScoreCvars("Caplimit: RED");
				return;
			}

			if (level.teamScores[TEAM_BLUE] >= g_capturelimit.integer) {
				trap_SendServerCommand(-1, "print \"Blue hit the capturelimit.\n\"");
				LogExit("Capturelimit hit.");
				level.osp_unk_1e00 = 1;
				OSP_UpdateScoreCvars("Caplimit: BLUE");
				return;
			}
			return;
		}

		if (g_fraglimit.integer) {
			if (level.teamScores[TEAM_RED] >= g_fraglimit.integer) {
				trap_SendServerCommand(-1, "print \"Red hit the fraglimit.\n\"");
				LogExit("Fraglimit hit.");
				level.osp_unk_1e00 = 1;
				OSP_UpdateScoreCvars("Fraglimit: RED");
				return;
			}

			if (level.teamScores[TEAM_BLUE] >= g_fraglimit.integer) {
				trap_SendServerCommand(-1, "print \"Blue hit the fraglimit.\n\"");
				LogExit("Fraglimit hit.");
				level.osp_unk_1e00 = 1;
				OSP_UpdateScoreCvars("Fraglimit: BLUE");
				return;
			}

			for (i=0 ; i< g_maxclients.integer ; i++) {
				cl = level.clients + i;
				if (cl->pers.connected != CON_CONNECTED) {
					continue;
				}
				if (cl->sess.sessionTeam != TEAM_FREE) {
					continue;
				}

				if (cl->ps.persistant[PERS_SCORE] >= g_fraglimit.integer) {
					char    name[256];

					LogExit("Fraglimit hit.");
					trap_SendServerCommand(-1, va("print \"%s" S_COLOR_WHITE " hit the fraglimit.\n\"",
					                              cl->pers.netname));
					SanitizeString(cl->pers.netname, name, 0);
					OSP_UpdateScoreCvars(va("Win: %s", name));
					return;
				}
			}
		}

		// OSP: tournament won by a score difference
		if (g_gametype.integer == GT_TOURNAMENT && match_winbyscore.integer > 0) {
			int     n = 0;
			int     scores[2] = { 0, 0 };
			int     clientNums[2] = { -1, -1 };
			char    name[256];

			for (i = 0 ; i < g_maxclients.integer && n < 2 ; i++) {
				cl = level.clients + i;
				if (cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam != TEAM_FREE) {
					continue;
				}
				scores[n] = cl->ps.persistant[PERS_SCORE];
				clientNums[n] = i;
				n++;
			}

			if (scores[0] == scores[1] || n != 2) {
				return;
			}
			if (scores[0] < scores[1]) {
				n = clientNums[1];
				if (scores[1] - scores[0] < match_winbyscore.integer) {
					return;
				}
			} else {
				n = clientNums[0];
				if (scores[0] - scores[1] < match_winbyscore.integer) {
					return;
				}
			}

			SanitizeString(level.clients[n].pers.netname, name, 0);
			trap_SendServerCommand(-1, va("print \"%s^7 wins the match by %d.\n\"", level.clients[n].pers.netname, match_winbyscore.integer));
			LogExit("Scorelimit hit.");
			level.osp_unk_1e00 = 1;
			OSP_UpdateScoreCvars(va("Win: %s", name));
		}
	}
}

/*
=================
OSP_IntervalPassed

OSP: true when a multiple of interval was passed during the last frame
(time is a time relative to some start, e.g. level.time - level.startTime)
=================
*/
int OSP_IntervalPassed(int time, int interval) {
	int     frameMsec;

	frameMsec = level.time - level.previousTime;
	return time % interval < (time - frameMsec) % interval;
}



/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/


/*
=============
CheckTournament

Once a frame, check for changes in tournement player state
(OSP: the warmup / ready / countdown / timeout logic of every mode)
=============
*/
void CheckTournament(void) {
	static int  lastIntermissionTime = 0;

	if (level.osp_unk_1dac) {
		return;
	}

	// OSP: the match clock in the scoreboard cvars
	if (g_timelimit.integer && level.time >= level.timeLeftTime && !level.intermissionQueued &&
	        !level.intermissiontime && !level.warmupTime && !osp_matchPaused) {
		int     left, secs, mins;

		left = (g_timelimit.integer * 60000 - (osp_matchTime - level.startTime)) / 1000;
		if (left < 0) {
			left = 0 - left;
		}
		secs = left % 60;
		mins = left / 60;
		level.timeLeftTime = level.time + level.timeLeftInterval;
		if (!level.overtimeCount && !level.suddenDeath) {
			OSP_UpdateScoreCvars(va("%d:%02d", mins, secs));
		} else {
			OSP_UpdateScoreCvars(va("OT %d:%02d", mins, secs));
		}
	}

	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if (!level.numConnectedClients || level.intermissionQueued || level.intermissiontime ||
	        lastIntermissionTime > level.intermissiontime) {
		lastIntermissionTime = level.intermissiontime;
		return;
	}

	if (g_gametype.integer == GT_TOURNAMENT) {

		// pull in a spectator if needed
		if (level.numNonSpectatorClients < 2) {
			AddTournamentPlayer();
		}

		// if we don't have two players, go back to "waiting for players"
		if (level.numPlayingClients < 2 ||
		        (level.warmupTime && !g_warmup.integer && !osp_matchPaused && !OSP_NumReady())) {
			if (!level.warmupTime) {
				trap_SendServerCommand(-1, "cp \"^1MATCH ABORTED!^7\nNot enough players...\n\"");
			}
			if (level.warmupTime != -1) {
				level.warmupTime = -1;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
				G_LogPrintf("Warmup:\n");
				osp_matchPaused = 0;
			}
			osp_matchState = 0;
			OSP_UpdateScoreCvars("Waiting for Players");
			return;
		}

		if (osp_matchPaused && osp_timeoutEndTime) {
			int     remaining;

			remaining = osp_timeoutEndTime - level.time;
			if (osp_timeoutEndTime < 0) {
				// paused by a referee: no end
				if (OSP_IntervalPassed(level.time, 10000)) {
					trap_SendServerCommand(-1, "cp \"Match paused by referee.\n\"");
				}
				return;
			}
			if (level.time > osp_timeoutEndTime) {
				osp_timeoutEndTime = 0;
				level.warmupTime = level.time + 5000;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
				trap_SendServerCommand(-1, "cp \"^1Timeout expired!^7 Match resuming...\n\"");
				return;
			}
			if (OSP_IntervalPassed(remaining, 10000) &&
			        (remaining <= 5000 || OSP_IntervalPassed(remaining, 1000))) {
				trap_SendServerCommand(-1, va("cp \"Match resuming in ^3%d^7 seconds.\n\"", remaining / 1000));
			}
			return;
		}

		if (level.warmupTime == 0) {
			return;
		}
		// if the warmup is changed at the console, restart it
		if (g_warmup.modificationCount != level.warmupModificationCount &&
		        (!OSP_NumReady() || level.numPlayingClients != 2) && !osp_matchPaused) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if (level.warmupTime < 0) {
			if (level.numPlayingClients == 2 && g_warmup.integer) {
				// fudge by -1 to account for extra delays
				level.warmupTime = level.time + (g_warmup.integer - 1) * 1000;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
			}
			osp_matchState = 1;
			return;
		}

		// if the warmup time has counted down, restart
		if (level.time > level.warmupTime) {
			if (!osp_matchPaused) {
				level.warmupTime += 10000;
				trap_Cvar_Set("g_restarted", "1");
				trap_SendServerCommand(-1, "print \"^3Match has begun!\n\"");
				level.osp_unk_3808 = 1;
				OSP_MapRestart();
				level.restarted = qtrue;
				if (!g_timelimit.integer) {
					OSP_UpdateScoreCvars("In Progress");
				}
				osp_matchState = 2;
				return;
			}
			OSP_UnpauseComplete();
			return;
		}
	} else if (OSP_IsRoundBased(g_gametype.integer)) {
		// not enough players on a team: back to warmup
		if (OSP_TeamPlayerCount(TEAM_RED) < 1 || OSP_TeamPlayerCount(TEAM_BLUE) < 1) {
			if (level.warmupTime != -1 || osp_roundsPlayed) {
				if (!level.warmupTime) {
					trap_SendServerCommand(-1, "cp \"^1MATCH ABORTED!^7\nNot enough players...\n\"");
				}
				osp_teamInfo[TEAM_RED].locked = 0;
				osp_teamInfo[TEAM_BLUE].locked = 0;
				osp_teamInfo[TEAM_RED].specLocked = 0;
				osp_teamInfo[TEAM_BLUE].specLocked = 0;
				OSP_RoundReset();
				osp_matchPaused = 0;
				level.warmupTime = -1;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
				G_LogPrintf("Warmup:\n");
				osp_roundsPlayed = 0;
				osp_unk_1c054 = 0;
				osp_matchResultPrint[0] = 0;
				level.teamScores[TEAM_RED] = 0;
				level.teamScores[TEAM_BLUE] = 0;
				trap_Cvar_Set("g_restarted", "0");
				trap_Cvar_Set("Score_Red", "WARMUP");
				trap_Cvar_Set("Score_Blue", "WARMUP");
				OSP_UpdateScoreCvars("Waiting for Players");
			}
			osp_matchState = 0;
			return;
		}

		if (osp_matchPaused && osp_timeoutEndTime) {
			int     remaining;

			remaining = osp_timeoutEndTime - level.time;
			if (osp_timeoutEndTime < 0) {
				// paused by a referee: no end
				if (OSP_IntervalPassed(level.time, 10000)) {
					trap_SendServerCommand(-1, "cp \"Match paused by referee.\n\"");
				}
				return;
			}
			if (level.time > osp_timeoutEndTime) {
				osp_timeoutEndTime = 0;
				level.warmupTime = level.time + 5000;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
				trap_SendServerCommand(-1, "cp \"^1Timeout expired!^7 Match resuming...\n\"");
				return;
			}
			if (OSP_IntervalPassed(remaining, 10000) &&
			        (remaining <= 5000 || OSP_IntervalPassed(remaining, 1000))) {
				trap_SendServerCommand(-1, va("cp \"Match resuming in ^3%d^7 seconds.\n\"", remaining / 1000));
			}
			return;
		}

		if (level.warmupTime == 0) {
			return;
		}
		// if the warmup is changed at the console, restart it
		if (g_warmup.modificationCount != level.warmupModificationCount && !OSP_NumReady() && !osp_matchPaused) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// start the countdown of the next round
		if (level.warmupTime < 0) {
			int     warmup;

			warmup = 0;
			if (!osp_roundsPlayed && !osp_unk_1c054) {
				warmup = g_warmup.integer;
			}
			if (warmup < match_roundtimer.integer) {
				warmup = match_roundtimer.integer;
			}
			if (warmup < 5) {
				warmup = 5;
			}
			if ((osp_roundsPlayed || osp_unk_1c054) && warmup > 20) {
				warmup = 20;
			}
			level.warmupTime = level.time + (warmup - 1) * 1000;
			trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
			OSP_UpdateScoreCvars("Starting");
			trap_Cvar_Set("Score_Red", va("%d", level.teamScores[TEAM_RED]));
			trap_Cvar_Set("Score_Blue", va("%d", level.teamScores[TEAM_BLUE]));
			osp_matchState = 1;
			return;
		}

		// the round starts
		if (level.time > level.warmupTime) {
			if (!osp_matchPaused) {
				OSP_UpdateScoreCvars(va("Round %d/%d", osp_roundsPlayed + 1, roundlimit.integer * 2 - 1));
				osp_unk_1c054 = 1;
				if (match_carespawn.integer) {
					level.warmupTime += 10000;
					trap_Cvar_Set("g_restarted", "1");
					trap_SendServerCommand(-1, "print \"^3Match has begun!\n\"");
					level.osp_unk_3808 = !osp_roundsPlayed;
					G_WriteSessionData();
					OSP_MapRestart();
					level.restarted = qtrue;
					osp_matchState = 2;
					return;
				}
				OSP_RoundStart();
				return;
			}
			OSP_UnpauseComplete();
			return;
		}
	} else if (g_gametype.integer != GT_SINGLE_PLAYER && (g_doWarmup.integer || osp_matchPaused)) {
		qboolean    notEnough = qfalse;

		if (g_gametype.integer >= GT_TEAM) {
			if (TeamCount(-1, TEAM_RED) < 1 || TeamCount(-1, TEAM_BLUE) < 1) {
				notEnough = qtrue;
			}
			if (!osp_matchState && g_doWarmup.integer && !g_warmup.integer && match_latejoin.integer && !OSP_NumReady()) {
				notEnough = qtrue;
			}
		} else if (level.numPlayingClients < 2 && !osp_matchPaused) {
			notEnough = qtrue;
		}

		if (notEnough) {
			if (level.warmupTime != -1) {
				if (!level.warmupTime) {
					trap_SendServerCommand(-1, "cp \"^1MATCH ABORTED!^7\nNot enough players...\n\"");
				}
				osp_teamInfo[TEAM_RED].locked = 0;
				osp_teamInfo[TEAM_BLUE].locked = 0;
				osp_teamInfo[TEAM_RED].specLocked = 0;
				osp_teamInfo[TEAM_BLUE].specLocked = 0;
				OSP_RoundReset();
				osp_matchPaused = 0;
				level.warmupTime = -1;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
				G_LogPrintf("Warmup:\n");
				if (g_gametype.integer >= GT_TEAM) {
					level.teamScores[TEAM_RED] = 0;
					level.teamScores[TEAM_BLUE] = 0;
					if (g_gametype.integer >= GT_TEAM) {
						trap_Cvar_Set("Score_Red", "WARMUP");
						trap_Cvar_Set("Score_Blue", "WARMUP");
						OSP_UpdateScoreCvars("Waiting for Players");
					}
				}
			}
			osp_matchState = 0;
			return; // still waiting for team members
		}

		if (osp_matchPaused && osp_timeoutEndTime) {
			int     remaining;

			remaining = osp_timeoutEndTime - level.time;
			if (osp_timeoutEndTime < 0) {
				// paused by a referee: no end
				if (OSP_IntervalPassed(level.time, 10000)) {
					trap_SendServerCommand(-1, "cp \"Match paused by referee.\n\"");
				}
				return;
			}
			if (level.time > osp_timeoutEndTime) {
				osp_timeoutEndTime = 0;
				level.warmupTime = level.time + 5000;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
				trap_SendServerCommand(-1, "cp \"^1Timeout expired!^7 Match resuming...\n\"");
				return;
			}
			if (OSP_IntervalPassed(remaining, 10000) &&
			        (remaining <= 5000 || OSP_IntervalPassed(remaining, 1000))) {
				trap_SendServerCommand(-1, va("cp \"Match resuming in ^3%d^7 seconds.\n\"", remaining / 1000));
			}
			return;
		}

		if (level.warmupTime == 0) {
			return;
		}
		// if the warmup is changed at the console, restart it
		if (g_warmup.modificationCount != level.warmupModificationCount && !OSP_NumReady() && !osp_matchPaused) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if (level.warmupTime < 0) {
			if (g_warmup.integer) {
				// fudge by -1 to account for extra delays
				level.warmupTime = level.time + (g_warmup.integer - 1) * 1000;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
				osp_matchState = 1;
				if (g_gametype.integer >= GT_TEAM) {
					OSP_UpdateScoreCvars("Starting");
					trap_Cvar_Set("Score_Red", va("%d", level.teamScores[TEAM_RED]));
					trap_Cvar_Set("Score_Blue", va("%d", level.teamScores[TEAM_BLUE]));
				}
			}
			return;
		}

		// if the warmup time has counted down, restart
		if (level.time > level.warmupTime) {
			if (!osp_matchPaused) {
				level.warmupTime += 10000;
				trap_Cvar_Set("g_restarted", "1");
				trap_SendServerCommand(-1, "print \"^3Match has begun!\n\"");
				level.osp_unk_3808 = 1;
				OSP_MapRestart();
				level.restarted = qtrue;
				if (!g_timelimit.integer && g_gametype.integer >= GT_TEAM) {
					OSP_UpdateScoreCvars("In Progress");
				}
				osp_matchState = 2;
				return;
			}
			OSP_UnpauseComplete();
			return;
		}
	}

	// no warmup: the match is always on
	if (g_gametype.integer == GT_SINGLE_PLAYER || !g_doWarmup.integer) {
		osp_matchState = 2;
	}
}


/*
==================
CheckVote
==================
*/
void CheckVote(void) {
	if (!level.voteTime || level.time - level.voteTime < 1000) {
		return;
	}
	if (level.time - level.voteTime >= VOTE_TIME) {
		if (osp_voteType == 27) {
			level.nextMode = 0;
		}
		osp_mapIndexSet = 0;
		trap_SendServerCommand(-1, va("print \"^2Vote FAILED! ^3(%s)\n\"", level.voteString));
		G_LogPrintf("Vote Failed: %s\n", level.voteString);
		OSP_AdminLogPrintf(va("Vote_Failed\t%s\t(Time Expired)", level.voteString));
	} else {
		int     percent;

		percent = vote_percent.integer;
		if (percent > 99) {
			percent = 99;
		}
		if (percent < 1) {
			percent = 1;
		}

		if (level.voteYes > percent * level.numVotingClients / 100) {
			// more yes votes than voters: a referee decision
			if (level.voteYes > level.numVotingClients + 1) {
				if (osp_voteType != 21) {
					trap_SendServerCommand(-1, va("print \"^5Referee changed setting! ^7(%s)\n\"", level.voteString));
				}
				G_LogPrintf("Referee Setting: %s\n", level.voteString);
				OSP_AdminLogPrintf(va("Referee_Setting\t%s", level.voteString));
			} else {
				trap_SendServerCommand(-1, "print \"^5Vote passed!\n\"");
				G_LogPrintf("Vote Passed: %s\n", level.voteString);
				OSP_AdminLogPrintf(va("Vote_Passed\t%s", level.voteString));
			}
			if (!OSP_Vote_Execute()) {
			}
		} else if (level.voteNo && level.voteNo >= (100 - percent) * level.numVotingClients / 100) {
			if (osp_voteType == 27) {
				level.nextMode = 0;
			}
			osp_mapIndexSet = 0;
			trap_SendServerCommand(-1, va("print \"^2Vote FAILED! ^3(%s)\n\"", level.voteString));
			G_LogPrintf("Vote Failed: %s\n", level.voteString);
			OSP_AdminLogPrintf(va("Vote_Failed\t%s\t(Player(s) Decline)", level.voteString));
		} else {
			// still waiting for a majority
			return;
		}
	}
	osp_voteType = 0;
	level.voteTime = 0;
	trap_SetConfigstring(CS_VOTE_TIME, "");
	trap_SetConfigstring(CS_VOTE_STRING, "");    // otherwise the last vote text stays in every later gamestate

}

/*
==================
PrintTeam
==================
*/
void PrintTeam(int team, char* message) {
	int i;

	for (i = 0 ; i < level.maxclients ; i++) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		trap_SendServerCommand(i, message);
	}
}

/*
==================
SetLeader
==================
*/
void SetLeader(int team, int client) {
	int i;

	if (level.clients[client].pers.connected == CON_DISCONNECTED) {
		PrintTeam(team, va("print \"%s is not connected\n\"", level.clients[client].pers.netname));
		return;
	}
	if (level.clients[client].sess.sessionTeam != team) {
		PrintTeam(team, va("print \"%s is not on the team anymore\n\"", level.clients[client].pers.netname));
		return;
	}
	for (i = 0 ; i < level.maxclients ; i++) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		if (level.clients[i].sess.teamLeader) {
			level.clients[i].sess.teamLeader = qfalse;
			ClientUserinfoChanged(i);
		}
	}
	level.clients[client].sess.teamLeader = qtrue;
	ClientUserinfoChanged(client);
	// OSP 1.03a: no "is the new team leader" print (id has one here)
}


/*
==================
CheckCvars
==================
*/
void CheckCvars(void) {
	static int lastMod = -1;

	if (g_password.modificationCount != lastMod) {
		lastMod = g_password.modificationCount;
		if (*g_password.string && Q_stricmp(g_password.string, "none")) {
			trap_Cvar_Set("g_needpass", "1");
		} else {
			trap_Cvar_Set("g_needpass", "0");
		}
	}
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink (gentity_t* ent) {
	float   thinktime;

	// OSP: a pause delays the think of everything but the clients
	if (osp_matchPaused && ent - g_entities >= g_maxclients.integer && ent->nextthink > level.time) {
		ent->nextthink += level.time - level.previousTime;
	}

	thinktime = ent->nextthink;
	if (thinktime <= 0) {
		return;
	}
	if (thinktime > level.time) {
		return;
	}

	ent->nextthink = 0;
	if (!ent->think) {
		G_Error (va("NULL ent->think (ent=%d)", ent - g_entities));
	}
	ent->think (ent);
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame(int levelTime) {
	int         i;
	gentity_t*   ent;

	// if we are waiting for the level to restart, do nothing
	if (level.restarted) {
		return;
	}

	if (osp_restart.integer) {
		OSP_InitGame();
		return;
	}

	// OSP: the match clock stands still during a pause
	if (!osp_matchPaused) {
		osp_matchTime = levelTime - osp_pausedTime;
	} else {
		osp_pausedTime = levelTime - osp_matchTime;
	}

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;

	// get any cvar changes (OSP: every 3 seconds)
	if (level.time > level.cvarUpdateTime) {
		G_UpdateCvars();
		level.cvarUpdateTime += 3000;
	}

	if (osp_numViewcamClients && !level.intermissiontime) {
		OSP_ViewcamRunFrame();
	}

	//
	// go through all allocated objects
	//
	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) {
		if (!ent->inuse) {
			continue;
		}

		if (ent->client && ent->osp_unk_228) {
			ent->osp_unk_228 = 0;
			ent->client->ps.externalEvent = 0;
			ent->client->ps.events[0] = 0;
			ent->client->ps.events[1] = 0;
		}

		// clear events that are too old
		if (level.time - ent->eventTime > EVENT_VALID_MSEC) {
			if (ent->s.event) {
				ent->s.event = 0;   // &= EV_EVENT_BITS;
				if (ent->client) {
					ent->client->ps.externalEvent = 0;
					// predicted events should never be set to zero
					//ent->client->ps.events[0] = 0;
					//ent->client->ps.events[1] = 0;
				}
			}
			if (ent->freeAfterEvent) {
				// tempEntities or dropped items completely go away after their event
				G_FreeEntity(ent);
				continue;
			} else if (ent->unlinkAfterEvent) {
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				trap_UnlinkEntity(ent);
			}
		}

		// temporary entities don't think
		if (ent->freeAfterEvent) {
			continue;
		}

		if (!ent->r.linked && ent->neverFree) {
			continue;
		}

		if (ent->s.eType == ET_MISSILE) {
			if (!osp_matchPaused) {
				G_RunMissile(ent);
			} else {
				ent->s.pos.trTime += level.time - level.previousTime;
				G_RunThink(ent);
			}
			continue;
		}

		if (ent->s.eType == ET_ITEM || ent->physicsObject) {
			G_RunItem(ent);
			continue;
		}

		if (ent->s.eType == ET_MOVER) {
			G_RunMover(ent);
			continue;
		}

		if (i < MAX_CLIENTS) {
			G_RunClient(ent);
			continue;
		}

		G_RunThink(ent);
	}

	// perform final fixups on the players
	ent = &g_entities[0];
	for (i=0 ; i < level.maxclients ; i++, ent++) {
		if (ent->inuse) {
			ClientEndFrame(ent);
			ent->osp_unk_228 = 1;
		}
	}

	// see if it is time to end the level
	CheckExitRules();

	// see if it is time to do a tournement restart
	CheckTournament();

	// update to team status?
	CheckTeamStatus();

	// cancel vote if timed out
	CheckVote();

	// for tracking changes
	CheckCvars();

	// OSP: every 30 seconds of warmup, see if everybody is ready
	if (level.warmupTime && (level.time / 1000) % 30 == 0 && !osp_matchPaused &&
	        level.time > 30000 && level.numPlayingClients > 1) {
		if (!OSP_NumReady() && (!g_warmup.integer || level.warmupTime > level.time + 10000)) {
			OSP_CheckReady();
		}
	}
}

/*
================
OSP_ResetCvarDefaults

OSP: resets the cvars flagged osp_setDefault to their default values
================
*/
void OSP_ResetCvarDefaults(void) {
	int         i;
	cvarTable_t* cv;

	OSP_HidePlayersInServerinfo(0);
	for (i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++) {
		if (cv->vmCvar && cv->osp_setDefault) {
			trap_Cvar_Set(cv->cvarName, cv->defaultString);
		}
	}
	G_UpdateCvars();
}
