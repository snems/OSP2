/*
 * g_osp_vote.c: callvote parsing (OSP_Vote_Parse, called by Cmd_CallVote_f for
 * "callvote" and "ref <vote>"), the vote help / usage messages, and the execution of a passed
 * vote by osp_voteType (OSP_Vote_Execute, from CheckVote). The code is close to Wolfenstein: ET's
 * g_vote.c, which comes from OSP.
 */

#include "g_local.h"

// OSP: functions of other files used here
extern  void    LogExit(const char* string);         // g_main.c

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t    vote_percent;
vmCvar_t    vote_limit;
vmCvar_t    vote_allow_weaponrespawn;
vmCvar_t    vote_allow_timenudge;
vmCvar_t    vote_allow_timelimit;
vmCvar_t    vote_allow_thrufloors;
vmCvar_t    vote_allow_teamdamage;
vmCvar_t    vote_allow_roundlimit;
vmCvar_t    vote_allow_respawn;
vmCvar_t    vote_allow_reload;
vmCvar_t    vote_allow_referee;
vmCvar_t    vote_allow_promode;
vmCvar_t    vote_allow_powerups;
vmCvar_t    vote_allow_poweruprespawn;
vmCvar_t    vote_allow_pmove;
vmCvar_t    vote_allow_overtime;
vmCvar_t    vote_allow_osphud;
vmCvar_t    vote_allow_notimer;
vmCvar_t    vote_allow_mutespecs;
vmCvar_t    vote_allow_machinegun;
vmCvar_t    vote_allow_maxpackets;
vmCvar_t    vote_allow_map;
vmCvar_t    vote_allow_lgcooldown;
vmCvar_t    vote_allow_kick;
vmCvar_t    vote_allow_items;
vmCvar_t    vote_allow_instagib;
vmCvar_t    vote_allow_hook;
vmCvar_t    vote_allow_fraglimit;
vmCvar_t    vote_allow_fastrail;
vmCvar_t    vote_allow_falling;
vmCvar_t    vote_allow_dropitems;
vmCvar_t    vote_allow_cq3;
vmCvar_t    vote_allow_ca_warmupfire;
vmCvar_t    vote_allow_ca_startrespawn;
vmCvar_t    vote_allow_capturelimit;
vmCvar_t    vote_allow_capfps;
vmCvar_t    vote_allow_armordamage;
vmCvar_t    vote_allow_altgraphics;
vmCvar_t    vote_allow_allcaptain;
// OSP: the bss below this file's cvars, around the globals declared in g_local.h
char    osp_voteMap[384];                       // map of a "mode" vote (OSP_Vote_Parse, OSP_Vote_Execute)
int     osp_numViewcams;
int     osp_viewcamCurrent;
int     osp_viewcamLast;
int     osp_viewcamRandomSpot;
int     osp_viewcamRandomTime;
int     osp_numViewcamClients;
// OSP: 256 bytes of bss above the cvars (the end of the bss before the stack);
// declared in g_local.h first
// so that lcc emits it last
char    osp_voteValue[256];     // argument of the vote being called (Com_sprintf of arg2 in the vote handlers)

// OSP: initialised data. Setting names printed by the toggle votes (as Wolfenstein: ET's
// g_vote.c, whose vote code comes from OSP).
static char* ALLOW = "ALLOW";
static char* ENABLE = "ENABLE";
static char* DISABLE = "DISABLE";
static char* ENABLED = "ENABLED";
static char* DISABLED = "DISABLED";

// OSP: g_session.c's global (OSP_Vote_MapExec), declared here and not in g_local.h
// so that g_session.c's bss order is unchanged
extern int  osp_unk_1c054;

// callvote table (help list of OSP_Vote_Help, usage texts of OSP_Vote_Usage)
ospVoteInfo_t   osp_voteTable[] = {
	{ 0x78, "allcaptain", " <0|1>^7\n  Allows all players to have limited team control commands" },
	{ 0xfb, "altgraphics", " <0|1>^7\n  Toggles clients' ability to use alternate graphics" },
	{ 0x20, "armordamage", " <0|1>^7\n  Specifies if armor is stripped on self-inflicted damage" },
	{ 0xdb, "bfg", " <0|1>^7\n  Enables/disables BFG weapon" },
	{ 0xdb, "bsuit", " <0|1>^7\n  Enables/disables BattleSuit powerup availability" },
	{ 0xfb, "capfps", " <0|1>^7\n  Toggles client FPS capping at 125" },
	{ 0x10, "cl/capturelimit", " <value>^7\n  Changes the current Capturelimit" },
	{ 0xfb, "clanbase", " [mapname]^7\n  Reloads default settings for ^2ClanBase TeamDM^7 mode.\n  Optional [mapname] specifies map to laod." },
	{ 0xfb, "cq3", " <0|1>^7\n  Toggles refined CQ3 settings" },
	{ 0x58, "dropitems", " <0|1>^7\n  Ability to drop weapons/ammo/flags" },
	{ 0xfb, "fallingdamage", " <0|1>^7\n  Enables/disables falling damage" },
	{ 0xfb, "fastrail", " <0|1>^7\n  CPM fast-rail switching\n  ^50 ^2-^7 DISABLE (Slow: 1.5 sec)\n  ^51 ^2-^7 ENABLE (Fast: 1.0 sec)" },
	{ 0xdb, "fl/fraglimit", " <value>^7\n  Changes the current fraglimit" },
	{ 0xfb, "hook", " <0|1>^7\n  Toggles hook/grapple availability" },
	{ 0xdb, "invis", " <0|1>^7\n  Enables/disables Invisibility powerup availability" },
	{ 0xfb, "instagib", " <0|1>^7\n  Enables/disables railgun-only instagib" },
	{ 0xfb, "kick", " <player_id>^7\n  Attempts to kick player from server" },
	{ 0xfb, "lgcooldown", " <0|1>^7\n  CPM LightningGun cooldown option\n  ^50 ^2-^7 DISABLE (No forced switch delay)\n  ^51 ^2-^7 ENABLE (200ms switch delay - 100ms if fastrail enabled)" },
	{ 0xfb, "map", " <map_id|mapname>^7\n  Votes for a new map to be loaded" },
	{ 0xfb, "map_restart", "^7\n  Restarts the current map" },
	{ 0xfb, "maxpackets_max", " <value>^7\n  Sets max cl_maxpackets setting for clients (0 disables)" },
	{ 0xfb, "maxpackets_min", " <value>^7\n  Sets min cl_maxpackets setting for clients (0 disables)" },
	{ 0xdb, "medkit", " <0|1>^7\n  Enables/disables MedKit powerup availability" },
	{ 0xfb, "mg_damage", " <value>^7\n  Sets damage for machinegun on a per bullet basis" },
	{ 0xfb, "mg_startammo", " <value>^7\n  Sets ammo amount a player spawns with the machinegun" },
	{ 0xfb, "mode", " <mode_id> [mapname]^7\n  Loads the specified server configuration (with optional load map option)" },
	{ 0xfb, "mutespecs", " <0|1>^7\n  Mutes spectator chat to players in the game" },
	{ 0xfb, "nextmap", "^7\n  Loads the next map in the map queue" },
	{ 0xfb, "notimer", " <0|1>^7\n  Disables the drawing of the match timer" },
	{ 0xfb, "osphud", " <0|1>^7\n  Toggles clients' ability to use cg_enableOSPHUD" },
	{ 0xdb, "overtime", " <0|1|2|3>^7\n  Sets overtime mode\n\n   ^50 ^2-^7 Sudden Death (first score decides)\n   ^51 ^2-^7 SD-Timed Overtime (sudden death after X rounds)\n   ^52 ^2-^7 No Overtime (can end in tie)\n   ^53 ^2-^7 Tie-Timed overtime (tie after X rounds)" },
	{ 0xfb, "pmove", " <0|1>^7\n  Enables/disables enhanced client sampling" },
	{ 0xdb, "poweruprespawn", "<value>^7\n  Sets power respawn delay (60, 90, 120s) [CPM only]" },
	{ 0xfb, "promode", " <0|1>^7\n  Toggles faster-paced CPM/ProMode Quake3 settings" },
	{ 0xdb, "quad", " <0|1>^7\n  Enables/disables Quad Damage powerup availability" },
	{ 0xfb, "referee", " <player_id>^7\n  Elects a player to have admin abilities" },
	{ 0xdb, "regen", " <0|1>^7\n  Enables/disables Regeneration powerup availability" },
	{ 0xfb, "reload", "<value>^7\n  Sets RG reload time for Instagib games (in ms, default is 1500)" },
	{ 0xfb, "respawn", " <0|1>^7\n  Sets value of maximum forced respawn delay" },
	{ 0x20, "rl/roundlimit", " <value>^7\n  Changes the current roundlimit" },
	{ 0xfb, "set1v1", " [mapname]^7\n  Reloads default settings for ^21v1^7 mode.\n  Optional [mapname] specifies map to laod." },
	{ 0xfb, "settdm", " [mapname]^7\n  Reloads default settings for ^2Teamplay DM^7 mode.\n  Optional [mapname] specifies map to laod." },
	{ 0xfb, "setctf", " [mapname]^7\n  Reloads default settings for ^2CTF^7 mode.\n  Optional [mapname] specifies map to laod." },
	{ 0xfb, "setca", " [mapname]^7\n  Reloads default settings for ^2ClanArena^7 mode.\n  Optional [mapname] specifies map to laod." },
	{ 0xfb, "setffa", " [mapname]^7\n  Reloads default settings for ^2Free-For-All DM^7 mode.\n  Optional [mapname] specifies map to laod." },
	{ 0xfb, "setft", " [mapname]^7\n  Reloads default settings for ^2FreezeTag TDM^7 mode.\n  Optional [mapname] specifies map to laod." },
	{ 0xfb, "setftv", " [mapname]^7\n  Reloads default settings for ^2FreezeTag TDM (Vanilla)^7 mode.\n  Optional [mapname] specifies map to laod." },
	{ 0x20, "startrespawn", " <0|1>^7\n  Specifies if clients are respawned on round start" },
	{ 0xf8, "teamdamage", " <0|1>^7\n  Toggles ability to hurt teammates" },
	{ 0xdb, "teleporter", " <0|1>^7\n  Enables/disables Personal Teleporter powerup availability" },
	{ 0xfb, "thrufloors", " <0|1>^7\n  Toggles if splash damage can transfer through floors/walls" },
	{ 0xdb, "tl/timelimit", " <value>^7\n  Changes the current timelimit" },
	{ 0xfb, "timenudge_max", " <value>^7\n  Sets max cl_timenudge setting for clients (0 disables)" },
	{ 0xfb, "timenudge_min", " <value>^7\n  Sets min cl_timenudge setting for clients (0 disables)" },
	{ 0xfb, "unreferee", " <player_id>^7\n  Elects a player to have admin abilities removed" },
	{ 0x20, "warmupfire", " <0|1>^7\n  Specifies if players can shoot weapons during warmup" },
	{ 0xdb, "weaponrespawn", " <value>^7\n  Selects weapon respawn time (in seconds)" },
	{ 0, NULL, NULL }           // terminator (OSP_Vote_Help counts sizeof - 1)
};

