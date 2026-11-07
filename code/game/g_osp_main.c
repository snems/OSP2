/*
 * g_osp_main.c: OSP's match logic: rounds, ready / countdown, match end stats, spawn weapons and
 * item rules, pause, auto record / screenshot, client authentication, physics modes, score
 * cvars, follow helpers, serverinfo hiding and the chat filter.
 */

#include "../qcommon/q_shared.h"
#include "g_local.h"

// OSP: functions of other files used here
void    G_ExplodeMissile(gentity_t* ent);    // g_missile.c
void    LogExit(const char* string);         // g_main.c
void    OSP_CA_AddRoundWin(int team);
void    OSP_SetMOTD(void);               // g_osp_motd.c
void    OSP_LoadCustomGfx(void);         // g_osp_customgfx.c
void    OSP_LoadDecals(void);            // g_osp_decals.c
void    OSP_LoadCustomModes(void);   // g_osp_config.c
void    OSP_LoadViewcam(void);       // g_osp_viewcam.c
void    OSP_AutoRecordStart(void);
extern  void    G_UpdateCvars(void);         // g_main.c
extern  void    CPM_UpdateSettings(int mode);    // bg_promode.c

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t    weapon_start;
vmCvar_t    weapon_have;
vmCvar_t    weapon_deaddrop;
vmCvar_t    warmup_armor;
vmCvar_t    start_bfg;
vmCvar_t    start_cells;
vmCvar_t    start_slugs;
vmCvar_t    start_lightning;
vmCvar_t    start_grenades;
vmCvar_t    start_rockets;
vmCvar_t    start_shells;
vmCvar_t    start_bullets;
vmCvar_t    start_health;
vmCvar_t    start_armor;
vmCvar_t    server_timenudgemax;
vmCvar_t    server_timenudgemin;
vmCvar_t    server_thrufloors;
vmCvar_t    server_record;
vmCvar_t    server_realping;
vmCvar_t    server_promode;
vmCvar_t    server_ospauth;
vmCvar_t    server_maxpacketsmax;
vmCvar_t    server_maxpacketsmin;
vmCvar_t    server_lgcooldown;
vmCvar_t    server_freezetag;
vmCvar_t    server_fastrail;
vmCvar_t    server_cq3;
vmCvar_t    server_batcher;
vmCvar_t    server_customclient;
vmCvar_t    Score_Time;
vmCvar_t    replace_flight;
vmCvar_t    replace_regen;
vmCvar_t    replace_haste;
vmCvar_t    replace_battlesuit;
vmCvar_t    replace_quad;
vmCvar_t    replace_invis;
vmCvar_t    replace_bfg;
vmCvar_t    replace_plasmagun;
vmCvar_t    replace_railgun;
vmCvar_t    replace_lightning;
vmCvar_t    replace_rlauncher;
vmCvar_t    replace_glauncher;
vmCvar_t    replace_shotgun;
vmCvar_t    replace_machinegun;
vmCvar_t    roundlimit;
vmCvar_t    railgun_damage;
vmCvar_t    powerup_disable;
vmCvar_t    osp_restart;
vmCvar_t    mg_damageTeamplay;
vmCvar_t    mg_damage;
vmCvar_t    match_winbyscore;
vmCvar_t    match_timeoutcount;
vmCvar_t    match_timeoutlength;
vmCvar_t    match_roundtimer;
vmCvar_t    match_mutespecs;
vmCvar_t    match_poweruprespawn;
vmCvar_t    match_overtimelength;
vmCvar_t    match_overtimecount;
vmCvar_t    match_overtime;
vmCvar_t    match_maxsuicides;
vmCvar_t    match_latejoin;
vmCvar_t    match_instagib;
vmCvar_t    match_hurtself;
vmCvar_t    match_dropitems;
vmCvar_t    match_cawarmupfire;
vmCvar_t    match_carespawn;
vmCvar_t    log_stat;
vmCvar_t    log_default;
vmCvar_t    item_disable;
vmCvar_t    instagib_reload;
vmCvar_t    hook_speedpull;
vmCvar_t    hook_speed;
vmCvar_t    hook_sky;
vmCvar_t    hook_holdtime;
vmCvar_t    hook_enable;
vmCvar_t    hook_delaytime;
vmCvar_t    gauntlet_disable;
vmCvar_t    gauntlet_damage;
vmCvar_t    freezetag_respawn;
vmCvar_t    freezetag_latejoin;
vmCvar_t    default_settings;
vmCvar_t    default_change;
vmCvar_t    bot_checktime;
vmCvar_t    armor_q2style;
int     osp_voteType;                       // declared in g_local.h
int     osp_mapWeapons[9];                  // declared in g_local.h

int     osp_pausedTime = 0;     // time spent paused (G_RunFrame; OSP_UnpauseComplete, OSP_Ref_Pause)
int     osp_matchState = 0;     // 0 waiting for players, 1 warmup countdown, 2 match in progress (CheckTournament)
// weapon names of the stats prints, indexed by weapon_t (OSP_PrintWeaponStats, OSP_LogWeaponStats)
char*    osp_weaponNames[WP_NUM_WEAPONS] = {
	"None", "Gauntlet", "MachineGun", "Shotgun", "G.Launcher", "R.Launcher", "LightningGun",
	"Railgun", "Plasmagun", "BFG", "Unknown"
};

/*
==================
OSP_IsRoundBased
==================
*/
int OSP_IsRoundBased(int gametype) {
	return BG_IsRoundBasedGametype(gametype);
}

/*
==================
OSP_InitGame

Called at the end of G_InitGame. A normal map start resets the match state (warmup) and loads
the OSP files; after OSP_SetRestart (osp_restart 1) the match goes on: the teams keep their
names, the team locks follow match_latejoin / team_nocontrols and get new join codes.
==================
*/
void OSP_InitGame(void) {
	int     i;

	if (!osp_restart.integer) {
		G_Printf("OSP2 Server Version: %s\n", XSTRINGIFY(OSP_VERSION));
		G_Printf("%s\n", "07 Nov 2026");
		G_Printf("%s\n", "https://github.com/snems/OSP2");

		osp_matchState = 0;
		osp_timeoutEndTime = 0;
		for (i = TEAM_RED; i <= TEAM_BLUE; i++) {
			osp_teamInfo[i].locked = 0;
			osp_teamInfo[i].specLocked = 0;
			osp_teamInfo[i].osp_unk_28 = 0;
			osp_teamInfo[i].timeouts = match_timeoutcount.integer;
			osp_teamInfo[i].joincode[0] = 0;
		}

		if (!g_doWarmup.integer && !g_timelimit.integer) {
			if (g_gametype.integer == GT_CTF && g_capturelimit.integer) {
				OSP_UpdateScoreCvars("Caplimit");
			} else if (g_fraglimit.integer && g_gametype.integer < GT_CTF) {
				OSP_UpdateScoreCvars("Fraglimit");
			} else if (!OSP_IsRoundBased(g_gametype.integer)) {
				OSP_UpdateScoreCvars("No Timelimit");
			}
		} else {
			OSP_UpdateScoreCvars("Waiting for Players");
		}

		if (!osp_adminLogDisabled) {
			G_Printf("\nAdmin logging info saved to: \"%s\"\n\n", admin_log.string);
		} else {
			G_Printf("\n*** Admin logging disabled! ***\n\n");
		}

		OSP_SetMOTD();
		OSP_LoadCustomGfx();
		OSP_LoadDecals();
	} else {
		int     j;
		char    joincode[5];

		trap_Cvar_Set("osp_restart", "0");
		trap_Cvar_Update(&osp_restart);

		osp_matchState = 2;
		osp_timeoutEndTime = 0;
		for (i = TEAM_RED; i <= TEAM_BLUE; i++) {
			osp_teamInfo[i].osp_unk_28 = 0;
			osp_teamInfo[i].timeouts = match_timeoutcount.integer;
			if (!match_latejoin.integer && !team_nocontrols.integer) {
				osp_teamInfo[i].locked = 1;
			}
			for (j = 0; j < 4; j++) {
				joincode[j] = random() * 26 + 'A';
			}
			joincode[4] = 0;
			strcpy(osp_teamInfo[i].joincode, joincode);
		}

		if (g_gametype.integer == 5 && match_carespawn.integer) {
			OSP_RoundStart();
		}
	}

	if (!g_doWarmup.integer && g_gametype.integer >= GT_TEAM) {
		level.osp_unk_1e00 = 1;
	}
	osp_matchPaused = 0;
	osp_unpausing = 0;
	level.osp_unk_3804 = level.time + 3000;
	level.intermissiontime = 0;
	level.intermissionQueued = 0;
	level.startMode = 0;

	OSP_LoadMaplist();
	OSP_LoadCustomModes();
	OSP_LoadViewcam();
	OSP_LoadPlayerNames();
	OSP_SaveDefaultSettings();

	if (server_ospauth.integer) {
		trap_Cvar_Set("sv_floodProtect", "0");
	}
}

/*
==================
OSP_ExitLevelHelper

Back to warmup: clears the team locks, spec locks and join codes, refills the timeouts.
==================
*/
void OSP_ExitLevelHelper(void) {
	int     i;

	osp_matchState = 0;
	OSP_HidePlayersInServerinfo(0);

	for (i = TEAM_RED; i <= TEAM_BLUE; i++) {
		osp_teamInfo[i].locked = 0;
		osp_teamInfo[i].specLocked = 0;
		osp_teamInfo[i].osp_unk_28 = 0;
		osp_teamInfo[i].timeouts = match_timeoutcount.integer;
		osp_teamInfo[i].joincode[0] = 0;
	}
}

/*
==================
OSP_SetRestart
==================
*/
void OSP_SetRestart(void) {
	trap_Cvar_Set("osp_restart", "1");
	trap_Cvar_Update(&osp_restart);
}

/*
==================
OSP_StartCountdown

Everybody is ready: the match starts in 10 seconds.
==================
*/
void OSP_StartCountdown(void) {
	if ((!g_warmup.integer || level.warmupTime > level.time + 10000) && level.warmupTime && OSP_NumReady()) {
		char*    s;

		s = "\"^3All players ready!^7\nMatch starting...\n\"";
		OSP_AutoRecordStart();
		trap_SendServerCommand(-1, va("cp %s; print %s", s, s));
		level.warmupTime = level.time + 10000;
		trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
		level.forceReady = 0;
	}
}

/*
==================
OSP_StopCountdown

Nobody is ready any more: back to warmup.
==================
*/
void OSP_StopCountdown(void) {
	if (OSP_NumReady() == 0) {
		OSP_AutoRecordStop();
		if (level.warmupTime > 0) {
			trap_SendServerCommand(-1, "cp \"^1COUNTDOWN STOPPED!^7  Back to warmup...\n\"");
		}
		level.warmupTime = -1;
		trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
		G_LogPrintf("Warmup:\n");
	}
}

/*
==================
OSP_RoundReset
==================
*/
void OSP_RoundReset(void) {
	int         i;
	gentity_t*   ent;

	for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected == CON_DISCONNECTED) {
			continue;
		}
		ent = &g_entities[i];
		ent->client->pers.ready = 0;
		ent->client->readyToExit = 0;
	}
	OSP_ResetPlayerReady();
}

/*
==================
OSP_CheckReady

Starts the countdown when every player is ready, otherwise reminds the players (once a second).
==================
*/
void OSP_CheckReady(void) {
	int         i;
	gentity_t*   ent;
	int         notReady;
	static int  nextPrint = 0;
	notReady = 0;
	if (OSP_IsRoundBased(g_gametype.integer) && osp_roundsPlayed) {
		return;
	}

	for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected == CON_DISCONNECTED || level.clients[i].sess.sessionTeam == TEAM_SPECTATOR) {
			continue;
		}
		if (!level.clients[i].pers.ready && !(g_entities[i].r.svFlags & SVF_BOT)) {
			notReady++;
		}
	}

	if (!notReady) {
		OSP_StartCountdown();
		return;
	}

	if (level.time < nextPrint) {
		return;
	}
	nextPrint = level.time + 1000;

	for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected == CON_DISCONNECTED) {
			continue;
		}
		ent = &g_entities[i];
		if (ent->client->pers.ready || ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
			trap_SendServerCommand(ent - g_entities, va("cp \"^3%d %s^7 not ready.\n\"", notReady, (notReady > 1) ? "players" : "player"));
		} else {
			trap_SendServerCommand(ent - g_entities, "cp \"Type ^3\\ready^7 in the console\nto start match.\n\"");
		}
	}
}

