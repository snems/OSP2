#include "cg_local.h"
#include "../game/bg_local.h"


int modePromodePhysKoeff; //1870ec
float modePromode_pm_airaccelerate_1; //3450
int modePredictionKoeff2;   //3454
float modePromode_pm_airaccelerate_2; //3458
float modeWishspeedLimit; //345c
int modePredictionKoeff1;   //3460
float modeWaterFriction;      //3464
float modeSwimScale1;         //3468
float modeSwimScale2;         //346c
float modeWaterAccelerate;    //3470
float modeShotgunKoeff;    //3494
int modeShotgunNumberOfPellets;    //3498
float modeUnused8;    //349c
int modeMaxAmmoShotgun;    //34a0
int modeGrenadeTime;    //34a4
int modeMaxAmmoGrenade;    //34a8
int modeMaxAmmoRocket;     //34b0
int modeMaxAmmoRail;       //34c0
int modeBeginWeaponChangeTime;    //34d0
int modeFinishWeaponChangeTime;    //34d4
int modePMNoAmmoTime;    //34d8
int modeUnknown1;    //34e4
qboolean modeHitLevelSounds;    //3514
int modePickupDistance;    //20e8
//
int modeUnknown2;    //3528
int modeUnknown3;    //352c
int modeUnknown4;    //3530
int modeShotgunPromode;    //3514


extern qboolean localClient;


qboolean CG_OSPIsGameTypeCA(int gametype)
{
	return gametype >= GT_CA ? qtrue : qfalse;
}

qboolean CG_OSPIsGameTypeFreeze()
{
	return cgs.gametype == GT_TEAM && cgs.osp.gameTypeFreeze;
}



qboolean CG_OSPIsStatsHidden(qboolean check_gametype, qboolean check_warmup)
{

	if (check_gametype)
	{
		if (cgs.gametype == GT_TOURNAMENT)
		{
			return CG_OSPIsStatsHidden(qfalse, check_warmup);
		}
		return qfalse;
	}

	if (check_warmup && cg.warmup)
	{
		return qfalse;
	}
	if (cg.intermissionStarted)
	{
		return qfalse;
	}
	if (cg.demoPlayback)
	{
		return qfalse;
	}
	if (cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR)
	{
		return qtrue;
	}
	return qfalse;
}



