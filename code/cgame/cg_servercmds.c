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
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include "cg_local.h"
#include "../qcommon/qcommon.h"
#include "cg_superhud.h"


/*
=================
CG_ParseScores

=================
*/
static void CG_ParseScores(void)
{
	int     i, powerups;

	cg.numScores = atoi(CG_Argv(1));
	if (cg.numScores > MAX_CLIENTS)
	{
		cg.numScores = MAX_CLIENTS;
	}

	cg.teamScores[0] = atoi(CG_Argv(2));
	cg.teamScores[1] = atoi(CG_Argv(3));

	memset(cg.scores, 0, sizeof(cg.scores));
	for (i = 0 ; i < cg.numScores ; i++)
	{
		//
		cg.scores[i].client = atoi(CG_Argv(i * 14 + 4));
		cg.scores[i].score = atoi(CG_Argv(i * 14 + 5));
		cg.scores[i].ping = atoi(CG_Argv(i * 14 + 6));
		cg.scores[i].time = atoi(CG_Argv(i * 14 + 7));
		cg.scores[i].scoreFlags = atoi(CG_Argv(i * 14 + 8));
		powerups = atoi(CG_Argv(i * 14 + 9));
		cg.scores[i].accuracy = atoi(CG_Argv(i * 14 + 10));
		cg.scores[i].impressiveCount = atoi(CG_Argv(i * 14 + 11));
		cg.scores[i].excellentCount = atoi(CG_Argv(i * 14 + 12));
		cg.scores[i].guantletCount = atoi(CG_Argv(i * 14 + 13));
		cg.scores[i].defendCount = atoi(CG_Argv(i * 14 + 14));
		cg.scores[i].assistCount = atoi(CG_Argv(i * 14 + 15));
		cg.scores[i].perfect = atoi(CG_Argv(i * 14 + 16));
		cg.scores[i].captures = atoi(CG_Argv(i * 14 + 17));

		if (cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS)
		{
			cg.scores[i].client = 0;
		}
		cgs.clientinfo[ cg.scores[i].client ].score = cg.scores[i].score;
		cgs.clientinfo[ cg.scores[i].client ].powerups = powerups;

		cg.scores[i].team = cgs.clientinfo[cg.scores[i].client].team;
	}
}