/*
==================
OSP_MatchEndStats

End of the match (CheckIntermissionExit): scores 0 sends everybody the weapon stats the
cg_autoAction bits ask for (tourney: both players'), scores 1 the final scoreboard (with a stats
dump for autoAction 1 when allowDump).
==================
*/
void OSP_MatchEndStats(int scores, qboolean allowDump) {
	int     i;
	int     p1, p2;

	if (level.warmupTime && osp_mapChangeReason) {
		return;
	}

	p1 = -1;
	p2 = -1;
	if (g_gametype.integer == GT_TOURNAMENT) {
		for (i = 0; i < level.numConnectedClients; i++) {
			if (level.clients[ level.sortedClients[i] ].pers.connected != CON_CONNECTED) {
				continue;
			}
			if (level.clients[ level.sortedClients[i] ].sess.sessionTeam != TEAM_SPECTATOR) {
				if (p1 > -1) {
					p2 = level.sortedClients[i];
					break;
				}
				p1 = level.sortedClients[i];
			}
		}
	}

	for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected != CON_CONNECTED) {
			continue;
		}

		if (scores == 0) {
			if (g_gametype.integer == GT_TOURNAMENT) {
				if (p1 > -1) {
					OSP_PrintWeaponStats(&g_entities[i], &g_entities[p1]);
				}
				if (p2 > -1) {
					OSP_PrintWeaponStats(&g_entities[i], &g_entities[p2]);
				}
			} else if ((level.clients[i].pers.autoAction & 16) && OSP_WinningTeam()) {
				OSP_Cmd_StatsAll_f(&g_entities[i], 0);
			} else if (level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ||
			           (OSP_IsRoundBased(g_gametype.integer) && level.clients[i].sess.roundTeam)) {
				if ((level.clients[i].pers.autoAction & 8) && OSP_WinningTeam()) {
					OSP_Cmd_StatsAll_f(&g_entities[i], level.clients[i].sess.sessionTeam);
				} else {
					OSP_PrintWeaponStats(&g_entities[i], &g_entities[i]);
				}
			} else if (level.clients[i].sess.sessionTeam == TEAM_SPECTATOR && level.clients[i].sess.spectatorState != SPECTATOR_FREE) {
				int     target;

				target = level.clients[i].sess.spectatorClient;
				if ((level.clients[i].pers.autoAction & 8) && OSP_WinningTeam()) {
					OSP_Cmd_StatsAll_f(&g_entities[i], level.clients[target].sess.sessionTeam);
				} else {
					OSP_PrintWeaponStats(&g_entities[i], &g_entities[target]);
				}
			}

			if (level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ||
			        (OSP_IsRoundBased(g_gametype.integer) && level.clients[i].sess.roundTeam)) {
				OSP_LogWeaponStats(&g_entities[i]);
			}
		} else if (scores == 1) {
			int     dump;

			dump = (!level.warmupTime && !osp_mapChangeReason && (level.clients[i].pers.autoAction & 1));
			if (g_gametype.integer == GT_TEAM) {
				if (!IS_FREEZETAG) {
					OSP_Scores_TDM(&g_entities[i], (allowDump && !level.warmupTime) ? dump : 0);
				} else {
					OSP_Scores_FT(&g_entities[i], (allowDump && !level.warmupTime) ? dump : 0);
				}
			} else if (g_gametype.integer == GT_CTF) {
				OSP_Scores_CTF(&g_entities[i], (allowDump && !level.warmupTime) ? dump : 0);
			} else if (g_gametype.integer == 5) {
				OSP_Scores_CA(&g_entities[i], (allowDump && !level.warmupTime && OSP_WinningTeam()) ? dump : 0);
			} else {
				OSP_Scores_FFA(&g_entities[i], (allowDump && !level.warmupTime) ? dump : 0);
			}

			if (OSP_IsRoundBased(g_gametype.integer)) {
				OSP_PrintMatchResult(&g_entities[i]);
			}
		}
	}
}

/*
==================
OSP_DamageStats

Damage given / received / team damage and the per-weapon hits and kills (G_Damage).
==================
*/
void OSP_DamageStats(gentity_t* targ, gentity_t* attacker, int damage, int mod) {
	int     dmg;
	int     weapon;

	weapon = WP_NONE;
	if (mod == MOD_SWITCHTEAM) {
		return;
	}
	if (!targ || !attacker || !targ->client || !attacker->client || targ == attacker || level.warmupTime ||
	        targ->s.eFlags == EF_DEAD || targ->client->ps.pm_type == PM_DEAD || mod == MOD_SWITCHTEAM || mod == MOD_SUICIDE) {
		return;
	}

	if (mod == MOD_TELEFRAG) {
		dmg = 100;
	} else {
		dmg = damage;
	}

	if (g_gametype.integer >= GT_TEAM && targ->client->sess.sessionTeam == attacker->client->sess.sessionTeam) {
		attacker->client->sess.teamDamage += dmg;
		return;
	}

	attacker->client->sess.damageGiven += dmg;
	if (g_gametype.integer == 5) {
		attacker->client->pers.caDamage += dmg;
		attacker->client->ps.persistant[PERS_SCORE] += attacker->client->pers.caDamage / 100;
		attacker->client->pers.caDamage %= 100;
	}
	targ->client->sess.damageReceived += dmg;

	switch (mod) {
		case MOD_GRENADE:
		case MOD_GRENADE_SPLASH:
			weapon = WP_GRENADE_LAUNCHER;
			break;
		case MOD_ROCKET:
		case MOD_ROCKET_SPLASH:
			weapon = WP_ROCKET_LAUNCHER;
			break;
		case MOD_PLASMA:
		case MOD_PLASMA_SPLASH:
			weapon = WP_PLASMAGUN;
			break;
		case MOD_BFG:
		case MOD_BFG_SPLASH:
			weapon = WP_BFG;
			break;
		case MOD_MACHINEGUN:
			weapon = WP_MACHINEGUN;
			break;
		case MOD_SHOTGUN:
			weapon = WP_SHOTGUN;
			break;
		case MOD_RAILGUN:
			weapon = WP_RAILGUN;
			break;
		case MOD_LIGHTNING:
			weapon = WP_LIGHTNING;
			break;
		case MOD_GAUNTLET:
			weapon = WP_GAUNTLET;
			break;
		default:
			weapon = WP_NONE;
			break;
	}

	if (weapon) {
		if (targ->health <= 0) {
			attacker->client->sess.weaponStats[weapon].kills++;
			targ->client->sess.weaponStats[weapon].deaths++;
		}
		attacker->client->sess.weaponStats[weapon].hits++;
	}
}

/*
==================
OSP_PrintWeaponStats

Sends player's weapon statistics to ent: an "xstats1" command for OSP clients >= 1007, else
the printed accuracy table.
==================
*/
void OSP_PrintWeaponStats(gentity_t* ent, gentity_t* player) {
	int     i;
	int     printed;
	char    name[MAX_STRING_CHARS];

	printed = 0;
	if (!ent || !player || (ent->r.svFlags & SVF_BOT)) {
		return;
	}

	if (ent->client->sess.ospClient >= 1007) {
		int     j;
		char    buf[MAX_STRING_CHARS];
		int     mask;

		mask = 0;
		buf[0] = 0;
		for (j = WP_GAUNTLET; j <= WP_BFG; j++) {
			if (player->client->sess.weaponStats[j].atts || player->client->sess.weaponStats[j].hits ||
			        player->client->sess.weaponStats[j].deaths) {
				mask |= 1 << j;
				Q_strcat(buf, sizeof(buf), va(" %d %d %d %d",
				                              player->client->sess.weaponStats[j].hits, player->client->sess.weaponStats[j].atts,
				                              player->client->sess.weaponStats[j].kills, player->client->sess.weaponStats[j].deaths));
			}
		}
		Q_strcat(buf, sizeof(buf), va(" %d %d %d %d %d %d %d %d",
		                              player->client->sess.wins >> 16, player->client->sess.losses >> 16,
		                              player->client->sess.damageGiven, player->client->sess.damageReceived,
		                              player->client->sess.megaHealthPickups, player->client->sess.greenArmorPickups,
		                              player->client->sess.redArmorPickups, player->client->sess.yellowArmorPickups));
		trap_SendServerCommand(ent - g_entities, va("xstats1 %d %d%s\n", player - g_entities, mask, buf));
		return;
	}

	SanitizeString(player->client->pers.netname, name, qfalse);
	for (i = 0; i < WP_BFG; i++) {
		if (player->client->sess.weaponStats[i].atts || player->client->sess.weaponStats[i].hits ||
		        player->client->sess.weaponStats[i].deaths) {
			if (!printed) {
				trap_SendServerCommand(ent - g_entities, va("print \"\nAccuracy info for ^3%s^7\nWeapon       Accrcy Hits/Atts Kills Deaths Pickup Drops\n-------------------------------------------------------\n\"", name));
			}
			trap_SendServerCommand(ent - g_entities, va("print \"^2%-12s^7 %6.1f ^5%4d/%-4d ^2%5d ^1%6d ^3%6d ^1%5d\n\"",
			                       osp_weaponNames[i],
			                       !(player->client->sess.weaponStats[i].atts & 0xffff) ? 0 :
			                       100 * (player->client->sess.weaponStats[i].hits & 0xffff) / (float)(player->client->sess.weaponStats[i].atts & 0xffff),
			                       player->client->sess.weaponStats[i].hits & 0xffff, player->client->sess.weaponStats[i].atts & 0xffff,
			                       player->client->sess.weaponStats[i].kills, player->client->sess.weaponStats[i].deaths,
			                       player->client->sess.weaponStats[i].atts >> 16, player->client->sess.weaponStats[i].hits >> 16));
			printed = 1;
		}
	}

	if (printed) {
		trap_SendServerCommand(ent - g_entities, va("print \"\nTotal armor taken : ^2%d^7 (%d GA, %d YA, %d RA)\nTotal health taken: ^2%d^7 (%d MHs)\n\nTotal damage given: ^3%d^7\nTotal damage rcvd : ^3%d^7\n\n\"",
		                       player->client->sess.wins >> 16, player->client->sess.greenArmorPickups,
		                       player->client->sess.yellowArmorPickups, player->client->sess.redArmorPickups,
		                       player->client->sess.losses >> 16, player->client->sess.megaHealthPickups,
		                       player->client->sess.damageGiven, player->client->sess.damageReceived));
	} else {
		trap_SendServerCommand(ent - g_entities, va("print \"\n^5%s^7 didn't shoot a thing!\n\n\"", name));
	}
}

/*
==================
OSP_LogWeaponStats

"Weapon_Stats:" log line: per weapon attacks:hits:pickups:drops, damage, armor / health taken.
==================
*/
void OSP_LogWeaponStats(gentity_t* ent) {
	int     i;
	char    buf[2048];

	strcpy(buf, va("Weapon_Stats: %d", ent - g_entities));
	for (i = 0; i < WP_BFG; i++) {   // (no BFG line)
		if (ent->client->sess.weaponStats[i].atts || ent->client->sess.weaponStats[i].hits) {
			strcat(buf, va(" %s:%d:%d:%d:%d", osp_weaponNames[i],
			               ent->client->sess.weaponStats[i].atts & 0xffff, ent->client->sess.weaponStats[i].hits & 0xffff,
			               ent->client->sess.weaponStats[i].atts >> 16, ent->client->sess.weaponStats[i].hits >> 16));
		}
	}

	if (g_gametype.integer < GT_TEAM) {
		strcat(buf, va(" Given:%d Recvd:%d Armor:%d Health:%d",
		               ent->client->sess.damageGiven, ent->client->sess.damageReceived,
		               ent->client->sess.wins >> 16, ent->client->sess.losses >> 16));
	} else {
		strcat(buf, va(" Given:%d Recvd:%d Armor:%d Health:%d TeamDmg:%d",
		               ent->client->sess.damageGiven, ent->client->sess.damageReceived,
		               ent->client->sess.wins >> 16, ent->client->sess.losses >> 16, ent->client->sess.teamDamage));
	}
	G_LogPrintf("%s\n", buf);
}

