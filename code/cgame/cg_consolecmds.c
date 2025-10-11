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
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "cg_superhud.h"

#define MODIFSTR_SIZE 256
qboolean modifState[5];
char modifStrDown[5][MODIFSTR_SIZE];
char modifStrUp[5][MODIFSTR_SIZE];


void CG_TargetCommand_f(void)
{
	int     targetNum;
	char    test[4];

	targetNum = CG_CrosshairPlayer();
	if (!targetNum)
	{
		return;
	}

	trap_Argv(1, test, 4);
	trap_SendConsoleCommand(va("gc %i %i", targetNum, atoi(test)));
}



/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f(void)
{
	trap_Cvar_Set("cg_viewsize", va("%i", (int)(cg_viewsize.integer + 10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f(void)
{
	trap_Cvar_Set("cg_viewsize", va("%i", (int)(cg_viewsize.integer - 10)));
}

/*
=================
CG_NextCrosshair_f

Keybinding command
=================
*/
static void CG_NextCrosshair_f(void)
{
	int curr = cg_drawCrosshair.integer + 1;

	if (curr < cgs.media.numberOfCrosshairs)
	{
		trap_Cvar_Set("cg_drawCrosshair", va("%i", curr));
	}

}

/*
=================
CG_PrefCrosshair_f

Keybinding command
=================
*/
static void CG_PrevCrosshair_f(void)
{
	int curr = cg_drawCrosshair.integer - 1;

	if (curr >= 0)
	{
		trap_Cvar_Set("cg_drawCrosshair", va("%i", curr));
	}
}

/*
=================
CG_NextCrosshairDecor_f

Keybinding command
=================
*/
static void CG_NextCrosshairDecor_f(void)
{
	int curr = ch_crosshairDecor.integer + 1;

	if (curr < cgs.media.numberOfCrosshairDecors)
	{
		trap_Cvar_Set("ch_crosshairDecor", va("%i", curr));
	}

}

/*
=================
CG_PrefCrosshairDecor_f

Keybinding command
=================
*/
static void CG_PrevCrosshairDecor_f(void)
{
	int curr = ch_crosshairDecor.integer - 1;

	if (curr >= 0)
	{
		trap_Cvar_Set("ch_crosshairDecor", va("%i", curr));
	}
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f(void)
{
	CG_Printf("(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
	          (int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2],
	          (int)cg.refdefViewAngles[YAW]);
}


static void CG_ScoresDown_f(void)
{
	if (!cg.demoPlayback && cg.scoresRequestTime < cg.time)
	{
		cg.scoresRequestTime = cg.time + 2000;
		trap_SendClientCommand("score");
		if (!cg.showScores)
		{
			cg.numScores = 0;
		}
	}
	cg.showScores = qtrue;
}

static void CG_ScoresUp_f(void)
{
	if (cg.showScores)
	{
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}


static void CG_TellTarget_f(void)
{
	int     clientNum;
	char    command[128];
	char    message[128];

	clientNum = CG_CrosshairPlayer();
	if (clientNum == -1)
	{
		return;
	}

	trap_Args(message, 128);
	Com_sprintf(command, 128, "tell %i %s", clientNum, message);
	trap_SendClientCommand(command);
}

static void CG_TellAttacker_f(void)
{
	int     clientNum;
	char    command[128];
	char    message[128];

	clientNum = CG_LastAttacker();
	if (clientNum == -1)
	{
		return;
	}

	trap_Args(message, 128);
	Com_sprintf(command, 128, "tell %i %s", clientNum, message);
	trap_SendClientCommand(command);
}

/*
==================
CG_StartOrbit_f
==================
*/

static void CG_StartOrbit_f(void)
{
	char var[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer("developer", var, sizeof(var));
	if (!atoi(var))
	{
		return;
	}
	if (cg_cameraOrbit.value != 0)
	{
		trap_Cvar_Set("cg_cameraOrbit", "0");
		trap_Cvar_Set("cg_thirdPerson", "0");
	}
	else
	{
		trap_Cvar_Set("cg_cameraOrbit", "5");
		trap_Cvar_Set("cg_thirdPerson", "1");
		trap_Cvar_Set("cg_thirdPersonAngle", "0");
		trap_Cvar_Set("cg_thirdPersonRange", "100");
	}
}

void CG_OSPAddStr_f(void)
{
	char new_value[256];
	char var_name[256];
	char var_value[256];
	int arg_c;
	arg_c = trap_Argc();
	if (arg_c < 3) return;

	trap_Argv(1, var_name, 256);
	trap_Cvar_VariableStringBuffer(var_name, var_value, 256);

	trap_Argv(2, new_value, 256);
	trap_Cvar_VariableStringBuffer(new_value, new_value, 256);

	Q_strcat(var_value, 256, new_value);
	trap_SendConsoleCommand(va("%s \"%s\";", var_name, var_value));
}

void CG_OSPVStrDown_f(void)
{
	char vstr[256];
	trap_Argv(1, vstr, 256);
	trap_SendConsoleCommand(va("vstr %s;", vstr));
}

void CG_OSPVStrUp_f(void)
{
	char vstr[256];
	trap_Argv(2, vstr, 256);
	trap_SendConsoleCommand(va("vstr %s;", vstr));
}

void CG_OSPFireDown_f(void)
{
	int arg_index = 1;
	int weapon_number;
	qboolean done = qfalse;

	if (cg.snap == NULL || cg.snap->ps.pm_flags & PMF_FOLLOW) return;

	while (!done)
	{
		weapon_number = atoi(CG_Argv(arg_index));
		if (weapon_number == 0) break;
		if ((weapon_number > 0) && (weapon_number < 11)
		        && (cg.snap->ps.stats[STAT_WEAPONS] & (1 << weapon_number))
		        && (cg.snap->ps.ammo[weapon_number])
		   )
		{
			trap_SendConsoleCommand(va("weapon %i; wait 2; +attack;", weapon_number));
			done = qtrue;
		}
	}
}

void CG_OSPFireUp_f(void)
{
	trap_SendConsoleCommand("-attack;");
}

void CG_OSPSelect_f(void)
{
	int arg_index = 1;
	int weapon_number;
	qboolean done = qfalse;

	if (cg.snap == NULL || cg.snap->ps.pm_flags & PMF_FOLLOW) return;

	while (!done)
	{
		weapon_number = atoi(CG_Argv(arg_index));
		if (weapon_number == 0) break;
		if ((weapon_number > 0) && (weapon_number < 11)
		        && (cg.snap->ps.stats[STAT_WEAPONS] & (1 << weapon_number))
		        && (cg.snap->ps.ammo[weapon_number])
		   )
		{
			trap_SendConsoleCommand(va("weapon %i;", weapon_number));
			done = qtrue;
		}
	}

}

void CG_OSPServerVersion_f(void)
{
	char* ptr;
	char* pos;
	char* pack;
	char buf[1024];


	CG_Printf("\n^3OSP Server version: ^5\"^7%s^5\"\n\n", Info_ValueForKey(CG_ConfigString(CS_SERVERINFO), "gameversion"));

	ptr = Info_ValueForKey(CG_ConfigString(CS_SYSTEMINFO), "sv_referencedPakNames");
	CG_Printf("^3Referenced .pk3s:\n");

	if (!ptr)
	{
		CG_Printf("   ^7NONE found\n\n");
	}
	else
	{
		Q_strncpyz(buf, ptr, 1024);
		pos = buf;

		while (pos != 0)
		{
			pack = pos;
			pos = strchr(pos, ' ');
			if (pos == NULL) return;
			*pos = 0;
			CG_Printf("   ^7%s\n", pack);

			++pos;

			while (*pos == ' ')
			{
				++pos;
			}
		}
		CG_Printf("\n");
	}
}

void CG_OSPHUDStyle_f(void)
{
}

void CG_OSPHUDNext_f(void)
{
}

void CG_OSPHUDprev_f(void)
{
}

void CG_OSPClientVersion_f(void)
{
	CG_Printf("^3OSP2 Client Version:^7 %s\n", OSP_VERSION);
}

#define CG_YES_NO_STR(VAL) ((VAL) ? "^2Yes" : "^1No")
void CG_OSPClientConfig_f(void)
{
	const char* physics = "VQ3";

	if (cgs.osp.server_mode & 1)
	{
		physics = "Promode";
	}
	else if (cgs.osp.server_mode & 2)
	{
		physics = "CQ3";
	}


	CG_Printf("^5OSP2 Server-forced settings:\n", OSP_VERSION);
	CG_Printf("    ^3Physics:                     ^2%s\n", physics);
	CG_Printf("    ^3Alternative weapons:         %s\n", CG_YES_NO_STR(cgs.osp.custom_client & OSP_CUSTOM_CLIENT_ALT_WEAPON_FLAG));
	CG_Printf("    ^3Timer(deprecated):           %s\n", CG_YES_NO_STR(cgs.osp.custom_client & OSP_CUSTOM_CLIENT_TIMER_FLAG));
	CG_Printf("    ^3FPS restriction(deprecated): %s\n", CG_YES_NO_STR((cgs.osp.custom_client & OSP_CUSTOM_CLIENT_MAXFPS_FLAG) == 0));
	CG_Printf("    ^3Damage info:                 %s\n", CG_YES_NO_STR(cgs.osp.custom_client_2 & OSP_CUSTOM_CLIENT_2_ENABLE_DMG_INFO));
	CG_Printf("    ^3Pmove allowed:               %s\n", CG_YES_NO_STR(cgs.osp.allow_pmove));
	CG_Printf("    ^3Timenudge minimum:           ^3%s\n", cgs.osp.serverConfigMinimumTimenudge ? va("%i", cgs.osp.serverConfigMinimumTimenudge) : "-");
	CG_Printf("    ^3Timenudge maximum:           ^3%s\n", cgs.osp.serverConfigMaximumTimenudge ? va("%i", cgs.osp.serverConfigMaximumTimenudge) : "-");
	CG_Printf("    ^3Maxpackets minimum:          ^3%s\n", cgs.osp.serverConfigMinimumMaxpackets ? va("%i", cgs.osp.serverConfigMinimumMaxpackets) : "-");
	CG_Printf("    ^3Maxpackets maximum:          ^3%s\n", cgs.osp.serverConfigMaximumMaxpackets ? va("%i", cgs.osp.serverConfigMinimumMaxpackets) : "-");
}

void CG_OSPCredits_f(void)
{
	char string[1024];

	strcpy(&string[0],   "     ^BOSP2 Tourney DM/CA/CTF ");
	strcpy(&string[128], " ^3https://github.com/snems/OSP2  ");
	CG_OSPDrawLeftSlidingWindow(1.5f, 1.5f, 12.0f, 0, 2, 128, 20, 20, string, 180.0f, colorYellow, colorLtGrey);

	strcpy(&string[0],   "    ^BBased on source codes:  ");
	strcpy(&string[128], "    ^BOSP             https://www.orangesmoothie.org  ");
	strcpy(&string[256], "    ^BCyrax           https://github.com/ec-/baseq3a  ");
	strcpy(&string[384], "    ^Bx0ry            https://github.com/xq3e/engine  ");
	strcpy(&string[512], "    ^BNeil Toronto    http://ra.is/unlagged  ");
	strcpy(&string[640], "    ^BRatmod          https://github.com/rdntcntrl/ratoa_gamecode");
	CG_OSPDrawLeftSlidingWindow(0.7f, 0.9f, 12.0f, 0.4f, 6, 128, 10, 10, string, 260.0f, colorWhite, colorBlue);

	strcpy(&string[0],   "    ^BSpecial thanks to:  ");
	strcpy(&string[128], "    ^BQ3MSK.ru        http://q3msk.ru  ");
	strcpy(&string[256], "    ^BMrX, Paragon, Zenx, Diwoc");
	CG_OSPDrawLeftSlidingWindow(0.7f, 0.9f, 12.0f, 0.4f, 3, 128, 10, 10, string, 310.0f, colorWhite, colorRed);
}

void CG_OSPMoTD_f(void)
{
	if (cgs.osp.numberOfStringsMotd > 0)
	{
		CG_OSPDrawLeftSlidingWindow(0.5, 1.2, 12.0, 1.0, cgs.osp.numberOfStringsMotd, 64, 10, 10, cgs.osp.motd[0], 400, NULL, NULL);
	}
}
void CG_OSPMyName_f(void)
{
	CG_Printf("Your name is: '%s^7'\n", cgs.clientinfo[ cg.clientNum ].name);
}
void CG_OSPWStatsDown_f(void)
{
	if (wstatsWndId == 0)
	{
		if (wstatsEnabled == qfalse)
		{
			trap_SendClientCommand("getstatsinfo");
			wstatsEnabled = qtrue;
		}
	}
}
void CG_OSPWStatsUp_f(void)
{
	if (wstatsWndId > 0)
	{
		OSP_SlidingPrintContext_t* window = &ospPrintContext[wstatsWndId - 1];
		window->hideBeforeRealtime = trap_Milliseconds() + window->timeAppearance;
		window->hideBeforeCGTime = cg.time + window->timeAppearance;
		window->timeHiding = window->hideBeforeRealtime;
	}
	wstatsWndId = 0;
	wstatsEnabled = 0;
}

void CG_ShudChatDown_f(void)
{
	cgs.osp.shud.forceChat = qtrue;
}

void CG_ShudChatUp_f(void)
{
	cgs.osp.shud.forceChat = qfalse;
}

void CG_ShudKey1Down_f(void)
{
	cgs.osp.shud.key[0] = qtrue;
}

void CG_ShudKey1Up_f(void)
{
	cgs.osp.shud.key[0] = qfalse;
}

void CG_ShudKey2Down_f(void)
{
	cgs.osp.shud.key[1] = qtrue;
}

void CG_ShudKey2Up_f(void)
{
	cgs.osp.shud.key[1] = qfalse;
}

void CG_ShudKey3Down_f(void)
{
	cgs.osp.shud.key[2] = qtrue;
}

void CG_ShudKey3Up_f(void)
{
	cgs.osp.shud.key[2] = qfalse;
}

void CG_ShudKey4Down_f(void)
{
	cgs.osp.shud.key[3] = qtrue;
}

void CG_ShudKey4Up_f(void)
{
	cgs.osp.shud.key[3] = qfalse;
}

void CG_OSPPrintTime_f(void)
{
	qtime_t qtime;
	trap_RealTime(&qtime);
	CG_Printf("\nCurrent time: ^3%02d:%02d:%02d (%02d %s %d)\n\n", qtime.tm_hour, qtime.tm_min, qtime.tm_sec, qtime.tm_mday, monthName[qtime.tm_mon], qtime.tm_year + 1900);
}


void CG_OSPModifDown(int layout)
{
	if (!modifState[layout] && trap_Argc() >= 2 && atoi(CG_Argv(1)))
	{
		if (modifStrDown[layout][0])
		{
			trap_SendConsoleCommand(&modifStrDown[layout][0]);
		}
		else
		{
			CG_Printf("^3*** %s not set.\n", CG_Argv(0));
		}
		modifState[layout] = qtrue;
	}
	else if (trap_Argc() >= 2 && !atoi(CG_Argv(1)))
	{
		if (Q_stricmp(CG_Argv(1), "?") == 0)
		{
			if (modifStrDown[layout][0])
			{
				CG_Printf("^5%s is set to: ^7%s\n", CG_Argv(0), modifStrDown[layout][0]);
			}
			else
			{
				CG_Printf("^3*** %s not set.\n", CG_Argv(0));
			}
		}
		else
		{
			char msg[256];
			trap_Args(&modifStrDown[layout][0], MODIFSTR_SIZE);
			trap_Args(&msg[0], MODIFSTR_SIZE);
		}
	}
}

void CG_OSPModifUp(int layout)
{
	if (modifState[layout] && trap_Argc() >= 2 && atoi(CG_Argv(1)))
	{
		if (modifStrUp[layout][0])
		{
			trap_SendConsoleCommand(&modifStrUp[layout][0]);
		}
		else
		{
			CG_Printf("^3*** %s not set.\n", CG_Argv(0));
		}
		modifState[layout] = qfalse;
	}
	else if (trap_Argc() >= 2 && !atoi(CG_Argv(1)))
	{
		if (Q_stricmp(CG_Argv(1), "?") == 0)
		{
			if (modifStrUp[layout][0])
			{
				CG_Printf("^5%s is set to: ^7%s\n", CG_Argv(0), modifStrUp[layout][0]);
			}
			else
			{
				CG_Printf("^3*** %s not set.\n", CG_Argv(0));
			}
		}
		else
		{
			trap_Args(&modifStrUp[layout][0], MODIFSTR_SIZE);
		}
	}
}


void CG_OSPModif1Down_f(void)
{
	CG_OSPModifDown(0);
}

void CG_OSPModif1Up_f(void)
{
	CG_OSPModifUp(0);
}

void CG_OSPModif2Down_f(void)
{
	CG_OSPModifDown(1);
}

void CG_OSPModif2Up_f(void)
{
	CG_OSPModifUp(1);
}

void CG_OSPModif3Down_f(void)
{
	CG_OSPModifDown(2);
}

void CG_OSPModif3Up_f(void)
{
	CG_OSPModifUp(2);
}

void CG_OSPModif4Down_f(void)
{
	CG_OSPModifDown(3);
}

void CG_OSPModif4Up_f(void)
{
	CG_OSPModifUp(3);
}

void CG_OSPModif5Down_f(void)
{
	CG_OSPModifDown(4);
}

void CG_OSPModif5Up_f(void)
{
	CG_OSPModifUp(4);
}

void CG_OSPActionDown_f(void)
{
}

void CG_OSPActionUp_f(void)
{
}

void CG_OSPAddPos_f(void)
{
	if (trap_Argc() < 2)
	{
		CG_Printf("usage: %s <text>\n", CG_Argv(0));
	}
	else
	{
		char name[MAX_QPATH];
		trap_Args(name, MAX_QPATH);
		CG_CustomLocationsAddEntry(cg.refdef.vieworg, name);
	}
}

void CG_OSPDecalAdd_f(void)
{
}

void CG_OSPDecalDec_f(void)
{
}

void CG_OSPDecalDisable_f(void)
{
}

void CG_OSPDecalDump_f(void)
{
}

void CG_OSPDecalEdit_f(void)
{
}

void CG_OSPDecalEnable_f(void)
{
}

void CG_OSPDecalGFXNext_f(void)
{
}

void CG_OSPDecalGFXPrev_f(void)
{
}

void CG_OSPDecalInc_f(void)
{
}

void CG_OSPDcalNext_f(void)
{
}

void CG_OSPDcalPrev_f(void)
{
}

void CG_OSPDecalRotClock_f(void)
{
}

void CG_OSPDecalRotCounter_f(void)
{
}

void CG_OSPDynamicMem_f(void)
{
	CG_DynamicMemReport();
}

void CG_ReloadHud_f(void)
{
	CG_SHUDLoadConfig();
}

void CG_PrintPlayerIDs_f(void)
{
	int p;
	CG_Printf("^7| ^1%4s ^7| ^5%32s | ^5%s\n", "XID", "Player name", "Player name raw");
	CG_Printf("^7------------------------------------------------------------------------------\n");
	for (p = 0; p < MAX_CLIENTS; ++p)
	{
		if (cgs.clientinfo[p].infoValid)
		{
			CG_Printf("^7| ^1%4s ^7| %32s | %s\n", cgs.clientinfo[p].xidStr, cgs.clientinfo[p].name_clean, cgs.clientinfo[p].name_codes);
		}
	}
}


void CG_Mute_f(void)
{
	int p;

	if (trap_Argc() < 2)
	{
		CG_ChatfilterDump();
		CG_Printf("\nusage: %s <player ID>\n", CG_Argv(0));
		return;
	}

	p = atoi(CG_Argv(1));
	if (p < MAX_CLIENTS && cgs.clientinfo[p].infoValid)
	{
		CG_ChatfilterAddName(cgs.clientinfo[p].name_original);
		CG_ChatfilterSaveFile(CG_CHATFILTER_DEFAULT_FILE);
		CG_Printf("\n^1Player %s is muted\n", cgs.clientinfo[p].name_clean);
	}
	else
	{
		CG_Printf("\n^1Wrong player ID\n");
	}
}

void CG_UnMute_f(void)
{
	int p;

	if (trap_Argc() < 2)
	{
		CG_ChatfilterDump();
		CG_Printf("\nusage: %s <player ID>\n", CG_Argv(0));
		return;
	}

	p = atoi(CG_Argv(1));
	if (p < MAX_CLIENTS && cgs.clientinfo[p].infoValid)
	{
		CG_ChatfilterDeleteName(cgs.clientinfo[p].name_original);
		CG_ChatfilterSaveFile(CG_CHATFILTER_DEFAULT_FILE);
		CG_Printf("\n^2Player %s is unmuted\n", cgs.clientinfo[p].name_clean);
	}
	else
	{
		CG_Printf("\n^1Wrong player ID\n");
	}
}

void CG_ShowFont_f(void)
{
	if (trap_Argc() < 2)
	{
		cgs.osp.testFont[0] = 0;
	}
	else
	{
		Com_sprintf(cgs.osp.testFont, MAX_QPATH, "%s", CG_Argv(1));
	}
}

void CG_DPI_f(void)
{
	float local_yaw;
	float local_sens;
	float local_dpi;
	float local_360;
	char var[MAX_TOKEN_CHARS];

	if (trap_Argc() < 2 || trap_Argc() > 4)
	{
		CG_Printf("\n^1Usage:\n");
		CG_Printf("\n^1dpi ^2<dpi_value> ^3[sens] [m_yaw]\n");
		return;
	}

	local_dpi = atof(CG_Argv(1));

	if (trap_Argc() > 2)
	{
		local_sens = atof(CG_Argv(2));
	}
	else
	{
		trap_Cvar_VariableStringBuffer("sensitivity", var, sizeof(var));
		local_sens = atof(var);
	}

	if (trap_Argc() > 3)
	{
		local_yaw = atof(CG_Argv(3));
	}
	else
	{
		trap_Cvar_VariableStringBuffer("m_yaw", var, sizeof(var));
		local_yaw = atof(var);
	}

	local_360 = (360.0f / (local_yaw * local_sens * local_dpi)) * 2.54f;

	CG_Printf("\n^1DPI:          ^2%d\n", (int)local_dpi);
	CG_Printf("^1sensitivity:  ^2%.3f\n", local_sens);
	CG_Printf("^1m_yaw:        ^2%.3f\n", local_yaw);
	CG_Printf("^1cm/360:       ^2%.2f\n", local_360);
}

void CG_Stub_f(void) { }

typedef struct
{
	char*    cmd;
	void (*function)(void);
} consoleCommand_t;

static consoleCommand_t commands[] =
{
	{ "hudstyle", CG_OSPHUDStyle_f },
	{ "hudnext", CG_OSPHUDNext_f },
	{ "hudprev", CG_OSPHUDprev_f },
	{ "addstr", CG_OSPAddStr_f },
	{ "+vstr", CG_OSPVStrDown_f },
	{ "-vstr", CG_OSPVStrUp_f },
	{ "+fire", CG_OSPFireDown_f },
	{ "-fire", CG_OSPFireUp_f },
	{ "select", CG_OSPSelect_f },
	{ "clientversion", CG_OSPClientVersion_f },
	{ "clientconfig", CG_OSPClientConfig_f },
	{ "credits", CG_OSPCredits_f },
	{ "motd", CG_OSPMoTD_f },
	{ "myname", CG_OSPMyName_f },
	{ "serverversion", CG_OSPServerVersion_f },
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },
	{ "+wstats", CG_OSPWStatsDown_f },
	{ "-wstats", CG_OSPWStatsUp_f },
	{ "+zoom", CG_ZoomDown_f },
	{ "-zoom", CG_ZoomUp_f },
	{ "+zoomtoggle", CG_ZoomToggle_f},
	{ "-zoomtoggle", CG_Stub_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "crossnext", CG_NextCrosshair_f },
	{ "crossprev", CG_PrevCrosshair_f },
	{ "crossdecornext", CG_NextCrosshairDecor_f },
	{ "crossdecorprev", CG_PrevCrosshairDecor_f },
	{ "weapon", CG_Weapon_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_attacker", CG_TellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "startOrbit", CG_StartOrbit_f },
	{ "currenttime", CG_OSPPrintTime_f },
	{ "+modif1", CG_OSPModif1Down_f },
	{ "-modif1", CG_OSPModif1Up_f },
	{ "+modif2", CG_OSPModif2Down_f },
	{ "-modif2", CG_OSPModif2Up_f },
	{ "+modif3", CG_OSPModif3Down_f },
	{ "-modif3", CG_OSPModif3Up_f },
	{ "+modif4", CG_OSPModif4Down_f },
	{ "-modif4", CG_OSPModif4Up_f },
	{ "+modif5", CG_OSPModif5Down_f },
	{ "-modif5", CG_OSPModif5Up_f },
	{ "+action", CG_OSPActionDown_f },
	{ "-action", CG_OSPActionUp_f },
	{ "+shudchat", CG_ShudChatDown_f },
	{ "-shudchat", CG_ShudChatUp_f },
	{ "+shudkey1", CG_ShudKey1Down_f },
	{ "-shudkey1", CG_ShudKey1Up_f },
	{ "+shudkey2", CG_ShudKey2Down_f },
	{ "-shudkey2", CG_ShudKey2Up_f },
	{ "+shudkey3", CG_ShudKey3Down_f },
	{ "-shudkey3", CG_ShudKey3Up_f },
	{ "+shudkey4", CG_ShudKey4Down_f },
	{ "-shudkey4", CG_ShudKey4Up_f },
	{ "cg_dynamicmem", CG_OSPDynamicMem_f },
	{ "addpos", CG_OSPAddPos_f },
	{ "decaladd", CG_OSPDecalAdd_f },
	{ "decaldec", CG_OSPDecalDec_f },
	{ "decaldisable", CG_OSPDecalDisable_f },
	{ "decaldump", CG_OSPDecalDump_f },
	{ "decaledit", CG_OSPDecalEdit_f },
	{ "decalenable", CG_OSPDecalEnable_f },
	{ "decalgfxnext", CG_OSPDecalGFXNext_f },
	{ "decalgfxprev", CG_OSPDecalGFXPrev_f },
	{ "decalinc", CG_OSPDecalInc_f },
	{ "decalnext", CG_OSPDcalNext_f },
	{ "decalprev", CG_OSPDcalPrev_f },
	{ "decalrotclock", CG_OSPDecalRotClock_f },
	{ "decalrotcounter", CG_OSPDecalRotCounter_f },
	{ "reloadHUD", CG_ReloadHud_f },
	{ "playersid", CG_PrintPlayerIDs_f },
	{ "mute", CG_Mute_f },
	{ "unmute", CG_UnMute_f },
	{ "showfont", CG_ShowFont_f },
	{ "dpi", CG_DPI_f },
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand(void)
{
	const char*  cmd;
	int     i;

	cmd = CG_Argv(0);

	for (i = 0 ; i < sizeof(commands) / sizeof(commands[0]) ; i++)
	{
		if (!Q_stricmp(cmd, commands[i].cmd))
		{
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands(void)
{
	int     i;

	for (i = 0 ; i < sizeof(commands) / sizeof(commands[0]) ; i++)
	{
		trap_AddCommand(commands[i].cmd);
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand("allready");
	trap_AddCommand("addbot");
	trap_AddCommand("callvote");
	trap_AddCommand("follow");
	trap_AddCommand("give");
	trap_AddCommand("god");
	trap_AddCommand("levelshot");
	trap_AddCommand("loaddeferred");
	trap_AddCommand("noclip");
	trap_AddCommand("notarget");
	trap_AddCommand("say");
	trap_AddCommand("say_team");
	trap_AddCommand("setviewpos");
	trap_AddCommand("team");
	trap_AddCommand("teamtask");
	trap_AddCommand("teamvote");
	trap_AddCommand("tell");
	trap_AddCommand("vote");
	trap_AddCommand("?");
	trap_AddCommand("acc");
	trap_AddCommand("accuracy");
	trap_AddCommand("autorecord");
	trap_AddCommand("autoscreenshot");
	trap_AddCommand("admin");
	trap_AddCommand("bottomshots");
	trap_AddCommand("captains");
	trap_AddCommand("clientversion");
	trap_AddCommand("coachdecline");
	trap_AddCommand("coachinvite");
	trap_AddCommand("coachkick");
	trap_AddCommand("coach");
	trap_AddCommand("commands");
	trap_AddCommand("cv");
	trap_AddCommand("drop");
	trap_AddCommand("follownext");
	trap_AddCommand("followprev");
	trap_AddCommand("followpowerup");
	trap_AddCommand("help");
	trap_AddCommand("hudnext");
	trap_AddCommand("hudprev");
	trap_AddCommand("hudstyle");
	trap_AddCommand("invite");
	trap_AddCommand("joincode");
	trap_AddCommand("kickplayer");
	trap_AddCommand("killstats");
	trap_AddCommand("kill");
	trap_AddCommand("lock");
	trap_AddCommand("locations");
	trap_AddCommand("maplist");
	trap_AddCommand("mapload");
	trap_AddCommand("myname");
	trap_AddCommand("notready");
	trap_AddCommand("nowin");
	trap_AddCommand("osp");
	trap_AddCommand("pause");
	trap_AddCommand("pickplayer");
	trap_AddCommand("playerlist");
	trap_AddCommand("playernames");
	trap_AddCommand("players");
	trap_AddCommand("r_allcaptain");
	trap_AddCommand("r_allready");
	trap_AddCommand("r_help");
	trap_AddCommand("r_kick");
	trap_AddCommand("r_lockred");
	trap_AddCommand("r_lockblue");
	trap_AddCommand("r_map");
	trap_AddCommand("r_mpause");
	trap_AddCommand("r_mutespecs");
	trap_AddCommand("r_pause");
	trap_AddCommand("r_promote");
	trap_AddCommand("r_putred");
	trap_AddCommand("r_putblue");
	trap_AddCommand("r_remove");
	trap_AddCommand("r_restart");
	trap_AddCommand("range");
	trap_AddCommand("readyteam");
	trap_AddCommand("ready");
	trap_AddCommand("ref");
	trap_AddCommand("referee");
	trap_AddCommand("remove");
	trap_AddCommand("resign");
	trap_AddCommand("scores");
	trap_AddCommand("settings");
	trap_AddCommand("snapshot");
	trap_AddCommand("snapshotdump");
	trap_AddCommand("snaps");
	trap_AddCommand("specinvite");
	trap_AddCommand("speclock");
	trap_AddCommand("speconly");
	trap_AddCommand("specunlock");
	trap_AddCommand("statsdump");
	trap_AddCommand("statsred");
	trap_AddCommand("statsblue");
	trap_AddCommand("statsall");
	trap_AddCommand("stats");
	trap_AddCommand("teamready");
	trap_AddCommand("time");
	trap_AddCommand("timein");
	trap_AddCommand("timeout");
	trap_AddCommand("topshots");
	trap_AddCommand("unlock");
	trap_AddCommand("unready");
	trap_AddCommand("vc_follow");
	trap_AddCommand("vc_free");
	trap_AddCommand("vc_info");
	trap_AddCommand("vc_proximity");
	trap_AddCommand("viewadd");
	trap_AddCommand("viewall");
	trap_AddCommand("viewblue");
	trap_AddCommand("viewcam");
	trap_AddCommand("viewcyclenext");
	trap_AddCommand("viewcycleprev");
	trap_AddCommand("viewfollow");
	trap_AddCommand("viewlist");
	trap_AddCommand("viewnext");
	trap_AddCommand("viewnone");
	trap_AddCommand("viewprev");
	trap_AddCommand("viewred");
	trap_AddCommand("viewremove");
	trap_AddCommand("decaladd");
	trap_AddCommand("decaldec");
	trap_AddCommand("decaldisable");
	trap_AddCommand("decaldump");
	trap_AddCommand("decaledit");
	trap_AddCommand("decalenable");
	trap_AddCommand("decalgfxnext");
	trap_AddCommand("decalgfxprev");
	trap_AddCommand("decalinc");
	trap_AddCommand("decalnext");
	trap_AddCommand("decalprev");
	trap_AddCommand("decalrotclock");
	trap_AddCommand("decalrotcounter");
	trap_AddCommand("testfont");
}