/*
==================
OSP_Vote_Help

Lists the votes of the current mode in four columns (osp_voteTable gametypeMask);
fShowVote adds the header and the usage line (Cmd_CallVote_f), OSP_Ref_Help passes 0
==================
*/
void OSP_Vote_Help(gentity_t* ent, qboolean fShowVote) {
	int     i, rows;
	int     vi[100];
	int     num_cmds;

	if (fShowVote) {
		trap_SendServerCommand(ent - g_entities, "print \"\nValid ^3callvote^7 commands are:\n^3----------------------------\n\"");
	}

	i = 0;
	num_cmds = sizeof(osp_voteTable) / sizeof(osp_voteTable[0]) - 1;
	rows = 0;
	for (i = 0; i < num_cmds; i++) {
		if (osp_voteTable[i].gametypeMask & (1 << g_gametype.integer)) {
			vi[rows++] = i;
		}
	}

	num_cmds = rows;
	rows = num_cmds / 4;
	if (num_cmds % 4) {
		rows++;
	}
	if (rows < 0) {
		return;
	}

	for (i = 0; i < rows; i++) {
		if (i + rows * 3 + 1 <= num_cmds) {
			trap_SendServerCommand(ent - g_entities, va("print \"^5%-17s%-17s%-17s%-17s\n\"", osp_voteTable[vi[i]].name,
			                       osp_voteTable[vi[i + rows]].name, osp_voteTable[vi[i + rows * 2]].name, osp_voteTable[vi[i + rows * 3]].name));
		} else if (i + rows * 2 + 1 <= num_cmds) {
			trap_SendServerCommand(ent - g_entities, va("print \"^5%-17s%-17s%-17s\n\"", osp_voteTable[vi[i]].name,
			                       osp_voteTable[vi[i + rows]].name, osp_voteTable[vi[i + rows * 2]].name));
		} else if (i + rows + 1 <= num_cmds) {
			trap_SendServerCommand(ent - g_entities, va("print \"^5%-17s%-17s\n\"", osp_voteTable[vi[i]].name,
			                       osp_voteTable[vi[i + rows]].name));
		} else {
			trap_SendServerCommand(ent - g_entities, va("print \"^5%-17s\n\"", osp_voteTable[vi[i]].name));
		}
	}

	if (fShowVote) {
		trap_SendServerCommand(ent - g_entities, "print \"\nUsage: ^3\\callvote <command> <params>\n^7For current settings/help, use: ^3\\callvote <command> ?\n\n\"");
	}
}

/*
==================
OSP_Vote_Usage

"<vote> ?" or no argument: prints the usage of osp_voteTable[vote] and returns 1
==================
*/
int OSP_Vote_Usage(gentity_t* ent, char* cmd, int vote) {
	char    arg[MAX_TOKEN_CHARS];

	if (!ent) {
		return 0;
	}

	trap_Argv(2, arg, sizeof(arg));
	if (!Q_stricmp(arg, "?") || trap_Argc() == 2) {
		trap_Argv(1, arg, sizeof(arg));
		trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", cmd, arg, osp_voteTable[vote].help));
		return 1;
	}

	return 0;
}

/*
==================
OSP_Vote_Disabled
==================
*/
void OSP_Vote_Disabled(gentity_t* ent, char* name) {
	trap_SendServerCommand(ent - g_entities, va("print \"\nSorry, ^3%s^7 voting has been disabled\n\"", name));
}

/*
==================
OSP_Vote_ProModeOnly

-1 (and a message) unless the ProMode physics run
==================
*/
int OSP_Vote_ProModeOnly(gentity_t* ent, char* name) {
	if (!(level.physicsMode & 1)) {
		trap_SendServerCommand(ent - g_entities, va("print \"^3%s^7 voting is only enabled in ProMode!\n\"", name));
		return -1;
	}

	return 0;
}

/*
==================
OSP_PrintInvalidPlayerID
==================
*/
void OSP_PrintInvalidPlayerID(gentity_t* ent) {
	trap_SendServerCommand(ent - g_entities, "print \"Use the ^3players^7 command to find a valid player ID.\n\"");
}

/*
==================
OSP_Vote_CurrentValue
==================
*/
void OSP_Vote_CurrentValue(gentity_t* ent, char* name, char* value) {
	trap_SendServerCommand(ent - g_entities, va("print \"^2%s^7 is currently ^3%s\n\n\"", name, value));
}

/*
==================
OSP_OvertimeModeName

Name of match_overtime mode (fLong: with match_overtimelength / match_overtimecount)
==================
*/
char* OSP_OvertimeModeName(qboolean fLong, int mode) {
	switch (mode) {
		case 0:
			if (!fLong) {
				return "Sudden Death";
			}
			return "Sudden Death (first score decides)";
		case 1:
			if (!fLong) {
				return "SD-Timed Round";
			}
			if (match_overtimecount.integer) {
				return va("SD-Timed Overtime (%d min rounds - sudden death after %d rounds)", match_overtimelength.integer, match_overtimecount.integer);
			} else {
				return va("SD-Timed Overtime (%d min rounds - play until winner)", match_overtimelength.integer);
			}
		case 2:
			if (!fLong) {
				return "No";
			}
			return "None (matches can end in a tie)";
		case 3:
			if (!fLong) {
				return "Tie-timed Round";
			}
			if (match_overtimecount.integer) {
				return va("Tie-timed Overtime (%d minute rounds - tie after %d rounds)", match_overtimelength.integer, match_overtimecount.integer);
			} else {
				return va("Tie-timed Overtime (%d minute rounds - play until winner)", match_overtimelength.integer);
			}
	}

	return "Unknown";
}