/*
==================
OSP_ItemAllowed

Nonzero when the item may spawn: instagib keeps only
the flags, powerup_disable / item_disable bits remove items; weapons that spawn are noted in
osp_mapWeapons (warmup spawns get them, OSP_SetSpawnWeapons).
==================
*/
int OSP_ItemAllowed(gentity_t* ent) {
	int     flags;

	flags = item_disable.integer;
	if (match_instagib.integer > 0) {
		return (ent->item->giType == IT_TEAM && (ent->item->giTag == PW_REDFLAG || ent->item->giTag == PW_BLUEFLAG));
	}

	if (ent->item->giType == IT_POWERUP || ent->item->giType == IT_HOLDABLE) {
		flags = powerup_disable.integer;
		if ((flags & 1) && ent->item->giTag == PW_QUAD && ent->item->giType == IT_POWERUP)
			return 0;
		if ((flags & 2) && ent->item->giTag == PW_BATTLESUIT && ent->item->giType == IT_POWERUP)
			return 0;
		if ((flags & 4) && ent->item->giTag == PW_HASTE && ent->item->giType == IT_POWERUP)
			return 0;
		if ((flags & 8) && ent->item->giTag == PW_REGEN && ent->item->giType == IT_POWERUP)
			return 0;
		if ((flags & 16) && ent->item->giTag == PW_INVIS && ent->item->giType == IT_POWERUP)
			return 0;
		if ((flags & 32) && ent->item->giTag == PW_FLIGHT && ent->item->giType == IT_POWERUP)
			return 0;
		if ((flags & 64) && ent->item->giTag == HI_TELEPORTER && ent->item->giType == IT_HOLDABLE)
			return 0;
		if ((flags & 128) && ent->item->giTag == HI_MEDKIT && ent->item->giType == IT_HOLDABLE)
			return 0;
		return 1;
	}

	// (OSP tests the type twice)
	if ((flags & 1) && ent->item->giType == IT_ARMOR && ent->item->giType == IT_ARMOR)
		return 0;
	if ((flags & 2) && ent->item->giType == IT_HEALTH && ent->item->giType == IT_HEALTH)
		return 0;
	if (ent->item->giTag == WP_BFG && (ent->item->giType == IT_WEAPON || ent->item->giType == IT_AMMO)) {
		if (flags & 4) {
			osp_mapWeapons[WP_BFG - 1] = 0;
			return 0;
		}
		osp_mapWeapons[WP_BFG - 1] = 1;
	}
	if (ent->item->giTag == WP_MACHINEGUN && (ent->item->giType == IT_WEAPON || ent->item->giType == IT_AMMO)) {
		if (flags & 8) {
			osp_mapWeapons[WP_MACHINEGUN - 1] = 0;
			return 0;
		}
		osp_mapWeapons[WP_MACHINEGUN - 1] = 1;
	}
	if (ent->item->giTag == WP_SHOTGUN && (ent->item->giType == IT_WEAPON || ent->item->giType == IT_AMMO)) {
		if (flags & 16) {
			osp_mapWeapons[WP_SHOTGUN - 1] = 0;
			return 0;
		}
		osp_mapWeapons[WP_SHOTGUN - 1] = 1;
	}
	if (ent->item->giTag == WP_ROCKET_LAUNCHER && (ent->item->giType == IT_WEAPON || ent->item->giType == IT_AMMO)) {
		if (flags & 64) {
			osp_mapWeapons[WP_ROCKET_LAUNCHER - 1] = 0;
			return 0;
		}
		osp_mapWeapons[WP_ROCKET_LAUNCHER - 1] = 1;
	}
	if (ent->item->giTag == WP_GRENADE_LAUNCHER && (ent->item->giType == IT_WEAPON || ent->item->giType == IT_AMMO)) {
		if (flags & 32) {
			osp_mapWeapons[WP_GRENADE_LAUNCHER - 1] = 0;
			return 0;
		}
		osp_mapWeapons[WP_GRENADE_LAUNCHER - 1] = 1;
	}
	if (ent->item->giTag == WP_LIGHTNING && (ent->item->giType == IT_WEAPON || ent->item->giType == IT_AMMO)) {
		if (flags & 128) {
			osp_mapWeapons[WP_LIGHTNING - 1] = 0;
			return 0;
		}
		osp_mapWeapons[WP_LIGHTNING - 1] = 1;
	}
	if (ent->item->giTag == WP_PLASMAGUN && (ent->item->giType == IT_WEAPON || ent->item->giType == IT_AMMO)) {
		if (flags & 256) {
			osp_mapWeapons[WP_PLASMAGUN - 1] = 0;
			return 0;
		}
		// OSP bug: tests the classname of a flag
		if (Q_stricmp(ent->classname, "team_CTF_blueflag")) {
			osp_mapWeapons[WP_PLASMAGUN - 1] = 1;
		}
	}
	if (ent->item->giTag == WP_RAILGUN && (ent->item->giType == IT_WEAPON || ent->item->giType == IT_AMMO)) {
		if (flags & 512) {
			osp_mapWeapons[WP_RAILGUN - 1] = 0;
			return 0;
		}
		osp_mapWeapons[WP_RAILGUN - 1] = 1;
	}
	return 1;
}

/*
==================
OSP_RegisterStartWeapons

Registers the weapons players can spawn with (weapon_have / weapon_start bits).
==================
*/
void OSP_RegisterStartWeapons(void) {
	if ((weapon_have.integer & 1) || (weapon_start.integer & 1)) {
		RegisterItem(BG_FindItemForWeapon(WP_MACHINEGUN));
	}
	RegisterItem(BG_FindItem("Bullets"));
	if ((weapon_have.integer & 2) || (weapon_start.integer & 2)) {
		RegisterItem(BG_FindItemForWeapon(WP_SHOTGUN));
	}
	if ((weapon_have.integer & 4) || (weapon_start.integer & 4)) {
		RegisterItem(BG_FindItemForWeapon(WP_GRENADE_LAUNCHER));
	}
	if ((weapon_have.integer & 8) || (weapon_start.integer & 8)) {
		RegisterItem(BG_FindItemForWeapon(WP_ROCKET_LAUNCHER));
	}
	if ((weapon_have.integer & 16) || (weapon_start.integer & 16)) {
		RegisterItem(BG_FindItemForWeapon(WP_LIGHTNING));
	}
	if ((weapon_have.integer & 64) || (weapon_start.integer & 64)) {
		RegisterItem(BG_FindItemForWeapon(WP_PLASMAGUN));
	}
	if ((weapon_have.integer & 128) || (weapon_start.integer & 128)) {
		RegisterItem(BG_FindItemForWeapon(WP_BFG));
	}
	RegisterItem(BG_FindItemForWeapon(WP_GRAPPLING_HOOK));
	RegisterItem(BG_FindItemForWeapon(WP_RAILGUN));
}

/*
==================
OSP_SetSpawnWeapons

Health, armor, ammo and weapons of a spawning player (start_* cvars; fixed warmup values, and
every weapon of the map in warmup; instagib: railgun and gauntlet).
==================
*/
void OSP_SetSpawnWeapons(gentity_t* ent) {
	gclient_t*   client;

	client = ent->client;
	if (!level.physicsMode) {
		ent->health = client->ps.stats[STAT_HEALTH] = start_health.integer;
	} else {
		ent->health = client->ps.stats[STAT_HEALTH] = atoi("100");
	}

	client->ps.stats[STAT_WEAPONS] = 0;
	client->ps.ammo[WP_GAUNTLET] = 0;
	client->ps.ammo[WP_GRAPPLING_HOOK] = -1;

	if (match_instagib.integer > 0) {
		client->ps.ammo[WP_RAILGUN] = 999;
		client->ps.ammo[WP_GAUNTLET] = -1;
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_RAILGUN);
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_GAUNTLET);
		client->weaponCount[WP_RAILGUN] = 1;
		client->weaponCount[WP_GAUNTLET] = 1;
		return;
	}

	if (g_gametype.integer != 5 && level.warmupTime && !osp_matchPaused) {
		client->ps.stats[STAT_ARMOR] = warmup_armor.integer;
		if (warmup_armor.integer) {
			client->ps.stats[STAT_ARMOR_TYPE] = (pm_armorPromode) ? ARMOR_RED : ARMOR_YELLOW;
		}
		client->ps.ammo[WP_MACHINEGUN] = (z_m_current.integer == 2) ? 100 : 50;
		client->ps.ammo[WP_SHOTGUN] = 30;
		client->ps.ammo[WP_GRENADE_LAUNCHER] = 50;
		client->ps.ammo[WP_ROCKET_LAUNCHER] = 50;
		client->ps.ammo[WP_LIGHTNING] = 150;
		client->ps.ammo[WP_RAILGUN] = 50;
		client->ps.ammo[WP_PLASMAGUN] = 150;
		client->ps.ammo[WP_BFG] = 0;
	} else {
		client->ps.stats[STAT_ARMOR] = start_armor.integer;
		if (start_armor.integer) {
			client->ps.stats[STAT_ARMOR_TYPE] = (pm_armorPromode) ? ARMOR_RED : ARMOR_YELLOW;
		}
		client->ps.ammo[WP_MACHINEGUN] = start_bullets.integer;
		client->ps.ammo[WP_SHOTGUN] = start_shells.integer;
		client->ps.ammo[WP_GRENADE_LAUNCHER] = start_grenades.integer;
		client->ps.ammo[WP_ROCKET_LAUNCHER] = start_rockets.integer;
		client->ps.ammo[WP_LIGHTNING] = start_lightning.integer;
		client->ps.ammo[WP_RAILGUN] = start_slugs.integer;
		client->ps.ammo[WP_PLASMAGUN] = start_cells.integer;
		client->ps.ammo[WP_BFG] = start_bfg.integer;
	}

	if (!gauntlet_disable.integer || (weapon_have.integer & 256) || (weapon_start.integer & 256) ||
	        (level.warmupTime && !osp_matchPaused && osp_mapWeapons[WP_GAUNTLET - 1])) {
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_GAUNTLET);
		client->weaponCount[WP_GAUNTLET] = 1;
		client->ps.ammo[WP_GAUNTLET] = -1;
	}
	if ((weapon_have.integer & 1) || (weapon_start.integer & 1) ||
	        (level.warmupTime && !osp_matchPaused && osp_mapWeapons[WP_MACHINEGUN - 1])) {
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_MACHINEGUN);
		client->weaponCount[WP_MACHINEGUN] = 1;
	}
	if ((weapon_have.integer & 2) || (weapon_start.integer & 2) ||
	        (level.warmupTime && !osp_matchPaused && osp_mapWeapons[WP_SHOTGUN - 1])) {
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_SHOTGUN);
		client->weaponCount[WP_SHOTGUN] = 1;
	}
	if ((weapon_have.integer & 4) || (weapon_start.integer & 4) ||
	        (level.warmupTime && !osp_matchPaused && osp_mapWeapons[WP_GRENADE_LAUNCHER - 1])) {
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_GRENADE_LAUNCHER);
		client->weaponCount[WP_GRENADE_LAUNCHER] = 1;
	}
	if ((weapon_have.integer & 8) || (weapon_start.integer & 8) ||
	        (level.warmupTime && !osp_matchPaused && osp_mapWeapons[WP_ROCKET_LAUNCHER - 1])) {
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_ROCKET_LAUNCHER);
		client->weaponCount[WP_ROCKET_LAUNCHER] = 1;
	}
	if ((weapon_have.integer & 16) || (weapon_start.integer & 16) ||
	        (level.warmupTime && !osp_matchPaused && osp_mapWeapons[WP_LIGHTNING - 1])) {
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_LIGHTNING);
		client->weaponCount[WP_LIGHTNING] = 1;
	}
	if ((weapon_have.integer & 32) || (weapon_start.integer & 32) ||
	        (level.warmupTime && !osp_matchPaused && osp_mapWeapons[WP_RAILGUN - 1])) {
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_RAILGUN);
		client->weaponCount[WP_RAILGUN] = 1;
	}
	if ((weapon_have.integer & 64) || (weapon_start.integer & 64) ||
	        (level.warmupTime && !osp_matchPaused && osp_mapWeapons[WP_PLASMAGUN - 1])) {
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_PLASMAGUN);
		client->weaponCount[WP_PLASMAGUN] = 1;
	}
	if ((weapon_have.integer & 128) || (weapon_start.integer & 128) ||
	        (level.warmupTime && !osp_matchPaused && osp_mapWeapons[WP_BFG - 1])) {
		client->ps.stats[STAT_WEAPONS] |= (1 << WP_BFG);
		client->weaponCount[WP_BFG] = 1;
	}
}

