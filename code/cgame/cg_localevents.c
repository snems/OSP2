#include "cg_local.h"

int global_handicap; // unused?
//	void         (*onChanged)(cvarTable_t *cvart);

void CG_LocalEventCvarChanged_cg_drawTeamOverlay(cvarTable_t* cvart)
{
	if ((cg_drawTeamOverlay.integer > 0) ||
	        ((!(cgs.osp.server_mode & OSP_SERVER_MODE_PROMODE)) &&
	         (ch_TeamCrosshairHealth.integer > 0 || ch_crosshairTeamInfo.integer > 0)))
	{
		trap_Cvar_Set("teamoverlay", "1");
	}
	else
	{
		trap_Cvar_Set("teamoverlay", "0");
	}
}

void CG_LocalEventCvarChanged_cl_maxpackets(cvarTable_t* cvart)
{
	if (cgs.osp.serverConfigMinimumMaxpackets)
	{
		if (cl_maxpackets.integer < cgs.osp.serverConfigMinimumMaxpackets)
		{
			CG_Printf("^3Server-imposed minimum maxpackets set to %d.  Adjusting...\n", cgs.osp.serverConfigMinimumMaxpackets);
			trap_Cvar_Set("cl_maxpackets", va("%d", cgs.osp.serverConfigMinimumMaxpackets));
		}
	}

	if (cgs.osp.serverConfigMaximumMaxpackets)
	{
		if (cl_maxpackets.integer > cgs.osp.serverConfigMaximumMaxpackets)
		{
			CG_Printf("^3Server-imposed maximum maxpackets set to %d.  Adjusting...\n", cgs.osp.serverConfigMaximumMaxpackets);
			trap_Cvar_Set("cl_maxpackets", va("%d", cgs.osp.serverConfigMaximumMaxpackets));
		}
	}

	if (cgs.osp.nextCvarsUpdateTime == 0)
	{
		cgs.osp.nextCvarsUpdateTime = (cg.time + 1200);
	}
}

void CG_LocalEventCvarChanged_cl_timenudge(cvarTable_t* cvart)
{
	if (cgs.osp.serverConfigMinimumTimenudge)
	{
		if (cgs.osp.serverConfigMinimumTimenudge > cl_timenudge.integer)
		{
			CG_Printf("^3Server-imposed minimum timenudge set to %d.  Adjusting...\n", cgs.osp.serverConfigMinimumTimenudge);
			trap_Cvar_Set("cl_timenudge", va("%d", cgs.osp.serverConfigMinimumTimenudge));
		}
		if (cgs.osp.serverConfigMaximumTimenudge)
		{
			if (cgs.osp.serverConfigMaximumTimenudge < cl_timenudge.integer)
			{
				CG_Printf("^3Server-imposed maximum timenudge set to %d.  Adjusting...\n", cgs.osp.serverConfigMaximumTimenudge);
				trap_Cvar_Set("cl_timenudge", va("%d", cgs.osp.serverConfigMaximumTimenudge));
			}
		}
		if (cgs.osp.nextCvarsUpdateTime == 0)
		{
			cgs.osp.nextCvarsUpdateTime = (cg.time + 1200);
		}
	}
}

void CG_LocalEventCvarChanged_com_maxfps(cvarTable_t* cvart)
{
	if (cgs.osp.serverConfigMinimumMaxpackets > 0 || com_maxfps.integer > 0)
	{
		if (cgs.osp.serverConfigMinimumMaxpackets > com_maxfps.integer)
		{
			CG_Printf("^3Server-imposed minimum maxfps set to %d.  Adjusting...\n", cgs.osp.serverConfigMinimumMaxpackets);
			trap_Cvar_Set("com_maxfps", va("%d", cgs.osp.serverConfigMinimumMaxpackets));
		}
	}
	CG_OSPCvarsRestrictValues();
}

void CG_LocalEventCvarChanged_ch_recordMessage(cvarTable_t* cvart)
{
	if (cg.snap)
	{
		trap_Cvar_Set("ui_recordSPDemo", ch_recordMessage.integer <= 0 ? "0" : "1");
	}
}