/*
==================
OSP_Vote_Parse

Checks the arguments of "callvote <cmd> <arg>" (and of a referee's "ref <cmd> <arg>":
fRefCommand) and sets up the vote: osp_voteType, osp_voteValue (the value the vote sets),
cmd / arg (the vote string shown to the players). Returns 1 when the vote may be called, -1
when it was refused (message sent), 0 when there is nothing to parse (Cmd_CallVote_f prints
the vote help). The "set<mode>" votes are "mode <id>" votes.
==================
*/
int OSP_Vote_Parse(gentity_t* ent, char* cmd, char* arg, char* arg2, int arg2Size, qboolean fRefCommand) {
	char*    s;
	int     i;

	s = "\\callvote";
	if (fRefCommand) {
		s = "\\ref";
	}

	if (trap_Argc() == 1) {
		return 0;
	}

	osp_voteMap[0] = 0;

	// mode shortcuts: "set<mode> [map]" is "mode <id> [map]"
	if (!Q_stricmp(cmd, "set1v1") || !Q_stricmp(cmd, "settdm") || !Q_stricmp(cmd, "cb") ||
	        !Q_stricmp(cmd, "clanbase") || !Q_stricmp(cmd, "setctf") || !Q_stricmp(cmd, "setca") ||
	        !Q_stricmp(cmd, "setft") || !Q_stricmp(cmd, "setftv") || !Q_stricmp(cmd, "setffa")) {
		if (*arg) {
			strcpy(arg2, arg);
		}

		if (!Q_stricmp(cmd, "setffa")) {
			if (!Q_stricmp(arg, "?")) {
				trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", s, cmd, osp_voteTable[44].help));
				return -1;
			}
			strcpy(arg, "0");
		} else if (!Q_stricmp(cmd, "set1v1")) {
			if (!Q_stricmp(arg, "?")) {
				trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", s, cmd, osp_voteTable[40].help));
				return -1;
			}
			strcpy(arg, "1");
		} else if (!Q_stricmp(cmd, "settdm")) {
			if (!Q_stricmp(arg, "?")) {
				trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", s, cmd, osp_voteTable[41].help));
				return -1;
			}
			strcpy(arg, "3");
		} else if (!Q_stricmp(cmd, "cb") || !Q_stricmp(cmd, "clanbase")) {
			if (!Q_stricmp(arg, "?")) {
				trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", s, cmd, osp_voteTable[7].help));
				return -1;
			}
			strcpy(arg, "2");
		} else if (!Q_stricmp(cmd, "setctf")) {
			if (!Q_stricmp(arg, "?")) {
				trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", s, cmd, osp_voteTable[42].help));
				return -1;
			}
			strcpy(arg, "4");
		} else if (!Q_stricmp(cmd, "setca")) {
			if (!Q_stricmp(arg, "?")) {
				trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", s, cmd, osp_voteTable[43].help));
				return -1;
			}
			strcpy(arg, "5");
		} else if (!Q_stricmp(cmd, "setft")) {
			if (!Q_stricmp(arg, "?")) {
				trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", s, cmd, osp_voteTable[45].help));
				return -1;
			}
			strcpy(arg, "6");
		} else {
			if (!Q_stricmp(arg, "?")) {
				trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", s, cmd, osp_voteTable[46].help));
				return -1;
			}
			strcpy(arg, "7");
		}

		strcpy(cmd, "mode");
	}

	if (!Q_stricmp(cmd, "map_restart") || !Q_stricmp(cmd, "maprestart")) {
		if (trap_Argc() > 2) {
			char    buf[MAX_TOKEN_CHARS];

			trap_Argv(2, buf, sizeof(buf));
			if (!Q_stricmp(buf, "?")) {
				trap_Argv(1, buf, sizeof(buf));
				trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", s, buf, osp_voteTable[19].help));
				return -1;
			}
		}
		osp_voteType = 24;
		return 1;
	} else if (!Q_stricmp(cmd, "nextmap")) {
		if (trap_Argc() > 2) {
			char    buf[MAX_TOKEN_CHARS];

			trap_Argv(2, buf, sizeof(buf));
			if (!Q_stricmp(buf, "?")) {
				trap_Argv(1, buf, sizeof(buf));
				trap_SendServerCommand(ent - g_entities, va("print \"\nUsage: ^3%s %s%s\n\n\"", s, buf, osp_voteTable[27].help));
				return -1;
			}
		}
		osp_voteType = 31;
		Com_sprintf(cmd, arg2Size, "Nextmap");
		return 1;
	} else if (!Q_stricmp(cmd, "mode")) {
		if (!level.numVoteModes) {
			OSP_Vote_Usage(ent, s, 25);
			OSP_Vote_Disabled(ent, "game mode");
			return -1;
		}
		if (!*arg || !Q_stricmp(arg, "?")) {
			OSP_Vote_Usage(ent, s, 25);
			trap_SendServerCommand(ent - g_entities, va("print \"Available modes: ^3%s\n^7Current mode: ^3%s (id: %d)%s\n\n\"",
			                       level.modeList, OSP_ModeName(z_m_current.integer), z_m_current.integer,
			                       (default_change.integer) ? "  ^1[MODIFIED -- see \\settings]" : ""));
			return -1;
		}
		if (atoi(arg) || !Q_stricmp(arg, "0") || !Q_stricmp(arg, "00")) {
			if (OSP_ModeAvailable(atoi(arg))) {
				osp_voteType = 27;
				Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
				if (*arg2) {
					if (!OSP_FindMapFile(arg2)) {
						trap_SendServerCommand(ent - g_entities, va("print \"\nSorry, this server does not have map ^3%s^7.\n\n\"", arg2));
						return -1;
					}
					strcpy(osp_voteMap, arg2);
					Com_sprintf(arg, arg2Size, "%s (map: %s)", OSP_ModeName(atoi(arg)), arg2);
				} else if (atoi(arg) == z_m_current.integer) {
					osp_voteMap[0] = 0;
					Com_sprintf(cmd, arg2Size, "Reset settings for:");
					Com_sprintf(arg, arg2Size, "%s", OSP_ModeName(atoi(arg)));
					return 1;
				} else if (atoi(arg) < osp_numModes && g_gametype.integer == osp_modes[atoi(arg)].gametype) {
					Com_sprintf(cmd, arg2Size, "Change settings to:");
					Com_sprintf(arg, arg2Size, "%s", OSP_ModeName(atoi(arg)));
					return 1;
				} else {
					if (level.warmupTime && atoi(arg) < osp_numModes) {
						trap_SendServerCommand(ent - g_entities, va("print \"\nCannot vote for a mode without specifying a valid map during warmup.\nUse: ^3\\callvote mode %d <map_name|map_id>\n\n\"", atoi(arg)));
						return -1;
					}
					osp_voteMap[0] = 0;
					Com_sprintf(arg, arg2Size, "%s (delayed: no map chosen)", OSP_ModeName(atoi(arg)));
				}
				Com_sprintf(cmd, arg2Size, "Change to MODE:");
				return 1;
			}
		}

		if (atoi(arg) < osp_numModes) {
			trap_SendServerCommand(ent - g_entities, va("print \"\n*** Default game mode ^2%s^7 has been disabled.\n\n\"", osp_modes[atoi(arg)].name));
		} else {
			trap_SendServerCommand(ent - g_entities, va("print \"\n*** Unknown game mode: ^2%s^7\n\n\"", arg));
		}
		trap_SendServerCommand(ent - g_entities, va("print \"Valid modes: ^3%s\n\n\"", level.modeList));
		return -1;
	} else if (!Q_stricmp(cmd, "respawn")) {
		if (!vote_allow_respawn.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, cmd, g_forcerespawn.string);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 38)) {
			OSP_Vote_CurrentValue(ent, cmd, g_forcerespawn.string);
			return -1;
		}
		if (atoi(arg) <= 0) {
			trap_SendServerCommand(ent - g_entities, "print \"Sorry, can't have a forced respawn time <= 0 seconds!\n\"");
			return -1;
		}
		osp_voteType = 42;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(arg, arg2Size, "%s", osp_voteValue);
		Com_sprintf(cmd, arg2Size, "Respawn time:");
		return 1;
	} else if (!Q_stricmp(cmd, "weaponrespawn") || !Q_stricmp(cmd, "wr")) {
		if (g_gametype.integer == 5) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, ^3%s^7 is not used for Clan Arena\n\"", cmd));
			return -1;
		}
		if (!vote_allow_weaponrespawn.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, cmd, (g_gametype.integer != GT_TEAM) ? g_weaponRespawn.string : g_weaponTeamRespawn.string);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 56)) {
			OSP_Vote_CurrentValue(ent, cmd, (g_gametype.integer != GT_TEAM) ? g_weaponRespawn.string : g_weaponTeamRespawn.string);
			return -1;
		}
		if (atoi(arg) < 1) {
			trap_SendServerCommand(ent - g_entities, "print \"Sorry, can't have a weapon respawn time < 1 seconds!\n\"");
			return -1;
		}
		osp_voteType = 50;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(arg, arg2Size, "%s", osp_voteValue);
		Com_sprintf(cmd, arg2Size, "Weapon Respawn time:");
		return 1;
	} else if (!Q_stricmp(cmd, "pmove") || !Q_stricmp(cmd, "125hz")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Enhanced Client Sampling", (pmove_fixed.integer) ? 1 : 0, vote_allow_pmove.integer, 31, 37);
	} else if (!Q_stricmp(cmd, "tl") || !Q_stricmp(cmd, "timelimit")) {
		if (g_gametype.integer == 5) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, ^3%s^7 is not used for Clan Arena\n\"", cmd));
			return -1;
		}
		if (!vote_allow_timelimit.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, cmd, g_timelimit.string);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 51)) {
			OSP_Vote_CurrentValue(ent, cmd, g_timelimit.string);
			return -1;
		}
		if (!g_fraglimit.integer && !atoi(arg)) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, can't have a 0 %s if fraglimit is also 0!\n\"", cmd));
			return -1;
		}
		if (atoi(arg) < 0) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, can't specify a %s < 0!\n\"", cmd));
			return -1;
		}
		osp_voteType = 47;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(arg, arg2Size, "%s", osp_voteValue);
		Com_sprintf(cmd, arg2Size, "timelimit");
		return 1;
	} else if (!Q_stricmp(cmd, "fl") || !Q_stricmp(cmd, "fraglimit")) {
		if (g_gametype.integer == 5) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, ^3%s^7 is not used for Clan Arena\n\"", cmd));
			return -1;
		}
		if (!vote_allow_fraglimit.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, cmd, g_fraglimit.string);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 12)) {
			OSP_Vote_CurrentValue(ent, cmd, g_fraglimit.string);
			return -1;
		}
		if (!g_timelimit.integer && !atoi(arg)) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, can't have a 0 %s if timelimit is also 0!\n\"", cmd));
			return -1;
		}
		if (atoi(arg) < 0) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, can't specify a %s < 0!\n\"", cmd));
			return -1;
		}
		osp_voteType = 18;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(arg, arg2Size, "%s", osp_voteValue);
		Com_sprintf(cmd, arg2Size, "fraglimit");
		return 1;
	} else if (!Q_stricmp(cmd, "cl") || !Q_stricmp(cmd, "capturelimit")) {
		if (g_gametype.integer != 4) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, ^3%s^7 is used only for CTF!!\n\"", cmd));
			return -1;
		}
		if (!vote_allow_capturelimit.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, cmd, g_capturelimit.string);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 6)) {
			OSP_Vote_CurrentValue(ent, cmd, g_capturelimit.string);
			return -1;
		}
		if (!g_timelimit.integer && !atoi(arg)) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, can't have a 0 %s if timelimit is also 0!\n\"", cmd));
			return -1;
		}
		if (atoi(arg) < 0) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, can't specify a %s < 0!\n\"", cmd));
			return -1;
		}
		osp_voteType = 13;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(arg, arg2Size, "%s", osp_voteValue);
		Com_sprintf(cmd, arg2Size, "capturelimit");
		return 1;
	} else if (!Q_stricmp(cmd, "rl") || !Q_stricmp(cmd, "roundlimit")) {
		if (!OSP_IsRoundBased(g_gametype.integer)) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, ^3%s^7 is used only for round-based games!!\n\"", cmd));
			return -1;
		}
		if (!vote_allow_roundlimit.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, cmd, roundlimit.string);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 39)) {
			OSP_Vote_CurrentValue(ent, cmd, roundlimit.string);
			return -1;
		}
		if (atoi(arg) < 1) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, can't have a %s less than 1!\n\"", cmd));
			return -1;
		}
		osp_voteType = 43;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(arg, arg2Size, "%s", osp_voteValue);
		Com_sprintf(cmd, arg2Size, "roundlimit");
		return 1;
	} else if (!Q_stricmp(cmd, "poweruprespawn")) {
		char*    name = "Powerup Respawn Delay";

		if (OSP_Vote_ProModeOnly(ent, name)) {
			return -1;
		}
		if (!vote_allow_poweruprespawn.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, name, match_poweruprespawn.string);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 32)) {
			OSP_Vote_CurrentValue(ent, name, match_poweruprespawn.string);
			return -1;
		}
		if (atoi(arg) != 60 && atoi(arg) != 90 && atoi(arg) != 120) {
			trap_SendServerCommand(ent - g_entities, va("print \"^2** Invalid %s: %s\n^3Valid settings: ^760,90,120\n\n", name, arg));
			return -1;
		}
		osp_voteType = 38;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(arg, arg2Size, "%s", osp_voteValue);
		Com_sprintf(cmd, arg2Size, name);
		return 1;
	} else if (!Q_stricmp(cmd, "fastrail")) {
		char*    name = "Fast Rail Switch";

		if (OSP_Vote_ProModeOnly(ent, name)) {
			return -1;
		}
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, name, (server_fastrail.integer) ? 1 : 0, vote_allow_fastrail.integer, 11, 17);
	} else if (!Q_stricmp(cmd, "reload")) {
		char*    name = "Rail Reload Time";

		if (!match_instagib.integer) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3%s^7 voting is only enabled in Instagib!\n\"", name));
			return -1;
		}
		if (!vote_allow_reload.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, cmd, instagib_reload.string);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 37)) {
			OSP_Vote_CurrentValue(ent, cmd, instagib_reload.string);
			return -1;
		}
		if (atoi(arg) < 100 || atoi(arg) > 5000) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3** Invalid %s: %s ^1(min: 100, max: 5000)\n\n", name, arg));
			return -1;
		}
		osp_voteType = 41;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(arg, arg2Size, "%s", osp_voteValue);
		Com_sprintf(cmd, arg2Size, name);
		return 1;
	} else if (!Q_stricmp(cmd, "lgcooldown")) {
		char*    name = "LightningGun Cooldown";

		if (OSP_Vote_ProModeOnly(ent, name)) {
			return -1;
		}
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, name, (server_lgcooldown.integer) ? 1 : 0, vote_allow_lgcooldown.integer, 17, 22);
	} else if (!Q_stricmp(cmd, "instagib")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Railgun-only Instagib", (match_instagib.integer) ? 1 : 0, vote_allow_instagib.integer, 15, 20);
	} else if (!Q_stricmp(cmd, "fallingdamage")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Falling Damage", !(g_dmflags.integer & DF_NO_FALLING) ? 1 : 0, vote_allow_falling.integer, 10, 16);
	} else if (!Q_stricmp(cmd, "overtime")) {
		char*    name = "Overtime Mode";

		if (!vote_allow_overtime.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, name, OSP_OvertimeModeName(qtrue, match_overtime.integer));
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 30)) {
			OSP_Vote_CurrentValue(ent, name, OSP_OvertimeModeName(qtrue, match_overtime.integer));
			return -1;
		}
		if (atoi(arg) < 0 || atoi(arg) >= 4) {
			trap_SendServerCommand(ent - g_entities, va("print \"\n^3%s %s%s\n\n\"", s, cmd, osp_voteTable[30].help));
			trap_SendServerCommand(ent - g_entities, va("print \"^3** Invalid overtime mode: %s\n\n", arg));
			return -1;
		}
		osp_voteType = 36;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(cmd, arg2Size, "%s", OSP_OvertimeModeName(qfalse, atoi(arg)));
		Com_sprintf(arg, arg2Size, name);
		return 1;
	} else if (!Q_stricmp(cmd, "altgraphics")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Alternate client graphics", (server_customclient.integer & 4) ? 1 : 0, vote_allow_altgraphics.integer, 1, 10);
	} else if (!Q_stricmp(cmd, "capfps")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Client FPS capping", !(server_customclient.integer & 32) ? 1 : 0, vote_allow_capfps.integer, 5, 12);
	} else if (!Q_stricmp(cmd, "osphud")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Client OSP-style HUD", (server_customclient.integer & 8) ? 1 : 0, vote_allow_osphud.integer, 29, 35);
	} else if (!Q_stricmp(cmd, "notimer")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Blackout of client timer", (server_customclient.integer & 16) ? 1 : 0, vote_allow_notimer.integer, 28, 32);
	} else if (!Q_stricmp(cmd, "mg_startammo")) {
		char*    name = "Machinegun starting ammo";

		if (!vote_allow_machinegun.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, "Machinegun configuration");
			OSP_Vote_CurrentValue(ent, name, start_bullets.string);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 24)) {
			OSP_Vote_CurrentValue(ent, name, start_bullets.string);
			return -1;
		}
		if (atoi(arg) < 0) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set starting ammo less than 0!\n\"");
			return -1;
		}
		if (atoi(arg) > 500) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set starting ammo greater than 500!\n\"");
			return -1;
		}
		osp_voteType = 25;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(arg, arg2Size, "%s", osp_voteValue);
		Com_sprintf(cmd, arg2Size, va("Set %s to", name));
		return 1;
	} else if (!Q_stricmp(cmd, "mg_damage")) {
		char*    name = "Machinegun damage";

		if (!vote_allow_machinegun.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, "Machinegun configuration");
			OSP_Vote_CurrentValue(ent, name, (g_gametype.integer == GT_TEAM) ? mg_damageTeamplay.string : mg_damage.string);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 23)) {
			OSP_Vote_CurrentValue(ent, name, (g_gametype.integer == GT_TEAM) ? mg_damageTeamplay.string : mg_damage.string);
			return -1;
		}
		if (atoi(arg) < 1) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set damage less than 1!\n\"");
			return -1;
		}
		if (atoi(arg) > 10) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set damage greater than 10!\n\"");
			return -1;
		}
		osp_voteType = 26;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(arg, arg2Size, "%s", osp_voteValue);
		Com_sprintf(cmd, arg2Size, va("Set %s to", name));
		return 1;
	} else if (!Q_stricmp(cmd, "maxpackets_min")) {
		char*    name = "client maxpackets minimum";

		if (!vote_allow_maxpackets.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, name, (server_maxpacketsmin.integer) ? server_maxpacketsmin.string : DISABLED);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 21)) {
			OSP_Vote_CurrentValue(ent, name, (server_maxpacketsmin.integer) ? server_maxpacketsmin.string : DISABLED);
			return -1;
		}
		if (atoi(arg) < 15) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set minimum maxpackets less than 15!\n\"");
			return -1;
		}
		if (atoi(arg) > 30) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set minimum maxpackets greater than 30!\n\"");
			return -1;
		}
		osp_voteType = 29;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(cmd, arg2Size, va("Clamp %s to", name));
		Com_sprintf(arg, arg2Size, "%s", (atoi(osp_voteValue)) ? osp_voteValue : DISABLED);
		return 1;
	} else if (!Q_stricmp(cmd, "maxpackets_max")) {
		char*    name = "client maxpackets maximum";

		if (!vote_allow_maxpackets.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, name, (server_maxpacketsmax.integer) ? server_maxpacketsmax.string : DISABLED);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 20)) {
			OSP_Vote_CurrentValue(ent, name, (server_maxpacketsmax.integer) ? server_maxpacketsmax.string : DISABLED);
			return -1;
		}
		if (atoi(arg) > 125) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set maximum maxpackets greater than 125!\n\"");
			return -1;
		}
		if (atoi(arg) < 30) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set maximum maxpackets less than 30!\n\"");
			return -1;
		}
		osp_voteType = 30;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(cmd, arg2Size, va("Clamp %s to", name));
		Com_sprintf(arg, arg2Size, "%s", (atoi(osp_voteValue)) ? osp_voteValue : DISABLED);
		return 1;
	} else if (!Q_stricmp(cmd, "timenudge_min")) {
		char*    name = "client timenudge minimum";

		if (!vote_allow_timenudge.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, name, (server_timenudgemin.integer) ? server_timenudgemin.string : DISABLED);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 53)) {
			OSP_Vote_CurrentValue(ent, name, (server_timenudgemin.integer) ? server_timenudgemin.string : DISABLED);
			return -1;
		}
		if (atoi(arg) > 0) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set minimum timenudge greater than 0!\n\"");
			return -1;
		}
		if (atoi(arg) < -200) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set minimum timenudge less than -200!\n\"");
			return -1;
		}
		osp_voteType = 33;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(cmd, arg2Size, va("Clamp %s to", name));
		Com_sprintf(arg, arg2Size, "%s", (atoi(osp_voteValue)) ? osp_voteValue : DISABLED);
		return 1;
	} else if (!Q_stricmp(cmd, "timenudge_max")) {
		char*    name = "client timenudge maximum";

		if (!vote_allow_timenudge.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			OSP_Vote_CurrentValue(ent, name, (server_timenudgemax.integer) ? server_timenudgemax.string : DISABLED);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 52)) {
			OSP_Vote_CurrentValue(ent, name, (server_timenudgemax.integer) ? server_timenudgemax.string : DISABLED);
			return -1;
		}
		if (atoi(arg) > 500) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set maximum timenudge greater than 500!\n\"");
			return -1;
		}
		// OSP bug kept: the test is -200, the message says 0
		if (atoi(arg) < -200) {
			trap_SendServerCommand(ent - g_entities, "print \"^3Cannot set maximum timenudge less than 0!\n\"");
			return -1;
		}
		osp_voteType = 34;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg));
		Com_sprintf(cmd, arg2Size, va("Clamp %s to", name));
		Com_sprintf(arg, arg2Size, "%s", (atoi(osp_voteValue)) ? osp_voteValue : DISABLED);
		return 1;
	} else if (!Q_stricmp(cmd, "armordamage")) {
		char*    name = "Armor Damage";

		if (g_gametype.integer != 5) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3%s^7 voting only enabled in Clan Arena mode!\n\"", name));
			return -1;
		}
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, name, (team_armordamage.integer) ? 1 : 0, vote_allow_armordamage.integer, 2, 11);
	} else if (!Q_stricmp(cmd, "cq3")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "CQ3 Server Settings", (level.physicsMode & 2) ? 1 : 0, vote_allow_cq3.integer, 8, 14);
	} else if (!Q_stricmp(cmd, "promode") || !Q_stricmp(cmd, "cpm")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Promode Server Settings", (level.physicsMode & 1) ? 1 : 0, vote_allow_promode.integer, 33, 39);
	} else if (!Q_stricmp(cmd, "dropitems")) {
		char*    name = "Drop Items";

		if (g_gametype.integer != 3 && g_gametype.integer != 4) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3%s^7 voting only enabled in Team DM/CTF modes!\n\"", name));
			return -1;
		}
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, name, (match_dropitems.integer) ? 1 : 0, vote_allow_dropitems.integer, 9, 15);
	} else if (!Q_stricmp(cmd, "startrespawn")) {
		char*    name = "Respawn on Round Start";

		if (g_gametype.integer != 5) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3%s^7 voting only enabled in Clan Arena mode!\n\"", name));
			return -1;
		}
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, name, (match_carespawn.integer) ? 1 : 0, vote_allow_ca_startrespawn.integer, 47, 44);
	} else if (!Q_stricmp(cmd, "warmupfire")) {
		char*    name = "Warmup Weapon Fire";

		if (g_gametype.integer != 5) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3%s^7 voting only enabled in Clan Arena mode!\n\"", name));
			return -1;
		}
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, name, (match_cawarmupfire.integer) ? 1 : 0, vote_allow_ca_warmupfire.integer, 55, 49);
	} else if (!Q_stricmp(cmd, "teamdamage")) {
		char*    name = "Team Damage";

		if (g_gametype.integer < 3) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3%s^7 voting only available for team-based modes!\n\"", name));
			return -1;
		}
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, name, (g_friendlyFire.integer) ? 1 : 0, vote_allow_teamdamage.integer, 48, 45);
	} else if (!Q_stricmp(cmd, "referee") || !Q_stricmp(cmd, "ref")) {
		char    buf[MAX_STRING_CHARS];
		int     pid;

		if (!vote_allow_referee.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			return -1;
		}
		if (level.numPlayingClients < 3 || (g_gametype.integer == 1 && level.numPlayingClients != 2)) {
			trap_SendServerCommand(ent - g_entities, "print \"Sorry, not enough clients in the game to vote for a referee\n\"");
			return -1;
		}

		Q_strncpyz(buf, arg, sizeof(buf));
		if (trap_Argc() > 3) {
			for (i = 3; i < trap_Argc(); i++) {
				Q_strcat(buf, sizeof(buf), " ");
				trap_Argv(i, arg, MAX_STRING_TOKENS);
				Q_strcat(buf, sizeof(buf), arg);
			}
		}

		if (ent->client->sess.referee && trap_Argc() == 2) {
			OSP_PrintInvalidPlayerID(ent);
			return -1;
		}
		if (trap_Argc() == 2) {
			pid = ent - g_entities;
		} else if (OSP_Vote_Usage(ent, s, 35)) {
			return -1;
		} else if ((pid = ClientNumberFromString(ent, arg)) == -1) {
			OSP_PrintInvalidPlayerID(ent);
			return -1;
		}

		if (level.clients[pid].sess.referee) {
			trap_SendServerCommand(ent - g_entities, va("print \"%s is already a referee!\n\"", level.clients[pid].pers.netname));
			return -1;
		}
		osp_voteType = 40;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", pid);
		Com_sprintf(cmd, arg2Size, "referee");
		Com_sprintf(arg, arg2Size, "%s", level.clients[pid].pers.netname);
		return 1;
	} else if (!Q_stricmp(cmd, "unreferee") || !Q_stricmp(cmd, "unref")) {
		char    buf[MAX_STRING_CHARS];
		int     pid;

		if (!vote_allow_referee.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			return -1;
		}

		Q_strncpyz(buf, arg, sizeof(buf));
		if (trap_Argc() > 3) {
			for (i = 3; i < trap_Argc(); i++) {
				Q_strcat(buf, sizeof(buf), " ");
				trap_Argv(i, arg, MAX_STRING_TOKENS);
				Q_strcat(buf, sizeof(buf), arg);
			}
		}

		if (ent->client->sess.referee && trap_Argc() == 2) {
			OSP_PrintInvalidPlayerID(ent);
			return -1;
		}
		if (trap_Argc() == 2) {
			pid = ent - g_entities;
		} else if (OSP_Vote_Usage(ent, s, 54)) {
			return -1;
		} else if ((pid = ClientNumberFromString(ent, arg)) == -1) {
			OSP_PrintInvalidPlayerID(ent);
			return -1;
		}

		if (!level.clients[pid].sess.referee) {
			trap_SendServerCommand(ent - g_entities, va("print \"%s isn't a referee!\n\"", level.clients[pid].pers.netname));
			return -1;
		}
		osp_voteType = 48;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", pid);
		Com_sprintf(cmd, arg2Size, "UN-referee");
		Com_sprintf(arg, arg2Size, "%s", level.clients[pid].pers.netname);
		return 1;
	} else if (!Q_stricmp(cmd, "kick")) {
		char    buf[MAX_STRING_CHARS];
		int     pid;

		if (!vote_allow_kick.integer && !ent->client->sess.referee) {
			OSP_Vote_Disabled(ent, cmd);
			return -1;
		}
		if (OSP_Vote_Usage(ent, s, 16)) {
			return -1;
		}

		Q_strncpyz(buf, arg, sizeof(buf));
		if (trap_Argc() > 3) {
			for (i = 3; i < trap_Argc(); i++) {
				Q_strcat(buf, sizeof(buf), " ");
				trap_Argv(i, arg, MAX_STRING_TOKENS);
				Q_strcat(buf, sizeof(buf), arg);
			}
		}

		if ((pid = ClientNumberFromString(ent, buf)) == -1) {
			OSP_PrintInvalidPlayerID(ent);
			return -1;
		}
		if (level.clients[pid].sess.referee) {
			trap_SendServerCommand(ent - g_entities, "print \"Can't vote to kick referees!\n\"");
			return -1;
		}
		osp_voteType = 21;
		Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", pid);
		Com_sprintf(cmd, arg2Size, "KICK");
		Com_sprintf(arg, arg2Size, "%s", level.clients[pid].pers.netname);
		return 1;
	} else if (!Q_stricmp(cmd, "hook")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "the Hook", (hook_enable.integer) ? 1 : 0, vote_allow_hook.integer, 13, 19);
	} else if (!Q_stricmp(cmd, "allcaptain") || !Q_stricmp(cmd, "allcaptains")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Team Allcaptain", (team_allcaptain.integer) ? 1 : 0, vote_allow_allcaptain.integer, 0, 9);
	} else if (!Q_stricmp(cmd, "thrufloors") || !Q_stricmp(cmd, "thrufloor")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Damage Through Floors", (server_thrufloors.integer) ? 1 : 0, vote_allow_thrufloors.integer, 50, 46);
	} else if (!Q_stricmp(cmd, "mutespecs") || !Q_stricmp(cmd, "mute")) {
		return OSP_Vote_Toggle(ent, cmd, arg, arg2Size, s, "Spectator Muting", (match_mutespecs.integer) ? 1 : 0, vote_allow_mutespecs.integer, 26, 28);
	} else {
		// powerup / item votes ("quad", "bfg", ...)
		return OSP_Vote_ItemToggle(ent, cmd, arg, arg2Size, fRefCommand);
	}
}