/*
==================
OSP_SelectSpawnWeapon

Selects the spawn weapon: the railgun in instagib,
else the best weapon of weapon_start.
==================
*/
void OSP_SelectSpawnWeapon(gentity_t* ent) {
	gclient_t*   client;

	client = ent->client;
	if (match_instagib.integer > 0) {
		client->ps.weapon = WP_RAILGUN;
	} else if (weapon_start.integer & 128) {
		client->ps.weapon = WP_BFG;
	} else if (weapon_start.integer & 64) {
		client->ps.weapon = WP_PLASMAGUN;
	} else if (weapon_start.integer & 32) {
		client->ps.weapon = WP_RAILGUN;
	} else if (weapon_start.integer & 16) {
		client->ps.weapon = WP_LIGHTNING;
	} else if (weapon_start.integer & 8) {
		client->ps.weapon = WP_ROCKET_LAUNCHER;
	} else if (weapon_start.integer & 4) {
		client->ps.weapon = WP_GRENADE_LAUNCHER;
	} else if (weapon_start.integer & 2) {
		client->ps.weapon = WP_SHOTGUN;
	} else if (weapon_start.integer & 1) {
		client->ps.weapon = WP_MACHINEGUN;
	} else if (weapon_start.integer & 256) {
		client->ps.weapon = WP_GAUNTLET;
	} else {
		client->ps.weapon = WP_NONE;
	}
	ent->s.weapon = client->ps.weapon;
	client->ps.weaponstate = WEAPON_READY;
}

/*
==================
OSP_PM_UseAmmo

Nonzero when firing uses ammo (not in clan arena warmup).
==================
*/
int OSP_PM_UseAmmo(void) {
	if (g_gametype.integer != 5 || !level.warmupTime) {
		return 1;
	}
	return 0;
}

/*
==================
OSP_PM_CanFire

Nonzero when firing is allowed (clan arena warmup
needs match_cawarmupfire).
==================
*/
int OSP_PM_CanFire(void) {
	if (g_gametype.integer != 5 || !level.warmupTime || match_cawarmupfire.integer) {
		return 1;
	}
	return 0;
}

/*
==================
OSP_Respawn

Puts a player back to free spectator mode.
==================
*/
void OSP_Respawn(gentity_t* ent) {
	ent->client->pers.ready = 0;
	ent->client->readyToExit = 0;
	ent->client->pers.invitedTeam = 0;
	ent->client->sess.roundTeam = ent->client->sess.sessionTeam;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	ent->client->ps.stats[STAT_HEALTH] = 0;
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;
	ent->client->sess.spectatorTime = level.time;
	ent->client->pers.osp_unk_2bc = level.time;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->sess.spectatorClient = 0;
	ClientUserinfoChanged(ent - g_entities);
	ClientSpawn(ent);
	OSP_ResetPlayerReady();
	CalculateRanks(0);
}

/*
==================
OSP_PrintMatchResult

Prints the team scores to ent (wins / leads / tied).
==================
*/
void OSP_PrintMatchResult(gentity_t* ent) {
	int     winner;
	int     leads;

	winner = OSP_WinningTeam();
	if (winner) {
		trap_SendServerCommand(ent - g_entities, va("print \"%s wins the match: ^3%d - %d\n\"",
		                       osp_teamNames[winner], level.teamScores[winner], level.teamScores[TEAM_BLUE - winner + TEAM_RED]));
	} else {
		leads = 0;
		if (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE]) {
			winner = TEAM_RED;
			leads = 1;
		} else if (level.teamScores[TEAM_RED] < level.teamScores[TEAM_BLUE]) {
			winner = TEAM_BLUE;
			leads = 1;
		}

		if (leads) {
			trap_SendServerCommand(ent - g_entities, va("print \"%s leads: ^3%d - %d\n\"",
			                       osp_teamNames[winner], level.teamScores[winner], level.teamScores[TEAM_BLUE - winner + TEAM_RED]));
		} else {
			// OSP prints the red score twice
			trap_SendServerCommand(ent - g_entities, va("print \"Match tied: ^3%d - %d\n\"",
			                       level.teamScores[TEAM_RED], level.teamScores[TEAM_RED]));
		}
	}
}

/*
==================
OSP_CA_RoundEnd

Clan arena: a round ends when a team has no living player left (after the last grenades and
rockets in flight exploded, at most 5 seconds).
==================
*/
void OSP_CA_RoundEnd(void) {
	gentity_t*   ent;
	int         i;
	int         score;
	static int  roundWinner = 0;    // TEAM_RED / TEAM_BLUE, -1 tied
	static int  roundEndTime = 0;   // wait for missiles until this time

	if (osp_matchState != 2 || osp_matchTime - level.startTime < 5000 || roundWinner) {
		return;
	}
	if (TeamCount(-1, TEAM_RED) && TeamCount(-1, TEAM_BLUE)) {
		return;
	}

	if (!roundEndTime || roundEndTime > osp_matchTime) {
		for (i = level.maxclients, ent = &g_entities[level.maxclients]; i < level.num_entities; i++, ent++) {
			if (!ent->inuse) {
				continue;
			}
			if (ent->think == G_ExplodeMissile) {
				roundEndTime = (roundEndTime) ? roundEndTime : osp_matchTime + 5000;
				return;
			}
		}
	}

	if (!TeamCount(-1, TEAM_RED) && TeamCount(-1, TEAM_BLUE)) {
		roundWinner = TEAM_BLUE;
	} else if (!TeamCount(-1, TEAM_BLUE) && TeamCount(-1, TEAM_RED)) {
		roundWinner = TEAM_RED;
	} else {
		roundWinner = -1;
	}

	if (roundWinner > 0) {
		level.teamScores[roundWinner]++;
		score = level.teamScores[roundWinner];
		trap_Cvar_Set("Score_Red", va("%d", level.teamScores[TEAM_RED]));
		trap_Cvar_Set("Score_Blue", va("%d", level.teamScores[TEAM_BLUE]));
		if (score < roundlimit.integer)
			osp_roundsPlayed++;
		OSP_CA_AddRoundWin(roundWinner);
		trap_SendServerCommand(-1, va("print \"%s wins the round!\n\"", osp_teamNames[roundWinner]));
		LogExit(va("ClanArena: %s wins round", osp_teamNames[roundWinner]));
	} else {
		trap_SendServerCommand(-1, "print \"Tied round!\n\"");
		LogExit("ClanArena: Tied Round");
	}
}

/*
==================
OSP_CA_AddRoundWin
==================
*/
void OSP_CA_AddRoundWin(int team) {
	int         i;
	gclient_t*   cl;

	for (i = 0; i < level.numConnectedClients; i++) {
		cl = &level.clients[ level.sortedClients[i] ];
		if (cl->sess.sessionTeam != team) {
			continue;
		}
		cl->sess.wins++;
	}
}

/*
==================
OSP_ExitLevelSession

Round-based match continues on the same map: players waiting for the next round get their
team back, the session is saved and the map restarted.
==================
*/
void OSP_ExitLevelSession(void) {
	gclient_t*   cl;
	int         i;

	for (i = 0; i < level.maxclients; i++) {
		cl = &level.clients[i];
		if (cl->pers.connected != CON_CONNECTED) {
			continue;
		}
		if (cl->sess.sessionTeam != TEAM_RED && cl->sess.sessionTeam != TEAM_BLUE &&
		        (cl->sess.roundTeam == TEAM_RED || cl->sess.roundTeam == TEAM_BLUE)) {
			cl->sess.sessionTeam = cl->sess.roundTeam;
		}
	}

	G_WriteSessionData();
	OSP_MapRestart();
	level.restarted = qtrue;
	level.changemap = NULL;
	level.intermissiontime = 0;
}

/*
==================
OSP_TeamPlayerCount
==================
*/
int OSP_TeamPlayerCount(int team) {
	int     i;
	int     count;

	count = 0;
	for (i = 0; i < level.numConnectedClients; i++) {
		if (OSP_ClientTeam(&g_entities[ level.sortedClients[i] ]) == team) {
			count++;
		}
	}
	return count;
}

/*
==================
OSP_ResetClientStats
==================
*/
void OSP_ResetClientStats(int clientNum) {
	gclient_t*   cl;
	int         i;

	cl = &level.clients[clientNum];
	if (g_gametype.integer != GT_TOURNAMENT) {
		cl->sess.wins = 0;
		cl->sess.losses = 0;
	}
	// armor / health taken live in the high 16 bits
	cl->sess.wins &= 0xffff;
	cl->sess.losses &= 0xffff;

	for (i = 0; i < WP_NUM_WEAPONS; i++) {
		cl->sess.weaponStats[i].hits = 0;
		cl->sess.weaponStats[i].atts = 0;
		cl->sess.weaponStats[i].kills = 0;
		cl->sess.weaponStats[i].deaths = 0;
	}

	cl->ps.persistant[PERS_SCORE] = 0;
	cl->ps.persistant[PERS_IMPRESSIVE_COUNT] = 0;
	cl->ps.persistant[PERS_EXCELLENT_COUNT] = 0;
	cl->ps.persistant[PERS_DEFEND_COUNT] = 0;
	cl->ps.persistant[PERS_ASSIST_COUNT] = 0;
	cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT] = 0;
	cl->ps.persistant[PERS_CAPTURES] = 0;
	cl->sess.deaths = 0;
	cl->sess.kills = 0;
	cl->sess.suicides = 0;
	cl->sess.damageGiven = 0;
	cl->sess.damageReceived = 0;
	cl->sess.teamDamage = 0;
	cl->sess.teamKills = 0;
	cl->sess.megaHealthPickups = 0;
	cl->sess.greenArmorPickups = 0;
	cl->sess.redArmorPickups = 0;
	cl->sess.yellowArmorPickups = 0;
	cl->pers.teamState.assists = 0;
	cl->pers.teamState.basedefense = 0;
	cl->pers.teamState.captures = 0;
	cl->pers.teamState.carrierdefense = 0;
	cl->pers.teamState.flagrecovery = 0;
}

/*
==================
OSP_ResetAllClientStats
==================
*/
void OSP_ResetAllClientStats(void) {
	int     i;

	for (i = 0; i < level.maxclients; i++) {
		OSP_ResetClientStats(i);
	}
}

/*
==================
OSP_HideItems

Removes the items of type / tag from the map (powerup_disable vote): visible ones are hidden,
pending respawns cancelled.
==================
*/
void OSP_HideItems(int type, int tag) {
	gentity_t*   ent;
	int         i;

	OSP_GlobalSound("sound/items/wearoff.wav");

	for (ent = &g_entities[level.maxclients], i = level.maxclients; i < level.num_entities; i++, ent++) {
		if (!ent->inuse) {
			continue;
		}
		if (!ent->item) {
			continue;
		}
		if (ent->item->giType != type && (type != IT_WEAPON || ent->item->giType != IT_AMMO)) {
			continue;
		}
		if (ent->item->giTag != tag) {
			continue;
		}

		if (!(ent->s.eFlags & EF_NODRAW) && ent->r.contents) {
			ent->r.svFlags |= SVF_NOCLIENT;
			ent->s.eFlags |= EF_NODRAW;
			ent->r.contents = 0;
			trap_LinkEntity(ent);
			if (ent->team) {
				RespawnItem(ent);
			}
		} else if (ent->nextthink && ent->think && !ent->team) {
			ent->nextthink = 0;
			ent->think = 0;
			trap_LinkEntity(ent);
		}
	}
}

