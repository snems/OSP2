#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

enum shudElementNameType_t
{
	SHUDENAME_TYPE_OWN,
	SHUDENAME_TYPE_NME,
};

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
	enum shudElementNameType_t type;
} shudElementName_t;

static void* CG_SHUDElementNameCreate(const superhudConfig_t* config, enum shudElementNameType_t type)
{
	shudElementName_t* element;

	SHUD_ELEMENT_INIT(element, config);

	element->type = type;

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.maxchars = MAX_QPATH;

	return element;
}

void* CG_SHUDElementNameOWNCreate(const superhudConfig_t* config)
{
	return CG_SHUDElementNameCreate(config, SHUDENAME_TYPE_OWN);
}

void* CG_SHUDElementNameNMECreate(const superhudConfig_t* config)
{
	return CG_SHUDElementNameCreate(config, SHUDENAME_TYPE_NME);
}

static void CG_SHUDElementNameGetPairFFA(const char** own, const char** nme)
{
	int clientNum;
	const char* info;
	*own = cgs.clientinfo[cg.snap->ps.clientNum].name;

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if (clientNum < 0 || clientNum >= MAX_CLIENTS)
	{
		return;
	}

	info = CG_ConfigString(CS_PLAYERS + clientNum);
	*nme = Info_ValueForKey(info, "n");
}

static void CG_SHUDElementNameGetPair1vs1(const char** own, const char** nme)
{
	int i;
	int k;

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		for (i = 0, k = 0; i < MAX_CLIENTS; ++i)
		{
			if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team != TEAM_SPECTATOR)
			{
				if (k == 0)
				{
					++k;
					*own = cgs.clientinfo[i].name;
				}
				else
				{
					*nme = cgs.clientinfo[i].name;
				}
			}
		}
	}
	else
	{
		*own = cgs.clientinfo[cg.snap->ps.clientNum].name;

		for (i = 0; i < MAX_CLIENTS; ++i)
		{
			if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team != TEAM_SPECTATOR)
			{
				*nme = cgs.clientinfo[i].name;
			}
		}
	}
}

static void CG_SHUDElementNameGetPairTeam(const char** own, const char** nme)
{
	team_t our_team;
	if (cgs.osp.gameTypeFreeze)
	{
		our_team = cgs.clientinfo[cg.clientNum].rt;
	}
	else
	{
		our_team = cgs.clientinfo[cg.clientNum].team;
	}

	if (our_team == TEAM_SPECTATOR)
	{
		if (cgs.osp.gameTypeFreeze)
		{
			our_team = cgs.clientinfo[cg.snap->ps.clientNum].rt;
		}
		else
		{
			our_team = cgs.clientinfo[cg.snap->ps.clientNum].team;
		}
	}

	switch (our_team)
	{
		case TEAM_SPECTATOR:
		case TEAM_RED:
			*own = "Red";
			*nme = "Blue";
			break;
		case TEAM_BLUE:
			*own = "Blue";
			*nme = "Red";
			break;
		case TEAM_FREE:
		case TEAM_4:
		case TEAM_5:
		case TEAM_6:
		case TEAM_7:
		case TEAM_NUM_TEAMS:
			break;
	}
}

static void CG_SHUDElementNameGetPair(const char** own, const char** nme)
{
	*own = "";
	*nme = "";
	switch (cgs.gametype)
	{
		case GT_SINGLE_PLAYER:
		case GT_FFA: //our name / last attacker or second
			CG_SHUDElementNameGetPairFFA(own, nme);
			break;
		case GT_TOURNAMENT: //our name and second player name
			CG_SHUDElementNameGetPair1vs1(own, nme);
			break;
		case GT_TEAM: // red/blue
		case GT_CTF:
		case GT_CA:
			CG_SHUDElementNameGetPairTeam(own, nme);
			break;
		case GT_MAX_GAME_TYPE:
			break;
	}
}

void CG_SHUDElementNameRoutine(void* context)
{
	shudElementName_t* element = (shudElementName_t*)context;
	const char*  own;
	const char*  nme;

	CG_SHUDElementNameGetPair(&own, &nme);

	CG_SHUDFill(&element->config);

	if (element->type == SHUDENAME_TYPE_OWN)
	{
		element->ctx.text = own;
	}
	else
	{
		element->ctx.text = nme;
	}
	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementNameDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