void CG_LocalEventCvarChanged_cg_followpowerup(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}
void CG_LocalEventCvarChanged_cg_followkiller(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}
void CG_LocalEventCvarChanged_cg_followviewcam(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}
void CG_LocalEventCvarChanged_cg_autoAction(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}
void CG_LocalEventCvarChanged_snaps(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}
void CG_LocalEventCvarChanged_cg_useScreenShotJPEG(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}

void CG_LocalEventCvarChanged_r_lodCurveError(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_r_subdivisions(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_r_znear(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_cg_trueLightning(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_r_shownormals(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_r_showtris(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_cg_shadows(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_r_fullbright(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}

void CG_LocalEventCvarChanged_cg_swapSkins(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_enemyModel(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_teamModel(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}


void CG_LocalEventCvarChanged_handicap(cvarTable_t* cvart)
{
	if (!cg.warmup)
	{
		if (cgs.gametype != GT_FFA && cgs.gametype != GT_SINGLE_PLAYER && handicap.integer < 5)
		{
			global_handicap = 100;
			trap_Cvar_Set("handicap", va("%d", global_handicap));
		}
	}
	global_handicap = handicap.integer;
}

void CG_LocalEventCvarChanged_s_ambient(cvarTable_t* cvart)
{
	if (s_ambient.modificationCount <= -1)
	{
		trap_SendConsoleCommand("vid_restart;\n");
	}
}

void CG_LocalEventCvarChanged_pmove_fixed(cvarTable_t* cvart)
{
	if (cgs.osp.allow_pmove == 0 && pmove_fixed.integer)
	{
		CG_Printf("^3Enhanced client sampling has been disabled on this server.\n");
		trap_Cvar_Set("pmove_fixed", "0");
	}
}

void CG_LocalEventCvarChanged_cg_altplasma(cvarTable_t* cvart)
{
	if (!(cgs.osp.custom_client & OSP_CUSTOM_CLIENT_ALT_WEAPON_FLAG) && cg_altPlasma.integer)
	{
		CG_Printf("^3Alternate PG graphics have been disabled on this server.\n");
		trap_Cvar_Set("cg_altPlasma", "0");
	}
}

void CG_LocalEventCvarChanged_cg_altlightning(cvarTable_t* cvart)
{
	if (!(cgs.osp.custom_client & OSP_CUSTOM_CLIENT_ALT_WEAPON_FLAG) && cg_altLightning.integer)
	{
		CG_Printf("^3Alternate LG graphics have been disabled on this server.\n");
		trap_Cvar_Set("cg_altLightning", "0");
	}
}

void CG_LocalEventCvarChanged_cg_altgrenades(cvarTable_t* cvart)
{
	if (!(cgs.osp.custom_client & OSP_CUSTOM_CLIENT_ALT_WEAPON_FLAG) && cg_altGrenades.integer)
	{
		CG_Printf("^3Alternate GL graphics have been disabled on this server.\n");
		trap_Cvar_Set("cg_altGrenades", "0");
	}
}

void CG_LocalEventCvarChanged_cg_enableOSPHUD(cvarTable_t* cvart)
{
	if (!(cgs.osp.custom_client & OSP_CUSTOM_CLIENT_OSP_HUD_FLAG) && cg_enableOSPHUD.integer)
	{
		CG_Printf("^3The OSP-based HUD has been disabled on this server.\n");
		trap_Cvar_Set("cg_enableOSPHUD", "0");
	}
}

void CG_LocalEventCvarChanged_cg_drawHitBox(cvarTable_t* cvart)
{
	if ((cgs.osp.custom_client_2 & OSP_CUSTOM_CLIENT_2_DISABLE_HIT_BOX_FLAG) && cg_drawHitBox.integer)
	{
		CG_Printf("^3Hitbox has been disabled on this server.\n");
		trap_Cvar_Set("cg_drawHitBox", "0");
	}
}

void CG_LocalEventCvarChanged_cg_playerModelColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_playerRailColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_playerFrozenColor(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_teamModelColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_teamRailColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_teamFrozenColor(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_enemyModelColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_enemyRailColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_enemyFrozenColor(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_fragSound(cvarTable_t* cvart)
{
	/* touch fragsound to reload it */
	(void)CG_GetFragSound();
}