/*
==================
OSP_ShowItems

Puts the hidden items of type / tag back (item_disable vote); for team items (RespawnItem's
code) only when no member of the team is visible or about to respawn.
==================
*/
void OSP_ShowItems(int type, int tag) {
	gentity_t*   ent;
	int         i;

	for (ent = &g_entities[level.maxclients], i = level.maxclients; i < level.num_entities; i++, ent++) {
		if (!ent->inuse || !ent->item) {
			continue;
		}
		if (ent->item->giType != type && (type != IT_WEAPON || ent->item->giType != IT_AMMO)) {
			continue;
		}
		if (ent->item->giTag != tag) {
			continue;
		}

		if ((ent->s.eFlags & EF_NODRAW) && !ent->r.contents) {
			if (ent->team) {
				gentity_t*   e;
				int         count;
				gentity_t*   master;

				e = ent;
				if (!e->teammaster) {
					G_Error("RespawnItem: bad teammaster");
				}
				master = e->teammaster;

				for (count = 0, e = master; e; e = e->teamchain) {
					if ((!(e->s.eFlags & EF_NODRAW) && e->r.contents) || (e->nextthink && e->think)) {
						count++;
					}
				}
				if (!count) {
					RespawnItem(ent);
				}
			} else {
				RespawnItem(ent);
			}
		}
	}
}

/*
==================
OSP_BuzzerSound
==================
*/
void OSP_BuzzerSound(void) {
	OSP_GlobalSound("sound/world/buzzer.wav");
}

// OSP: the item replacement tables (q3asm puts
// byte data into the lit segment). Index order is the replace_* cvars' bit order.
typedef struct {
	char    item[128];
	char    ammo[128];      // "X" for powerups
	char    replItem[128];  // what the item spawns as (OSP_InitItemNames, OSP_ReplaceSpawnItem)
	char    replAmmo[128];
} ospItemReplace_t;

#define OSP_NUM_REPLACE_ITEMS   14

ospItemReplace_t    osp_itemNames[OSP_NUM_REPLACE_ITEMS] = {
	{ "weapon_machinegun", "ammo_bullets" },
	{ "weapon_shotgun", "ammo_shells" },
	{ "weapon_grenadelauncher", "ammo_grenades" },
	{ "weapon_rocketlauncher", "ammo_rockets" },
	{ "weapon_lightning", "ammo_lightning" },
	{ "weapon_railgun", "ammo_slugs" },
	{ "weapon_plasmagun", "ammo_cells" },
	{ "weapon_bfg", "ammo_bfg" },
	{ "item_invis", "X" },
	{ "item_quad", "X" },
	{ "item_enviro", "X" },
	{ "item_haste", "X" },
	{ "item_regen", "X" },
	{ "item_flight", "X" }
};

ospItemReplace_t    osp_itemReplace[OSP_NUM_REPLACE_ITEMS] = {
	{ "weapon_machinegun", "ammo_bullets", "weapon_machinegun", "ammo_bullets" },
	{ "weapon_shotgun", "ammo_shells", "weapon_shotgun", "ammo_shells" },
	{ "weapon_grenadelauncher", "ammo_grenades", "weapon_grenadelauncher", "ammo_grenades" },
	{ "weapon_rocketlauncher", "ammo_rockets", "weapon_rocketlauncher", "ammo_rockets" },
	{ "weapon_lightning", "ammo_lightning", "weapon_lightning", "ammo_lightning" },
	{ "weapon_railgun", "ammo_slugs", "weapon_railgun", "ammo_slugs" },
	{ "weapon_plasmagun", "ammo_cells", "weapon_plasmagun", "ammo_cells" },
	{ "weapon_bfg", "ammo_bfg", "weapon_bfg", "ammo_bfg" },
	{ "item_invis", "X", "item_invis", "X" },
	{ "item_quad", "X", "item_quad", "X" },
	{ "item_enviro", "X", "item_enviro", "X" },
	{ "item_haste", "X", "item_haste", "X" },
	{ "item_regen", "X", "item_regen", "X" },
	{ "item_flight", "X", "item_flight", "X" }
};

/*
==================
OSP_InitItemNames

replace_<item> is a bit mask: the lowest set bit picks the item it spawns as.
==================
*/
void OSP_InitItemNames(void) {
	int     j;
	int     i;
	int     bit;
	int     replace[OSP_NUM_REPLACE_ITEMS];

	replace[0] = replace_machinegun.integer;
	replace[1] = replace_shotgun.integer;
	replace[2] = replace_glauncher.integer;
	replace[3] = replace_rlauncher.integer;
	replace[4] = replace_lightning.integer;
	replace[5] = replace_railgun.integer;
	replace[6] = replace_plasmagun.integer;
	replace[7] = replace_bfg.integer;
	replace[8] = replace_invis.integer;
	replace[9] = replace_quad.integer;
	replace[10] = replace_battlesuit.integer;
	replace[11] = replace_haste.integer;
	replace[12] = replace_regen.integer;
	replace[13] = replace_flight.integer;

	for (i = 0; i < OSP_NUM_REPLACE_ITEMS; i++) {
		for (j = 0, bit = 1; j < OSP_NUM_REPLACE_ITEMS; j++, bit <<= 1) {
			if (replace[i] & bit) {
				strcpy(osp_itemReplace[i].replItem, osp_itemNames[j].item);
				strcpy(osp_itemReplace[i].replAmmo, osp_itemNames[j].ammo);
				break;
			}
		}
	}
}

/*
==================
OSP_ReplaceSpawnItem

Replaces a spawned item's classname by its replacement (replace_* cvars).
==================
*/
void OSP_ReplaceSpawnItem(char* classname) {
	int     i;

	for (i = 0; i < OSP_NUM_REPLACE_ITEMS; i++) {
		if (!Q_stricmp(classname, osp_itemReplace[i].item)) {
			strcpy(classname, osp_itemReplace[i].replItem);
			return;
		}
		if (!Q_stricmp(classname, osp_itemReplace[i].ammo)) {
			strcpy(classname, osp_itemReplace[i].replAmmo);
			return;
		}
	}
}

/*
==================
OSP_ResetPlayerReady

Sends the ready mask (bots count as ready in warmup) in STAT_CLIENTS_READY.
==================
*/
void OSP_ResetPlayerReady(void) {
	int         i;
	gclient_t*   cl;
	int         ready;

	ready = 0;
	if (level.warmupTime) {
		for (i = 0; i < g_maxclients.integer; i++) {
			cl = &level.clients[i];
			if (cl->pers.connected != CON_CONNECTED) {
				continue;
			}
			if (i < 16 && (!OSP_IsRoundBased(g_gametype.integer) || (!osp_roundsPlayed && level.warmupTime)) &&
			        (g_entities[i].r.svFlags & SVF_BOT)) {
				ready |= 1 << i;
			} else if (i < 16 && cl->pers.ready) {
				ready |= 1 << i;
			}
		}
	}

	for (i = 0; i < g_maxclients.integer; i++) {
		cl = &level.clients[i];
		if (cl->pers.connected != CON_CONNECTED) {
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = ready;
	}
}

/*
==================
OSP_SaveDefaultSettings

Saves the current settings in default_settings
(default_change restores them, OSP_RestoreDefaultSettings).
==================
*/
void OSP_SaveDefaultSettings(void) {
	int     pure;

	if (default_settings.string[0] && Q_stricmp(default_settings.string, "none")) {
		return;
	}

	pure = trap_Cvar_VariableIntegerValue("sv_pure");
	trap_Cvar_Set("default_change", "0");
	trap_Cvar_Set("default_settings", va("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
	                                     g_timelimit.integer, g_fraglimit.integer, g_capturelimit.integer, roundlimit.integer,
	                                     item_disable.integer, powerup_disable.integer, hook_enable.integer, match_mutespecs.integer,
	                                     g_dmflags.integer, g_friendlyFire.integer, team_armordamage.integer, team_allcaptain.integer,
	                                     g_forcerespawn.integer, server_thrufloors.integer, match_dropitems.integer, server_promode.integer,
	                                     server_customclient.integer, server_maxpacketsmin.integer, server_maxpacketsmax.integer,
	                                     server_timenudgemin.integer, server_timenudgemax.integer, pure, mg_damage.integer,
	                                     mg_damageTeamplay.integer, match_overtime.integer, pmove_fixed.integer, railgun_damage.integer,
	                                     start_slugs.integer, weapon_deaddrop.integer, weapon_have.integer, weapon_start.integer,
	                                     server_cq3.integer));
}

/*
==================
OSP_RestoreDefaultSettings

default_change set: restores the settings that
OSP_SaveDefaultSettings stored in default_settings.
==================
*/
void OSP_RestoreDefaultSettings(void) {
	int     v[100];
	char    buf[MAX_STRING_CHARS];

	if (!default_change.integer) {
		return;
	}
	trap_Cvar_Set("default_change", "0");
	trap_Cvar_Update(&default_change);

	if (!default_settings.string[0] || !Q_stricmp(default_settings.string, "none")) {
		return;
	}

	trap_Cvar_VariableStringBuffer("default_settings", buf, sizeof(buf));
	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
	       &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7],
	       &v[8], &v[9], &v[10], &v[11], &v[12], &v[13], &v[14], &v[15],
	       &v[16], &v[17], &v[18], &v[19], &v[20], &v[21], &v[22], &v[23],
	       &v[24], &v[25], &v[26], &v[27], &v[28], &v[29], &v[30], &v[31]);
	trap_Cvar_Set("timelimit", va("%d", v[0]));
	trap_Cvar_Set("fraglimit", va("%d", v[1]));
	trap_Cvar_Set("capturelimit", va("%d", v[2]));
	trap_Cvar_Set("roundlimit", va("%d", v[3]));
	trap_Cvar_Set("item_disable", va("%d", v[4]));
	trap_Cvar_Set("powerup_disable", va("%d", v[5]));
	trap_Cvar_Set("hook_enable", va("%d", v[6]));
	trap_Cvar_Set("match_mutespecs", va("%d", v[7]));
	trap_Cvar_Set("dmflags", va("%d", v[8]));
	trap_Cvar_Set("g_friendlyFire", va("%d", v[9]));
	trap_Cvar_Set("team_armordamage", va("%d", v[10]));
	trap_Cvar_Set("team_allcaptain", va("%d", v[11]));
	trap_Cvar_Set("g_forcerespawn", va("%d", v[12]));
	trap_Cvar_Set("server_thrufloors", va("%d", v[13]));
	trap_Cvar_Set("match_dropitems", va("%d", v[14]));
	trap_Cvar_Set("server_promode", va("%d", v[15]));
	trap_Cvar_Set("server_customclient", va("%d", v[16]));
	trap_Cvar_Set("server_maxpacketsmin", va("%d", v[17]));
	trap_Cvar_Set("server_maxpacketsmax", va("%d", v[18]));
	trap_Cvar_Set("server_timenudgemin", va("%d", v[19]));
	trap_Cvar_Set("server_timenudgemax", va("%d", v[20]));
	trap_Cvar_Set("sv_pure", va("%d", v[21]));
	trap_Cvar_Set("mg_damage", va("%d", v[22]));
	trap_Cvar_Set("mg_damageTeamplay", va("%d", v[23]));
	trap_Cvar_Set("match_overtime", va("%d", v[24]));
	trap_Cvar_Set("pmove_fixed", va("%d", v[25]));
	trap_Cvar_Set("railgun_damage", va("%d", v[26]));
	trap_Cvar_Set("start_slugs", va("%d", v[27]));
	trap_Cvar_Set("weapon_deaddrop", va("%d", v[28]));
	trap_Cvar_Set("weapon_have", va("%d", v[29]));
	trap_Cvar_Set("weapon_start", va("%d", v[30]));
	trap_Cvar_Set("server_cq3", va("%d", v[31]));

	G_UpdateCvars();
}

