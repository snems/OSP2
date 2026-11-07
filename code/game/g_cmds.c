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
#include "g_local.h"

#include "../ui/menudef.h"          // for the voice chats

// OSP: defined in g_main.c; declared here, not in g_local.h, so that g_main.c keeps
// its bss order
extern vmCvar_t osp_gamename;

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage(gentity_t* ent) {
	gclient_t*   cl;
	int         clientNum;
	char        entry[1024];
	int         i;
	int         stringlength;
	int         j;
	int         scoreFlags;
	int         ping;
	int         accuracy;
	int         perfect;
	char        string[1400];
	int         numSorted;

	// send the latest information on all clients
	scoreFlags = 0;
	stringlength = 0;

	// OSP: at most 18 entries
	numSorted = level.numConnectedClients > 18 ? 18 : level.numConnectedClients;
	string[0] = 0;

	for (i=0 ; i < numSorted ; i++) {
		clientNum = level.sortedClients[i];
		cl = &level.clients[clientNum];

		accuracy = cl->accuracy_shots ? cl->accuracy_hits * 100 / cl->accuracy_shots : 0;
		perfect = (cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0) ? 1 : 0;
		ping = (cl->pers.connected == CON_CONNECTING) ? -1 : (cl->ps.ping < 999 ? cl->ps.ping : 999);
		scoreFlags = OSP_ClientScore(g_entities + clientNum);

		Com_sprintf(entry, sizeof(entry),
		            " %i %i %i %i %i %i %i %i %i %i %i %i %i %i", clientNum,
		            cl->ps.persistant[PERS_SCORE], ping, (osp_matchTime - cl->pers.enterTime)/60000,
		            scoreFlags, g_entities[clientNum].s.powerups, accuracy,
		            cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
		            cl->ps.persistant[PERS_EXCELLENT_COUNT],
		            cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT],
		            cl->ps.persistant[PERS_DEFEND_COUNT],
		            cl->ps.persistant[PERS_ASSIST_COUNT],
		            perfect,
		            cl->ps.persistant[PERS_CAPTURES]);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy(string + stringlength, entry);
		stringlength += j;
	}

	trap_SendServerCommand(ent-g_entities, va("scores %i %i %i%s", i,
	                       level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
	                       string));
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f(gentity_t* ent) {
	DeathmatchScoreboardMessage(ent);
}