void CG_OSPSetMode(int value)
{
#define OSP_SET_MODE_VARIANT(VAL, VAL_VQ3, VAL_CQ3, VAL_PROMODE) (VAL&OSP_SERVER_MODE_PROMODE ? VAL_PROMODE : (VAL&OSP_SERVER_MODE_CQ3 ? VAL_CQ3 : VAL_VQ3))

	modePromodePhysKoeff               = OSP_SET_MODE_VARIANT(value, 0, 0, 100);
	pm_friction                        = OSP_SET_MODE_VARIANT(value, 6, 6, 6);
	modePredictionKoeff1               = OSP_SET_MODE_VARIANT(value, 0, 0, 1);
	modePromode_pm_airaccelerate_1     = OSP_SET_MODE_VARIANT(value, 1.0f, 1.0f, 2.5f);
	modePredictionKoeff2               = OSP_SET_MODE_VARIANT(value, 0, 0, 150);
	modePromode_pm_airaccelerate_2     = OSP_SET_MODE_VARIANT(value, 1, 1, 70);
	modeWishspeedLimit                 = OSP_SET_MODE_VARIANT(value, 400, 400, 30);
	pm_accelerate                      = OSP_SET_MODE_VARIANT(value, 10, 10, 16);
	modeSwimScale1                     = OSP_SET_MODE_VARIANT(value, 0.5f, 0.54f, 0.54f);
	modeSwimScale2                     = OSP_SET_MODE_VARIANT(value, 0.7f, 0.585f, 0.585f);
	modeWaterFriction                  = OSP_SET_MODE_VARIANT(value, 1.0f, 0.5f, 0.5f);
	modeWaterAccelerate                = OSP_SET_MODE_VARIANT(value, 4.0f, 5.0f, 5.0f);
	modeShotgunKoeff                   = OSP_SET_MODE_VARIANT(value, 700, 900, 900);
	modeShotgunNumberOfPellets         = OSP_SET_MODE_VARIANT(value, 11, 16, 16);
	modeMaxAmmoShotgun                 = OSP_SET_MODE_VARIANT(value, 200, 100, 100);
	modeGrenadeTime                    = OSP_SET_MODE_VARIANT(value, 800, 800, 700);
	modeMaxAmmoGrenade                 = OSP_SET_MODE_VARIANT(value, 200, 100, 100);
	modeMaxAmmoRocket                  = OSP_SET_MODE_VARIANT(value, 200, 100, 100);
	modeMaxAmmoRail                    = OSP_SET_MODE_VARIANT(value, 200, 100, 100);
	modeBeginWeaponChangeTime          = OSP_SET_MODE_VARIANT(value, 200, 200, 0);
	modeFinishWeaponChangeTime         = OSP_SET_MODE_VARIANT(value, 250, 250, 0);
	modePMNoAmmoTime                   = OSP_SET_MODE_VARIANT(value, 500, 500, 100);
	modeUnknown1                       = OSP_SET_MODE_VARIANT(value, 0, 1, 1);
	modeHitLevelSounds                 = OSP_SET_MODE_VARIANT(value, qfalse, qtrue, qtrue);
	modePickupDistance                 = OSP_SET_MODE_VARIANT(value, 36, 66, 66);
	modeUnknown2                       = value & OSP_SERVER_MODE_PROMODE_OPT1 ? 1 : 0;
	modeUnknown3                       = value & OSP_SERVER_MODE_PROMODE_OPT2 ? 200 : 0;
	modeUnknown4                       = value & OSP_SERVER_MODE_PROMODE_OPT2 ? 100 : 0;
	modeShotgunPromode                 = OSP_SET_MODE_VARIANT(value, 0, 2, 1);
}




void CG_OSPUpdateUserInfo(qboolean arg)
{
	int flags;
	char* cmd;
	int using_jpeg;
	if (cg.demoPlayback) return;
	cgs.osp.nextCvarsUpdateTime = 0;
	flags = 0;
	cmd = arg ? "uvinfo" : "uinfo";

	if (cg_followpowerup.integer != 0)
	{
		flags |=  1;
	}
	if (cg_followkiller.integer != 0)
	{
		flags |= 2;
	}
	if (cg_followviewcam.integer != 0)
	{
		flags |= 4;
	}
	using_jpeg = cg_useScreenShotJPEG.integer ? qtrue : qfalse;
	trap_SendClientCommand(va("%s %d %d %d %d %d %d %d\n", cmd, cl_maxpackets.integer, snaps.integer, cl_timenudge.integer, customLocationsEnabled, flags, cg_autoAction.integer, using_jpeg));
}


void CG_OSPCvarsRestrictValues(void)
{
	qboolean changed = qfalse;
	if (cg_trueLightning.integer > 1)
	{
		trap_Cvar_Set("cg_trueLightning", "0.5");
	}

	if (cgs.localServer)
	{
		return;
	}

	if (r_fullbright.integer)
	{
		trap_Cvar_Set("r_fullbright", "0");
		changed = qtrue;
	}

	if (r_lodCurveError.integer != 250)
	{
		trap_Cvar_Set("r_lodcurveerror", "250");
		changed = qtrue;
	}

	if (r_subdivisions.integer > 80)
	{
		trap_Cvar_Set("r_subdivisions", "80");
		changed = qtrue;
	}

	if (r_znear.integer != 4)
	{
		trap_Cvar_Set("r_znear", "4");
		changed = qtrue;
	}


	if ((cgs.osp.custom_client & OSP_CUSTOM_CLIENT_MAXFPS_FLAG) == 0)
	{
		if (com_maxfps.integer > 250 || com_maxfps.integer <= 0)
		{
			trap_Cvar_Set("com_maxfps", "125");
		}
	}

	if (r_shownormals.integer != 0)
	{
		trap_Cvar_Set("r_shownormals", "0");
		changed = qtrue;
	}

	if (r_showtris.integer != 0)
	{
		trap_Cvar_Set("r_showtris", "0");
		changed = qtrue;
	}

	if (cg_shadows.integer > 1)
	{
		trap_Cvar_Set("cg_shadows", "1");
		changed = qtrue;
	}

	if (cg_shadows.integer < 0)
	{
		trap_Cvar_Set("cg_shadows", "0");
		changed = qtrue;
	}

	if (changed)
	{
		trap_SendConsoleCommand("vid_restart;\n");
	}
}