/*
==================
OSP_Vote_ItemToggle

Powerup / item votes ("quad", "bfg", "bsuit", "invis", "regen", "medkit", "teleporter");
0 when arg is none of them
==================
*/
int OSP_Vote_ItemToggle(gentity_t* ent, char* arg, char* arg2, int arg2Size, qboolean fRefCommand) {
	int     i;
	int     bits[8] = { 0, 1, 4, 2, 16, 8, 128, 64 };
	int     types[8] = { 0, 2, 1, 3, 4, 5, 6, 7 };
	int     votes[8] = { 34, 34, 3, 4, 14, 36, 22, 49 };
	char*    names[8] = { "none", "quad", "bfg", "bsuit", "invis", "regen", "medkit", "teleporter" };
	char*    longNames[8] = { "none", "Quad Damage", "BFG", "Battle Suit", "Invisibility", "Regeneration", "MedKit", "Teleporter" };
	char*    cmd = "\\callvote";

	if (fRefCommand) {
		cmd = "\\ref";
	}

	for (i = 1; i < 8; i++) {
		int     allowed, disabled;

		if (bits[i] == 4) {
			allowed = vote_allow_items.integer;
			disabled = item_disable.integer;
		} else {
			allowed = vote_allow_powerups.integer;
			disabled = powerup_disable.integer;
		}

		if (!Q_stricmp(arg, names[i])) {
			if (g_gametype.integer == 5) {
				trap_SendServerCommand(ent - g_entities, "print \"Sorry, cannot vote for powerups/items in ClanArena!\n\"");
				return -1;
			}

			if (!(allowed & bits[i]) && !ent->client->sess.referee) {
				OSP_Vote_Disabled(ent, longNames[i]);
				OSP_Vote_CurrentValue(ent, longNames[i], (disabled & bits[i]) ? DISABLED : ENABLED);
				return -1;
			}

			if (OSP_Vote_Usage(ent, cmd, votes[i])) {
				OSP_Vote_CurrentValue(ent, longNames[i], (disabled & bits[i]) ? DISABLED : ENABLED);
				return -1;
			}

			if (atoi(arg2) && !(disabled & bits[i])) {
				trap_SendServerCommand(ent - g_entities, va("print \"The ^2%s^7 is already enabled!\n\"", longNames[i]));
				return -1;
			}
			if (!atoi(arg2) && (disabled & bits[i])) {
				trap_SendServerCommand(ent - g_entities, va("print \"The ^2%s^7 is already disabled!\n\"", longNames[i]));
				return -1;
			}

			osp_voteType = types[i];
			Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg2) ? 1 : 0);
			Com_sprintf(arg, arg2Size, atoi(arg2) ? ENABLE : DISABLE);
			Com_sprintf(arg2, arg2Size, longNames[i]);
			return 1;
		}
	}

	return 0;
}