/*
==================
OSP_UnpauseComplete
==================
*/
void OSP_UnpauseComplete(void) {
	level.warmupTime = 0;
	trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
	trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime + osp_pausedTime));
	OSP_GlobalSound("sound/feedback/fight.wav");
	trap_SendServerCommand(-1, "cp \"^1Match has RESUMED!!^7\n\"");
	osp_matchPaused = 0;
	osp_unpausing = 0;
	G_LogPrintf("Unpause: Complete\n");
}

/*
==================
OSP_FollowPowerupPickup

ent picked up a powerup: spectators with followpowerup switch to him (unless they follow a
flag carrier, or may not follow his team).
==================
*/
void OSP_FollowPowerupPickup(gentity_t* ent) {
	gentity_t*   other;
	int         i;

	for (i = 0; i < level.numConnectedClients; i++) {
		other = &g_entities[ level.sortedClients[i] ];
		if (other->client->pers.connected != CON_CONNECTED || other->client->sess.sessionTeam != TEAM_SPECTATOR ||
		        (other->client->sess.spectatorState != SPECTATOR_FOLLOW && other->client->sess.spectatorState != SPECTATOR_VIEWCAM_FOLLOW)) {
			continue;
		}
		if (other->client->pers.followPowerup &&
		        !g_entities[ other->client->sess.spectatorClient ].client->ps.powerups[PW_REDFLAG] &&
		        !g_entities[ other->client->sess.spectatorClient ].client->ps.powerups[PW_BLUEFLAG]) {
			int     team;
			int     invite;

			team = ent->client->sess.sessionTeam;
			invite = other->client->sess.specInvite;
			if (g_gametype.integer >= GT_TEAM && !other->client->sess.referee &&
			        ((other->client->pers.followTeam && other->client->pers.followTeam != team) ||
			         (osp_teamInfo[team].specLocked && (!invite || (invite != team && invite != TEAM_SPECTATOR))))) {
				return;
			}
			other->client->sess.spectatorClient = ent - g_entities;
			if (other->client->sess.spectatorState == SPECTATOR_VIEWCAM_FOLLOW) {
				other->client->ps.eFlags ^= EF_TELEPORT_BIT;
				OSP_ViewcamThink(other, 400);
			}
		}
	}
}

/*
==================
OSP_Cmd_AutoRecord

Makes the client record a demo (record) or take a screenshot. fClient: the client asked for it
("autorecord" / "autoscreenshot", cg_autoAction); otherwise the server forces it (server_record:
16 "<host>-<player>-[<dcount>]" names, 4 "-<host>" suffix, 8 date suffix) and logs it.
==================
*/
void OSP_Cmd_AutoRecord(int clientNum, qboolean record, qboolean fClient) {
	char    buf[2048];
	char    name[2048];
	char    suffix[2048];
	char*    s;

	if ((server_record.integer & 16) && !fClient) {
		char    dcount[MAX_STRING_CHARS];
		char    host[MAX_STRING_CHARS];
		char    player[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer("sv_hostname", buf, sizeof(buf));
		OSP_SanitizeFilename(buf, strlen(buf), host);
		trap_Cvar_VariableStringBuffer("sv_dcount", dcount, sizeof(dcount));
		OSP_SanitizeFilename(level.clients[clientNum].pers.netname, strlen(level.clients[clientNum].pers.netname), player);
		Q_strncpyz(name, va("%s-%s-[%s]", host, player, dcount), sizeof(name));
	} else {
		if ((server_record.integer & 4) && !fClient) {
			char    host[MAX_STRING_CHARS];

			trap_Cvar_VariableStringBuffer("sv_hostname", buf, sizeof(buf));
			OSP_SanitizeFilename(buf, strlen(buf), host);
			strcpy(suffix, va("-%s", host));
		} else {
			suffix[0] = 0;
		}

		if ((server_record.integer & 8) || fClient) {
			qtime_t ct;

			trap_RealTime(&ct);
			strcat(suffix, va("-%d.%02d.%02d-%02d.%02d.%02d", ct.tm_year + 1900, ct.tm_mon + 1, ct.tm_mday, ct.tm_hour, ct.tm_min, ct.tm_sec));
		}

		if (level.clients[clientNum].pers.connected != CON_CONNECTED) {
			return;
		}

		s = OSP_DemoName(clientNum);
		if (g_gametype.integer == GT_TOURNAMENT || (g_gametype.integer >= GT_TEAM && osp_numPlayerNames)) {
			OSP_SanitizeFilename(level.clients[clientNum].pers.netname, strlen(level.clients[clientNum].pers.netname), buf);
			Q_strncpyz(name, va("%s-(%s)%s", s, buf, suffix), sizeof(name));
		} else {
			Q_strncpyz(name, va("%s%s", s, suffix), sizeof(name));
		}
	}

	if (record) {
		trap_SendServerCommand(clientNum, va("stuff set g_synchronousclients 1;record ' %s' ;set g_synchronousclients 0;\n", name));
		if (!fClient) {
			trap_Cvar_Set(va("demosession%d", clientNum), name);
		}
	} else {
		if (!level.clients[clientNum].sess.ospClient) {
			trap_SendServerCommand(clientNum, va("print \"\n\n%s\n\n\"", "OSP2 Tourney DM/CA/CTF v(%s)", XSTRINGIFY(OSP_VERSION)));
		}
		trap_SendServerCommand(clientNum, va("stuff %s ' %s';\n",
		                                     (!fClient || !level.clients[clientNum].pers.screenshotJPEG) ? "screenshot" : "screenshotJPEG", name));
		if (!fClient) {
			G_LogPrintf("PlayerScreenshot:\t%d\t%s\n", clientNum, name);
		}
	}
}

/*
==================
OSP_AutoRecordStart

Countdown started: demo recording for everybody (server_record 1 / 16) or the clients with
cg_autoAction 4
==================
*/
void OSP_AutoRecordStart(void) {
	int     i;

	G_Printf("All players ready.  Countdown started!\n");
	for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected != CON_CONNECTED) {
			continue;
		}
		if (strstr(level.clients[i].pers.netname, "QTV") == level.clients[i].pers.netname) {
			continue;
		}
		if (strstr(level.clients[i].pers.netname, "GTV") == level.clients[i].pers.netname) {
			continue;
		}
		if ((server_record.integer & 1) || (server_record.integer & 16)) {
			OSP_Cmd_AutoRecord(i, 1, 0);
			OSP_BatcherInfo(&g_entities[i]);
		} else if (level.clients[i].pers.autoAction & 4) {
			OSP_Cmd_AutoRecord(i, 1, 1);
		}
	}
}

/*
==================
OSP_AutoRecordStop

Match end: stops the demos started by OSP_AutoRecordStart and logs the forced ones
("demosession<n>" cvars)
==================
*/
void OSP_AutoRecordStop(void) {
	int     i;
	char    buf[2048];

	if (level.warmupTime) {
		return;
	}

	for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected != CON_CONNECTED) {
			continue;
		}
		if (strstr(level.clients[i].pers.netname, "QTV") == level.clients[i].pers.netname) {
			continue;
		}
		if (strstr(level.clients[i].pers.netname, "GTV") == level.clients[i].pers.netname) {
			continue;
		}
		if ((server_record.integer & 1) || (server_record.integer & 16)) {
			trap_SendServerCommand(i, "stuff stoprecord\n");
			OSP_Batcher(&g_entities[i]);
			buf[0] = 0;
			trap_Cvar_VariableStringBuffer(va("demosession%d", i), buf, sizeof(buf));
			if (buf[0]) {
				G_LogPrintf("PlayerDemo:\t%d\t%s\n", i, buf);
				trap_Cvar_Set(va("demosession%d", i), "");
			}
		} else if ((level.clients[i].pers.autoAction & 4) && (!OSP_IsRoundBased(g_gametype.integer) || OSP_WinningTeam())) {
			trap_SendServerCommand(i, "stuff stoprecord\n");
		}
	}
}

/*
==================
OSP_ClearClients

"stuff clear" for the clients that dump their stats at the end of a match (cg_autoAction 1)
==================
*/
void OSP_ClearClients(void) {
	int     i;

	if (level.warmupTime || osp_mapChangeReason) {
		return;
	}

	for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected != CON_CONNECTED) {
			continue;
		}
		if ((level.clients[i].pers.autoAction & 1) && (!OSP_IsRoundBased(g_gametype.integer) || OSP_WinningTeam())) {
			trap_SendServerCommand(i, "stuff clear;\n");
		}
	}
}

/*
==================
OSP_AutoScreenshot

End-of-match screenshots: forced by server_record (2 / 16), else cg_autoAction 2
==================
*/
void OSP_AutoScreenshot(void) {
	int     i;

	if (level.warmupTime || osp_mapChangeReason) {
		return;
	}

	for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected != CON_CONNECTED) {
			continue;
		}
		if (strstr(level.clients[i].pers.netname, "QTV") == level.clients[i].pers.netname) {
			continue;
		}
		if (strstr(level.clients[i].pers.netname, "GTV") == level.clients[i].pers.netname) {
			continue;
		}
		if ((server_record.integer & 2) || (server_record.integer & 16)) {
			OSP_Cmd_AutoRecord(i, 0, 0);
		} else if ((level.clients[i].pers.autoAction & 2) && (!OSP_IsRoundBased(g_gametype.integer) || OSP_WinningTeam())) {
			OSP_Cmd_AutoRecord(i, 0, 1);
		}
	}
}


/*
==================
OSP_DemoName

Demo file name of an autorecord: "<player>-vs-<player>" (1v1), "<red>-vs-<blue>" (team names
from the player name file), "<player>-TDM/-CTF/-CA/-FFA", then "-<mapname>"
==================
*/
char* OSP_DemoName(int clientNum) {
	static char demoName[MAX_OSPATH];
	int         i;
	int         j[2] = { -1, -1 };
	char        info[MAX_INFO_STRING];
	char        name1[MAX_STRING_CHARS];
	char        name2[MAX_STRING_CHARS];

	if (g_gametype.integer == GT_TOURNAMENT) {
		for (i = 0; i < level.maxclients; i++) {
			if (level.clients[i].pers.connected != CON_CONNECTED) {
				continue;
			}
			if (level.clients[i].sess.sessionTeam != TEAM_SPECTATOR) {
				if (j[0] < 0) {
					j[0] = i;
				} else {
					j[1] = i;
					break;
				}
			}
		}

		if (j[0] < 0 || j[1] < 0) {
			strcpy(demoName, va("1v1-unknown-%d", level.time));
			G_LogPrintf("DEMORECORD: j[0]=%d, j[1]=%d\n", j[0], j[1]);
		} else {
			OSP_SanitizeFilename(level.clients[j[0]].pers.netname, strlen(level.clients[j[0]].pers.netname), name1);
			OSP_SanitizeFilename(level.clients[j[1]].pers.netname, strlen(level.clients[j[1]].pers.netname), name2);
			strcpy(demoName, va("%s-vs-%s", name1, name2));
		}
	} else {
		OSP_SanitizeFilename(level.clients[clientNum].pers.netname, strlen(level.clients[clientNum].pers.netname), name1);
		if (g_gametype.integer >= GT_TEAM && osp_numPlayerNames) {
			strcpy(demoName, va("%s-vs-%s", osp_teamInfo[TEAM_RED].teamName, osp_teamInfo[TEAM_BLUE].teamName));
		} else if (g_gametype.integer == GT_TEAM) {
			strcpy(demoName, va("%s-TDM", name1));
		} else if (g_gametype.integer == GT_CTF) {
			strcpy(demoName, va("%s-CTF", name1));
		} else if (g_gametype.integer == 5) {
			strcpy(demoName, va("%s-CA", name1));
		} else {
			strcpy(demoName, va("%s-FFA", name1));
		}
	}

	trap_GetServerinfo(info, sizeof(info));
	strcat(demoName, va("-%s", Info_ValueForKey(info, "mapname")));
	return demoName;
}