/*
==================
CheatsOk
==================
*/
qboolean    CheatsOk(gentity_t* ent) {
	if (!g_cheats.integer) {
		trap_SendServerCommand(ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return qfalse;
	}
	if (ent->health <= 0) {
		trap_SendServerCommand(ent-g_entities, va("print \"You must be alive to use this command.\n\""));
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char*    ConcatArgs(int start) {
	int     i, c, tlen;
	static char line[MAX_STRING_CHARS];
	int     len;
	char    arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for (i = start ; i < c ; i++) {
		trap_Argv(i, arg, sizeof(arg));
		tlen = strlen(arg);
		if (len + tlen >= MAX_STRING_CHARS - 1) {
			break;
		}
		memcpy(line + len, arg, tlen);
		len += tlen;
		if (i != c - 1) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

// OSP accepts ^X / ^x followed by six hex digits as a color code
#define OSP_IS_HEX( c ) ( ( ( c ) >= '0' && ( c ) <= '9' ) || ( ( c ) >= 'A' && ( c ) <= 'F' ) || ( ( c ) >= 'a' && ( c ) <= 'f' ) )

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString(char* in, char* out, qboolean toLower) {
	while (*in) {
		if (*in == 27 || *in == Q_COLOR_ESCAPE) {
			in++;       // skip color code
			// OSP: "^Xrrggbb" / "^xrrggbb" hex colors
			if ((*in == 'X' || *in == 'x') &&
			        in[1] && OSP_IS_HEX(in[1]) && in[2] && OSP_IS_HEX(in[2]) &&
			        in[3] && OSP_IS_HEX(in[3]) && in[4] && OSP_IS_HEX(in[4]) &&
			        in[5] && OSP_IS_HEX(in[5]) && in[6] && OSP_IS_HEX(in[6])) {
				in += 7;
				continue;
			}
			if (*in) {
				in++;
				continue;
			}
			break;
		}
		if (*in < 32) {
			in++;
			continue;
		}
		if (toLower) {
			*out++ = tolower(*in++);
		} else {
			*out++ = *in++;
		}
	}

	*out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString(gentity_t* to, char* s) {
	gclient_t*   cl;
	int         idnum;
	char        s2[MAX_STRING_CHARS];
	char        n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi(s);
		if (idnum < 0 || idnum >= level.maxclients) {
			trap_SendServerCommand(to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
			return -1;
		}

		cl = &level.clients[idnum];
		if (cl->pers.connected == CON_DISCONNECTED) {
			trap_SendServerCommand(to-g_entities, va("print \"Client %i is not active.\n\"", idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString(s, s2, qtrue);
	for (idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++) {
		if (cl->pers.connected == CON_DISCONNECTED) {
			continue;
		}
		SanitizeString(cl->pers.netname, n2, qtrue);
		if (!strcmp(n2, s2)) {
			return idnum;
		}
	}

	trap_SendServerCommand(to-g_entities, va("print \"\"%s\" is not connected.\n\"", s));
	return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f(gentity_t* ent)
{
	char*        name;
	gitem_t*     it;
	int         i;
	qboolean    give_all;
	gentity_t*       it_ent;
	trace_t     trace;

	if (!CheatsOk(ent)) {
		return;
	}

	name = ConcatArgs(1);

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all || Q_stricmp(name, "health") == 0)
	{
		ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_NUM_WEAPONS) - 1 -
		                                      (1 << WP_GRAPPLING_HOOK) - (1 << WP_NONE);
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i = 0 ; i < MAX_WEAPONS ; i++) {
			ent->client->ps.ammo[i] = 999;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		ent->client->ps.stats[STAT_ARMOR] = 200;

		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "excellent") == 0) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "impressive") == 0) {
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "defend") == 0) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "assist") == 0) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}

	// spawn a specific item right on the player
	if (!give_all) {
		it = BG_FindItem(name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy(ent->r.currentOrigin, it_ent->s.origin);
		it_ent->classname = it->classname;
		G_SpawnItem(it_ent, it);
		FinishSpawningItem(it_ent);
		memset(&trace, 0, sizeof(trace));
		Touch_Item(it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity(it_ent);
		}
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f(gentity_t* ent)
{
	char*    msg;

	if (!CheatsOk(ent)) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE))
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand(ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f(gentity_t* ent) {
	char*    msg;

	if (!CheatsOk(ent)) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET))
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand(ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f(gentity_t* ent) {
	char*    msg;

	if (!CheatsOk(ent)) {
		return;
	}

	if (ent->client->noclip) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand(ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f(gentity_t* ent) {
	if (!CheatsOk(ent)) {
		return;
	}

	// doesn't work in single player
	if (g_gametype.integer != 0) {
		trap_SendServerCommand(ent-g_entities,
		                       "print \"Must be in g_gametype 0 for levelshot\n\"");
		return;
	}

	BeginIntermission();
	trap_SendServerCommand(ent-g_entities, "clientLevelShot");
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_TeamTask_f(gentity_t* ent) {
	char userinfo[MAX_INFO_STRING];
	char        arg[MAX_TOKEN_CHARS];
	int task;
	int client = ent->client - level.clients;

	if (trap_Argc() != 2) {
		return;
	}
	trap_Argv(1, arg, sizeof(arg));
	task = atoi(arg);

	trap_GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap_SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}



/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f(gentity_t* ent) {
	if (is_spectator(ent->client) || ent->health <= 0 || osp_matchPaused) {
		return;
	}
	// OSP: suicides count against match_maxsuicides in team games
	if (!level.warmupTime && g_gametype.integer >= GT_TEAM) {
		ent->client->sess.suicidePenalty += 1.0f;
		if (match_maxsuicides.integer && ent->client->sess.suicidePenalty > match_maxsuicides.integer) {
			char    name[MAX_NETNAME];
			char    userinfo[MAX_INFO_STRING];

			if (level.numFilters < MAX_OSP_FILTERS) {
				trap_GetUserinfo(ent - g_entities, userinfo, sizeof(userinfo));
				Q_strncpyz(name, Info_ValueForKey(userinfo, "name"), sizeof(name));
				OSP_NormalizeName(ent->client->pers.netname, MAX_NETNAME, name, 1);
				osp_filters[level.numFilters].type = 4;
				strcpy(osp_filters[level.numFilters].name, name);
				strcpy(osp_filters[level.numFilters].ip, ent->client->sess.ip);
				strcpy(osp_filters[level.numFilters].password, "none");
				level.numFilters++;
			}
			trap_SendServerCommand(-1, va("print \"%s has been banned due to excessive suicide!\n\"", ent->client->pers.netname));
			trap_SendConsoleCommand(EXEC_APPEND, va("clientkick %d\n", ent - g_entities));
			return;
		}
	}
	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die(ent, ent, ent, 100000, MOD_SUICIDE);
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange(gclient_t* client, int oldTeam)
{
	qboolean    doPrint = qtrue;
	char*        teamName = NULL;

	// OSP: no announcements during the first two seconds of a match
	if (level.time - level.startTime < 2000 && !level.warmupTime) {
		return;
	}

	switch (client->sess.sessionTeam) {
		case TEAM_FREE:
			teamName = "battle";
			break;
		case TEAM_RED:
			teamName = va(S_COLOR_RED "%s" S_COLOR_WHITE " team", osp_teamInfo[TEAM_RED].teamName);
			break;
		case TEAM_BLUE:
			teamName = va(S_COLOR_BLUE "%s" S_COLOR_WHITE " team", osp_teamInfo[TEAM_BLUE].teamName);
			break;
		case TEAM_SPECTATOR:
			if (oldTeam != TEAM_SPECTATOR) {
				teamName = S_COLOR_YELLOW "SPECTATORS";
			} else {
				doPrint = qfalse;
			}
			break;
		default:
		// [[fallthrough]];
		case TEAM_NUM_TEAMS:
			teamName = S_COLOR_RED "Wrong team!";
			doPrint = qtrue;
			break;
	}

	if (doPrint) {
		trap_SendServerCommand(-1, va("cp \"%s\n^N" S_COLOR_WHITE "joined the %s.\n\"", client->pers.netname, teamName));
	}
}

// OSP: spec lock tests of SetTeam. A spectator who is not a referee and lacks
// the invitation of a spec-locked team gets SPECTATOR_LOCKED instead of the requested view
// (bit 1 of sess.specInvite = red, bit 2 = blue, as in g_active.c's OSP_SPECLOCKED).
#define OSP_SPECLOCKED_FOLLOW( client ) \
    ( !client->sess.referee \
    && ( ( ( client->sess.specInvite == 0 || client->sess.specInvite == 2 ) && osp_teamInfo[TEAM_RED].specLocked ) \
    || ( ( client->sess.specInvite == 0 || client->sess.specInvite == 1 ) && osp_teamInfo[TEAM_BLUE].specLocked ) ) )
#define OSP_SPECLOCKED_FREE( client ) \
    ( !client->sess.referee && !level.intermissiontime && !level.intermissionQueued && g_gametype.integer >= GT_TEAM \
    && ( ( osp_teamInfo[TEAM_RED].specLocked && ( client->sess.specInvite == 0 || client->sess.specInvite == 2 ) ) \
    || ( osp_teamInfo[TEAM_BLUE].specLocked && client->sess.specInvite < 2 ) ) )
/*
=================
SetTeam
=================
*/
void SetTeam(gentity_t* ent, char* s) {
	gclient_t*           client;
	int                 team;
	int                 oldTeam;
	spectatorState_t    specState;
	int                 clientNum;
	int                 specClient;

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if (!Q_stricmp(s, "scoreboard") || !Q_stricmp(s, "score")) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if (!Q_stricmp(s, "follow1")) {
		if (client->sess.coachTeam) {
			return;
		}
		team = TEAM_SPECTATOR;
		if (OSP_SPECLOCKED_FOLLOW(client)) {
			specState = SPECTATOR_LOCKED;
		} else {
			specState = SPECTATOR_FOLLOW;
			specClient = -1;
		}
	} else if (!Q_stricmp(s, "follow2")) {
		if (client->sess.coachTeam) {
			return;
		}
		team = TEAM_SPECTATOR;
		if (OSP_SPECLOCKED_FOLLOW(client)) {
			specState = SPECTATOR_LOCKED;
		} else {
			specState = SPECTATOR_FOLLOW;
			specClient = -2;
		}
	} else if (!Q_stricmp(s, "spectator") || !Q_stricmp(s, "s")) {
		if (client->sess.coachTeam) {
			return;
		}
		team = TEAM_SPECTATOR;
		if (OSP_SPECLOCKED_FREE(client)) {
			specState = SPECTATOR_LOCKED;
		} else {
			specState = SPECTATOR_FREE;
		}
	} else if (!Q_stricmp(s, "viewcam") || !Q_stricmp(s, "vc")) {
		if (client->sess.coachTeam) {
			return;
		}
		team = TEAM_SPECTATOR;
		if (OSP_SPECLOCKED_FREE(client)) {
			specState = SPECTATOR_LOCKED;
		} else {
			specState = SPECTATOR_FREE;
		}
		if (osp_numViewcams && specState == SPECTATOR_FREE) {
			specState = SPECTATOR_VIEWCAM;
		}
	} else if (OSP_CheckClientFilters(client->pers.netname, ent - g_entities, 1) == 2) {
		// OSP: filtered names are forced to spectate
		team = TEAM_SPECTATOR;
		if (OSP_SPECLOCKED_FREE(client)) {
			specState = SPECTATOR_LOCKED;
		} else {
			specState = SPECTATOR_FREE;
		}
	} else if (g_gametype.integer >= GT_TEAM) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if (!Q_stricmp(s, "red") || !Q_stricmp(s, "r") || !Q_stricmp(s, "re")) {
			team = TEAM_RED;
		} else if (!Q_stricmp(s, "blue") || !Q_stricmp(s, "b") || !Q_stricmp(s, "bl") || !Q_stricmp(s, "blu")) {
			team = TEAM_BLUE;
		} else if (!Q_stricmp(s, "any")) {
			// pick the team with the least number of players
			team = PickTeam(clientNum);
			if (!OSP_CheckTeamJoin(team, ent, 0)) {
				team = TEAM_SPECTATOR - team;
				if (!OSP_CheckTeamJoin(team, ent, 0)) {
					team = TEAM_SPECTATOR;
					if (OSP_SPECLOCKED_FREE(client)) {
						specState = SPECTATOR_LOCKED;
					} else {
						specState = SPECTATOR_FREE;
					}
				}
			}
		} else {
			if (client->sess.coachTeam) {
				return;
			}
			team = TEAM_SPECTATOR;
			if (OSP_SPECLOCKED_FREE(client)) {
				specState = SPECTATOR_LOCKED;
			} else {
				specState = SPECTATOR_FREE;
			}
		}

		if (!OSP_CheckTeamJoin(team, ent, 1)) {
			return;
		}

		if (g_teamForceBalance.integer) {
			int     counts[TEAM_NUM_TEAMS];

			if (!OSP_IsRoundBased(g_gametype.integer)) {
				counts[TEAM_BLUE] = TeamCount(client->ps.clientNum, TEAM_BLUE);
				counts[TEAM_RED] = TeamCount(client->ps.clientNum, TEAM_RED);
			} else {
				counts[TEAM_BLUE] = OSP_TeamPlayerCount(TEAM_BLUE);
				counts[TEAM_RED] = OSP_TeamPlayerCount(TEAM_RED);
			}

			// We allow a spread of two
			if (team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1) {
				trap_SendServerCommand(ent - g_entities,
				                       "print \"Red team has too many players.\n\"");
				return; // ignore the request
			}
			if (team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1) {
				trap_SendServerCommand(ent - g_entities,
				                       "print \"Blue team has too many players.\n\"");
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	OSP_MV_RemoveAll(ent);
	if (specState == SPECTATOR_LOCKED) {
		ent->client->pers.followTeam = 0;
	}

	// override decision if limiting the players
	if ((g_gametype.integer == GT_TOURNAMENT)
	        && level.numNonSpectatorClients >= 2) {
		team = TEAM_SPECTATOR;
	} else if (g_maxGameClients.integer > 0 &&
	           level.numNonSpectatorClients >= g_maxGameClients.integer) {
		team = TEAM_SPECTATOR;
	} else if (g_gametype.integer == GT_FFA && !level.warmupTime && g_doWarmup.integer && !match_latejoin.integer &&
	           !(ent->r.svFlags & SVF_BOT)) {
		// OSP: no late joining a running FFA match
		if (team != TEAM_SPECTATOR) {
			trap_SendServerCommand(ent - g_entities, "cp \"^1** Match in Progress!! **\n^3You can join next round\n\"");
		}
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	oldTeam = client->sess.sessionTeam;
	if (team == oldTeam && (team != TEAM_SPECTATOR || client->sess.coachTeam)) {
		return;
	}

	if (team != oldTeam || team != TEAM_SPECTATOR) {
		client->pers.invitedTeam = 0;
		client->pers.coachInvite = 0;
		client->sess.coachTeam = 0;
		client->pers.followTeam = 0;
	}

	if (team != oldTeam) {
		client->pers.ready = 0;
		client->readyToExit = qfalse;
		if (!team_nocontrols.integer && client->sess.captain && team != TEAM_SPECTATOR) {
			OSP_SetCaptain(ent);
		}
		if (g_gametype.integer >= GT_TEAM && TeamCount(ent - g_entities, oldTeam) == 0) {
			osp_teamInfo[oldTeam].specLocked = 0;
		}
	}

	// OSP: the first player of a team becomes its captain
	if (g_gametype.integer >= GT_TEAM && team != TEAM_SPECTATOR && !team_nocontrols.integer &&
	        TeamCount(ent - g_entities, team) == 0) {
		client->sess.captain = 1;
		if (!level.intermissiontime) {
			trap_SendServerCommand(ent - g_entities, "print \"** ^5You are team captain!!^7\n\"");
		}
	} else {
		client->sess.captain = 0;
	}

	//
	// execute the team change
	//

	// if the player was dead leave the body
	if (client->ps.stats[STAT_HEALTH] <= 0) {
		CopyToBodyQue(ent);
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if (!level.intermissiontime || !OSP_IsRoundBased(g_gametype.integer)) {
		if (oldTeam != TEAM_SPECTATOR) {
			// Kill him (makes sure he loses flags, etc)
			ent->flags &= ~FL_GODMODE;
			client->ps.stats[STAT_HEALTH] = ent->health = 0;
			player_die(ent, ent, ent, 100000, MOD_SWITCHTEAM);
		}
		if (team == TEAM_SPECTATOR) {
			// they go to the end of the line for tournements
			if (oldTeam != team || g_gametype.integer == GT_TOURNAMENT) {
				client->sess.spectatorTime = level.time;
			}
			client->pers.ready = 0;
			client->readyToExit = qfalse;
			OSP_StartCountdown();
		} else if (g_warmup.integer && level.warmupTime && !osp_matchPaused && !(ent->r.svFlags & SVF_BOT)) {
			if (!OSP_IsRoundBased(g_gametype.integer) || !osp_roundsPlayed) {
				level.warmupTime = level.time + (1000 * g_warmup.integer - 1000);
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
			}
		}
	}

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	if (!level.intermissiontime || g_gametype.integer != 5) {
		BroadcastTeamChange(client, oldTeam);
	}

	// get and distribute relevent paramters
	ClientUserinfoChanged(clientNum);

	if (!level.intermissiontime || !OSP_IsRoundBased(g_gametype.integer)) {
		ClientBegin(clientNum);
	} else if (!level.intermissiontime && OSP_IsRoundBased(g_gametype.integer)) {
		client->readyToExit = qtrue;
		client->pers.osp_unk_2bc = level.time;
		trap_SendServerCommand(ent - g_entities, va("print \"You will join the %s team next round.\n\"", osp_teamNames[team]));
	}

	OSP_MV_Validate();
	OSP_ResetPlayerReady();
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing(gentity_t* ent) {
	if (!IS_FREEZETAG) {
		ent->client->sess.sessionTeam = TEAM_SPECTATOR;
	} else {
		// OSP freeze tag: a frozen player keeps his team
		SetClientViewAngle(ent, ent->client->ps.viewangles);
		memset(ent->client->ps.powerups, 0, sizeof(ent->client->ps.powerups));
	}
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f(gentity_t* ent) {
	char        s[MAX_TOKEN_CHARS];
	int         oldTeam;

	if (level.intermissiontime && !OSP_IsRoundBased(g_gametype.integer)) {
		return;
	}
	if (ent->client->sess.nameTeam) {
		return;
	}

	if (trap_Argc() != 2) {
		char*    team = S_COLOR_YELLOW "Spectator";

		oldTeam = ent->client->sess.sessionTeam;
		switch (oldTeam) {
			case TEAM_BLUE:
				team = S_COLOR_BLUE "RED" S_COLOR_WHITE;    // sic
				break;
			case TEAM_RED:
				team = S_COLOR_RED "RED" S_COLOR_WHITE;
				break;
			case TEAM_FREE:
				team = S_COLOR_GREEN "FREE" S_COLOR_WHITE;
				break;
		}
		trap_SendServerCommand(ent-g_entities, va("print \"You are currently on the %s" S_COLOR_WHITE " team\n\"", team));
		return;
	}

	if (ent->client->switchTeamTime > level.time) {
		trap_SendServerCommand(ent-g_entities, "print \"May not switch teams more than once per 3 seconds.\n\"");
		return;
	}

	// if they are playing a tournement game, count as a loss
	if (g_gametype.integer == GT_TOURNAMENT && !level.warmupTime
	        && ent->client->sess.sessionTeam == TEAM_FREE) {
		ent->client->sess.losses++;
	}

	if (OSP_IsRoundBased(g_gametype.integer) &&
	        (ent->client->sess.sessionTeam == TEAM_BLUE || ent->client->sess.sessionTeam == TEAM_RED)) {
		ent->client->sess.roundTeam = 0;
	}

	trap_Argv(1, s, sizeof(s));

	if (s[0] == 's' || s[0] == 'S') {
		ent->client->sess.roundTeam = 0;
		ent->client->pers.followTeam = 0;
	}

	SetTeam(ent, s);

	ent->client->pers.mvSavedCount = 0;
	ent->client->switchTeamTime = level.time + 3000;
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f(gentity_t* ent) {
	int     i;
	char    arg[MAX_TOKEN_CHARS];

	if (trap_Argc() != 2 && ent->client->sess.spectatorState != SPECTATOR_VIEWCAM_FOLLOW) {
		return;
	}
	if (ent->client->sess.nameTeam) {
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	i = ClientNumberFromString(ent, arg);
	if (i == -1 && ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		if (ent->client->sess.spectatorState == SPECTATOR_VIEWCAM_FOLLOW) {
			i = ent->client->sess.spectatorClient;
		} else if (g_gametype.integer >= GT_TEAM) {
			// OSP: "follow red" / "follow blue" locks the view on a team
			int     team = 0;

			if (!Q_stricmp(arg, "blue") || !Q_stricmp(arg, "b") || !Q_stricmp(arg, "bl") || !Q_stricmp(arg, "blu")) {
				team = TEAM_BLUE;
			} else if (!Q_stricmp(arg, "red") || !Q_stricmp(arg, "r") || !Q_stricmp(arg, "re")) {
				team = TEAM_RED;
			} else {
				return;
			}

			if (!TeamCount(ent - g_entities, team)) {
				if (ent->client->sess.sessionTeam != team) {
					trap_SendServerCommand(ent - g_entities, va("print \"The %s team is empty!  Follow command ignored.\n\"", osp_teamNames[team]));
				} else {
					trap_SendServerCommand(ent - g_entities, va("print \"The %s team would be empty!  Follow command ignored.\n\"", osp_teamNames[team]));
				}
				return;
			}

			if (ent->client->pers.followTeam != team) {
				int     invite = ent->client->sess.specInvite;

				if (!ent->client->sess.referee && osp_teamInfo[team].specLocked && (!invite || (invite != team && invite != 3))) {
					trap_SendServerCommand(ent - g_entities, va("print \"Sorry, the %s team is locked from spectators.\n\"", osp_teamNames[team]));
					return;
				}
				ent->client->pers.followTeam = team;
				ent->client->sess.roundTeam = TEAM_SPECTATOR;
				trap_SendServerCommand(ent - g_entities, va("print \"Spectator follow is now locked on the %s team.\n\"", osp_teamNames[team]));
				Cmd_FollowCycle_f(ent, 1);
				ent->client->pers.followTeam = team;
				ent->client->sess.roundTeam = TEAM_SPECTATOR;
			} else {
				ent->client->pers.followTeam = 0;
				ent->client->sess.roundTeam = TEAM_SPECTATOR;
				trap_SendServerCommand(ent - g_entities, va("print \"%s team spectating is now disbled.\n\"", osp_teamNames[team]));
			}
			ClientUserinfoChanged(ent - g_entities);
			return;
		} else {
			return;
		}
	}

	// can't follow self
	if (&level.clients[ i ] == ent->client) {
		return;
	}

	// OSP: frozen players stay with their body
	if (ent->frozen && !is_spectator(ent->client)) {
		return;
	}

	// OSP freeze tag: players can only follow their own team
	if (IS_FREEZETAG && ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
	        level.clients[ i ].sess.sessionTeam != ent->client->sess.sessionTeam) {
		return;
	}

	// can't follow another spectator
	if (is_spectator(&level.clients[ i ])) {
		return;
	}

	// OSP: coaches only follow their team
	if (ent->client->sess.coachTeam && ent->client->sess.coachTeam != level.clients[ i ].sess.sessionTeam) {
		return;
	}

	// if they are playing a tournement game, count as a loss
	if (g_gametype.integer == GT_TOURNAMENT && !level.warmupTime
	        && ent->client->sess.sessionTeam == TEAM_FREE) {
		ent->client->sess.losses++;
	}

	if (ent->client->pers.mvCount) {
		OSP_MV_RemoveAll(ent);
	}

	// first set them to spectator
	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
		SetTeam(ent, "spectator");
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f(gentity_t* ent, int dir) {
	int     clientnum;

	// OSP: multiview and viewcam have their own cycling
	if (ent->client->pers.mvCount > 0) {
		OSP_Cmd_ViewNext_f(ent, dir);
		return;
	}
	if (ent->client->sess.spectatorState == SPECTATOR_VIEWCAM_FOLLOW) {
		OSP_ViewcamCyclePlayer(ent, dir);
		return;
	}
	if (!is_spectator(ent->client)) {
		return;
	}
	if (IS_FREEZETAG) {
		if (ent->frozen && !is_spectator(ent->client)) {
			return;
		}
		if (FT_RespawnPlayer(ent)) {
			return;
		}
	}

	// if they are playing a tournement game, count as a loss
	if (g_gametype.integer == GT_TOURNAMENT && !level.warmupTime
	        && ent->client->sess.sessionTeam == TEAM_FREE) {
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if (ent->client->sess.spectatorState == SPECTATOR_NOT) {
		SetTeam(ent, "s");
	}

	clientnum = OSP_FollowNext(ent, dir, ent->client->pers.followTeam);
	if (clientnum == -1) {
		ent->client->pers.followTeam = 0;
		SetTeam(ent, "s");
		return;
	}

	// this is good, we can use it
	ent->client->sess.spectatorClient = clientnum;
	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
}


/*
==================
G_Say
==================
*/

static void G_SayTo(gentity_t* ent, gentity_t* other, int mode, int color, const char* name, const char* message, qboolean broadcast) {
	int     entTeam;
	int     otherTeam;

	if (!other || !other->inuse || !other->client || other->client->pers.connected != CON_CONNECTED) {
		return;
	}
	// OSP: coaches talk to the team they coach
	entTeam = OSP_ClientTeam(ent);
	otherTeam = OSP_ClientTeam(other);
	if (mode == SAY_TEAM && entTeam != otherTeam) {
		return;
	}

	trap_SendServerCommand(other-g_entities, va("%s \"%s%c%c%s\"",
	                       mode == SAY_TEAM ? "tchat" : "chat",
	                       name, Q_COLOR_ESCAPE, color, message));
}

#define EC      "\x19"

void G_Say(gentity_t* ent, gentity_t* target, int mode, const char* chatText) {
	int         j;
	gentity_t*   other;
	char*        text;
	int         color;
	char        name[64];
	char        posName[64];
	int         team;
	char        location[64];

	if (!OSP_ChatFilter(chatText)) {
		return;
	}

	team = OSP_ClientTeam(ent);
	if (g_gametype.integer < GT_TOURNAMENT && mode == SAY_TEAM && !ent->client->sess.referee && team != TEAM_SPECTATOR) {
		mode = SAY_ALL;
	}
	// OSP: muted spectators can only talk to the other spectators
	if (((g_gametype.integer == GT_TOURNAMENT && team != TEAM_FREE) || (g_gametype.integer >= GT_TEAM && team == TEAM_SPECTATOR)) &&
	        match_mutespecs.integer && !level.warmupTime && !level.intermissiontime && !osp_matchPaused && !ent->client->sess.referee) {
		mode = SAY_TEAM;
	}

	switch (mode) {
		default:
		case SAY_ALL:
			G_LogPrintf("say: %s: %s\n", ent->client->pers.netname, chatText);
			Com_sprintf(name, sizeof(name), "%s" S_COLOR_WHITE EC": ", ent->client->pers.netname);
			color = COLOR_GREEN;
			break;
		case SAY_TEAM:
			G_LogPrintf("sayteam: %s: %s\n", ent->client->pers.netname, chatText);
			// OSP clients that understand the position tokens get the sender's origin
			Com_sprintf(posName, sizeof(posName), "(%s" S_COLOR_WHITE ") (^I$%i$%i$%i^N): ", ent->client->pers.netname,
			            (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2]);
			if (Team_GetLocationMsg(ent, location, sizeof(location))) {
				Com_sprintf(name, sizeof(name), EC"(%s%s"EC") (%s)"EC": ",
				            ent->client->pers.netname, osp_gamename.integer ? "^N" S_COLOR_WHITE : S_COLOR_WHITE, location);
			} else {
				Com_sprintf(name, sizeof(name), EC"(%s%s"EC")"EC": ",
				            ent->client->pers.netname, osp_gamename.integer ? "^N" S_COLOR_WHITE : S_COLOR_WHITE);
			}
			color = COLOR_CYAN;
			break;
		case SAY_TELL:
			if (target && g_gametype.integer >= GT_TEAM &&
			        OSP_ClientTeam(target) == team &&
			        Team_GetLocationMsg(ent, location, sizeof(location)))
				Com_sprintf(name, sizeof(name), EC"[%s" S_COLOR_WHITE EC"] (%s)"EC": ", ent->client->pers.netname, location);
			else
				Com_sprintf(name, sizeof(name), EC"[%s" S_COLOR_WHITE EC"]"EC": ", ent->client->pers.netname);
			color = COLOR_MAGENTA;
			break;
	}

	text = OSP_ChatTokens(ent, color, chatText);

	if (target) {
		G_SayTo(ent, target, mode, color, name, text, qfalse);
		return;
	}

	// echo the text to the console
	if (g_dedicated.integer) {
		G_Printf("%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		if (mode == SAY_TEAM && other->client->pers.osp_unk_3ac) {
			G_SayTo(ent, other, mode, color, posName, text, qtrue);
		} else {
			G_SayTo(ent, other, mode, color, name, text, qtrue);
		}
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f(gentity_t* ent, int mode, qboolean arg0) {
	char*        p;

	if (trap_Argc() < 2 && !arg0) {
		return;
	}

	p = ConcatArgs(arg0 ? 0 : 1);

	G_Say(ent, NULL, mode, p);
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f(gentity_t* ent) {
	gentity_t*   target;
	int         targetNum;
	char*        p;
	char        arg[MAX_TOKEN_CHARS];

	if (trap_Argc() < 2) {
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	targetNum = atoi(arg);
	if (targetNum < 0 || targetNum >= level.maxclients) {
		return;
	}

	target = &g_entities[targetNum];
	if (!target || !target->inuse || !target->client) {
		return;
	}

	// OSP: no tells from spectators to players in round-based modes
	if (OSP_IsRoundBased(g_gametype.integer) && ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
	        target->client->sess.sessionTeam != TEAM_SPECTATOR && !ent->client->sess.referee) {
		return;
	}

	p = ConcatArgs(2);

	G_LogPrintf("tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p);
	G_Say(ent, target, SAY_TELL, p);
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if (ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say(ent, ent, SAY_TELL, p);
	}
}





static char* gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

void Cmd_GameCommand_f(gentity_t* ent) {
	int     player;
	int     order;
	char    str[MAX_TOKEN_CHARS];

	trap_Argv(1, str, sizeof(str));
	player = atoi(str);
	trap_Argv(2, str, sizeof(str));
	order = atoi(str);

	if (player < 0 || player >= MAX_CLIENTS) {
		return;
	}
	if (order < 0 || order > sizeof(gc_orders)/sizeof(char*)) {
		return;
	}
	G_Say(ent, &g_entities[player], SAY_TELL, gc_orders[order]);
	G_Say(ent, ent, SAY_TELL, gc_orders[order]);
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f(gentity_t* ent) {
	trap_SendServerCommand(ent-g_entities, va("print \"%s\n\"", vtos(ent->s.origin)));
}

/*
==================
Cmd_CallVote_f
==================
*/
qboolean Cmd_CallVote_f(gentity_t* ent, qboolean fRefCommand) {
	int     i;
	char    arg1[MAX_STRING_TOKENS];
	char    arg2[MAX_STRING_TOKENS];
	char    arg3[MAX_STRING_TOKENS];

	if (!fRefCommand) {
		if (!g_allowVote.integer && !ent->client->sess.referee) {
			trap_SendServerCommand(ent-g_entities, "print \"Voting not allowed here.\n\"");
			return qtrue;
		}

		if (level.voteTime) {
			trap_SendServerCommand(ent-g_entities, "print \"A vote is already in progress.\n\"");
			return qtrue;
		}

		// OSP: no votes during the first 20 seconds of a map
		if (g_gametype.integer != 5 && !ent->client->pers.localClient && level.time - level.startTime < 20000) {
			trap_SendServerCommand(ent-g_entities, va("print \"Voting will be re-enabled in %d seconds.\n\"",
			                       (20000 - (level.time - level.startTime)) / 1000));
			return qtrue;
		}
	}

	// make sure it is a valid command to vote on
	trap_Argv(1, arg1, sizeof(arg1));
	trap_Argv(2, arg2, sizeof(arg2));
	trap_Argv(3, arg3, sizeof(arg3));

	// no vote argument is legitimately longer than a map or player name; longer ones only
	// serve to overflow or to flood everyone with oversized vote strings
	if (strchr(arg1, ';') || strchr(arg2, ';') || strchr(arg3, ';') ||
	        strlen(arg1) >= 32 || strlen(arg2) >= MAX_QPATH || strlen(arg3) >= MAX_QPATH) {
		if (!fRefCommand && !ent->client->sess.referee) {
			trap_SendServerCommand(ent-g_entities, "print \"Invalid vote string!\n\"");
		} else {
			trap_SendServerCommand(ent-g_entities, "print \"Invalid ref command string!\n\"");
		}
		return qfalse;
	}

	if (!Q_stricmp(arg1, "map")) {
		if (!vote_allow_map.integer && !ent->client->sess.referee) {
			trap_SendServerCommand(ent-g_entities, "print \"Sorry, ^3map^7 voting has been disabled\n\"");
			return qtrue;
		}
	} else if ((i = OSP_Vote_Parse(ent, arg1, arg2, arg3, sizeof(arg3), fRefCommand))) {
		if (i < 0) {
			return qtrue;
		}
	} else {
		if (!fRefCommand) {
			OSP_Vote_Help(ent, 1);
		}
		return qfalse;
	}

	if (!ent->client->sess.referee && !ent->client->sess.coachTeam && ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
	        level.numVotingClients > 0 && level.numNonSpectatorClients > 0) {
		trap_SendServerCommand(ent-g_entities, "print \"^3Spectators can't call a vote with players in the game!\n\"");
		return qfalse;
	}

	if (!fRefCommand && !ent->client->sess.referee && vote_limit.integer > 0 && trap_Argc() > 2 &&
	        ent->client->pers.votesCalled >= vote_limit.integer && !level.warmupTime) {
		trap_SendServerCommand(ent-g_entities, "print \"You have called the maximum number of votes for this map.\n\"");
		return qtrue;
	}

	if (!Q_stricmp(arg1, "map")) {
		char    s[MAX_STRING_CHARS];

		if (map_queue.integer) {
			if (!OSP_Vote_Map(ent, arg2, fRefCommand)) {
				return qtrue;
			}
			Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s", arg1, arg2);
			osp_voteType = 23;
		} else {
			// special case for map changes, we want to reset the nextmap setting
			// this allows a player to change maps, but not upset the map rotation
			trap_Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
			if (*s) {
				Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s; set nextmap \"%s\"", arg1, arg2, s);
			} else {
				Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s", arg1, arg2);
			}
		}
	} else {
		Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s", arg1, arg2);
	}

	if (!level.warmupTime) {
		ent->client->pers.votesCalled++;
	}

	// start the voting, the caller automatically votes yes
	level.voteTime = level.time;
	if (ent->client->sess.referee) {
		// OSP: a referee's vote passes at once
		level.voteYes = level.numVotingClients + 10;
		OSP_AdminLogPrintf(va("Referee_Setting\t%s\t%s", ent->client->pers.netname, level.voteString));
		if (osp_voteType != 21) {
			trap_SendServerCommand(-1, "cp \"^1** Referee Server Setting Change **\n\"");
		}
		OSP_GlobalSound("sound/items/wearoff.wav");
	} else {
		level.voteYes = 1;
		trap_SendServerCommand(-1, va("print \"%s" S_COLOR_WHITE " called a vote.\n\"", ent->client->pers.netname));
		trap_SendServerCommand(-1, va("cp \"%s\n" S_COLOR_WHITE "called a vote.\n\"", ent->client->pers.netname));
		OSP_AdminLogPrintf(va("Vote_Call\t%s\t%s", ent->client->pers.netname, level.voteString));
		OSP_GlobalSound("sound/world/klaxon2.wav");
	}
	level.voteNo = 0;

	for (i = 0 ; i < level.maxclients ; i++) {
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->client->ps.eFlags |= EF_VOTED;

	trap_SetConfigstring(CS_VOTE_TIME, va("%i", level.voteTime));
	trap_SetConfigstring(CS_VOTE_STRING, va(S_COLOR_YELLOW "%s" S_COLOR_WHITE, level.voteString));
	trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteYes));
	trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteNo));
	return qtrue;
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f(gentity_t* ent) {
	char        msg[64];

	if (!level.voteTime) {
		trap_SendServerCommand(ent-g_entities, "print \"No vote in progress.\n\"");
		return;
	}
	if (ent->client->ps.eFlags & EF_VOTED) {
		trap_SendServerCommand(ent-g_entities, "print \"Vote already cast.\n\"");
		return;
	}

	trap_SendServerCommand(ent-g_entities, "print \"Vote cast.\n\"");

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv(1, msg, sizeof(msg));

	if (msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1') {
		// OSP: a referee decides the vote
		if (ent->client->sess.referee && osp_voteType != 48) {
			level.voteYes = level.numVotingClients;
			level.voteNo = 0;
		} else {
			level.voteYes++;
		}
		trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteYes));
	} else {
		if (ent->client->sess.referee && osp_voteType != 48) {
			level.voteNo = level.numVotingClients;
			level.voteYes = 0;
		} else {
			level.voteNo++;
		}
		trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteNo));
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f(gentity_t* ent) {
	vec3_t      origin, angles;
	char        buffer[MAX_TOKEN_CHARS];
	int         i;

	if (!g_cheats.integer) {
		trap_SendServerCommand(ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return;
	}
	if (trap_Argc() != 5) {
		trap_SendServerCommand(ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear(angles);
	for (i = 0 ; i < 3 ; i++) {
		trap_Argv(i + 1, buffer, sizeof(buffer));
		origin[i] = atof(buffer);
	}

	trap_Argv(4, buffer, sizeof(buffer));
	angles[YAW] = atof(buffer);

	TeleportPlayer(ent, origin, angles);
}



/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f(gentity_t* ent) {
	/*
	    int max, n, i;

	    max = trap_AAS_PointReachabilityAreaIndex( NULL );

	    n = 0;
	    for ( i = 0; i < max; i++ ) {
	        if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
	            n++;
	    }

	    //trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
	    trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
	*/
}

/*
=================
ClientCommand
=================
*/
void ClientCommand(int clientNum) {
	char    cmd[MAX_TOKEN_CHARS];
	gentity_t* ent;

	ent = g_entities + clientNum;
	if (!ent->client) {
		return;     // not fully in game yet
	}

	// OSP: client authentication still pending
	if (ent->client->pers.authTime > 0 && server_ospauth.integer) {
		OSP_CheckAuthTimeout(ent);
	}

	trap_Argv(0, cmd, sizeof(cmd));

	if (Q_stricmp(cmd, "score") == 0) {
		Cmd_Score_f(ent);
		return;
	}
	if (Q_stricmp(cmd, "say") == 0) {
		Cmd_Say_f(ent, SAY_ALL, qfalse);
		return;
	}
	if (Q_stricmp(cmd, "say_team") == 0) {
		Cmd_Say_f(ent, SAY_TEAM, qfalse);
		return;
	}
	if (Q_stricmp(cmd, "tell") == 0) {
		Cmd_Tell_f(ent);
		return;
	}
	if (Q_stricmp(cmd, "team") == 0) {
		if (!OSP_CmdHelpCheck(ent, 37)) {
			Cmd_Team_f(ent);
		}
		return;
	}
	if (Q_stricmp(cmd, "uinfo") == 0) {
		OSP_Cmd_UInfo_f(ent);
		return;
	}
	if (Q_stricmp(cmd, "uvinfo") == 0) {
		OSP_Cmd_UInfo_f(ent);
		OSP_MV_SendViewList(ent);
		return;
	}
	if (!Q_stricmp(cmd, "ref") || !Q_stricmp(cmd, "admin") || !Q_stricmp(cmd, "referee")) {
		if (!OSP_CmdHelpCheck(ent, 24)) {
			OSP_Cmd_Ref_f(ent, 0);
		}
		return;
	}
	if (!Q_stricmp(cmd, "players") || !Q_stricmp(cmd, "playerlist")) {
		OSP_Cmd_Players_f(ent);
		return;
	}
	if (!Q_stricmp(cmd, "getstatsinfo")) {
		OSP_Cmd_GetStatsInfo_f(ent);
		return;
	}
	if (!Q_stricmp(cmd, "stats") || !Q_stricmp(cmd, "stat")) {
		OSP_Cmd_Stats_f(ent, qfalse);
		return;
	}
	if (!Q_stricmp(cmd, "statsdump")) {
		OSP_Cmd_Stats_f(ent, qtrue);
		return;
	}
	if (!Q_stricmp(cmd, "statsall")) {
		OSP_Cmd_StatsAll_f(ent, 0);
		return;
	}
	if (!Q_stricmp(cmd, "statsblue")) {
		OSP_Cmd_StatsAll_f(ent, TEAM_BLUE);
		return;
	}
	if (!Q_stricmp(cmd, "statsred")) {
		OSP_Cmd_StatsAll_f(ent, TEAM_RED);
		return;
	}
	if (!Q_stricmp(cmd, "topshots") || !Q_stricmp(cmd, "acc") || !Q_stricmp(cmd, "accuracy")) {
		OSP_Cmd_TopShots_f(ent, qtrue);
		return;
	}
	if (!Q_stricmp(cmd, "bottomshots")) {
		OSP_Cmd_TopShots_f(ent, qfalse);
		return;
	}
	if (server_ospauth.integer && !Q_stricmp(cmd, "htuatneilc")) {
		OSP_Cmd_ClientAuth_f(ent);
		return;
	}
	if (!Q_stricmp(cmd, "autoscreenshot")) {
		if (!OSP_CmdHelpCheck(ent, 22)) {
			OSP_Cmd_AutoRecord(ent - g_entities, 0, 1);
		}
		return;
	}
	if (!Q_stricmp(cmd, "scores") && g_gametype.integer != GT_SINGLE_PLAYER) {
		if (g_gametype.integer == GT_TEAM) {
			if (!IS_FREEZETAG) {
				OSP_Scores_TDM(ent, 0);
			} else {
				OSP_Scores_FT(ent, 0);
			}
		} else if (g_gametype.integer == GT_CTF) {
			OSP_Scores_CTF(ent, 0);
		} else if (g_gametype.integer == 5) {
			OSP_Scores_CA(ent, 0);
		} else {
			OSP_Scores_FFA(ent, 0);
		}
		return;
	}
	if (!Q_stricmp(cmd, "viewlist")) {
		OSP_MV_SendViewList(ent);
		return;
	}
	if (ent->client->sess.referee && OSP_RefCommand(ent, cmd, 0)) {
		return;
	}
	if (!Q_stricmp(cmd, "callvote") || !Q_stricmp(cmd, "cv")) {
		OSP_CmdHelpCheck(ent, 40);
		Cmd_CallVote_f(ent, qfalse);
		return;
	}
	if (!Q_stricmp(cmd, "vote")) {
		Cmd_Vote_f(ent);
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime) {
		Cmd_Say_f(ent, qfalse, qtrue);
		return;
	}

	if (Q_stricmp(cmd, "give") == 0)
		Cmd_Give_f(ent);
	else if (Q_stricmp(cmd, "god") == 0)
		Cmd_God_f(ent);
	else if (Q_stricmp(cmd, "notarget") == 0)
		Cmd_Notarget_f(ent);
	else if (Q_stricmp(cmd, "noclip") == 0)
		Cmd_Noclip_f(ent);
	else if (Q_stricmp(cmd, "kill") == 0)
		Cmd_Kill_f(ent);
	else if (Q_stricmp(cmd, "follow") == 0) {
		if (!OSP_CmdHelpCheck(ent, 38)) {
			Cmd_Follow_f(ent);
		}
	}
	else if (Q_stricmp(cmd, "follownext") == 0)
		Cmd_FollowCycle_f(ent, 1);
	else if (Q_stricmp(cmd, "followprev") == 0)
		Cmd_FollowCycle_f(ent, -1);
	else if (Q_stricmp(cmd, "where") == 0)
		Cmd_Where_f(ent);
	else if (Q_stricmp(cmd, "gc") == 0)
		Cmd_GameCommand_f(ent);
	else if (!OSP_ClientCommand(ent, cmd, qfalse))
		trap_SendServerCommand(clientNum, va("print \"Unknown command: ^5%s" S_COLOR_WHITE "\nType ^3\\help" S_COLOR_WHITE " for a list of all commands.\n\"", cmd));
}