/*
==================
OSP_Vote_Execute

Runs the passed vote (osp_voteType, argument osp_voteValue; CheckVote); 0 when there is
nothing to run
==================
*/
int OSP_Vote_Execute(void) {
	int     cc;

	if (strstr(level.voteString, "timelimit 0") == level.voteString && !level.warmupTime) {
		OSP_UpdateScoreCvars("In Progress");
	}

	switch (osp_voteType) {
		case 9:     // allcaptain
			OSP_Vote_SetCvar("Team AllCaptain", "team_allcaptain");
			break;
		case 10:    // altgraphics
			cc = server_customclient.integer;
			trap_SendServerCommand(-1, va("print \"^3Alternate client graphics is: ^5%s\n\"", atoi(osp_voteValue) ? ENABLED : DISABLED));
			if (atoi(osp_voteValue)) {
				cc |= 4;
			} else {
				cc &= ~4;
			}
			trap_SendConsoleCommand(EXEC_APPEND, va("server_customclient %d\n", cc));
			trap_Cvar_Set("default_change", "1");
			break;
		case 11:    // armordamage
			OSP_Vote_SetCvar("Armor Damage", "team_armordamage");
			break;
		case 1:     // bfg
			OSP_Vote_ItemSet("BFG", qfalse, 4, WP_BFG, IT_WEAPON, atoi(osp_voteValue));
			break;
		case 3:     // bsuit
			OSP_Vote_ItemSet("Battle Suit", qtrue, 2, PW_BATTLESUIT, IT_POWERUP, atoi(osp_voteValue));
			break;
		case 13:    // capturelimit
			OSP_Vote_SetValueBroadcast("Capturelimit");
			break;
		case 14:    // cq3
			OSP_Vote_SetCvar("CQ3", "server_cq3");
			if (atoi(osp_voteValue)) {
				trap_Cvar_Set("server_promode", "0");
			}
			trap_SendServerCommand(-1, va("cp \"^3CQ3 settings\nhave been ^3%s\n\"", atoi(osp_voteValue) ? ENABLED : DISABLED));
			break;
		case 15:    // dropitems
			OSP_Vote_SetCvar("Drop Items", "match_dropitems");
			break;
		case 16:    // fallingdamage
			OSP_Vote_FallingDamage();
			break;
		case 17:    // fastrail
			OSP_Vote_SetCvar("Fast Rail Switch", "server_fastrail");
			break;
		case 18:    // fraglimit
			OSP_Vote_SetValueBroadcast("Fraglimit");
			break;
		case 19:    // hook
			OSP_Vote_SetCvar("The Hook", "hook_enable");
			break;
		case 4:     // invis
			OSP_Vote_ItemSet("Invisibility", qtrue, 16, PW_INVIS, IT_POWERUP, atoi(osp_voteValue));
			break;
		case 20:    // instagib
			OSP_Vote_Instagib();
			break;
		case 21:    // kick
			trap_SendConsoleCommand(EXEC_APPEND, va("clientkick %d\n", atoi(osp_voteValue)));
			trap_SendServerCommand(-1, va("cp \"%s\n^3has been kicked!\n\"", level.clients[atoi(osp_voteValue)].pers.netname));
			break;
		case 22:    // lgcooldown
			OSP_Vote_SetCvar("LightningGun Cooldown", "server_lgcooldown");
			break;
		case 23:    // map
			if (map_queue.integer && osp_numMaps) {
				OSP_Vote_MapExec();
				break;
			}
			return 0;
		case 24:    // map_restart
			OSP_MatchEndStats(0, 0);
			OSP_MatchEndStats(1, 0);
			OSP_MapRestart();
			trap_SendServerCommand(-1, "cp \"^1*** Level Restarted! ***\n\"");
			break;
		case 6:     // medkit
			OSP_Vote_ItemSet("Medkit", qtrue, 128, HI_MEDKIT, IT_HOLDABLE, atoi(osp_voteValue));
			break;
		case 25:    // mg_startammo
			OSP_Vote_SetValue("Machinegun Starting Ammo", "start_bullets");
			break;
		case 26:    // mg_damage
			OSP_Vote_SetValue("Machinegun Damage", (g_gametype.integer != 3) ? "mg_damage" : "mg_damageTeamplay");
			break;
		case 27:    // mode
			level.nextMode = atoi(osp_voteValue) + 1;
			OSP_Vote_Mode(osp_voteMap);
			break;
		case 30:    // maxpackets_max
			OSP_Vote_SetValue("Maximum client maxpackets", "server_maxpacketsmax");
			break;
		case 29:    // maxpackets_min
			OSP_Vote_SetValue("Minimum client maxpackets", "server_maxpacketsmin");
			break;
		case 28:    // mutespecs
			OSP_Vote_SetCvar("Spectator Muting", "match_mutespecs");
			break;
		case 31:    // nextmap
			OSP_Vote_Nextmap();
			break;
		case 12:    // capfps
			cc = server_customclient.integer;
			trap_SendServerCommand(-1, va("print \"^3Capped Client FPS Settings: ^5%s\n\"", atoi(osp_voteValue) ? ENABLED : DISABLED));
			if (!atoi(osp_voteValue)) {
				cc |= 32;
			} else {
				cc &= ~32;
			}
			trap_SendConsoleCommand(EXEC_APPEND, va("server_customclient %d\n", cc));
			trap_Cvar_Set("default_change", "1");
			break;
		case 32:    // notimer
			trap_SendServerCommand(-1, va("print \"^3Blackout of client timer is: ^5%s\n\"", atoi(osp_voteValue) ? ENABLED : DISABLED));
			cc = server_customclient.integer;
			if (atoi(osp_voteValue)) {
				cc |= 16;
			} else {
				cc &= ~16;
			}
			trap_SendConsoleCommand(EXEC_APPEND, va("server_customclient %d\n", cc));
			trap_Cvar_Set("default_change", "1");
			break;
		case 34:    // timenudge_max
			OSP_Vote_SetValue("Maximum client timenudge", "server_timenudgemax");
			break;
		case 33:    // timenudge_min
			OSP_Vote_SetValue("Minimum client timenudge", "server_timenudgemin");
			break;
		case 35:    // osphud
			cc = server_customclient.integer;
			trap_SendServerCommand(-1, va("print \"^3Client OSP-style HUD is: ^5%s\n\"", atoi(osp_voteValue) ? "ALLOWED" : DISABLED));
			if (atoi(osp_voteValue)) {
				cc |= 8;
			} else {
				cc &= ~8;
			}
			trap_SendConsoleCommand(EXEC_APPEND, va("server_customclient %d\n", cc));
			trap_Cvar_Set("default_change", "1");
			break;
		case 36:    // overtime
			trap_Cvar_Set("match_overtime", osp_voteValue);
			trap_Cvar_Update(&match_overtime);
			trap_SendServerCommand(-1, va("print \"^3Overtime Mode set to: ^5%s\n\"", OSP_OvertimeModeName(qtrue, atoi(osp_voteValue))));
			trap_Cvar_Set("default_change", "1");
			break;
		case 37:    // pmove
			OSP_Vote_Pmove();
			break;
		case 38:    // poweruprespawn
			OSP_Vote_SetValue("Powerup Respawn Time", "match_poweruprespawn");
			break;
		case 39:    // promode
			OSP_Vote_SetCvar("ProMode", "server_promode");
			if (atoi(osp_voteValue)) {
				trap_Cvar_Set("server_cq3", "0");
			}
			trap_SendServerCommand(-1, va("cp \"^3CPM ProMode settings\nhave been ^3%s\n\"", atoi(osp_voteValue) ? ENABLED : DISABLED));
			break;
		case 2:     // quad
			OSP_Vote_ItemSet("Quad", qtrue, 1, PW_QUAD, IT_POWERUP, atoi(osp_voteValue));
			break;
		case 40:    // referee
			OSP_Vote_Referee();
			break;
		case 5:     // regen
			OSP_Vote_ItemSet("Regeneration", qtrue, 8, PW_REGEN, IT_POWERUP, atoi(osp_voteValue));
			break;
		case 41:    // reload
			OSP_Vote_SetValue("Railgun Reload Time", "instagib_reload");
			break;
		case 42:    // respawn
			OSP_Vote_SetValue("Respawn Time", "g_forcerespawn");
			break;
		case 43:    // roundlimit
			OSP_Vote_SetValueBroadcast("Roundlimit");
			break;
		case 44:    // startrespawn
			OSP_Vote_SetCvar("Respawn on Round Start", "match_carespawn");
			break;
		case 45:    // teamdamage
			OSP_Vote_SetCvar("Hurt Teammates", "g_friendlyFire");
			break;
		case 7:     // teleporter
			OSP_Vote_ItemSet("Teleporter", qtrue, 64, HI_TELEPORTER, IT_HOLDABLE, atoi(osp_voteValue));
			break;
		case 46:    // thrufloors
			OSP_Vote_SetCvar("Damage Through Floors", "server_thrufloors");
			break;
		case 47:    // timelimit
			OSP_Vote_SetValueBroadcast("Timelimit");
			break;
		case 48:    // unreferee
			OSP_Vote_Unreferee();
			break;
		case 49:    // warmupfire
			OSP_Vote_SetCvar("Warmup Weapon Fire", "match_cawarmupfire");
			break;
		case 50:    // weaponrespawn
			OSP_Vote_SetValue("Weapon Respawn Time", (g_gametype.integer != 3) ? "g_weaponRespawn" : "g_weaponTeamRespawn");
			break;
		default:
			return 0;
	}

	return 1;
}