/*
==================
OSP_BatcherInfo

server_batcher: tells the client which demo batcher to announce (autorecord start)
==================
*/
void OSP_BatcherInfo(gentity_t* ent) {
	if (!server_batcher.string[0] || !Q_stricmp(server_batcher.string, "none") || osp_mapChangeReason) {
		return;
	}
	trap_SendServerCommand(ent - g_entities, va("batcherinfo -g demos -a %s", server_batcher.string));
}

/*
==================
OSP_Batcher

server_batcher: runs the client's demo batcher (autorecord stop)
==================
*/
void OSP_Batcher(gentity_t* ent) {
	if (!server_batcher.string[0] || !Q_stricmp(server_batcher.string, "none") || osp_mapChangeReason) {
		return;
	}
	trap_SendServerCommand(ent - g_entities, va("batcher -g demos -a %s", server_batcher.string));
}

/*
==================
OSP_SendAuthRequest

Sends "psohtua <challenge>" ("authosp" reversed); the client has 5 seconds to answer with
"htuatneilc" (OSP_Cmd_ClientAuth_f)
==================
*/
void OSP_SendAuthRequest(gentity_t* ent) {
	if (!server_ospauth.integer) {
		ent->client->pers.authTime = -1;
	}
	if (ent->client->pers.authTime != 0) {
		return;
	}

	ent->client->pers.authTime = level.time + 5000;
	ent->client->pers.authChallenge = level.time;
	trap_SendServerCommand(ent - g_entities, va("psohtua %d", ent->client->pers.authChallenge));
	G_Printf("Sent authentication request to '%s^7'\n", ent->client->pers.netname);
}

/*
==================
OSP_CheckAuthTimeout

Asks again after a missed reply (twice), then drops the client
==================
*/
void OSP_CheckAuthTimeout(gentity_t* ent) {
	if (!server_ospauth.integer) {
		ent->client->pers.authTime = -1;
		return;
	}
	if (ent->client->pers.authTime <= 0 || ent->client->pers.authTime > level.time) {
		return;
	}

	if (ent->client->pers.authRetries++ < 2) {
		ent->client->pers.authTime = 0;
		OSP_SendAuthRequest(ent);
		return;
	}

	trap_SendServerCommand(-1, va("print \"*** %s ^3authorization time-out\n", ent->client->pers.netname));
	trap_DropClient(ent->client - level.clients, ": ^3No authentication reply");
}

/*
==================
OSP_Cmd_ClientAuth_f

"htuatneilc <secret>": the client's answer to OSP_SendAuthRequest
==================
*/
void OSP_Cmd_ClientAuth_f(gentity_t* ent) {
	char    secret[64];
	char    reply[MAX_TOKEN_CHARS];

	if (!server_ospauth.integer || ent->client->pers.authTime < 0) {
		G_Printf(">> Rcvd non-auth from %s (so:%d, at:%d)\n", ent->client->pers.netname, server_ospauth.integer, ent->client->pers.authTime);
		return;
	}

	trap_Argv(1, reply, sizeof(reply));
	memset(secret, 0, sizeof(secret));
	BG_OSPAuthInit();
	BG_OSPAuthUpdateName(ent->client->pers.netname);
	BG_OSPAuthUpdateName(va("%d", ent->client->pers.authChallenge));
	BG_OSPAuthGetSecret(secret);

	if (Q_stricmp(secret, reply)) {
		G_Printf(">>> Client authentication error\n");
		ent->client->pers.authRetries++;
		if (ent->client->pers.authRetries > 3) {
			trap_SendServerCommand(-1, va("print \"*** %s ^3is unauthorized!\n", ent->client->pers.netname));
			trap_DropClient(ent->client - level.clients, ": ^3Illegal authentication reply");
			return;
		}
		G_Printf(">>> Trying again (%d)\n", ent->client->pers.authRetries);
		OSP_SendAuthRequest(ent);
		return;
	}

	G_Printf("%s has passed authorization.\n", ent->client->pers.netname);
	ent->client->pers.authTime = -1;
}

void OSP_EmptyStub(void) {}

/*
==================
OSP_SetPhysicsMode

level.physicsMode from server_promode / server_cq3, the CPM settings (PROMODE bits 16 / 32:
server_fastrail, server_lgcooldown), powerup respawn times and g_weapon(team)respawn
==================
*/
void OSP_SetPhysicsMode(void) {
	int     respawn;
	int     mode;
	char*    s;

	respawn = match_poweruprespawn.integer;
	level.physicsMode = server_promode.integer ? 1 : (server_cq3.integer ? 2 : 0);
	mode = !server_promode.integer ? (server_cq3.integer ? 2 : 0) :
	       ((server_fastrail.integer ? 16 : 0) | 1 | (server_lgcooldown.integer ? 32 : 0));
	CPM_UpdateSettings(mode);
	trap_SetConfigstring(CS_OSP_PHYSICS, va("%d", mode));

	respawn = (respawn <= 60) ? 60 : ((respawn > 60 && respawn < 120) ? 90 : 120);
	// register-free frame: the unbraced if / else and the ?: statement match OSP's frame layout
	if (!(level.physicsMode & 1))
		modeRespawnPowerup = modeRespawnPowerupCTF = 120;
	else
		(g_gametype.integer != GT_CTF) ? (modeRespawnPowerup = respawn) : (modeRespawnPowerupCTF = respawn);

	G_Printf("Game session settings: %s\n", (level.physicsMode & 1) ? "PROMODE" : ((level.physicsMode & 2) ? "CQ3" : "VQ3"));

	if (level.physicsMode) {
		level.cpmUsed = 1;
	}
	if (level.cpmUsed) {
		if (level.physicsMode) {
			switch (g_gametype.integer) {
				case GT_TOURNAMENT:
					s = "15";
					break;
				case GT_TEAM:
					s = "30";
					break;
				case GT_CTF:
					s = "5";
					break;
				default:
					s = "5";
					break;
			}
		} else {
			s = (g_gametype.integer == GT_TEAM) ? "30" : "5";
		}
		trap_Cvar_Set((g_gametype.integer == GT_TEAM) ? "g_weaponteamrespawn" : "g_weaponrespawn", s);
	}
}