/********************************************************************
 * Config string changes
 ********************************************************************/
/*
 * Restrict pmove_fixed
 */
void CG_OSPConfigPmoveSet(int value)
{
	cgs.osp.allow_pmove = value;
	CG_CvarTouch("pmove_fixed");
}

/*
 * Restrict cl_maxpackets
 */
void CG_OSPConfigMinMaxPacketsSet(int value)
{
	cgs.osp.serverConfigMinimumMaxpackets = value;
	CG_CvarTouch("cl_maxpackets");
}
void CG_OSPConfigMaxMaxPacketsSet(int value)
{
	cgs.osp.serverConfigMaximumMaxpackets = value;
	CG_CvarTouch("cl_maxpackets");
}

/*
 * Restrict cl_timenudge
 */
void CG_OSPConfigMinTimenudgeSet(int value)
{
	cgs.osp.serverConfigMinimumTimenudge = value;
	CG_CvarTouch("cl_timenudge");
}

void CG_OSPConfigMaxTimenudgeSet(int value)
{
	cgs.osp.serverConfigMaximumTimenudge = value;
	CG_CvarTouch("cl_timenudge");
}

/*
 * Unknown cs 0x368
 */
void CG_OSPConfig0x368Set(qboolean value)
{
	cgs.osp.configString_0x368 = value;
}

/*
 * Clan base TDM
 */
void CG_OSPConfigClanBaseTDMSet(qboolean value)
{
	cgs.osp.clanBaseTeamDM = value;
}

/*
 * Custom client
 */
void CG_OSPConfigCustomClientSet(int value)
{
	cgs.osp.custom_client = value;
	CG_CvarTouch("cg_altPlasma");
	CG_CvarTouch("cg_altGrenades");
	CG_CvarTouch("cg_altLightning");
	CG_CvarTouch("cg_enableOSPHUD");
	CG_CvarTouch("cg_shud");
}

/*
 * Custom client 2
 */
void CG_OSPConfigCustomClient2Set(int value)
{
	cgs.osp.custom_client_2 = value;
	CG_CvarTouch("cg_hitSounds");
}

/*
 * Server mode
 */
void CG_OSPConfigModeSet(int value)
{
	if (value & 1)
	{
		cgs.osp.server_mode = OSP_SERVER_MODE_PROMODE;
	}
	else if (value & 2)
	{
		cgs.osp.server_mode = OSP_SERVER_MODE_CQ3;
	}
	else
	{
		cgs.osp.server_mode = OSP_SERVER_MODE_VQ3;
	}
	CG_OSPSetMode(cgs.osp.server_mode);
}

/*
 * Freeze mode
 */
void CG_OSPConfigFreezeModeSet(int value)
{
	if (value == 0)
	{
		cgs.osp.gameTypeFreeze = 0;
	}
	else if (value == 2)
	{
		cgs.osp.gameTypeFreeze = 2;
	}
	else
	{
		cgs.osp.gameTypeFreeze = 1;
	}
}

qboolean CG_IsSpectator(void)
{
	return (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR);
}

qboolean CG_IsFollowing(void)
{
	if (!cg.demoPlayback)
	{
		if (!(cg.snap->ps.pm_flags & PMF_FOLLOW))
		{
			return qfalse;
		}
		else if (cg.snap->ps.pm_type == PM_FREEZE || cg.intermissionStarted || cg.snap->ps.clientNum == cg.clientNum)
		{
			return qfalse;
		}
	}
	else if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR)
	{
		return qfalse;
	}
	return qtrue;
}