/*
==================
OSP_Vote_Pmove

"pmove": pmove_fixed (configstring CS_OSP_PMOVEFIXED)
==================
*/
void OSP_Vote_Pmove(void) {
	trap_SendServerCommand(-1, va("print \"^3Enhanced Client Sampling is: ^5%s\n\"", atoi(osp_voteValue) ? ENABLED : DISABLED));
	trap_SetConfigstring(CS_OSP_PMOVEFIXED, osp_voteValue);
	trap_SendConsoleCommand(EXEC_APPEND, va("pmove_fixed %s\n", osp_voteValue));
	trap_Cvar_Set("default_change", "1");
}

/*
==================
OSP_Vote_Nextmap
==================
*/
void OSP_Vote_Nextmap(void) {
	if (map_queue.integer && osp_numMaps) {
		osp_mapChangeReason = 0;
		OSP_NextMap(4);
		G_Printf("Vote: nextmap\n");
		if (!level.intermissiontime) {
			LogExit("Vote: nextmap");
		}
	} else {
		G_Printf("Vote: nextmap\n");
		if (!level.intermissiontime) {
			LogExit("Vote: nextmap");
		}
	}
}

/*
==================
OSP_Vote_Map

Checks a map vote: 0 (map list and usage) for "?", no map, too many arguments or an unknown map
==================
*/
int OSP_Vote_Map(gentity_t* ent, char* map, qboolean fRefCommand) {
	char*    cmd = "callvote";

	if (fRefCommand) {
		cmd = "ref";
	}

	if (!Q_stricmp(map, "?") || trap_Argc() == 2) {
		OSP_Cmd_MapList_f(ent);
		OSP_Vote_Usage(ent, cmd, 18);
		return 0;
	}

	if (trap_Argc() > 3) {
		trap_SendServerCommand(ent - g_entities, "print \"Too many map voting parameters!\n\"");
		OSP_Vote_Usage(ent, cmd, 18);
		return 0;
	}

	if (!OSP_ValidateMap(ent, map, qtrue)) {
		OSP_Cmd_MapList_f(ent);
		return 0;
	}

	return 1;
}