/*
============
OSP_RadiusDamage

G_RadiusDamage for G_MissileImpact: a client is only hit when the trace from point (10 units
before the impact) to its origin is free
============
*/
qboolean OSP_RadiusDamage(vec3_t origin, gentity_t* inflictor, float damage, float radius, gentity_t* ignore, int mod, vec3_t point) {
	float       points, dist;
	gentity_t*   ent;
	int         entityList[MAX_GENTITIES];
	int         numListedEntities;
	vec3_t      mins, maxs;
	vec3_t      v;
	vec3_t      dir;
	int         i, e;
	qboolean    hitClient = qfalse;
	gentity_t*   attacker = inflictor->parent;
	trace_t     tr;

	if (radius < 1) {
		radius = 1;
	}

	for (i = 0 ; i < 3 ; i++) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

	for (e = 0 ; e < numListedEntities ; e++) {
		ent = &g_entities[entityList[ e ]];

		if (g_gametype.integer == 5 && ent != attacker &&
		        (level.warmupTime || (inflictor->timestamp && inflictor->timestamp < level.startTime)))
			continue;
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		// find the distance from the edge of the bounding box
		for (i = 0 ; i < 3 ; i++) {
			if (origin[i] < ent->r.absmin[i]) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if (origin[i] > ent->r.absmax[i]) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength(v);
		if (dist >= radius) {
			continue;
		}

		trap_Trace(&tr, point, NULL, NULL, ent->r.currentOrigin, ENTITYNUM_NONE, MASK_SOLID);
		if (tr.fraction != 1.0 && ent->client) {
			continue;
		}

		points = damage * (1.0 - dist / radius);

		if (CanDamage (ent, origin)) {
			if (LogAccuracyHit(ent, attacker)) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			if (ent == attacker) {
				dir[2] += 24;
			} else {
				dir[2] += modeUnk_2744;
			}
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
	}

	return hitClient;
}

/*
==================
OSP_GlobalSound
==================
*/
void OSP_GlobalSound(char* sound) {
	gentity_t*   te;

	te = G_TempEntity(level.intermission_origin, EV_GLOBAL_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
	te->r.svFlags |= SVF_BROADCAST;
}

/*
==================
OSP_UpdateScoreCvars

Score_Time (and with level.scoresChanged or a 1.27l+ engine the team scores) in the serverinfo
==================
*/
void OSP_UpdateScoreCvars(char* scoreTime) {
	if (g_gametype.integer == GT_TOURNAMENT || g_gametype.integer >= GT_TEAM) {
		OSP_HidePlayersInServerinfo(1);
		trap_Cvar_Set("Score_Time", scoreTime);
		if (level.osp_unk_1e00 || level.engineCheck == 2) {
			level.osp_unk_1e00 = 0;
			if (g_gametype.integer >= GT_TEAM) {
				trap_Cvar_Set("Score_Red", va("%d", level.teamScores[TEAM_RED]));
				trap_Cvar_Set("Score_Blue", va("%d", level.teamScores[TEAM_BLUE]));
			}
		}
	}
}

/*
==================
OSP_FollowNext

The next client ent may follow in direction dir (Cmd_FollowCycle_f); -1 when the cycle came
back to a disconnected start. Freeze tag players follow only their team.
==================
*/
int OSP_FollowNext(gentity_t* ent, int dir, int team) {
	int     clientnum;
	int     original;

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	do {
		clientnum += dir;
		if (clientnum >= level.maxclients) {
			clientnum = 0;
		}
		if (clientnum < 0) {
			clientnum = level.maxclients - 1;
		}

		if (clientnum == original && level.clients[ clientnum ].pers.connected != CON_CONNECTED) {
			return -1;
		}

		// can only follow connected clients
		if (level.clients[ clientnum ].pers.connected != CON_CONNECTED) {
			continue;
		}

		// freeze tag: players follow their own team
		if (g_gametype.integer == GT_TEAM && server_freezetag.integer && ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
		        level.clients[ clientnum ].sess.sessionTeam != ent->client->sess.sessionTeam) {
			continue;
		}

		// can't follow another spectator
		if (is_spectator(&level.clients[ clientnum ])) {
			continue;
		}

		if (OSP_FollowHelper(ent, clientnum, team)) {
			continue;
		}
		break;
	} while (clientnum != original);

	return clientnum;
}

/*
==================
OSP_FollowHelper

Nonzero when ent may not follow clientNum: a team that is spec-locked (without an invite), a
followTeam lock (team) on the other team, or a coach of the other team; referees may follow
anybody
==================
*/
int OSP_FollowHelper(gentity_t* ent, int clientNum, int team) {
	return g_gametype.integer >= GT_TEAM && !ent->client->sess.referee &&
	       ((level.clients[clientNum].sess.sessionTeam == TEAM_RED && ent->client->sess.sessionTeam != TEAM_RED &&
	         ent->client->sess.coachTeam != TEAM_RED &&
	         ((osp_teamInfo[TEAM_RED].specLocked && (ent->client->sess.specInvite == 0 || ent->client->sess.specInvite == 2)) ||
	          (team && team != TEAM_RED) || ent->client->sess.coachTeam == TEAM_BLUE)) ||
	        (level.clients[clientNum].sess.sessionTeam == TEAM_BLUE && ent->client->sess.sessionTeam != TEAM_BLUE &&
	         ent->client->sess.coachTeam != TEAM_BLUE &&
	         ((osp_teamInfo[TEAM_BLUE].specLocked && ent->client->sess.specInvite < 2) ||
	          (team && team != TEAM_BLUE) || ent->client->sess.coachTeam == TEAM_RED)));
}

/*
==================
OSP_MOTDHelper

Copies src to dst until one of the limits is reached; color codes ("^x") don't count as
visible characters. Returns the number of visible characters copied.
==================
*/
int OSP_MOTDHelper(char* src, int srcSize, char* dst, int dstSize, int maxVisible) {
	int     len;
	int     visible;

	len = 0;
	visible = 0;
	while (*src && len < srcSize && len < dstSize && visible < maxVisible) {
		if (*src == '^') {
			if (len + 2 >= srcSize && len + 2 >= dstSize) {
				break;
			}
			*dst++ = *src++;
			*dst++ = *src++;
			len += 2;
		} else {
			*dst++ = *src++;
			len++;
			visible++;
		}
	}
	*dst = 0;
	return visible;
}

/*
==================
OSP_CheckEngineVersion

Nonzero when the server's engine ("version" serverinfo, second word) is 1.27l or newer;
copies that word to version ("unknown" when there is none).
==================
*/
int OSP_CheckEngineVersion(char* version) {
	char    info[MAX_INFO_STRING];
	char*    s, *v;

	if (version) {
		strcpy(version, "unknown");
	}
	trap_GetConfigstring(CS_SERVERINFO, info, sizeof(info));
	if (info[0]) {
		s = Info_ValueForKey(info, "version");
		if (!s || !*s) {
			return 0;
		}
		v = strchr(s, ' ') + 1;
		s = strchr(v, ' ');
		*s = 0;
		if (version) {
			strcpy(version, v);
		}
		if (Q_stricmp(v, "1.27l") >= 0) {
			return 1;
		}
	}
	return 0;
}

/*
==================
OSP_BroadcastCheckThink

think of the "delay_broadcastcheck" entity: checks the engine once, then applies
OSP_HidePlayersInServerinfo( s.modelindex )
==================
*/
void OSP_BroadcastCheckThink(gentity_t* ent) {
	char    version[32];

	if (OSP_CheckEngineVersion(version)) {
		level.engineCheck = 2;
		level.timeLeftInterval = 800;
	} else {
		level.engineCheck = 1;
	}
	OSP_HidePlayersInServerinfo(ent->s.modelindex);
	ent->nextthink = 0;
	ent->think = NULL;
	G_FreeEntity(ent);
}

/*
==================
OSP_DelayBroadcastCheck
==================
*/
void OSP_DelayBroadcastCheck(int time, int hide) {
	gentity_t*   te;

	te = G_Spawn();
	te->s.eType = ET_ITEM;
	te->r.svFlags |= SVF_NOCLIENT;
	te->s.eFlags |= EF_NODRAW;
	te->s.modelindex = hide;
	te->classname = "delay_broadcastcheck";
	te->nextthink = time;
	te->think = OSP_BroadcastCheckThink;
	trap_LinkEntity(te);
}

/*
==================
OSP_HidePlayersInServerinfo

With a 1.27l+ engine the players are hidden from the serverinfo (SVF_NOSERVERINFO) while a
match runs (hide nonzero, no warmup, no intermission). The engine check runs once, a second
after the first call in the first 2.5 s of a map.
==================
*/
void OSP_HidePlayersInServerinfo(int hide) {
	static int  checkScheduled = 0;
	int         i;

	if (!level.engineCheck && level.time - level.startTime < 2500) {
		if (!checkScheduled) {
			OSP_DelayBroadcastCheck(level.time + 1000, hide);
			checkScheduled = 1;
		}
		return;
	}
	if (level.engineCheck == 1) {
		return;
	}

	for (i = 0; i < level.maxclients; i++) {
		if (hide && level.clients[i].pers.connected && !level.warmupTime && !level.intermissionQueued && !level.intermissiontime) {
			g_entities[i].r.svFlags |= SVF_NOSERVERINFO;
		} else {
			g_entities[i].r.svFlags &= ~SVF_NOSERVERINFO;
		}
	}
}

/*
==================
OSP_StatsDump

cg_autoAction 1: the client dumps its console to stats/<date>/<time>.txt
==================
*/
void OSP_StatsDump(gentity_t* ent) {
	qtime_t ct;

	trap_RealTime(&ct);
	trap_SendServerCommand(ent - g_entities, va("stuff currenttime; condump stats/%d.%02d.%02d/%02d%02d%02d.txt",
	                       ct.tm_year + 1900, ct.tm_mon + 1, ct.tm_mday, ct.tm_hour, ct.tm_min, ct.tm_sec));
}

/*
==================
OSP_SetInstagib

match_instagib: removes the items (and stops their respawn thinks) or, when it was switched
off during the map (changed: G_UpdateCvars), restores the mode settings and the items; a change
also re-arms the players
==================
*/
void OSP_SetInstagib(int changed) {
	gentity_t*   ent;
	int         i;
	char*        s;

	s = (match_instagib.integer > 0) ? "EN" : "DIS";
	if (osp_matchTime - level.startTime > 10000) {
		trap_SendServerCommand(-1, va("cp \"^1Instagib %sABLED!\n\"", s));
		trap_SendServerCommand(-1, va("print \"^1Instagib %sABLED!\n\"", s));
	}
	G_Printf("Instagib %sABLED!\n", s);

	if (match_instagib.integer > 0) {
		for (i = level.maxclients, ent = &g_entities[level.maxclients]; i < level.num_entities; i++, ent++) {
			if (ent->inuse && ent->item && !OSP_ItemAllowed(ent)) {
				if (!(ent->s.eFlags & EF_NODRAW) && ent->r.contents) {
					ent->r.svFlags |= SVF_NOCLIENT;
					ent->s.eFlags |= EF_NODRAW;
					ent->r.contents = 0;
					trap_LinkEntity(ent);
					if (ent->team) {
						RespawnItem(ent);
					}
				} else if (ent->nextthink && ent->think && !ent->team) {
					ent->nextthink = 0;
					ent->think = 0;
					trap_LinkEntity(ent);
				}
			}
		}
	} else if (changed) {
		OSP_RestoreDefaultSettings();
		for (i = level.maxclients, ent = &g_entities[level.maxclients]; i < level.num_entities; i++, ent++) {
			if (!ent->inuse || !ent->item || !OSP_ItemAllowed(ent)) {
				continue;
			}
			ent->r.contents = CONTENTS_TRIGGER;
			ent->s.eFlags &= ~EF_NODRAW;
			ent->r.svFlags &= ~SVF_NOCLIENT;
			ent->s.eFlags &= ~EF_BACKPACK;
			FinishSpawningItem(ent);
		}
	}

	if (changed) {
		int     j;

		for (i = 0, ent = g_entities; i < level.maxclients; i++, ent++) {
			if (!ent->inuse || ent->client->pers.connected != CON_CONNECTED ||
			        ent->client->sess.sessionTeam == TEAM_SPECTATOR || ent->client->ps.pm_type == PM_DEAD) {
				continue;
			}
			OSP_SetSpawnWeapons(ent);
			OSP_SelectSpawnWeapon(ent);
			for (j = 0; j < MAX_POWERUPS; j++) {
				ent->client->ps.powerups[j] = 0;
			}
		}
	}
}

/*
==================
OSP_ClientWelcome

think of the "delay_printf" entity (OSP_DelayPrintf): message 0 is the welcome centerprint plus
the late filter / ban check and the OSP client check (auth request for cgame 1008, drop for a
missing or old cgame); message 1 repeats the last match result
==================
*/
void OSP_ClientWelcome(gentity_t* ent) {
	gentity_t*   player;

	player = ent->parent;
	switch (ent->s.modelindex) {
		case 0:
		{
			register char*   mode;  // register: matches OSP's frame layout
			int     res;
			char*    gametype;

			mode = (level.physicsMode & 1) ? "\n^5Challenge ProMode Enabled\n^3http://www.promode.org" :
			       ((level.physicsMode & 2) ? "\n^5CQ3 Enabled\n^3http://www.promode.org" : " ");
			if (g_gametype.integer == GT_CTF) {
				gametype = "CTF";
			} else if (g_gametype.integer == 5) {
				gametype = "CA";
			} else {
				gametype = "DM";
			}
			trap_SendServerCommand(player - g_entities, va("cp \"Welcome to ^3OSP2 Tourney %s v(%s)\n^3https://github.com/snems/OSP2\n%s\"", gametype, XSTRINGIFY(OSP_VERSION), mode));

			res = OSP_CheckClientFilters(player->client->pers.netname, player - g_entities, 0);
			if (res != 1 && res != 2 && res != 6) {
				trap_DropClient(player - g_entities, ": ^3Failed filter check");
				G_Printf("** Late ban check failed: %d\n", res);
				OSP_AdminLogPrintf(va("Late ConnectReject:\tName=%s\t%d", player->client->pers.netname, res));
				return;
			}

			if (server_ospauth.integer && !(player->r.svFlags & SVF_BOT) &&
			        OSP_CheckClientFilters(player->client->pers.netname, player - g_entities, 1) != 2) {
				if (player->client->sess.ospClient != 1008) {
					if (!player->client->sess.ospClient) {
						trap_DropClient(player - g_entities, ": ^3Missing OSP client cgame");
						G_Printf("** Invalid client cgame: %s (%s) Check=%d\n", player->client->pers.netname, player->client->sess.ip,
						         OSP_CheckClientFilters(player->client->pers.netname, player - g_entities, 1));
						return;
					}
					trap_DropClient(player - g_entities, ": ^3Invalid OSP client cgame");
					G_Printf("** Old client cgame(%d): %s (%s)\n", player->client->sess.ospClient, player->client->pers.netname, player->client->sess.ip);
					return;
				}
				player->client->pers.authTime = 0;
				OSP_SendAuthRequest(player);
			} else {
				player->client->pers.authTime = -1;
			}
		}
		break;
		case 1:
			trap_SendServerCommand(player - g_entities, osp_matchResultPrint);
			trap_SendServerCommand(player - g_entities, osp_matchResultCP);
			break;
	}

	ent->nextthink = 0;
	ent->think = NULL;
	G_FreeEntity(ent);
}

/*
==================
OSP_DelayPrintf

Spawns an invisible "delay_printf" entity whose think (OSP_ClientWelcome) prints message
type to ent at time.
==================
*/
void OSP_DelayPrintf(int type, int time, gentity_t* ent) {
	gentity_t*   te;

	te = G_Spawn();
	te->s.eType = ET_ITEM;
	te->r.svFlags |= SVF_NOCLIENT;
	te->s.eFlags |= EF_NODRAW;
	te->s.modelindex = type;
	te->parent = ent;
	te->classname = "delay_printf";
	te->nextthink = time;
	te->think = OSP_ClientWelcome;
	trap_LinkEntity(te);
}

/*
==================
OSP_SortClientsCompare

qsort callback of OSP_SortClients: client numbers by sanitized name
==================
*/
int QDECL OSP_SortClientsCompare(const void* a, const void* b) {
	char    name1[MAX_NETNAME];
	char    name2[MAX_NETNAME];

	SanitizeString(level.clients[*(int*)a].pers.netname, name1, qfalse);
	SanitizeString(level.clients[*(int*)b].pers.netname, name2, qfalse);
	return Q_strncmp(name1, name2, sizeof(name1));
}

/*
==================
OSP_SortClients

level.sortedByName: the connected clients sorted by name
==================
*/
void OSP_SortClients(void) {
	int     i;

	for (i = 0; i < level.numConnectedClients; i++) {
		level.sortedByName[i] = level.sortedClients[i];
	}
	qsort(level.sortedByName, level.numConnectedClients, sizeof(int), OSP_SortClientsCompare);
}

/*
==================
OSP_ClientScore

The scoreboard's extra score column: TDM net frags, freeze tag thaws (low 16 bits of losses)
==================
*/
int OSP_ClientScore(gentity_t* ent) {
	if (g_gametype.integer != GT_TEAM) {
		return 0;
	}
	if (!IS_FREEZETAG) {
		return ent->client->sess.kills - ent->client->sess.deaths - ent->client->sess.suicides - ent->client->sess.teamKills;
	}
	return ent->client->sess.losses & 0xffff;
}

/*
==================
OSP_ChatFilter

0 when the normalized text contains a filtered word
==================
*/
int OSP_ChatFilter(const char* text) {
	char    buf[MAX_STRING_CHARS];

	OSP_NormalizeName((char*)text, sizeof(buf), buf, 1);
	if (strstr(buf, "nigger") || strstr(buf, "n1gger")) {
		return 0;
	}
	return 1;
}