/*
=================
CG_ParseTeamInfo

=================
*/
static void CG_ParseTeamInfo(void)
{
	int     i;
	int     client;
	clientInfo_t* ci;

	numSortedTeamPlayers = atoi(CG_Argv(1));
	if (numSortedTeamPlayers > TEAM_MAXOVERLAY)
	{
		numSortedTeamPlayers = TEAM_MAXOVERLAY;
	}

	for (i = 0 ; i < numSortedTeamPlayers ; i++)
	{
		client = atoi(CG_Argv(i * 6 + 2));

		ci = &cgs.clientinfo[ client ];

		sortedTeamPlayers[i] = client;

		if (customLocationsEnabled)
		{
			CG_CustomLocationsSetLocation(CG_Argv(i * 6 + 3), ci->customLocation);
		}
		ci->location = atoi(CG_Argv(i * 6 + 3));
		ci->health = atoi(CG_Argv(i * 6 + 4));
		ci->armor = atoi(CG_Argv(i * 6 + 5));
		ci->curWeapon = atoi(CG_Argv(i * 6 + 6));
		ci->powerups = atoi(CG_Argv(i * 6 + 7));
	}
}


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo(void)
{
	const char*  info;
	char*    mapname;

	info = CG_ConfigString(CS_SERVERINFO);
	cgs.gametype = atoi(Info_ValueForKey(info, "g_gametype"));
	trap_Cvar_Set("g_gametype", va("%i", cgs.gametype));
	cgs.dmflags = atoi(Info_ValueForKey(info, "dmflags"));
	cgs.teamflags = atoi(Info_ValueForKey(info, "teamflags"));
	cgs.fraglimit = atoi(Info_ValueForKey(info, "fraglimit"));
	cgs.capturelimit = atoi(Info_ValueForKey(info, "capturelimit"));
	cgs.timelimit = atoi(Info_ValueForKey(info, "timelimit"));
	cgs.maxclients = atoi(Info_ValueForKey(info, "sv_maxclients"));
	mapname = Info_ValueForKey(info, "mapname");
	Com_sprintf(cgs.mapname, sizeof(cgs.mapname), "maps/%s.bsp", mapname);
	Q_strncpyz(cgs.redTeam, Info_ValueForKey(info, "g_redTeam"), sizeof(cgs.redTeam));
	trap_Cvar_Set("g_redTeam", cgs.redTeam);
	Q_strncpyz(cgs.blueTeam, Info_ValueForKey(info, "g_blueTeam"), sizeof(cgs.blueTeam));
	trap_Cvar_Set("g_blueTeam", cgs.blueTeam);
}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup(void)
{
	const char*  info;
	int         warmup;

	info = CG_ConfigString(CS_WARMUP);

	warmup = atoi(info);
	cg.warmupCount = -1;

	if (warmup == 0 && cg.warmup)
	{

	}
	else if (warmup > 0 && cg.warmup <= 0)
	{
		{
			trap_S_StartLocalSound(cgs.media.countPrepareSound, CHAN_ANNOUNCER);
		}
	}

	cg.warmup = warmup;
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues(void)
{
	const char* s;

	cgs.scores1 = atoi(CG_ConfigString(CS_SCORES1));
	cgs.scores2 = atoi(CG_ConfigString(CS_SCORES2));
	cgs.levelStartTime = atoi(CG_ConfigString(CS_LEVEL_START_TIME));
	if (cgs.gametype == GT_CTF)
	{
		s = CG_ConfigString(CS_FLAGSTATUS);
		cgs.redflag = s[0] - '0';
		cgs.blueflag = s[1] - '0';
	}
	cg.warmup = atoi(CG_ConfigString(CS_WARMUP));
}

/*
=====================
CG_ShaderStateChanged
=====================
*/
void CG_ShaderStateChanged(void)
{
	char originalShader[MAX_QPATH];
	char newShader[MAX_QPATH];
	char timeOffset[16];
	const char* o;
	char* n, *t;

	o = CG_ConfigString(CS_SHADERSTATE);
	while (o && *o)
	{
		n = strstr(o, "=");
		if (n && *n)
		{
			strncpy(originalShader, o, n - o);
			originalShader[n - o] = 0;
			n++;
			t = strstr(n, ":");
			if (t && *t)
			{
				strncpy(newShader, n, t - n);
				newShader[t - n] = 0;
			}
			else
			{
				break;
			}
			t++;
			o = strstr(t, "@");
			if (o)
			{
				strncpy(timeOffset, t, o - t);
				timeOffset[o - t] = 0;
				o++;
				trap_R_RemapShader(originalShader, newShader, timeOffset);
			}
		}
		else
		{
			break;
		}
	}
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified(void)
{
	const char*  str;
	int     num;

	num = atoi(CG_Argv(1));

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	trap_GetGameState(&cgs.gameState);

	// look up the individual string that was modified
	str = CG_ConfigString(num);

	// do something with it if necessary
	if (num == CS_MUSIC)
	{
		CG_StartMusic();
	}
	else if (num == CS_SERVERINFO)
	{
		CG_ParseServerinfo();
	}
	else if (num == CS_WARMUP)
	{
		CG_ParseWarmup();
	}
	else if (num == CS_SCORES1)
	{
		cgs.osp.osp_teamcount1 = -1;
		Q_sscanf(str, "%i %i", &cgs.scores1, &cgs.osp.osp_teamcount1);
	}
	else if (num == CS_SCORES2)
	{
		cgs.osp.osp_teamcount2 = -1;
		Q_sscanf(str, "%i %i", &cgs.scores2, &cgs.osp.osp_teamcount2);
	}
	else if (num == CS_LEVEL_START_TIME)
	{
		cgs.levelStartTime = atoi(str);
	}
	else if (num == CS_VOTE_TIME)
	{
		cgs.voteTime = atoi(str);
		cgs.voteModified = qtrue;
	}
	else if (num == CS_VOTE_YES)
	{
		CG_OSPWStatsUp_f();
		cgs.voteYes = atoi(str);
		cgs.voteModified = qtrue;
	}
	else if (num == CS_VOTE_NO)
	{
		cgs.voteNo = atoi(str);
		cgs.voteModified = qtrue;
	}
	else if (num == CS_VOTE_STRING)
	{
		Q_strncpyz(cgs.voteString, str, sizeof(cgs.voteString));
	}
	else if (num == CS_INTERMISSION)
	{
		CG_OSPWStatsUp_f();
		cg.intermissionStarted = atoi(str);
		if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR &&
		        CG_OSPIsGameTypeCA(cgs.gametype) &&
		        cgs.clientinfo[cg.snap->ps.clientNum].rt == TEAM_SPECTATOR)
		{
			return;
		}
		CG_OSPWStatsDown_f();
	}
	else if (num >= CS_MODELS && num < CS_MODELS + MAX_MODELS)
	{
		cgs.gameModels[ num - CS_MODELS ] = trap_R_RegisterModel(str);
	}
	else if (num >= CS_SOUNDS && num < CS_SOUNDS + MAX_MODELS)
	{
		if (str[0] != '*')       // player specific sounds don't register here
		{
			cgs.gameSounds[ num - CS_SOUNDS] = trap_S_RegisterSound(str, qfalse);
		}
	}
	else if (num >= CS_PLAYERS && num < CS_PLAYERS + MAX_CLIENTS)
	{
		CG_NewClientInfo(num - CS_PLAYERS);
		CG_BuildSpectatorString();
	}
	else if (num == CS_FLAGSTATUS)
	{
		if (cgs.gametype == GT_CTF)
		{
			// format is rb where its red/blue, 0 is at base, 1 is taken, 2 is dropped
			cgs.redflag = str[0] - '0';
			cgs.blueflag = str[1] - '0';
			if (strlen(str) > 3)
			{
				cgs.osp.flagStatus = 1;
				cgs.osp.redflag = str[2] - '0';
				cgs.osp.blueflag = str[3] - '0';
			}
		}
	}
	else if (num == CS_OSP_ALLOW_PMOVE)
	{
		CG_OSPConfigPmoveSet(atoi(str));
	}
	else if (num == CS_OSP_MAXPACKETS_MIN)
	{
		CG_OSPConfigMinMaxPacketsSet(atoi(str));
	}
	else if (num == CS_OSP_MAXPACKETS_MAX)
	{
		CG_OSPConfigMaxMaxPacketsSet(atoi(str));
	}
	else if (num == CS_OSP_TIMENUDGE_MIN)
	{
		CG_OSPConfigMinTimenudgeSet(atoi(str));
	}
	else if (num == CS_OSP_TIMENUDGE_MAX)
	{
		CG_OSPConfigMaxTimenudgeSet(atoi(str));
	}
	else if (num == CS_OSP_AUTH)
	{
		CG_OSPConfig0x368Set(atoi(str) & 1);
	}
	else if (num == CS_OSP_CLAN_BASE_TEAM_DM)
	{
		CG_OSPConfigClanBaseTDMSet(atoi(str));
	}
	else if (num == CS_OSP_CUSTOM_CLIENT)
	{
		CG_OSPConfigCustomClientSet(atoi(str));
		CG_OSPCvarsRestrictValues();
	}
	else if (num == CS_OSP_CUSTOM_CLIENT2)
	{
		CG_OSPConfigCustomClient2Set(atoi(str));
	}
	else if (num == CS_OSP_SERVER_MODE)
	{
		CG_OSPConfigModeSet(atoi(str));
	}
	else if (num == CS_OSP_FREEZE_GAME_TYPE)
	{
		CG_OSPConfigFreezeModeSet(atoi(str));
	}
	else if (num == CS_SHADERSTATE)
	{
		CG_ShaderStateChanged();
	}

}

/*
=======================
CG_AddToTeamChat

=======================
*/
void CG_AddToTeamChat(char* str, int size)
{
	int len;
	int w;
	int h;
	int chatHeight;
	char* p, *ls;
	int lastcolor;
	int overlayWidth;
	int teamchatWidth;

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
	{
		chatHeight = cg_teamChatHeight.integer;
	}
	else
	{
		chatHeight = TEAMCHAT_HEIGHT;
	}

	if (chatHeight <= 0 || cg_teamChatTime.integer <= 0)
	{
		// team chat disabled, dump into normal chat
		cgs.teamChatPos = cgs.teamLastChatPos = 0;
		return;
	}

	len = 0;
	p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
	*p = 0;
	lastcolor = '7';
	ls = NULL;
	CG_OSPGetClientFontSize(&cf_Teamchat, &w, &h);

	if (cg_drawTeamOverlay.integer == 2)
	{
		overlayWidth = teamOverlayWidth;
	}
	else
	{
		overlayWidth = 0;
	}

	teamchatWidth = (SCREEN_WIDTH - overlayWidth) / w;

	(*(p) = 0);
	while (*str)
	{
		if (len > teamchatWidth - 1)
		{
			if (ls)
			{
				str -= (p - ls);
				str++;
				p -= (p - ls);
			}
			*p = 0;

			cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;

			cgs.teamChatPos++;
			p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
			if (ch_InverseTeamChat.integer != 0)
			{
				Q_strncpyz(cgs.teamChatMsgs[cgs.teamChatPos % chatHeight], cgs.teamChatMsgs[(cgs.teamChatPos - 1) % chatHeight], TEAMCHAT_WIDTH * 3 + 1);
				p = cgs.teamChatMsgs[(cgs.teamChatPos - 1) % chatHeight];
			}
			else
			{
				p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
			}
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
		}

		if (Q_IsColorString(str))
		{
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}
		if (*str == ' ')
		{
			ls = p;
		}
		*p++ = *str++;
		len++;
	}

	*p = 0;

	cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;
	cgs.teamChatPos++;

	if (cgs.teamChatPos - cgs.teamLastChatPos > chatHeight)
		cgs.teamLastChatPos = cgs.teamChatPos - chatHeight;
}

/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart(void)
{
	if (cg_showmiss.integer)
	{
		CG_Printf("CG_MapRestart\n");
	}

	CG_InitLocalEntities();
	CG_InitMarkPolys();
	CG_ClearParticles();

	// make sure the "3 frags left" warnings play again
	cg.fraglimitWarnings = 0;

	cg.timelimitWarnings = 0;

	cg.intermissionStarted = qfalse;

	cgs.voteTime = 0;

	cg.mapRestart = qtrue;

	cg.scoreFadeTime = 0;

	CG_OSPWStatsUp_f();

	CG_StartMusic();

	trap_S_ClearLoopingSounds(qtrue);

	if (cgs.osp.isOSPv1)
	{
		CG_OSPUpdateUserInfo(qtrue);
	}

	// we really should clear more parts of cg here and stop sounds

	// play the "fight" sound if this is a restart without warmup
	if (cg.warmup == 0 /* && cgs.gametype == GT_TOURNAMENT */)
	{
		trap_S_StartLocalSound(cgs.media.countFightSound, CHAN_ANNOUNCER);
		if (!cg_shud.integer) CG_CenterPrint("^1FIGHT!", 20, GIANTCHAR_WIDTH * 2);
	}
	trap_Cvar_Set("cg_thirdPerson", "0");
}


/*
=================
CG_RemoveChatEscapeChar
=================
*/
void CG_RemoveChatEscapeChar(char* text)
{
	int i, l;
	char command;

	l = 0;
	for (i = 0; text[i]; i++)
	{
		if (text[i] == '\x19')
			continue;

		if (text[i] != '^')
		{
			text[l++] = text[i];
			continue;
		}

		command = text[i + 1];

		if (command == 'X' || command == 'x')
		{
			float tmp;
			if (CG_Hex16GetColor(&text[i + 2], &tmp) &&
			        CG_Hex16GetColor(&text[i + 4], &tmp) &&
			        CG_Hex16GetColor(&text[i + 6], &tmp))
			{
				i += 6;
			}
			++i;
		}
		else if (command)
		{
			switch (command)
			{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					text[l++] = text[i];
					break;
				case '9':
					++i;
					text[l++] = '^';
					text[l++] = '7';
					break;
				case '8':
					++i;
					text[l++] = '^';
					text[l++] = '3';
					break;
				case '^':
					++i;
					text[l++] = '^';
					text[l++] = '^';
					break;
				default:
					++i;
			}
		}

	}
	text[l] = '\0';
}

void CG_StringMakeEscapeCharRAW(const char* in, char* out, int max)
{
	int i, l;
	qboolean fix_color = qfalse;

	l = 0;
	for (i = 0; in[i] && l < max - 5; ++i)
	{
		out[l++] = in[i];
		if (fix_color)
		{
			// small hack. as client responds ^^4 as color anyway, lets make it looks like ^^4^74
			out[l++] = '^';
			out[l++] = '7';
			out[l++] = in[i];
			fix_color = qfalse;
		}
		if (in[i] == '^')
		{
			out[l++] = '^';
			if ((in[i + 1] >= '0' && in[i + 1] <= '9') || (in[i + 1] >= 'a' && in[i + 1] <= 'z') || (in[i + 1] >= 'A' && in[i + 1] <= 'Z'))
			{
				fix_color = qtrue;
			}
		}
	}
	out[l] = '\0';
}

/*
=================
CG_RemoveChatEscapeCharAll
=================
*/
void CG_RemoveChatEscapeCharAll(char* text)
{
	int i, l;
	char command;

	l = 0;
	for (i = 0; text[i]; i++)
	{
		if (text[i] == '\x19')
			continue;

		if (text[i] != '^')
		{
			text[l++] = text[i];
			continue;
		}

		command = text[i + 1];

		if (command == 'X' || command == 'x')
		{
			float tmp;
			if (CG_Hex16GetColor(&text[i + 2], &tmp) &&
			        CG_Hex16GetColor(&text[i + 4], &tmp) &&
			        CG_Hex16GetColor(&text[i + 6], &tmp))
			{
				i += 6;
			}
		}
		else if (command == '^')
		{
			text[l++] = '^';
		}
		++i;
	}
	text[l] = '\0';
}

static void CG_OSPPrintXStats(void)
{
	int i;
	int index = 1;
	int wstats_condition;
	char str[32];
	qboolean damage_activity = qfalse;
	int client_id;

	index = 1;
	damage_activity = qfalse;

	client_id = atoi(CG_Argv(index++));
	wstats_condition = atoi(CG_Argv(index++));

	Q_strncpyz(&str[0], cgs.clientinfo[client_id].name, 32);

	CG_Printf(va("\n^7Accuracy info for: ^3%s^7\n\n", str));
	CG_Printf("Weapon        Accrcy Hits/Atts Kills Deaths Pickup Drops\n");
	CG_Printf("--------------------------------------------------------\n");
	if (wstats_condition == 0)
	{
		CG_Printf("No weapon info available.\n");
		return;
	}
	/* цикл по оружию */
	for (i = 1; i < 10 ; ++i)
	{
		if ((wstats_condition & (1 << i)) != 0)
		{
			int atts_value;
			int hits_value;
			int kills_value;
			int deaths_value;
			float accuracy;
			hits_value = atoi(CG_Argv(index++));
			atts_value = atoi(CG_Argv(index++));
			kills_value = atoi(CG_Argv(index++));
			deaths_value = atoi(CG_Argv(index++));

			CG_Printf(va("^3%-12s: ", weaponNames[i]));

			if (atts_value & cgs.osp.stats_mask || hits_value & cgs.osp.stats_mask)
			{
				if (!(atts_value & cgs.osp.stats_mask))
				{
					accuracy = 0.0;
				}
				else
				{
					accuracy = 100.0 * ((hits_value & cgs.osp.stats_mask) / (float)(atts_value & cgs.osp.stats_mask));
				}
				CG_Printf(va("^7%6.1f ^5%4d/%-4d ", accuracy, hits_value & cgs.osp.stats_mask, atts_value & cgs.osp.stats_mask));
				damage_activity = qtrue;
			}
			else
			{
				CG_Printf("                 ");
				if (kills_value || deaths_value)
				{
					damage_activity = qtrue;
				}
			}

			if (i > 2)
			{
				CG_Printf(va("^2%5d ^1%6d ^3%6d ^1%5d\n", kills_value, deaths_value, atts_value >> cgs.osp.stats_shift, hits_value >> cgs.osp.stats_shift));
			}
			else
			{
				CG_Printf(va("^2%5d ^1%6d\n", kills_value, deaths_value));
			}
		}
	}

	if (CG_OSPIsStatsHidden(qtrue, qtrue))
	{
		CG_Printf("\n^3Damage Given: ^7XX  ^2Armor Taken : ^7XX\n");
		CG_Printf("^3Damage Recvd: ^7XX  ^2Health Taken: ^7XX\n");
	}
	else
	{
		int yellow_armor;
		int green_armor;
		int red_armor;
		int megahealth;
		int armor_taken;
		int health_taken;
		int damage_given;
		int damage_rcvd;
		char* char_ptr1;
		char* char_ptr2;
		char tmp_str[32];
		char tmp_str2[32];
		damage_given = atoi(CG_Argv((index + 2)));
		damage_rcvd = atoi(CG_Argv((index + 3)));

		armor_taken = atoi(CG_Argv(index));
		health_taken = atoi(CG_Argv((index + 1)));
		megahealth = atoi(CG_Argv((index + 4)));
		green_armor = atoi(CG_Argv((index + 5)));
		red_armor = atoi(CG_Argv((index + 6)));
		yellow_armor = atoi(CG_Argv((index + 7)));

		tmp_str[0] = 0;
		tmp_str2[0] = 0;
		if (megahealth != 0)
		{
			strcpy(tmp_str, va("^2(^7%d ^5MH^2)", megahealth));
		}
		if (green_armor != 0)
		{
			if (red_armor == 0 && yellow_armor == 0)
			{
				char_ptr1 = "^2)";
			}
			else
			{
				char_ptr1 = "^7,";
			}

			strcpy(tmp_str, va("^2(^7%d ^3GA%s", green_armor, char_ptr1));
		}

		if (yellow_armor != 0)
		{
			if (green_armor == 0)
			{
				char_ptr1 = "^2(^7";
				char_ptr2 = "";
			}
			else
			{
				char_ptr1 = " ";
				if (red_armor != 0)
				{
					char_ptr2 = "^7,";
				}
				else
				{
					char_ptr2 = "^2)";
				}
			}
			strcat(tmp_str, va("%s%d ^3YA%s", char_ptr1, yellow_armor, char_ptr2));
		}
		if (red_armor != 0)
		{
			char_ptr1 = " ";
			if (yellow_armor == 0 && green_armor == 0)
			{
				char_ptr1 = "^2(^7";
			}
			strcat(tmp_str, va("%s%d ^1RA^2)", char_ptr1, red_armor));
		}

		if (damage_activity != 0)
		{
			CG_Printf("\n^3Damage Given: ^7%-6d ^2Armor Taken : ^7%d %s\n", damage_given, armor_taken, tmp_str);
			CG_Printf("^3Damage Recvd: ^7%-6d ^2Health Taken: ^7%d %s\n", damage_rcvd, health_taken, tmp_str2);
		}
		else
		{
			CG_Printf("\n^2Armor Taken : ^7%d %s\n", armor_taken, tmp_str);
			CG_Printf("^2Health Taken: ^7%d %s\n\n", health_taken, tmp_str2);
		}
	}
}

static void CG_OSPPrintSSet(void)
{
	int i;
	char buf[512];
	char* prefix;
	int k;
	int argNum;

	prefix = "";
	argNum = trap_Argc();
	for (k = 0, buf[0] = 0, i = 1; i < argNum; i += 2)
	{
		strcat(buf, va("%s%20s ", prefix, CG_Argv(i)));
		strcat(buf, va("%-3s", CG_Argv((i + 1))));

		prefix = "  ";
		if (k++ == 3)
		{
			CG_Printf("%s\n", buf);
			k = 0;
			prefix = "";
			buf[0] = 0;
		}
	}

	if (buf[0])
	{
		CG_Printf("%s\n", buf);
	}
}

static void CG_ServerCommandStuff(void)
{
	char command[1024];
	char command2[1024];
	int len;

	if (cg.demoPlayback)
	{
		return;
	}

	trap_Args(&command[0], 0x400);
	command[1023] = 0;

	len = strlen(&command[0]);

	{
		char* ptr = &command[0];
		char* out = &command2[0];
		int i;
		for (i = 0; *ptr && i < len;)
		{
			if (*ptr == '\'')
			{
				++ptr;
				if (*ptr == '\'')
				{
					*out++ = '\'';
				}
				else
				{
					*out++ = '"';
				}
				++ptr;
				++i;
			}
			else
			{
				*out++ = *ptr++;
				++i;
			}
		}
		*out = 0;
	}

	if (strstr(command2, "screenshot") ||
	        strstr(command2, "set g_synchronous") ||
	        strstr(command2, "clear") ||
	        strstr(command2, "currenttime") ||
	        strstr(command2, "stoprecord"))
	{
		trap_SendConsoleCommand(command2);
	}
}

/*
=================
CG_InjectCustomLoc

Replace location coordinates with location name
=================
*/
static void CG_InjectCustomLoc(char* str, int size)
{
	char* loc_start;
	char* loc_end;

	if (customLocationsEnabled != 0)
	{
		char* cloc_begin, *cloc_end;
		int free_left;
		vec3_t cloc;
		if (CG_CustomLocationsTeamChatCode(str, cloc, &cloc_begin, &cloc_end))
		{
			int location_len;
			const char* location_name;
			char* tmp;

			location_name = CG_CustomLocationsGetName(cloc);
			location_len = strlen(location_name); //size of message without location

			tmp = Z_Malloc(size);
			OSP_MEMORY_CHECK(tmp);

			Q_strncpyz(tmp, cloc_end, size);
			free_left = size - (cloc_begin - str);
			Q_strncpyz(cloc_begin, location_name, free_left);
			free_left -= location_len;
			Q_strncpyz(cloc_begin + location_len, tmp, free_left);

			Z_Free(tmp);
		}
	}

	if (ch_FilterLocationsTeamchat.integer)
	{
		loc_start = strchr(str, '(');
		if (loc_start)
		{
			loc_start = strchr(loc_start + 1, '(');
			if (loc_start)
			{
				loc_end = strchr(loc_start, ')');
				if (loc_end)
				{
					strcpy(loc_start - 1, loc_end + 1);
				}
			}
		}
	}
}

/*
=================
CG_OSPAstats

Print topshots weapon or bottomshots weapon
=================
*/
static void CG_OSPAstats(qboolean top)
{
	int a = 1;
	int s = 0;
	float average;
	int number_of_entries;
	int atts;
	float acc;
	int hits;
	char* color_code;
	int client_num;
	int kills;
	int deaths;
	float level;
	float treshold;

	number_of_entries = atoi(CG_Argv(a++));
	if (number_of_entries == 0)
	{
		return;
	}

	average = atoi(CG_Argv(a++));

	CG_Printf("\n^3  Acc Hits/Atts Kills Deaths\n");
	CG_Printf("----------------------------------------------------------\n");
	for (s = 0; s < number_of_entries; ++s)
	{
		client_num = atoi(CG_Argv(a++));
		atts = atoi(CG_Argv(a++));
		hits = atoi(CG_Argv(a++));
		kills = atoi(CG_Argv(a++));
		deaths = atoi(CG_Argv(a++));

		acc = atts > 0 ? (float)(100 * atts) / (float)hits : 0;
		level = top ? acc : (float)average + 0.999f;
		treshold = top ? average : acc;
		color_code = level >= treshold ? "^3" : "^7";

		CG_Printf(va("%s%5.1f ^5%4d/%-4d ^2%5d ^1%6d %s%s\n", color_code, acc, hits, atts, kills, deaths, color_code, cgs.clientinfo[client_num].name_clean));
	}
	CG_Printf("\n\n");
}

/*
=================
CG_OSPBstats

Print topshots weapon or bottomshots weapon
=================
*/
static void CG_OSPBstats(qboolean top)
{
	int a = 1;
	int number_of_weapon;
	int hits;
	int atts;
	int client_num;
	int kills;
	int deaths;
	float acc;

	number_of_weapon = atoi(CG_Argv(a++));
	if (number_of_weapon == 0)
	{
		CG_Printf("\n^3No qualifying %sshot info available.\n\n", top ? "top" : "bottom");
		return;
	}

	CG_Printf("\n^2%sst Match Accuracies:\n", top ? "Be" : "Wor");
	CG_Printf("^3WP   Acc Hits/Atts Kills Deaths\n");
	CG_Printf("-------------------------------------------------------------\n");

	while (number_of_weapon)
	{
		client_num = atoi(CG_Argv(a++));
		atts = atoi(CG_Argv(a++));
		hits = atoi(CG_Argv(a++));
		kills = atoi(CG_Argv(a++));
		deaths = atoi(CG_Argv(a++));

		acc = atts > 0 ? (float)(100 * atts) / (float)hits : 0;

		CG_Printf(va("^3%s ^7%5.1f ^5%4d/%-4d ^2%5d ^1%6d ^7%s\n", weaponShortNames[number_of_weapon], acc, atts, hits, kills, deaths, cgs.clientinfo[client_num].name_clean));

		number_of_weapon = atoi(CG_Argv(a++));
	}
	CG_Printf("\n\n");
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/

void CG_ServerCommand(void)
{
	const char* cmd;
	char        text[1024];

	cmd = CG_Argv(0);
	if (!cmd)
	{
		return;
	}
//psohtua
	if (strcmp(cmd, "psohtua") == 0)
	{
		return;
	}
//cp
	if (strcmp(cmd, "cp") == 0)
	{
		const char* arg;
		arg = CG_Argv(1);
		if (!strstr(arg, "Multi-view demo created with"))
		{
			CG_CenterPrint(arg, 120, SMALLCHAR_WIDTH);
		}
		return;
	}
//cs
	if (strcmp(cmd, "cs") == 0)
	{
		CG_ConfigStringModified();
		return;
	}
//print
	if (strcmp(cmd, "print") == 0)
	{
		Q_strncpyz(text, CG_Argv(1), 1024);
		CG_RemoveChatEscapeChar(text);
		CG_Printf("%s", text);
		return;
	}
//chat
	if (strcmp(cmd, "chat") == 0)
	{
		messageAllowed_t ma;
		qboolean isVanilaChatEnabled;
		qboolean isShudChatEnabled;

		Q_strncpyz(text, CG_Argv(1), 1024);

		ma = CG_ChatCheckMessageAllowed(text);
		if (ma == MESSAGE_NOTALLOWED)
		{
			return;
		}

		isVanilaChatEnabled = cg_chatEnable.integer & CG_CHAT_COMMMON_ENABLED;
		isShudChatEnabled = cg_shudChatEnable.integer & CG_CHAT_COMMMON_ENABLED && ma == MESSAGE_ALLOWED_PLAYER;

		if (!isVanilaChatEnabled && !isShudChatEnabled)
		{
			return;
		}

		if (!cg_nochatbeep.integer)
		{
			trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
		}

		CG_RemoveChatEscapeChar(text);

		if (isVanilaChatEnabled)
		{
			CG_Printf("%s\n", text);
		}
		if (isShudChatEnabled)
		{
			CG_SHUDEventChat(text);
		}
		return;
	}
//tchat
	if (strcmp(cmd, "tchat") == 0)
	{
		messageAllowed_t ma;
		qboolean isVanilaChatEnabled;
		qboolean isShudChatEnabled;

		Q_strncpyz(text, CG_Argv(1), 1024);
		ma = CG_ChatCheckMessageAllowed(text);
		if (ma == MESSAGE_NOTALLOWED)
		{
			return;
		}

		isVanilaChatEnabled = cg_chatEnable.integer & CG_CHAT_TEAM_ENABLED;
		isShudChatEnabled = cg_shudChatEnable.integer & CG_CHAT_TEAM_ENABLED && ma == MESSAGE_ALLOWED_PLAYER;

		if (!isVanilaChatEnabled && !isShudChatEnabled)
		{
			return;
		}

		if (!cg_noTeamChatBeep.integer)
		{
			trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
		}

		CG_RemoveChatEscapeChar(text);
		CG_InjectCustomLoc(text, 1024);

		if (isVanilaChatEnabled)
		{
			CG_AddToTeamChat(text, 1024);
			if (!ch_TeamchatOnly.integer || cgs.gametype == GT_TOURNAMENT)
			{
				CG_Printf("%s\n", text);
			}
		}

		if (isShudChatEnabled)
		{
			CG_SHUDEventChat(text);
		}

		return;
	}
//scores
	if (Q_stricmp(cmd, "scores") == 0)
	{
		CG_ParseScores();
		return;
	}
//tinfo
	if (Q_stricmp(cmd, "tinfo") == 0)
	{
		CG_ParseTeamInfo();
		return;
	}
//map_restart
	if (Q_stricmp(cmd, "map_restart") == 0)
	{
		CG_MapRestart();
		return;
	}
//remapShader
	if (Q_stricmp(cmd, "remapShader") == 0)
	{
		if (trap_Argc() == 4)
		{
			const char* arg[3];
			arg[0] = CG_Argv(1);
			arg[1] = CG_Argv(2);
			arg[2] = CG_Argv(3);
			trap_R_RemapShader(arg[0], arg[1], arg[2]);
		}
		return;
	}
//statsinfo
	if (Q_stricmp(cmd, "statsinfo") == 0)
	{
		CG_OSPShowStatsInfo();
		return;
	}
//viewlist
	if (Q_stricmp(cmd, "viewlist") == 0)
	{
		return;
	}
//sset
	if (Q_stricmp(cmd, "sset") == 0)
	{
		CG_OSPPrintSSet();
		return;
	}
//viewactive
	if (Q_stricmp(cmd, "viewactive") == 0)
	{
		CG_Printf("viewactive is not implimented\n");
		return;
	}
//stuff
	if (Q_stricmp(cmd, "stuff") == 0)
	{
		CG_ServerCommandStuff();
		return;
	}
//crecord
	if (Q_stricmp(cmd, "crecord") == 0)
	{
		return;
	}
//cscreenshot
	if (Q_stricmp(cmd, "cscreenshot") == 0)
	{
		return;
	}
//xstats1
	if (Q_stricmp(cmd, "xstats1") == 0)
	{
		CG_OSPPrintXStats();
		return;
	}
//astats
	if (Q_stricmp(cmd, "astats") == 0)
	{
		CG_OSPAstats(qtrue);
		return;
	}
//astatsb
	if (Q_stricmp(cmd, "astatsb") == 0)
	{
		CG_OSPAstats(qfalse);
		return;
	}
//bstats
	if (Q_stricmp(cmd, "bstats") == 0)
	{
		CG_OSPBstats(qtrue);
		return;
	}
//bstatsb
	if (Q_stricmp(cmd, "bstatsb") == 0)
	{
		CG_OSPBstats(qfalse);
		return;
	}
//clientLevelShot
	if (Q_stricmp(cmd, "clientLevelShot") == 0)
	{
		cg.levelShot = qtrue;
		return;
	}
//UKNOWN COMMAND
//	CG_Printf("Unknown client game command: %s\n", cmd);
}


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands(int latestSequence)
{
	while (cgs.serverCommandSequence < latestSequence)
	{
		if (trap_GetServerCommand(++cgs.serverCommandSequence))
		{
			CG_ServerCommand();
		}
	}
}