/*
==================
OSP_Vote_Referee
==================
*/
void OSP_Vote_Referee(void) {
	if (level.clients[atoi(osp_voteValue)].pers.connected == CON_DISCONNECTED) {
		trap_SendServerCommand(-1, "print \"Player left before becoming referee\n\"");
		return;
	}

	level.clients[atoi(osp_voteValue)].sess.referee = 2;
	level.clients[atoi(osp_voteValue)].sess.specInvite = 3;
	trap_SendServerCommand(atoi(osp_voteValue), "print \"Type: ^3ref ?^7 for a list of referee commands.\n\"");
	trap_SendServerCommand(-1, va("cp \"%s^7 is now a referee\n\"", level.clients[atoi(osp_voteValue)].pers.netname));
}

/*
==================
OSP_Vote_Unreferee
==================
*/
void OSP_Vote_Unreferee(void) {
	level.clients[atoi(osp_voteValue)].sess.referee = 0;
	level.clients[atoi(osp_voteValue)].sess.specInvite = 0;
	trap_SendServerCommand(-1, va("cp \"%s^7\nis no longer a referee\n\"", level.clients[atoi(osp_voteValue)].pers.netname));
}

/*
==================
OSP_Vote_MapExec

Map vote passed: ends the level (the map list's next map is the voted one). OSP_NextMap( 1 )
runs twice (OSP)
==================
*/
void OSP_Vote_MapExec(void) {
	trap_SetConfigstring(CS_WARMUP, "0");
	if (!level.intermissiontime) {
		LogExit(va("Voted: %s", level.voteString));
		OSP_NextMap(1);
	} else {
		G_LogPrintf("Voted: %s", level.voteString);
		osp_mapChangeReason = 0;
		OSP_NextMap(1);
	}
	OSP_NextMap(1);
	osp_roundsPlayed = 0;
	osp_unk_1c054 = 0;
	osp_matchResultPrint[0] = 0;
	trap_Cvar_Set("g_restarted", "0");
}

/*
==================
OSP_Vote_Mode

Mode vote passed (level.nextMode = mode + 1): applies the mode now (warmup, same base
gametype) or at the next map / map change
==================
*/
void OSP_Vote_Mode(char* map) {
	int     mode;
	int     gametype;

	mode = level.nextMode - 1;
	gametype = -1;

	if (level.nextMode <= 0) {
		return;
	}

	if ((!map || !*map) && mode == z_m_current.integer) {
		OSP_SetGametype(z_m_current.integer);
		level.nextMode = 0;
		trap_SendServerCommand(-1, "cp \"^1Server settings set back to default\n\"");
		trap_SendServerCommand(-1, "print \"\n^3*** Settings set back to default\n\n\"");
		return;
	}

	if (mode < osp_numModes) {
		if (g_gametype.integer == osp_modes[mode].gametype && (!map || !*map)) {
			level.nextMode = 0;
			trap_SendServerCommand(-1, va("print \"\nLoaded ^3%s^7 settings.\n\"", OSP_ModeName(mode)));
			OSP_SetGametype(mode);
			G_LogPrintf(va("Voted: %s\n", level.voteString));
			trap_Cvar_Set("z_m_current", va("%d", mode));
			return;
		}

		if (level.warmupTime && (!map || !*map)) {
			trap_SendServerCommand(-1, va("cp \"\nCannot vote for a mode without\nspecifying a valid map during warmup.\nUse: ^3\\callvote mode %d <map_name>\n\n\"", mode));
			trap_SendServerCommand(-1, va("print \"\nCannot vote for a mode without specifying a valid map during warmup.\nUse: ^3\\callvote mode %d <map_name|map_id>\n\n\"", mode));
			level.nextMode = 0;
			return;
		}

		gametype = g_gametype.integer;
		trap_Cvar_Set("g_gametype", va("%d", osp_modes[mode].gametype));
		trap_Cvar_Update(&g_gametype);
	}

	OSP_LoadModeMaplist(mode, map);
	if (gametype >= 0 && gametype < osp_numModes) {
		trap_Cvar_Set("g_gametype", va("%d", gametype));
		trap_Cvar_Update(&g_gametype);
	}

	if (mode >= osp_numModes) {
		trap_SetConfigstring(CS_WARMUP, "0");
		trap_Cvar_Set("z_m_current", va("%d", mode));
		if (!level.intermissiontime) {
			LogExit(va("Voted: %s", level.voteString));
		} else {
			trap_SendServerCommand(-1, va("print \"\nNext mode: ^3%s^7\n\"", OSP_ModeName(mode)));
			OSP_SetGametype(mode);
			G_LogPrintf(va("Voted: %s\n", level.voteString));
		}
		return;
	}

	if (map && *map) {
		trap_SetConfigstring(CS_WARMUP, "0");
		osp_mapChangeReason = 1;
		if (!level.intermissiontime) {
			LogExit(va("Voted: %s", level.voteString));
		} else {
			trap_SendServerCommand(-1, va("print \"\nNext mode: ^3%s^7\n\"", OSP_ModeName(mode)));
			OSP_SetGametype(mode);
			G_LogPrintf(va("Voted: %s\n", level.voteString));
		}
		return;
	}

	trap_SendServerCommand(-1, "cp \"^3Mode changed!\n^7Use ^3\\callvote map^7 to put\nnew mode into effect immediately.\nMode will automatically be changed\nat the end of this map.\n\"");
	trap_SendServerCommand(-1, "print \"^3*** Mode changed!\n^7*** Use ^3\\callvote map^7 to put new mode into\n*** effect immediately. Mode will automatically be\n*** changed at the end of this map.\n\"");
	G_LogPrintf(va("Voted: %s\n", level.voteString));
	if (level.intermissiontime) {
		OSP_SetGametype(mode);
	}
}

/*
==================
OSP_Vote_FallingDamage
==================
*/
void OSP_Vote_FallingDamage(void) {
	int     flags;

	flags = g_dmflags.integer;
	if (atoi(osp_voteValue)) {
		flags &= ~DF_NO_FALLING;
	} else {
		flags |= DF_NO_FALLING;
	}
	trap_SendServerCommand(-1, va("print \"^3Falling Damage is: ^5%s\n\"", atoi(osp_voteValue) ? ENABLED : DISABLED));
	trap_SendConsoleCommand(EXEC_APPEND, va("dmflags %d\n", flags));
	trap_Cvar_Set("default_change", "1");
}

/*
==================
OSP_Vote_ItemSet

Item / powerup vote passed: sets the item_disable or powerup_disable bit and shows or hides the
item (type, tag); name NULL: no message
==================
*/
void OSP_Vote_ItemSet(char* name, qboolean fPowerup, int bit, int tag, int type, qboolean enable) {
	int     value;

	if (name) {
		trap_SendServerCommand(-1, va("print \"^3%s is: ^5%s\n\"", name, atoi(osp_voteValue) ? ENABLED : DISABLED));
	}

	if (!fPowerup) {
		value = enable ? (item_disable.integer & ~bit) : (item_disable.integer | bit);
		trap_Cvar_Set("item_disable", va("%d", value));
		trap_Cvar_Update(&item_disable);
	} else {
		value = enable ? (powerup_disable.integer & ~bit) : (powerup_disable.integer | bit);
		trap_Cvar_Set("powerup_disable", va("%d", value));
		trap_Cvar_Update(&powerup_disable);
	}

	if (enable) {
		OSP_ShowItems(type, tag);
	} else {
		OSP_HideItems(type, tag);
	}
	trap_Cvar_Set("default_change", "1");
}

/*
==================
OSP_Vote_Instagib
==================
*/
void OSP_Vote_Instagib(void) {
	trap_Cvar_Set("match_instagib", osp_voteValue);
	trap_Cvar_Update(&match_instagib);
}

/*
==================
OSP_Vote_Toggle

0/1 votes: checks vote_allow (referees may always vote), the usage and the current value, then
sets osp_voteType / osp_voteValue; arg becomes ALLOW / DISABLE and arg2 the setting's name
==================
*/
int OSP_Vote_Toggle(gentity_t* ent, char* arg, char* arg2, int arg2Size, char* cmd, char* name, int curValue, int allowed, int vote, int voteType) {
	if (!allowed && !ent->client->sess.referee) {
		OSP_Vote_Disabled(ent, name);
		OSP_Vote_CurrentValue(ent, name, curValue ? ENABLED : DISABLED);
		return -1;
	}

	if (OSP_Vote_Usage(ent, cmd, vote)) {
		OSP_Vote_CurrentValue(ent, name, curValue ? ENABLED : DISABLED);
		return -1;
	}

	if ((atoi(arg2) && curValue) || (!atoi(arg2) && !curValue)) {
		trap_SendServerCommand(ent - g_entities, va("print \"^3%s^5 is already %s!\n", name, curValue ? ENABLED : DISABLED));
		return -1;
	}

	osp_voteType = voteType;
	Com_sprintf(osp_voteValue, sizeof(osp_voteValue), "%d", atoi(arg2) ? 1 : 0);
	Com_sprintf(arg, arg2Size, "%s", atoi(arg2) ? ALLOW : DISABLE);
	Com_sprintf(arg2, arg2Size, name);
	return 1;
}

/*
==================
OSP_Vote_SetCvar

0/1 setting vote passed: "<cvar> <osp_voteValue>"
==================
*/
void OSP_Vote_SetCvar(char* name, char* cvar) {
	trap_SendServerCommand(-1, va("print \"^3%s is: ^5%s\n\"", name, atoi(osp_voteValue) ? ENABLED : DISABLED));
	trap_SendConsoleCommand(EXEC_APPEND, va("%s %s\n", cvar, osp_voteValue));
	trap_Cvar_Set("default_change", "1");
}

/*
==================
OSP_Vote_SetValue

Value vote passed: "<cvar> <osp_voteValue>"
==================
*/
void OSP_Vote_SetValue(char* name, char* cvar) {
	trap_SendServerCommand(-1, va("print \"^3%s set to: ^5%s\n\"", name, osp_voteValue));
	trap_SendConsoleCommand(EXEC_APPEND, va("%s %s\n", cvar, osp_voteValue));
	trap_Cvar_Set("default_change", "1");
}

/*
==================
OSP_Vote_SetValueBroadcast

Limit vote passed (timelimit, fraglimit, capturelimit, roundlimit): runs level.voteString
==================
*/
void OSP_Vote_SetValueBroadcast(char* name) {
	OSP_HidePlayersInServerinfo(0);
	trap_SendServerCommand(-1, va("print \"^3%s set to: ^5%s\n\"", name, osp_voteValue));
	trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.voteString));
	trap_Cvar_Set("default_change", "1");
}
