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
// cg_players.c -- handle the media and animation for player entities
#include "cg_local.h"
#include "../qcommon/l_crc.h"

char*    cg_customSoundNames[MAX_CUSTOM_SOUNDS] =
{
	"*death1.wav",
	"*death2.wav",
	"*death3.wav",
	"*jump1.wav",
	"*pain25_1.wav",
	"*pain50_1.wav",
	"*pain75_1.wav",
	"*pain100_1.wav",
	"*falling1.wav",
	"*gasp.wav",
	"*drown.wav",
	"*fall1.wav",
	"*taunt.wav"
};

void CG_UpdateOurClientInfo(void);
static void CG_UpdateOtherClientsInfo(void);
void CG_UpdateAllClientsInfo(void);
static void CG_ForceNewClientInfo(clientInfo_t* old, clientInfo_t* new);

//unknown variable
int global_viewlistFirstOption = 0;
/*
================
CG_CustomSound

================
*/
sfxHandle_t CG_CustomSound(int clientNum, const char* soundName)
{
	clientInfo_t* ci;
	int         i;

	if (soundName[0] != '*')
	{
		return trap_S_RegisterSound(soundName, qfalse);
	}

	if (clientNum < 0 || clientNum >= MAX_CLIENTS)
	{
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	for (i = 0 ; i < MAX_CUSTOM_SOUNDS && cg_customSoundNames[i] ; i++)
	{
		if (!strcmp(soundName, cg_customSoundNames[i]))
		{
			return ci->sounds[i];
		}
	}

	CG_Error("Unknown custom sound: %s", soundName);
	return 0;
}



/*
=============================================================================

CLIENT INFO

=============================================================================
*/

/*
======================
CG_ParseAnimationFile

Read a configuration file containing animation coutns and rates
models/players/visor/animation.cfg, etc
======================
*/
static qboolean CG_ParseAnimationFile(const char* filename, clientInfo_t* ci)
{
	char*        text_p, *prev;
	int         len;
	int         i;
	char*        token;
	float       fps;
	int         skip;
	char        text[20000];
	fileHandle_t    f;
	animation_t* animations;

	animations = ci->animations;

	// load the file
	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if (len <= 0)
	{
		return qfalse;
	}
	if (len >= sizeof(text) - 1)
	{
		CG_Printf("File %s too long\n", filename);
		return qfalse;
	}
	trap_FS_Read(text, len, f);
	text[len] = 0;
	trap_FS_FCloseFile(f);

	// parse the text
	text_p = text;
	skip = 0;   // quite the compiler warning

	ci->footsteps = FOOTSTEP_NORMAL;
	VectorClear(ci->headOffset);
	ci->gender = GENDER_MALE;
	ci->fixedlegs = qfalse;
	ci->fixedtorso = qfalse;

	// read optional parameters
	while (1)
	{
		prev = text_p;  // so we can unget
		token = COM_Parse(&text_p);
		if (!token)
		{
			break;
		}
		if (!Q_stricmp(token, "footsteps"))
		{
			token = COM_Parse(&text_p);
			if (!token)
			{
				break;
			}
			if (!Q_stricmp(token, "default") || !Q_stricmp(token, "normal"))
			{
				ci->footsteps = FOOTSTEP_NORMAL;
			}
			else if (!Q_stricmp(token, "boot"))
			{
				ci->footsteps = FOOTSTEP_BOOT;
			}
			else if (!Q_stricmp(token, "flesh"))
			{
				ci->footsteps = FOOTSTEP_FLESH;
			}
			else if (!Q_stricmp(token, "mech"))
			{
				ci->footsteps = FOOTSTEP_MECH;
			}
			else if (!Q_stricmp(token, "energy"))
			{
				ci->footsteps = FOOTSTEP_ENERGY;
			}
			else
			{
				CG_Printf("Bad footsteps parm in %s: %s\n", filename, token);
			}
			continue;
		}
		else if (!Q_stricmp(token, "headoffset"))
		{
			for (i = 0 ; i < 3 ; i++)
			{
				token = COM_Parse(&text_p);
				if (!token)
				{
					break;
				}
				ci->headOffset[i] = atof(token);
			}
			continue;
		}
		else if (!Q_stricmp(token, "sex"))
		{
			token = COM_Parse(&text_p);
			if (!token)
			{
				break;
			}
			if (token[0] == 'f' || token[0] == 'F')
			{
				ci->gender = GENDER_FEMALE;
			}
			else if (token[0] == 'n' || token[0] == 'N')
			{
				ci->gender = GENDER_NEUTER;
			}
			else
			{
				ci->gender = GENDER_MALE;
			}
			continue;
		}
		else if (!Q_stricmp(token, "fixedlegs"))
		{
			ci->fixedlegs = qtrue;
			continue;
		}
		else if (!Q_stricmp(token, "fixedtorso"))
		{
			ci->fixedtorso = qtrue;
			continue;
		}

		// if it is a number, start parsing animations
		if (token[0] >= '0' && token[0] <= '9')
		{
			text_p = prev;  // unget the token
			break;
		}
		Com_Printf("unknown token '%s' is %s\n", token, filename);
	}

	// read information for each frame
	for (i = 0 ; i < MAX_ANIMATIONS ; i++)
	{

		token = COM_Parse(&text_p);
		if (!*token)
		{
			if (i >= TORSO_GETFLAG)
			{
				animations[i].firstFrame = animations[TORSO_GESTURE].firstFrame;
				animations[i].frameLerp = animations[TORSO_GESTURE].frameLerp;
				animations[i].initialLerp = animations[TORSO_GESTURE].initialLerp;
				animations[i].loopFrames = animations[TORSO_GESTURE].loopFrames;
				animations[i].numFrames = animations[TORSO_GESTURE].numFrames;
				animations[i].reversed = qfalse;
				animations[i].flipflop = qfalse;
				continue;
			}
			break;
		}
		animations[i].firstFrame = atoi(token);
		// leg only frames are adjusted to not count the upper body only frames
		if (i == LEGS_WALKCR)
		{
			skip = animations[LEGS_WALKCR].firstFrame - animations[TORSO_GESTURE].firstFrame;
		}
		if (i >= LEGS_WALKCR && i < TORSO_GETFLAG)
		{
			animations[i].firstFrame -= skip;
		}

		token = COM_Parse(&text_p);
		if (!*token)
		{
			break;
		}
		animations[i].numFrames = atoi(token);

		animations[i].reversed = qfalse;
		animations[i].flipflop = qfalse;
		// if numFrames is negative the animation is reversed
		if (animations[i].numFrames < 0)
		{
			animations[i].numFrames = -animations[i].numFrames;
			animations[i].reversed = qtrue;
		}

		token = COM_Parse(&text_p);
		if (!*token)
		{
			break;
		}
		animations[i].loopFrames = atoi(token);

		token = COM_Parse(&text_p);
		if (!*token)
		{
			break;
		}
		fps = atof(token);
		if (fps == 0)
		{
			fps = 1;
		}
		animations[i].frameLerp = 1000 / fps;
		animations[i].initialLerp = 1000 / fps;
	}

	if (i != MAX_ANIMATIONS)
	{
		CG_Printf("Error parsing animation file: %s", filename);
		return qfalse;
	}

	// crouch backward animation
	memcpy(&animations[LEGS_BACKCR], &animations[LEGS_WALKCR], sizeof(animation_t));
	animations[LEGS_BACKCR].reversed = qtrue;
	// walk backward animation
	memcpy(&animations[LEGS_BACKWALK], &animations[LEGS_WALK], sizeof(animation_t));
	animations[LEGS_BACKWALK].reversed = qtrue;
	// flag moving fast
	animations[FLAG_RUN].firstFrame = 0;
	animations[FLAG_RUN].numFrames = 16;
	animations[FLAG_RUN].loopFrames = 16;
	animations[FLAG_RUN].frameLerp = 1000 / 15;
	animations[FLAG_RUN].initialLerp = 1000 / 15;
	animations[FLAG_RUN].reversed = qfalse;
	// flag not moving or moving slowly
	animations[FLAG_STAND].firstFrame = 16;
	animations[FLAG_STAND].numFrames = 5;
	animations[FLAG_STAND].loopFrames = 0;
	animations[FLAG_STAND].frameLerp = 1000 / 20;
	animations[FLAG_STAND].initialLerp = 1000 / 20;
	animations[FLAG_STAND].reversed = qfalse;
	// flag speeding up
	animations[FLAG_STAND2RUN].firstFrame = 16;
	animations[FLAG_STAND2RUN].numFrames = 5;
	animations[FLAG_STAND2RUN].loopFrames = 1;
	animations[FLAG_STAND2RUN].frameLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].initialLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].reversed = qtrue;
	//
	// new anims changes
	//
//	animations[TORSO_GETFLAG].flipflop = qtrue;
//	animations[TORSO_GUARDBASE].flipflop = qtrue;
//	animations[TORSO_PATROL].flipflop = qtrue;
//	animations[TORSO_AFFIRMATIVE].flipflop = qtrue;
//	animations[TORSO_NEGATIVE].flipflop = qtrue;
	//
	return qtrue;
}

/*
==========================
CG_RegisterClientSkin
==========================
*/
static qboolean CG_RegisterClientSkin(clientInfo_t* ci, const char* teamName, const char* modelName, const char* skinName, const char* headModelName, const char* headSkinName)
{
	char filename[MAX_QPATH];

	Com_sprintf(filename, MAX_QPATH, "models/players/%s/%slower_%s.skin", modelName, teamName, skinName);
	ci->legsSkin = trap_R_RegisterSkin(filename);
	if (!ci->legsSkin)
	{
		Com_Printf("Leg skin load failure: %s\n", filename);
	}

	Com_sprintf(filename, MAX_QPATH, "models/players/%s/%supper_%s.skin", modelName, teamName, skinName);
	ci->torsoSkin = trap_R_RegisterSkin(filename);
	if (ci->torsoSkin == 0)
	{
		Com_Printf("Torso skin load failure: %s\n", filename);
	}

	if (headModelName[0] == '*')
	{
		Com_sprintf(filename, MAX_QPATH, "models/players/heads/%s/head_%s.skin", &headModelName[1], headSkinName);
	}
	else
	{
		Com_sprintf(filename, MAX_QPATH, "models/players/%s/%shead_%s.skin", headModelName, teamName, headSkinName);
	}
	ci->headSkin = trap_R_RegisterSkin(filename);
	if (ci->headSkin == 0)
	{
		if ((char) * (headModelName) != '*')
		{
			Com_sprintf(filename, MAX_QPATH, "models/players/heads/%s/head_%s.skin", headModelName, headSkinName);
			ci->headSkin = trap_R_RegisterSkin(filename);
		}
	}
	if (ci->headSkin == 0)
	{
		Com_Printf("Head skin load failure: %s\n", filename);
	}

	if (!ci->legsSkin || !ci->torsoSkin || !ci->headSkin)
	{
		return qfalse;
	}
	return qtrue;
}

/*
==========================
CG_RegisterClientModelname
==========================
*/
static qboolean CG_RegisterClientModelname(clientInfo_t* ci, const char* modelName, const char* skinName, const char* headModelName, const char* headSkinName, const char* teamName)
{
	char    filename[MAX_QPATH * 2];
	const char*      headName;
	char vertex_light_str[4];
	int vertex_light_num;
	qboolean failed_flag = qfalse;

	if (headModelName[0] == '\0')
	{
		headName = modelName;
	}
	else
	{
		headName = headModelName;
	}
	Com_sprintf(filename, sizeof(filename), "models/players/%s/lower.md3", modelName);
	ci->legsModel = trap_R_RegisterModel(filename);
	if (!ci->legsModel)
	{
		Com_sprintf(filename, sizeof(filename), "models/players/characters/%s/lower.md3", modelName);
		ci->legsModel = trap_R_RegisterModel(filename);
		if (!ci->legsModel)
		{
			Com_Printf("Failed to load model file %s\n", filename);
			return qfalse;
		}
	}

	Com_sprintf(filename, sizeof(filename), "models/players/%s/upper.md3", modelName);
	ci->torsoModel = trap_R_RegisterModel(filename);
	if (!ci->torsoModel)
	{
		Com_sprintf(filename, sizeof(filename), "models/players/characters/%s/upper.md3", modelName);
		ci->torsoModel = trap_R_RegisterModel(filename);
		if (!ci->torsoModel)
		{
			Com_Printf("Failed to load model file %s\n", filename);
			return qfalse;
		}
	}


	if (headName[0] == '*')
	{
		Com_sprintf(filename, sizeof(filename), "models/players/heads/%s/%s.md3", &headModelName[1], &headModelName[1]);
	}
	else
	{
		Com_sprintf(filename, sizeof(filename), "models/players/%s/head.md3", headName);
	}
	ci->headModel = trap_R_RegisterModel(filename);
	// if the head model could not be found and we didn't load from the heads folder try to load from there
	if (!ci->headModel && headName[0] != '*')
	{
		Com_sprintf(filename, sizeof(filename), "models/players/heads/%s/%s.md3", headModelName, headModelName);
		ci->headModel = trap_R_RegisterModel(filename);
	}
	if (!ci->headModel)
	{
		Com_Printf("Failed to load model file %s\n", filename);
		return qfalse;
	}


	trap_Cvar_VariableStringBuffer("r_vertexLight", vertex_light_str, 4);
	vertex_light_num = atoi(vertex_light_str);
	if (vertex_light_num)
	{
		trap_Cvar_Set("r_vertexLight", "0");
	}

	// if any skins failed to load, return failure
	if (!CG_RegisterClientSkin(ci, teamName, modelName, skinName, headName, headSkinName))
	{
		failed_flag = qtrue;
		if (teamName && *teamName)
		{
			Com_Printf("Failed to load skin file: %s : %s : %s, %s : %s\n", teamName, modelName, skinName, headName, headSkinName);
		}
		else
		{
			Com_Printf("Failed to load skin file: %s : %s, %s : %s\n", modelName, skinName, headName, headSkinName);
		}
	}

	if (vertex_light_num)
	{
		trap_Cvar_Set("r_vertexLight", "1");
	}

	if (failed_flag)
	{
		return qfalse;
	}

	// load the animations
	Com_sprintf(filename, sizeof(filename), "models/players/%s/animation.cfg", modelName);
	if (!CG_ParseAnimationFile(filename, ci))
	{
		Com_Printf("Failed to load animation file %s\n", filename);
		return qfalse;
	}
	if (headName[0] == '*')
	{
		Com_sprintf(filename, sizeof(filename), "models/players/heads/%s/icon_%s.tga", &headName[1], headSkinName);
	}
	else
	{
		Com_sprintf(filename, sizeof(filename), "models/players/%s/icon_%s.tga", headName, headSkinName);
	}

	ci->modelIcon = trap_R_RegisterShaderNoMip(filename);
	if (!ci->modelIcon)
	{
		if (headName[0] != '*')
		{
			Com_sprintf(filename, sizeof(filename), "models/players/heads/%s/icon_%s.tga", headName, headSkinName);
			ci->modelIcon = trap_R_RegisterShaderNoMip(filename);
		}
	}
	if (!ci->modelIcon)
	{
		Com_Printf("Failed to load icon file: %s\n", filename);
		return qfalse;
	}

	return qtrue;
}


/*
===================
CG_LoadClientInfo

Load it now, taking the disk hits.
===================
*/
static void CG_LoadClientInfo(clientInfo_t* ci)
{
	const char*  dir, *fallback;
	int         i, modelloaded;
	const char*  s;
	int         clientNum;
	char        teamname[MAX_QPATH];

	teamname[0] = 0;
	modelloaded = qtrue;
	if (!CG_RegisterClientModelname(ci, ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname))
	{
		if (cg_buildScript.integer)
		{
			CG_Error("CG_RegisterClientModelname( %s, %s, %s, %s %s ) failed", ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname);
		}

		// fall back to default team name
		if (cgs.gametype >= GT_TEAM)
		{
			if (!CG_RegisterClientModelname(ci, ci->modelName, ci->skinName, ci->headModelName, ci->skinName, teamname))
			{
				const char* skin = ci->rt == TEAM_RED ? "red" : "blue";
				if (!CG_RegisterClientModelname(ci, "sarge", skin, "sarge", skin, teamname))
				{
					CG_Error("DEFAULT_TEAM_MODEL / skin (%s/%s) failed to register", DEFAULT_TEAM_MODEL, skin);
					modelloaded = qfalse;
				}
			}
		}
		else
		{
			if (!CG_RegisterClientModelname(ci, ci->modelName, "default", ci->headModelName, "default", teamname))
			{
				if (!CG_RegisterClientModelname(ci, "sarge", "default", "sarge", "default", teamname))
				{
					CG_Error("DEFAULT_TEAM_MODEL / skin (%s/%s) failed to register", DEFAULT_TEAM_MODEL, "default");
					modelloaded = qfalse;
				}
			}
		}
	}

	ci->newAnims = qfalse;
	if (ci->torsoModel)
	{
		orientation_t tag;
		// if the torso model has the "tag_flag"
		if (trap_R_LerpTag(&tag, ci->torsoModel, 0, 0, 1, "tag_flag"))
		{
			ci->newAnims = qtrue;
		}
	}

	// sounds
	dir = ci->modelName;
	fallback = (cgs.gametype >= GT_TEAM) ? DEFAULT_TEAM_MODEL : DEFAULT_MODEL;

	for (i = 0 ; i < MAX_CUSTOM_SOUNDS ; i++)
	{
		s = cg_customSoundNames[i];
		if (!s)
		{
			break;
		}
		ci->sounds[i] = 0;
		// if the model didn't load use the sounds of the default model
		if (modelloaded)
		{
			ci->sounds[i] = trap_S_RegisterSound(va("sound/player/%s/%s", dir, s + 1), qfalse);
		}
		if (!ci->sounds[i])
		{
			ci->sounds[i] = trap_S_RegisterSound(va("sound/player/%s/%s", fallback, s + 1), qfalse);
		}
	}

	// reset any existing players and bodies, because they might be in bad
	// frames for this new model
	clientNum = ci - cgs.clientinfo;
	for (i = 0 ; i < MAX_GENTITIES ; i++)
	{
		if (cg_entities[i].currentState.clientNum == clientNum
		        && cg_entities[i].currentState.eType == ET_PLAYER)
		{
			CG_ResetPlayerEntity(&cg_entities[i]);
		}
	}
}

static void CG_ForceNewClientInfo(clientInfo_t* old, clientInfo_t* new)
{
	CG_LoadClientInfo(new);

	new->infoValid = qtrue;
	memcpy(old, new, sizeof(clientInfo_t));
}


static void CG_UpdateModelFromString(char* modelName, char* skinName, const char* resultModelString, qboolean isOurClient, team_t team)
{
	const char* nameModel;
	const char* nameSkin = NULL;
	qboolean isPmSkin = qfalse;
	char* ptr;

	char tmpStr[MAX_QPATH];
	Q_strncpyz(tmpStr, resultModelString, MAX_QPATH);

	nameModel = tmpStr;

	ptr = strchr(tmpStr, '/');
	if (ptr && ptr[1])
	{
		*ptr = 0;
		nameSkin = ++ptr;
	}

	isPmSkin = (nameSkin && (Q_stricmp(nameSkin, "pm") == 0)) ? qtrue : qfalse;

	if (isOurClient)
	{
		// our player only pm or default
		if (qfalse && !isPmSkin)
		{
			nameSkin = "default";
		}
	}
	else if (cgs.gametype >= GT_TEAM)
	{
		// in team games users able to set pm skin only
		if (!nameSkin || !isPmSkin)
		{
			if (team == TEAM_BLUE)
			{
				nameSkin = cg_swapSkins.integer ? "red" : "blue";
			}
			else if (team == TEAM_RED)
			{
				nameSkin = cg_swapSkins.integer ? "blue" : "red";
			}
			else
			{
				nameSkin = "default";
			}
		}
	}
	else if (!isPmSkin)
	{
		nameSkin = "default";
	}

	Q_strncpyz(modelName, nameModel, MAX_QPATH);
	Q_strncpyz(skinName, nameSkin ? nameSkin : "default", MAX_QPATH);
}

static void CG_ClientInfoUpdateModel(clientInfo_t* ci, qboolean isOurClient, qboolean isTeamGame, const char* config, int clientNum)
{
	char modelName[MAX_QPATH];
	char headModelName[MAX_QPATH];
	const char* resultModelString = "keel/pm";
	const char* resultHModelString = "keel/pm";
	const char* cfgModelString = Info_ValueForKey(config, "model");
	const char* cfgHModelString = Info_ValueForKey(config, "hmodel");

	// сначала решим какую строку модели использовать
	if (isOurClient)
	{
		trap_Cvar_VariableStringBuffer("model", modelName, sizeof(modelName));
		resultModelString = modelName;
		trap_Cvar_VariableStringBuffer("headmodel", headModelName, sizeof(headModelName));
		resultHModelString = headModelName;
	}
	else
	{
		const char* forceModelString = cg_forceModel.integer ? cfgModelString : NULL;
		const char* forceHModelString = cg_forceModel.integer ? cfgHModelString : NULL;
		const char* enemyModelString = NULL;
		const qboolean useOriginal = cg_spectOrigModel.integer && CG_IsFollowing() && cg.snap->ps.clientNum == clientNum;

		if (cg_enemyModel.string[0])
		{
			enemyModelString = cg_enemyModel.string;
		}

		if (!isTeamGame)
		{
			// FFA or 1vs1

			if (useOriginal)
			{
				resultModelString = cfgModelString;
				resultHModelString = cfgHModelString;
			}
			else if (enemyModelString)
			{
				resultModelString = enemyModelString;
				resultHModelString = enemyModelString;
			}
			else if (forceModelString)
			{
				resultModelString = forceModelString;
				resultHModelString = forceHModelString;
			}
			else
			{
				resultModelString = cfgModelString;
				resultHModelString = cfgHModelString;
			}
		}
		else
		{
			const char* teamModelString = cg_teamModel.string[0] ? cg_teamModel.string : NULL;
			// team games
			qboolean isOurTeam = cgs.clientinfo[cg.clientNum].rt == ci->rt;
			qboolean isRedTeamWhileSpec = cgs.clientinfo[cg.clientNum].rt == TEAM_SPECTATOR && ci->rt == TEAM_RED;
			qboolean isTeamMate = isOurTeam || isRedTeamWhileSpec;

			if (isTeamMate)
			{
				if (useOriginal)
				{
					resultModelString = cfgModelString;
					resultHModelString = cfgHModelString;
				}
				else if (teamModelString)
				{
					resultModelString = teamModelString;
					resultHModelString = teamModelString;
				}
				else if (forceModelString)
				{
					resultModelString = forceModelString;
					resultHModelString = forceHModelString;
				}
				else
				{
					resultModelString = cfgModelString;
					resultHModelString = cfgHModelString;
				}
			}
			else
			{
				if (useOriginal)
				{
					resultModelString = cfgModelString;
					resultHModelString = cfgHModelString;
				}
				else if (enemyModelString)
				{
					resultModelString = enemyModelString;
					resultHModelString = enemyModelString;
				}
				else if (forceModelString)
				{
					resultModelString = forceModelString;
					resultHModelString = forceHModelString;
				}
				else
				{
					resultModelString = cfgModelString;
					resultHModelString = cfgHModelString;
				}
			}
		}
	}
	CG_UpdateModelFromString(&ci->modelName[0], &ci->skinName[0], resultModelString, isOurClient, ci->rt);
	CG_UpdateModelFromString(&ci->headModelName[0], &ci->headSkinName[0], resultHModelString, isOurClient, ci->rt);
	ci->isPmSkin = (Q_stricmp(ci->skinName, "pm") == 0) ? qtrue : qfalse;
}

/*
 * Check is name invisible
 */
static qboolean CG_PlayerNameIsInvisible(clientInfo_t* ci)
{
	int i;
	int len = strlen(ci->name_clean);
	if (len == 0) return qtrue;
	for (i = 0; i < len; ++i)
	{
		if (ci->name_clean[i] != ' ')
		{
			return qfalse;
		}
	}
	return qtrue;
}

#define NUM_TO_HEX_CHAR(III) (((III&0x0f) < 10) ? ('0' + (III&0x0f)) : ('a' + ((III&0x0f) - 10)))
static void CG_PlayerXIDCalc(const char* cmd, clientInfo_t* ci)
{
	const char* data_c1;
	const char* data_c2;
	const char* data_c3;
	const char* data_c4;
	char* skin;
	unsigned short sign = 0;
	char tmp[MAX_QPATH];

	CRC_Init(&sign);

	data_c1 = Info_ValueForKey(cmd, "c1");
	CRC_ContinueProcessString(&sign, data_c1, strlen(data_c1));

	data_c2 = Info_ValueForKey(cmd, "c2");
	CRC_ContinueProcessString(&sign, data_c2, strlen(data_c2));

	data_c3 = Info_ValueForKey(cmd, "model");
	Q_strncpyz(tmp, data_c3, MAX_QPATH);

	skin = strchr(tmp, '/');
	if (skin)
	{
		*skin = '\0';
	}

	CRC_ContinueProcessString(&sign, tmp, strlen(tmp));

	data_c4 = Info_ValueForKey(cmd, "hmodel");
	Q_strncpyz(tmp, data_c4, MAX_QPATH);

	skin = strchr(tmp, '/');
	if (skin)
	{
		*skin = '\0';
	}

	CRC_ContinueProcessString(&sign, tmp, strlen(tmp));

	ci->xid = sign;

	ci->xidStr[0] = NUM_TO_HEX_CHAR((ci->xid >> 12) & 0x0f);
	ci->xidStr[1] = NUM_TO_HEX_CHAR((ci->xid >> 8) & 0x0f);
	ci->xidStr[2] = NUM_TO_HEX_CHAR((ci->xid >> 4) & 0x0f);
	ci->xidStr[3] = NUM_TO_HEX_CHAR(ci->xid & 0x0f);
	ci->xidStr[4] = 0;
}

/*
===================
CG_IsOnlyTeamTaskChanged

Check if only teamTask field changed in client info.
Returns qtrue if only teamTask changed, qfalse otherwise.
===================
*/
static qboolean CG_IsOnlyTeamTaskChanged(const clientInfo_t* ci, const clientInfo_t* newInfo, const char* configstring)
{
	int clientNum;
	const char* oldConfigstring;
	char newCS[MAX_STRING_CHARS];
	char oldCS[MAX_STRING_CHARS];

	if (!ci->infoValid)
	{
		return qfalse;
	}

	/* Get old configstring */
	clientNum = ci - cgs.clientinfo;
	oldConfigstring = CG_ConfigString(clientNum + CS_PLAYERS);

	if (!oldConfigstring || !oldConfigstring[0])
	{
		return qfalse;
	}

	/* Copy configstrings and remove "tt" field from both */
	Q_strncpyz(newCS, configstring, sizeof(newCS));
	Q_strncpyz(oldCS, oldConfigstring, sizeof(oldCS));

	Info_RemoveKey(newCS, "tt");
	Info_RemoveKey(oldCS, "tt");

	/* Compare configstrings without "tt" field */
	if (Q_stricmp(newCS, oldCS) == 0)
	{
		/* All fields except teamTask match - check if teamTask actually changed */
		if (newInfo->teamTask != ci->teamTask)
		{
			/* Only teamTask changed */
			return qtrue;
		}
	}

	return qfalse;
}

void CG_NewClientInfo(int clientNum)
{
	clientInfo_t* ci;
	clientInfo_t newInfo;
	const char*  configstring;
	const char*  v;
	const qboolean isOurClient = clientNum == cg.clientNum;
	const qboolean isTeamGame = cgs.gametype >= GT_TEAM;

	ci = &cgs.clientinfo[clientNum];

	configstring = CG_ConfigString(clientNum + CS_PLAYERS);
	if (!configstring[0])
	{
		memset(ci, 0, sizeof(*ci));
		return;     // player just left
	}

	// build into a temp buffer so the defer checks can use
	// the old value
	memset(&newInfo, 0, sizeof(newInfo));

	CG_PlayerColorsLoadDefault(&newInfo.colors);

	CG_PlayerXIDCalc(configstring, &newInfo);

	// isolate the player's name
	v = Info_ValueForKey(configstring, "n");
	Q_strncpyz(newInfo.name_original, v, sizeof(newInfo.name));
	Q_strncpyz(newInfo.name_clean, v, sizeof(newInfo.name));
	CG_RemoveChatEscapeCharAll(newInfo.name_clean);
	newInfo.nameIsInvisible = CG_PlayerNameIsInvisible(&newInfo);

	CG_StringMakeEscapeCharRAW(newInfo.name_original, newInfo.name_codes, sizeof(newInfo.name_codes));

	if (cg_playersXID.integer)
	{
		if (cg_playersXID.integer == 2)
		{
			int p;
			qboolean is_exists = qfalse;
			// check, is player with this name is exitst
			for (p = 0; p < MAX_CLIENTS; ++p)
			{
				if (p == clientNum)
				{
					continue;
				}
				//ugly linear search
				if (cgs.clientinfo[p].infoValid && !cgs.clientinfo[p].nameIsInvisible && strcmp(newInfo.name_clean, cgs.clientinfo[p].name_clean) == 0)
				{
					is_exists = qtrue;
					// okay, it exits. check is another client have xid in the name and append it if not
					if (strcmp(cgs.clientinfo[p].name_original, cgs.clientinfo[p].name) == 0)
					{
						Com_sprintf(cgs.clientinfo[p].name, sizeof(ci->name), "^8%s^7:%s", cgs.clientinfo[p].xidStr, cgs.clientinfo[p].name_original);
					}
					break;
				}
			}

			if (newInfo.nameIsInvisible)
			{
				Com_sprintf(newInfo.name, sizeof(newInfo.name), "^8%s^7:\'%s^7\'", newInfo.xidStr, newInfo.name_codes);
			}
			else if (is_exists || (strcmp(newInfo.name_original, "^1NONAME") == 0))
			{
				Com_sprintf(newInfo.name, sizeof(newInfo.name), "^8%s^7:%s", newInfo.xidStr, newInfo.name_original);
			}
			else
			{
				Q_strncpyz(newInfo.name, newInfo.name_original, sizeof(newInfo.name));
			}
		}
		else
		{
			Com_sprintf(newInfo.name, sizeof(newInfo.name), "^8%s^7:%s", newInfo.xidStr, newInfo.name_original);
		}
	}
	else
	{
		Q_strncpyz(newInfo.name, newInfo.name_original, sizeof(newInfo.name));
	}

	// bot skill
	v = Info_ValueForKey(configstring, "skill");
	newInfo.botSkill = atoi(v);

	// colors
	v = Info_ValueForKey(configstring, "c1");
	CG_OSPColorsFromString(v, newInfo.colors.railCore, newInfo.colors.head, newInfo.colors.torso, newInfo.colors.legs);

	v = Info_ValueForKey(configstring, "c2");
	CG_OSPColorFromChar(v[0], newInfo.colors.railRings);


	// handicap
	v = Info_ValueForKey(configstring, "hc");
	newInfo.handicap = atoi(v);

	// wins
	v = Info_ValueForKey(configstring, "w");
	newInfo.wins = atoi(v);

	// losses
	v = Info_ValueForKey(configstring, "l");
	newInfo.losses = atoi(v);

	// team
	v = Info_ValueForKey(configstring, "t");
	newInfo.team = atoi(v);


	// rt
	v = Info_ValueForKey(configstring, "rt");
	if (v && v[0])
	{
		newInfo.rt = atoi(v);
		if (!newInfo.rt)
		{
			newInfo.rt = TEAM_SPECTATOR;
		}
	}
	else
	{
		newInfo.rt = newInfo.team;
	}

	/* team task */
	v = Info_ValueForKey(configstring, "tt");
	newInfo.teamTask = atoi(v);

	//st
	v = Info_ValueForKey(configstring, "st");
	if (v && v[0])
	{
		int tmp = atoi(v);

		if (tmp)
		{
			newInfo.team = (tmp & 7) + 5;
		}
		if (tmp & 8)
		{
			newInfo.st = tmp & 7;
		}
	}

	/* Check if only teamTask changed - if so, skip full reload */
	if (CG_IsOnlyTeamTaskChanged(ci, &newInfo, configstring))
	{
		/* Only teamTask changed - update it without full reload */
		ci->teamTask = newInfo.teamTask;
		/* Don't reset team overlay, don't reload models, don't update other clients */
		return;
	}

	CG_ClientInfoUpdateModel(&newInfo, isOurClient, isTeamGame, configstring, clientNum);
	CG_ClientInfoUpdateColors(&newInfo, clientNum);
	CG_ForceNewClientInfo(ci, &newInfo);

	/* If we changed team, update models & colors */
	if (isOurClient)
	{
		static team_t ourTeam = -1;
		static int ourClientNum = -1;
		const qboolean isTeamChanged = cgs.clientinfo[cg.clientNum].rt != ourTeam;
		const qboolean isClientIdChanged = ourClientNum != cg.clientNum;
		numSortedTeamPlayers = 0; //reset team overlay
		if (isTeamGame && isTeamChanged)
		{
			ourTeam = cgs.clientinfo[cg.clientNum].rt;
			CG_UpdateOtherClientsInfo();
		}

		if (isTeamGame && isClientIdChanged)
		{
			ourClientNum = cg.clientNum;
			CG_UpdateOtherClientsInfo();
		}
	}
}

void CG_UpdateOurClientInfo(void)
{
	CG_NewClientInfo(cg.clientNum);
}

static void CG_UpdateOtherClientsInfo(void)
{
	int index;

	for (index = 0; index < MAX_CLIENTS; ++index)
	{
		if (index != cg.clientNum && CG_ConfigString(CS_PLAYERS + index))
		{
			CG_NewClientInfo(index);
		}
	}
}

void CG_UpdateAllClientsInfo(void)
{
	int index;

	for (index = 0; index < MAX_CLIENTS; ++index)
	{
		if (CG_ConfigString(CS_PLAYERS + index))
		{
			CG_NewClientInfo(index);
		}
	}
}

/*
=============================================================================

PLAYER ANIMATION

=============================================================================
*/


/*
===============
CG_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_SetLerpFrameAnimation(clientInfo_t* ci, lerpFrame_t* lf, int newAnimation)
{
	animation_t* anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if (newAnimation < 0 || newAnimation >= MAX_TOTALANIMATIONS)
	{
		CG_Error("Bad animation number: %i", newAnimation);
	}

	anim = &ci->animations[ newAnimation ];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

	if (cg_debugAnim.integer)
	{
		CG_Printf("Anim: %i\n", newAnimation);
	}
}

/*
===============
CG_RunLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
static void CG_RunLerpFrame(clientInfo_t* ci, lerpFrame_t* lf, int newAnimation, float speedScale)
{
	int         f, numFrames;
	animation_t* anim;

	// debugging tool to get no animations
	if (cg_animSpeed.integer == 0)
	{
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if (newAnimation != lf->animationNumber || !lf->animation)
	{
		CG_SetLerpFrameAnimation(ci, lf, newAnimation);
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if (cg.time >= lf->frameTime)
	{
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if (!anim->frameLerp)
		{
			return;     // shouldn't happen
		}
		if (cg.time < lf->animationTime)
		{
			lf->frameTime = lf->animationTime;      // initial lerp
		}
		else
		{
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		f = (lf->frameTime - lf->animationTime) / anim->frameLerp;
		f *= speedScale;        // adjust for haste, etc

		numFrames = anim->numFrames;
		if (anim->flipflop)
		{
			numFrames *= 2;
		}
		if (f >= numFrames)
		{
			f -= numFrames;
			if (anim->loopFrames)
			{
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			}
			else
			{
				f = numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = cg.time;
			}
		}
		if (anim->reversed)
		{
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		}
		else if (anim->flipflop && f >= anim->numFrames)
		{
			lf->frame = anim->firstFrame + anim->numFrames - 1 - (f % anim->numFrames);
		}
		else
		{
			lf->frame = anim->firstFrame + f;
		}
		if (cg.time > lf->frameTime)
		{
			lf->frameTime = cg.time;
			if (cg_debugAnim.integer)
			{
				CG_Printf("Clamp lf->frameTime\n");
			}
		}
	}

	if (lf->frameTime > cg.time + 200)
	{
		lf->frameTime = cg.time;
	}

	if (lf->oldFrameTime > cg.time)
	{
		lf->oldFrameTime = cg.time;
	}
	// calculate current lerp value
	if (lf->frameTime == lf->oldFrameTime)
	{
		lf->backlerp = 0;
	}
	else
	{
		lf->backlerp = 1.0 - (float)(cg.time - lf->oldFrameTime) / (lf->frameTime - lf->oldFrameTime);
	}
}


/*
===============
CG_ClearLerpFrame
===============
*/
static void CG_ClearLerpFrame(clientInfo_t* ci, lerpFrame_t* lf, int animationNumber)
{
	lf->frameTime = lf->oldFrameTime = cg.time;
	CG_SetLerpFrameAnimation(ci, lf, animationNumber);
	lf->oldFrame = lf->frame = lf->animation->firstFrame;
}


/*
===============
CG_PlayerAnimation
===============
*/
static void CG_PlayerAnimation(centity_t* cent, int* legsOld, int* legs, float* legsBackLerp,
                               int* torsoOld, int* torso, float* torsoBackLerp)
{
	clientInfo_t*    ci;
	int             clientNum;
	float           speedScale;

	clientNum = cent->currentState.clientNum;

	if (cg_noPlayerAnims.integer)
	{
		*legsOld = *legs = *torsoOld = *torso = 0;
		return;
	}

	if (cent->currentState.powerups & (1 << PW_HASTE))
	{
		speedScale = 1.5;
	}
	else
	{
		speedScale = 1;
	}

	ci = &cgs.clientinfo[ clientNum ];

	// do the shuffle turn frames locally
	if (cent->pe.legs.yawing && (cent->currentState.legsAnim & ~ANIM_TOGGLEBIT) == LEGS_IDLE)
	{
		CG_RunLerpFrame(ci, &cent->pe.legs, LEGS_TURN, speedScale);
	}
	else
	{
		CG_RunLerpFrame(ci, &cent->pe.legs, cent->currentState.legsAnim, speedScale);
	}

	*legsOld = cent->pe.legs.oldFrame;
	*legs = cent->pe.legs.frame;
	*legsBackLerp = cent->pe.legs.backlerp;

	if (cg_noTaunt.integer & ((cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) == TORSO_GESTURE))
	{
		cent->currentState.torsoAnim = (cent->currentState.torsoAnim & ANIM_TOGGLEBIT) | TORSO_STAND;
	}

	CG_RunLerpFrame(ci, &cent->pe.torso, cent->currentState.torsoAnim, speedScale);
	*torsoOld = cent->pe.torso.oldFrame;
	*torso = cent->pe.torso.frame;
	*torsoBackLerp = cent->pe.torso.backlerp;
}

/*
=============================================================================

PLAYER ANGLES

=============================================================================
*/

/*
==================
CG_SwingAngles
==================
*/
static void CG_SwingAngles(float destination, float swingTolerance, float clampTolerance,
                           float speed, float* angle, qboolean* swinging)
{
	float   swing;
	float   move;
	float   scale;

	if (!*swinging)
	{
		// see if a swing should be started
		swing = AngleSubtract(*angle, destination);
		if (swing > swingTolerance || swing < -swingTolerance)
		{
			*swinging = qtrue;
		}
	}

	if (!*swinging)
	{
		return;
	}

	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract(destination, *angle);
	scale = fabs(swing);
	if (scale < swingTolerance * 0.5)
	{
		scale = 0.5;
	}
	else if (scale < swingTolerance)
	{
		scale = 1.0;
	}
	else
	{
		scale = 2.0;
	}

	// swing towards the destination angle
	if (swing >= 0)
	{
		move = cg.frametime * scale * speed;
		if (move >= swing)
		{
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod(*angle + move);
	}
	else if (swing < 0)
	{
		move = cg.frametime * scale * -speed;
		if (move <= swing)
		{
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod(*angle + move);
	}

	// clamp to no more than tolerance
	swing = AngleSubtract(destination, *angle);
	if (swing > clampTolerance)
	{
		*angle = AngleMod(destination - (clampTolerance - 1));
	}
	else if (swing < -clampTolerance)
	{
		*angle = AngleMod(destination + (clampTolerance - 1));
	}
}

/*
=================
CG_AddPainTwitch
=================
*/
static void CG_AddPainTwitch(centity_t* cent, vec3_t torsoAngles)
{
	int     t;
	float   f;

	t = cg.time - cent->pe.painTime;
	if (t >= PAIN_TWITCH_TIME)
	{
		return;
	}

	f = 1.0 - (float)t / PAIN_TWITCH_TIME;

	if (cent->pe.painDirection)
	{
		torsoAngles[ROLL] += 20 * f;
	}
	else
	{
		torsoAngles[ROLL] -= 20 * f;
	}
}


/*
===============
CG_PlayerAngles

Handles seperate torso motion

  legs pivot based on direction of movement

  head always looks exactly at cent->lerpAngles

  if motion < 20 degrees, show in head only
  if < 45 degrees, also show in torso
===============
*/
static void CG_PlayerAngles(centity_t* cent, vec3_t legs[3], vec3_t torso[3], vec3_t head[3])
{
	vec3_t      legsAngles, torsoAngles, headAngles;
	float       dest;
	static  int movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
	vec3_t      velocity;
	float       speed;
	int         dir, clientNum;
	clientInfo_t*    ci;

	VectorCopy(cent->lerpAngles, headAngles);
	headAngles[YAW] = AngleMod(headAngles[YAW]);
	VectorClear(legsAngles);
	VectorClear(torsoAngles);

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ((cent->currentState.legsAnim & ~ANIM_TOGGLEBIT) != LEGS_IDLE
	        || (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND)
	{
		// if not standing still, always point all in the same direction
		cent->pe.torso.yawing = qtrue;  // always center
		cent->pe.torso.pitching = qtrue;    // always center
		cent->pe.legs.yawing = qtrue;   // always center
	}

	// adjust legs for movement dir
	if (cent->currentState.eFlags & EF_DEAD)
	{
		// don't let dead bodies twitch
		dir = 0;
	}
	else
	{
		dir = cent->currentState.angles2[YAW];
		if (dir < 0 || dir > 7)
		{
			CG_Error("Bad player movement angle");
		}
	}
	legsAngles[YAW] = headAngles[YAW] + movementOffsets[ dir ];
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[ dir ];

	// torso
	CG_SwingAngles(torsoAngles[YAW], 25, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing);
	CG_SwingAngles(legsAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing);

	torsoAngles[YAW] = cent->pe.torso.yawAngle;
	legsAngles[YAW] = cent->pe.legs.yawAngle;


	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if (headAngles[PITCH] > 180)
	{
		dest = (-360 + headAngles[PITCH]) * 0.75f;
	}
	else
	{
		dest = headAngles[PITCH] * 0.75f;
	}
	CG_SwingAngles(dest, 15, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching);
	torsoAngles[PITCH] = cent->pe.torso.pitchAngle;

	//
	clientNum = cent->currentState.clientNum;
	if (clientNum >= 0 && clientNum < MAX_CLIENTS)
	{
		ci = &cgs.clientinfo[ clientNum ];
		if (ci->fixedtorso)
		{
			torsoAngles[PITCH] = 0.0f;
		}
	}

	// --------- roll -------------


	// lean towards the direction of travel
	VectorCopy(cent->currentState.pos.trDelta, velocity);
	speed = VectorNormalize(velocity);
	if (speed)
	{
		vec3_t  axis[3];
		float   side;

		speed *= 0.05f;

		AnglesToAxis(legsAngles, axis);
		side = speed * DotProduct(velocity, axis[1]);
		if (cg_showPlayerLean.integer != 0)
		{
			legsAngles[ROLL] -= side;
		}

		side = speed * DotProduct(velocity, axis[0]);
		legsAngles[PITCH] += side;
	}

	//
	clientNum = cent->currentState.clientNum;
	if (clientNum >= 0 && clientNum < MAX_CLIENTS)
	{
		ci = &cgs.clientinfo[ clientNum ];
		if (ci->fixedlegs)
		{
			legsAngles[YAW] = torsoAngles[YAW];
			legsAngles[PITCH] = 0.0f;
			legsAngles[ROLL] = 0.0f;
		}
	}

	// pain twitch
	CG_AddPainTwitch(cent, torsoAngles);

	// pull the angles back out of the hierarchial chain
	AnglesSubtract(headAngles, torsoAngles, headAngles);
	AnglesSubtract(torsoAngles, legsAngles, torsoAngles);
	AnglesToAxis(legsAngles, legs);
	AnglesToAxis(torsoAngles, torso);
	AnglesToAxis(headAngles, head);
}


//==========================================================================

/*
===============
CG_HasteTrail
===============
*/
static void CG_HasteTrail(centity_t* cent)
{
	localEntity_t*   smoke;
	vec3_t          origin;
	int             anim;

	if (cent->trailTime > cg.time)
	{
		return;
	}
	anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
	if (anim != LEGS_RUN && anim != LEGS_BACK)
	{
		return;
	}

	cent->trailTime += 100;
	if (cent->trailTime < cg.time)
	{
		cent->trailTime = cg.time;
	}

	VectorCopy(cent->lerpOrigin, origin);
	origin[2] -= 16;

	smoke = CG_SmokePuff(origin, vec3_origin,
	                     8,
	                     1, 1, 1, 1,
	                     500,
	                     cg.time,
	                     0,
	                     0,
	                     cgs.media.hastePuffShader);

	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}

/*
===============
CG_BreathPuffs
===============
*/
static void CG_BreathPuffs(centity_t* cent, refEntity_t* head)
{
	clientInfo_t* ci;
	vec3_t up, origin;
	int contents;

	ci = &cgs.clientinfo[ cent->currentState.clientNum ];

	if (!cg_enableBreath.integer)
	{
		return;
	}
	if (cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson)
	{
		return;
	}
	if (cent->currentState.eFlags & EF_DEAD)
	{
		return;
	}
	if (cgs.osp.gameTypeFreeze && (cent->currentState.weapon == WP_NONE) &&
	        (cent->currentState.powerups & (1 << PW_BATTLESUIT)))
	{
		return;
	}
	contents = trap_CM_PointContents(head->origin, 0);
	if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{
		return;
	}
	if (ci->breathPuffTime > cg.time)
	{
		return;
	}

	VectorSet(up, 0, 0, 8);
	VectorMA(head->origin, 8, head->axis[0], origin);
	VectorMA(origin, -4, head->axis[2], origin);
	CG_SmokePuff(origin, up, 16, 1, 1, 1, 0.66f, 1500, cg.time, cg.time + 400, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader);
	ci->breathPuffTime = cg.time + 2000;
}

/*
===============
CG_TrailItem
===============
*/
static void CG_TrailItem(centity_t* cent, qhandle_t hModel)
{
	refEntity_t     ent;
	vec3_t          angles;
	vec3_t          axis[3];

	VectorCopy(cent->lerpAngles, angles);
	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AnglesToAxis(angles, axis);

	memset(&ent, 0, sizeof(ent));
	VectorMA(cent->lerpOrigin, -16, axis[0], ent.origin);
	ent.origin[2] += 16;
	angles[YAW] += 90;
	AnglesToAxis(angles, ent.axis);

	ent.hModel = hModel;
	trap_R_AddRefEntityToScene(&ent);
}


/*
===============
CG_PlayerPowerups
===============
*/
static void CG_PlayerPowerups(centity_t* cent)
{
	int     powerups;

	if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR)
	{
		int clientNum = cent->currentState.clientNum;
		if (cg_spectGlow.integer || clientNum == cg.snap->ps.generic1 - 32)
		{
			if (cgs.clientinfo[clientNum].team == TEAM_BLUE)
			{
				trap_R_AddLightToScene(cent->lerpOrigin, (float)((rand() & 0x1f) + 200), 0.2, 0.2, 1.0);
				trap_R_AddLightToScene(cent->lerpOrigin, (float)((rand() & 0x1f) + 200), 0.2, 0.2, 1.0);
			}
			else if (cgs.clientinfo[clientNum].team == TEAM_RED)
			{
				trap_R_AddLightToScene(cent->lerpOrigin, (float)((rand() & 0x1f) + 200), 1.0, 0.2, 0.2);
				trap_R_AddLightToScene(cent->lerpOrigin, (float)((rand() & 0x1f) + 200), 1.0, 0.2, 0.2);
			}
			else if (cgs.clientinfo[clientNum].team == TEAM_FREE)
			{
				trap_R_AddLightToScene(cent->lerpOrigin, (float)((rand() & 0x1f) + 0xc8), 1.0, 1.0, 1.0);
			}
		}
	}

	powerups = cent->currentState.powerups;
	if (!powerups)
	{
		return;
	}

	// quad gives a dlight
	if (powerups & (1 << PW_QUAD))
	{
		trap_R_AddLightToScene(cent->lerpOrigin, 200 + (rand() & 31), 0.2f, 0.2f, 1);
	}

	// flight plays a looped sound
	if (powerups & (1 << PW_FLIGHT))
	{
		trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.flightSound);
	}

	// redflag
	if (powerups & (1 << PW_REDFLAG))
	{
		CG_TrailItem(cent, cgs.media.redFlagModel);
		trap_R_AddLightToScene(cent->lerpOrigin, 200 + (rand() & 31), 1.0, 0.2f, 0.2f);
	}

	// blueflag
	if (powerups & (1 << PW_BLUEFLAG))
	{
		CG_TrailItem(cent, cgs.media.blueFlagModel);
		trap_R_AddLightToScene(cent->lerpOrigin, 200 + (rand() & 31), 0.2f, 0.2f, 1.0);
	}

	// neutralflag
	if (powerups & (1 << PW_NEUTRALFLAG))
	{
		CG_TrailItem(cent, cgs.media.neutralFlagModel);
		trap_R_AddLightToScene(cent->lerpOrigin, 200 + (rand() & 31), 1.0, 1.0, 1.0);
	}

	// haste leaves smoke trails
	if (powerups & (1 << PW_HASTE))
	{
		CG_HasteTrail(cent);
	}
}


/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
static void CG_PlayerFloatSprite(centity_t* cent, qhandle_t shader, vec4_t color)
{
	int             rf;
	refEntity_t     ent;

	if (cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson)
	{
		rf = RF_THIRD_PERSON;       // only show in mirrors
	}
	else
	{
		rf = 0;
	}

	memset(&ent, 0, sizeof(ent));
	VectorCopy(cent->lerpOrigin, ent.origin);
	ent.origin[2] += 48;
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = rf;
	if (color)
	{
		MAKERGBA(ent.shaderRGBA, color[0] * 255, color[1] * 255, color[2] * 255, 255);
	}
	else
	{
		MAKERGBA(ent.shaderRGBA, 255, 255, 255, 255);
	}
	trap_R_AddRefEntityToScene(&ent);
}



/*
===============
CG_PlayerSprites

Float sprites over the player's head
===============
*/
static void CG_PlayerSprites(centity_t* cent)
{
	clientInfo_t* cl;

	if (cent->currentState.eFlags & EF_CONNECTION)
	{
		CG_PlayerFloatSprite(cent, cgs.media.connectionShader, NULL);
		return;
	}

	if (cent->currentState.eFlags & EF_TALK)
	{
		CG_PlayerFloatSprite(cent, cgs.media.balloonShader, NULL);
		return;
	}

	if (cent->currentState.eFlags & EF_AWARD_IMPRESSIVE)
	{
		CG_PlayerFloatSprite(cent, cgs.media.medalImpressive, NULL);
		return;
	}

	if (cent->currentState.eFlags & EF_AWARD_EXCELLENT)
	{
		CG_PlayerFloatSprite(cent, cgs.media.medalExcellent, NULL);
		return;
	}

	if (cent->currentState.eFlags & EF_AWARD_GAUNTLET)
	{
		CG_PlayerFloatSprite(cent, cgs.media.medalGauntlet, NULL);
		return;
	}

	if (cent->currentState.eFlags & EF_AWARD_DEFEND)
	{
		CG_PlayerFloatSprite(cent, cgs.media.medalDefend, NULL);
		return;
	}

	if (cent->currentState.eFlags & EF_AWARD_ASSIST)
	{
		CG_PlayerFloatSprite(cent, cgs.media.medalAssist, NULL);
		return;
	}

	if (cent->currentState.eFlags & EF_AWARD_CAP)
	{
		CG_PlayerFloatSprite(cent, cgs.media.medalCapture, NULL);
		return;
	}

	cl = &cgs.clientinfo[ cent->currentState.clientNum ];

	if (cg_drawFriend.integer)
	{
		if (!(cent->currentState.eFlags & EF_DEAD) && cg.snap->ps.persistant[PERS_TEAM] == cl->team && cgs.gametype >= GT_TEAM)
		{
			if (cg_teamFrozenFoe.integer && cgs.osp.gameTypeFreeze && cent->currentState.powerups & (1 << PW_BATTLESUIT) && cent->currentState.weapon == WP_NONE)
			{
				CG_PlayerFloatSprite(cent, cgs.media.frozenFoeTagShader, NULL);
			}
			else
			{
				vec4_t color;
				// Black color for low hp is transparent, skip it
				if (cl->health > 0)
				{
					CG_GetColorForHealth(cl->health, cl->armor, color, NULL);
				}
				else
				{
					VectorCopy(colorRed, color);
				}
				CG_PlayerFloatSprite(cent, cgs.media.friendShader, color);
			}
			return;
		}
	}
}

/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
#define SHADOW_DISTANCE     128
static qboolean CG_PlayerShadow(centity_t* cent, float* shadowPlane)
{
	vec3_t      end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t     trace;
	float       alpha;

	*shadowPlane = 0;

	if (cg_shadows.integer == 0)
	{
		return qfalse;
	}

	// no shadows when invisible
	if (cent->currentState.powerups & (1 << PW_INVIS))
	{
		return qfalse;
	}

	// send a trace down from the player to the ground
	VectorCopy(cent->lerpOrigin, end);
	end[2] -= SHADOW_DISTANCE;

	trap_CM_BoxTrace(&trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID);

	// no shadow if too high
	if (trace.fraction == 1.0 || trace.startsolid || trace.allsolid)
	{
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if (cg_shadows.integer != 1)     // no mark for stencil or projection shadows
	{
		return qtrue;
	}

	// fade the shadow out with height
	alpha = 1.0 - trace.fraction;

	// bk0101022 - hack / FPE - bogus planes?
	//assert( DotProduct( trace.plane.normal, trace.plane.normal ) != 0.0f )

	// add the mark as a temporary, so it goes directly to the renderer
	// without taking a spot in the cg_marks array
	CG_ImpactMark(cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal,
	              cent->pe.legs.yawAngle, alpha, alpha, alpha, 1, qfalse, 24, qtrue, qtrue);

	return qtrue;
}


/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
static void CG_PlayerSplash(centity_t* cent)
{
	vec3_t      start, end;
	trace_t     trace;
	int         contents;
	polyVert_t  verts[4];

	if (!cg_shadows.integer)
	{
		return;
	}

	VectorCopy(cent->lerpOrigin, end);
	end[2] -= 24;

	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = trap_CM_PointContents(end, 0);
	if (!(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA)))
	{
		return;
	}

	VectorCopy(cent->lerpOrigin, start);
	start[2] += 32;

	// if the head isn't out of liquid, don't make a mark
	contents = trap_CM_PointContents(start, 0);
	if (contents & (CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{
		return;
	}

	// trace down to find the surface
	trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA));

	if (trace.fraction == 1.0)
	{
		return;
	}

	// create a mark polygon
	VectorCopy(trace.endpos, verts[0].xyz);
	verts[0].xyz[0] -= 32;
	verts[0].xyz[1] -= 32;
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorCopy(trace.endpos, verts[1].xyz);
	verts[1].xyz[0] -= 32;
	verts[1].xyz[1] += 32;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorCopy(trace.endpos, verts[2].xyz);
	verts[2].xyz[0] += 32;
	verts[2].xyz[1] += 32;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorCopy(trace.endpos, verts[3].xyz);
	verts[3].xyz[0] += 32;
	verts[3].xyz[1] -= 32;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene(cgs.media.wakeMarkShader, 4, verts);
}



void CG_AddRefEntityFrozenPlayer(refEntity_t* ent, entityState_t* state, int team)
{
	const clientInfo_t* ci;

	if (state->clientNum >= MAX_CLIENTS) return;
	ci = &cgs.clientinfo[state->clientNum];

	ent->shaderRGBA[0] = ci->colors.frozen[0] * 255;
	ent->shaderRGBA[1] = ci->colors.frozen[1] * 255;
	ent->shaderRGBA[2] = ci->colors.frozen[2] * 255;

	ent->shaderRGBA[3] = 255;
	ent->customShader = cgs.media.frozenShader;
	trap_R_AddRefEntityToScene(ent);
}


/*
===============
CG_AddRefEntityWithPowerups

Adds a piece with modifications or duplications for powerups
Also called by CG_Missile for quad rockets, but nobody can tell...
===============
*/
void CG_AddRefEntityWithPowerups(refEntity_t* ent, entityState_t* state, int team)
{

	if (state->powerups & (1 << PW_INVIS))
	{
		ent->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene(ent);
	}
	else
	{
		/*
		if ( state->eFlags & EF_KAMIKAZE ) {
		    if (team == TEAM_BLUE)
		        ent->customShader = cgs.media.blueKamikazeShader;
		    else
		        ent->customShader = cgs.media.redKamikazeShader;
		    trap_R_AddRefEntityToScene( ent );
		}
		else {*/
		trap_R_AddRefEntityToScene(ent);
		//}

		if (state->powerups & (1 << PW_QUAD))
		{
			if (team == TEAM_RED)
				ent->customShader = cgs.media.redQuadShader;
			else
				ent->customShader = cgs.media.quadShader;
			trap_R_AddRefEntityToScene(ent);
		}
		if (state->powerups & (1 << PW_REGEN))
		{
			if (((cg.time / 100) % 10) == 1)
			{
				ent->customShader = cgs.media.regenShader;
				trap_R_AddRefEntityToScene(ent);
			}
		}

		if (state->powerups & (1 << PW_BATTLESUIT))
		{
			if (state->weapon == WP_NONE)
			{
				CG_AddRefEntityFrozenPlayer(ent, state, team);
			}
			else
			{
				ent->customShader = cgs.media.battleSuitShader;
				trap_R_AddRefEntityToScene(ent);
			}
		}
	}
}

/*
=================
CG_LightVerts
=================
*/
int CG_LightVerts(vec3_t normal, int numVerts, polyVert_t* verts)
{
	int             i, j;
	float           incoming;
	vec3_t          ambientLight;
	vec3_t          lightDir;
	vec3_t          directedLight;

	trap_R_LightForPoint(verts[0].xyz, ambientLight, directedLight, lightDir);

	for (i = 0; i < numVerts; i++)
	{
		incoming = DotProduct(normal, lightDir);
		if (incoming <= 0)
		{
			verts[i].modulate[0] = ambientLight[0];
			verts[i].modulate[1] = ambientLight[1];
			verts[i].modulate[2] = ambientLight[2];
			verts[i].modulate[3] = 255;
			continue;
		}
		j = (ambientLight[0] + incoming * directedLight[0]);
		if (j > 255)
		{
			j = 255;
		}
		verts[i].modulate[0] = j;

		j = (ambientLight[1] + incoming * directedLight[1]);
		if (j > 255)
		{
			j = 255;
		}
		verts[i].modulate[1] = j;

		j = (ambientLight[2] + incoming * directedLight[2]);
		if (j > 255)
		{
			j = 255;
		}
		verts[i].modulate[2] = j;

		verts[i].modulate[3] = 255;
	}
	return qtrue;
}

/*
===============
CG_AddHitBox
===============
*/
void CG_AddHitBox(centity_t* cent, team_t team)
{
	polyVert_t verts[4];
	clientInfo_t* ci;
	int i;
	vec3_t mins = {-15, -15, -24};
	vec3_t maxs = {15, 15, 32};
	float extx, exty, extz;
	vec3_t corners[8];

	if (!cg_drawHitBox.integer || !cg.demoPlayback)
	{
		return;
	}

	// don't draw it if it's us in first-person
	if (cent->currentState.number == cg.predictedPlayerState.clientNum &&
	        !cg.renderingThirdPerson)
	{
		return;
	}

	// don't draw it for dead players
	if (cent->currentState.eFlags & EF_DEAD)
	{
		return;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR
	        && cg.snap->ps.persistant[PERS_TEAM] != TEAM_FREE
	        && cg.snap->ps.persistant[PERS_TEAM] == team)
	{
		return;
	}

	// if they don't exist, forget it
	if (!cgs.osp.hboxShader || !cgs.osp.hboxShader_nocull)
	{
		return;
	}

	// get the player's client info
	ci = &cgs.clientinfo[cent->currentState.clientNum];

	// if it's us
	if (cent->currentState.number == cg.predictedPlayerState.clientNum)
	{
		// use the view height
		maxs[2] = cg.predictedPlayerState.viewheight + 6;
	}
	else
	{
		int x, zd, zu;

		// otherwise grab the encoded bounding box
		x = (cent->currentState.solid & 255);
		zd = ((cent->currentState.solid >> 8) & 255);
		zu = ((cent->currentState.solid >> 16) & 255) - 32;

		mins[0] = mins[1] = -x;
		maxs[0] = maxs[1] = x;
		mins[2] = -zd;
		maxs[2] = zu;
	}

	// get the extents (size)
	extx = maxs[0] - mins[0];
	exty = maxs[1] - mins[1];
	extz = maxs[2] - mins[2];


	// set the polygon's texture coordinates
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;

	// set the polygon's vertex colors
	if (ci->team == TEAM_RED)
	{
		for (i = 0; i < 4; i++)
		{
			verts[i].modulate[0] = 160;
			verts[i].modulate[1] = 0;
			verts[i].modulate[2] = 0;
			verts[i].modulate[3] = 255;
		}
	}
	else if (ci->team == TEAM_BLUE)
	{
		for (i = 0; i < 4; i++)
		{
			verts[i].modulate[0] = 0;
			verts[i].modulate[1] = 0;
			verts[i].modulate[2] = 192;
			verts[i].modulate[3] = 255;
		}
	}
	else
	{
		for (i = 0; i < 4; i++)
		{
			verts[i].modulate[0] = 0;
			verts[i].modulate[1] = 128;
			verts[i].modulate[2] = 0;
			verts[i].modulate[3] = 255;
		}
	}

	VectorAdd(cent->lerpOrigin, maxs, corners[3]);

	VectorCopy(corners[3], corners[2]);
	corners[2][0] -= extx;

	VectorCopy(corners[2], corners[1]);
	corners[1][1] -= exty;

	VectorCopy(corners[1], corners[0]);
	corners[0][0] += extx;

	for (i = 0; i < 4; i++)
	{
		VectorCopy(corners[i], corners[i + 4]);
		corners[i + 4][2] -= extz;
	}

	// top
	VectorCopy(corners[0], verts[0].xyz);
	VectorCopy(corners[1], verts[1].xyz);
	VectorCopy(corners[2], verts[2].xyz);
	VectorCopy(corners[3], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.osp.hboxShader, 4, verts);

	// bottom
	VectorCopy(corners[7], verts[0].xyz);
	VectorCopy(corners[6], verts[1].xyz);
	VectorCopy(corners[5], verts[2].xyz);
	VectorCopy(corners[4], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.osp.hboxShader, 4, verts);

	// top side
	VectorCopy(corners[3], verts[0].xyz);
	VectorCopy(corners[2], verts[1].xyz);
	VectorCopy(corners[6], verts[2].xyz);
	VectorCopy(corners[7], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.osp.hboxShader_nocull, 4, verts);

	// left side
	VectorCopy(corners[2], verts[0].xyz);
	VectorCopy(corners[1], verts[1].xyz);
	VectorCopy(corners[5], verts[2].xyz);
	VectorCopy(corners[6], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.osp.hboxShader_nocull, 4, verts);

	// right side
	VectorCopy(corners[0], verts[0].xyz);
	VectorCopy(corners[3], verts[1].xyz);
	VectorCopy(corners[7], verts[2].xyz);
	VectorCopy(corners[4], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.osp.hboxShader_nocull, 4, verts);

	// bottom side
	VectorCopy(corners[1], verts[0].xyz);
	VectorCopy(corners[0], verts[1].xyz);
	VectorCopy(corners[4], verts[2].xyz);
	VectorCopy(corners[5], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.osp.hboxShader_nocull, 4, verts);
}

/*
===============
CG_Player
===============
*/
void CG_Player(centity_t* cent)
{
	clientInfo_t*    ci;
	refEntity_t     legs;
	refEntity_t     torso;
	refEntity_t     head;
	int             clientNum;
	int             renderfx;
	qboolean        shadow;
	float           shadowPlane;
	qboolean        paintItBlack;
	float           paintBlackLevel;

	if (cg.clientNum == cent->currentState.clientNum && (global_viewlistFirstOption || cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR))
	{
		return;
	}


	clientNum = cent->currentState.clientNum;

	if (clientNum < 0 || clientNum >= MAX_CLIENTS)
	{
		CG_Error("Bad clientNum on player entity");
	}
	ci = &cgs.clientinfo[ clientNum ];

	if (!ci->infoValid)
	{
		return;
	}
	if (!ci->legsModel && !ci->torsoModel && !ci->headModel)
	{
		return;
	}

	renderfx = 0;

	if (cent->currentState.number == cg.snap->ps.clientNum)
	{
		if (!cg.renderingThirdPerson)
		{
			renderfx = RF_THIRD_PERSON;         // only draw in mirrors
		}
		else
		{
			if (cg_cameraMode.integer)
			{
				return;
			}
		}
	}
	memset(&legs, 0, sizeof(legs));
	memset(&torso, 0, sizeof(torso));
	memset(&head, 0, sizeof(head));

	CG_PlayerAngles(cent, legs.axis, torso.axis, head.axis);
	CG_PlayerAnimation(cent, &legs.oldframe, &legs.frame, &legs.backlerp, &torso.oldframe, &torso.frame, &torso.backlerp);
	CG_PlayerPowerups(cent);
	CG_PlayerSprites(cent);

	// add the shadow
	shadow = CG_PlayerShadow(cent, &shadowPlane);

	// add a water splash if partially in and out of water
	CG_PlayerSplash(cent);

	if (cg_shadows.integer == 3 && shadow)
	{
		renderfx |= RF_SHADOW_PLANE;
	}
	renderfx |= RF_LIGHTING_ORIGIN;         // use the same origin for all
	//
	paintItBlack = cg_deadBodyBlack.integer && (
	                   cent->currentState.eFlags & EF_DEAD || //dead
	                   (cgs.osp.gameTypeFreeze && (cent->currentState.weapon == WP_NONE) && (cent->currentState.powerups & (1 << PW_BATTLESUIT)))// or frozen
	               );
	if (paintItBlack)
	{
		switch (cg_deadBodyBlack.integer)
		{
			case 1:
				paintBlackLevel = 165;
				break;
			case 2:
				paintBlackLevel = 110;
				break;
			default:
			case 3:
				paintBlackLevel = 55;
				break;
		}
	}

	if ((cent->currentState.eFlags & EF_DEAD) && cg_deadBodyInvisible.integer)
	{
		if (cent->currentState.weapon == WP_NONE)   // show player model with weapon during intermission vote - don't make it invisible
		{
			legs.customShader = cgs.media.invisShader;
			torso.customShader = cgs.media.invisShader;
			head.customShader = cgs.media.invisShader;
		}
	}
	//
	// add the legs
	//
	legs.hModel = ci->legsModel;
	legs.customSkin = ci->legsSkin;

	VectorCopy(cent->lerpOrigin, legs.origin);

	VectorCopy(cent->lerpOrigin, legs.lightingOrigin);
	legs.shadowPlane = shadowPlane;
	legs.renderfx = renderfx;
	VectorCopy(legs.origin, legs.oldorigin);    // don't positionally lerp at all

	if (ci->isPmSkin)
	{
		float tmpf;
		const float maxf = (float)MAX_QINT;

		tmpf = 255.0 * ci->colors.legs[0];
		if (tmpf >= maxf)
		{
			legs.shaderRGBA[0] = -1 * (tmpf - (float)MAX_QINT);
		}
		else
		{
			legs.shaderRGBA[0] = tmpf;
		}

		tmpf = 255.0 * ci->colors.legs[1];
		if (tmpf >= maxf)
		{
			legs.shaderRGBA[1] = -1 * (tmpf - (float)MAX_QINT);
		}
		else
		{
			legs.shaderRGBA[1] = tmpf;
		}

		tmpf = 255.0 * ci->colors.legs[2];
		if (tmpf >= maxf)
		{
			legs.shaderRGBA[2] = -1 * (tmpf - (float)MAX_QINT);
		}
		else
		{
			legs.shaderRGBA[2] = tmpf;
		}
		legs.shaderRGBA[3] = 0xff;
	}

	if (paintItBlack)
	{
		legs.shaderRGBA[0] = legs.shaderRGBA[1] = legs.shaderRGBA[2] = paintBlackLevel;
	}

	CG_AddRefEntityWithPowerups(&legs, &cent->currentState, ci->team);

	//
	// add the torso
	//
	torso.hModel = ci->torsoModel;
	torso.customSkin = ci->torsoSkin;
	VectorCopy(cent->lerpOrigin, torso.lightingOrigin);
	CG_PositionRotatedEntityOnTag(&torso, &legs, ci->legsModel, "tag_torso");
	torso.shadowPlane = shadowPlane;
	torso.renderfx = renderfx;

	if (ci->isPmSkin)
	{
		float tmpf;
		const float maxf = (float)MAX_QINT;

		tmpf = 255.0 * ci->colors.torso[0];
		if (tmpf >= maxf)
		{
			torso.shaderRGBA[0] = -1 * (tmpf - (float)MAX_QINT);
		}
		else
		{
			torso.shaderRGBA[0] = tmpf;
		}

		tmpf = 255.0 * ci->colors.torso[1];
		if (tmpf >= maxf)
		{
			torso.shaderRGBA[1] = -1 * (tmpf - (float)MAX_QINT);
		}
		else
		{
			torso.shaderRGBA[1] = tmpf;
		}

		tmpf = 255.0 * ci->colors.torso[2];
		if (tmpf >= maxf)
		{
			torso.shaderRGBA[2] = -1 * (tmpf - (float)MAX_QINT);
		}
		else
		{
			torso.shaderRGBA[2] = tmpf;
		}
		torso.shaderRGBA[3] = 0xff;
	}

	if (paintItBlack)
	{
		torso.shaderRGBA[0] = torso.shaderRGBA[1] = torso.shaderRGBA[2] = paintBlackLevel;
	}

	CG_AddRefEntityWithPowerups(&torso, &cent->currentState, ci->team);

	//
	// add the head
	//
	head.hModel = ci->headModel;
	head.customSkin = ci->headSkin;
	VectorCopy(cent->lerpOrigin, head.lightingOrigin);
	CG_PositionRotatedEntityOnTag(&head, &torso, ci->torsoModel, "tag_head");
	head.shadowPlane = shadowPlane;
	head.renderfx = renderfx;

	if (ci->isPmSkin)
	{
		float tmpf;
		const float maxf = (float)MAX_QINT;

		tmpf = 255.0 * ci->colors.head[0];
		if (tmpf >= maxf)
		{
			head.shaderRGBA[0] = -1 * (tmpf - (float)MAX_QINT);
		}
		else
		{
			head.shaderRGBA[0] = tmpf;
		}

		tmpf = 255.0 * ci->colors.head[1];
		if (tmpf >= maxf)
		{
			head.shaderRGBA[1] = -1 * (tmpf - (float)MAX_QINT);
		}
		else
		{
			head.shaderRGBA[1] = tmpf;
		}

		tmpf = 255.0 * ci->colors.head[2];
		if (tmpf >= maxf)
		{
			head.shaderRGBA[2] = -1 * (tmpf - (float)MAX_QINT);
		}
		else
		{
			head.shaderRGBA[2] = tmpf;
		}
		head.shaderRGBA[3] = 0xff;
	}

	if (paintItBlack)
	{
		head.shaderRGBA[0] = head.shaderRGBA[1] = head.shaderRGBA[2] = paintBlackLevel;
	}

	if (cgs.osp.gameTypeFreeze != 0)
	{
		CG_BreathPuffs(cent, &head);
	}
	CG_AddRefEntityWithPowerups(&head, &cent->currentState, ci->team);
	CG_AddPlayerWeapon(&torso, NULL, cent, ci->team);

	CG_AddHitBox(cent, ci->team);

}


//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity(centity_t* cent)
{
	cent->errorTime = -99999;       // guarantee no error decay added
	cent->extrapolated = qfalse;

	CG_ClearLerpFrame(&cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.legs, cent->currentState.legsAnim);
	CG_ClearLerpFrame(&cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.torso, cent->currentState.torsoAnim);

	BG_EvaluateTrajectory(&cent->currentState.pos, cg.time, cent->lerpOrigin);
	BG_EvaluateTrajectory(&cent->currentState.apos, cg.time, cent->lerpAngles);

	VectorCopy(cent->lerpOrigin, cent->rawOrigin);
	VectorCopy(cent->lerpAngles, cent->rawAngles);

	memset(&cent->pe.legs, 0, sizeof(cent->pe.legs));
	cent->pe.legs.yawAngle = cent->rawAngles[YAW];
	cent->pe.legs.yawing = qfalse;
	cent->pe.legs.pitchAngle = 0;
	cent->pe.legs.pitching = qfalse;

	memset(&cent->pe.torso, 0, sizeof(cent->pe.legs));
	cent->pe.torso.yawAngle = cent->rawAngles[YAW];
	cent->pe.torso.yawing = qfalse;
	cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
	cent->pe.torso.pitching = qfalse;

	if (cg_debugPosition.integer)
	{
		CG_Printf("%i ResetPlayerEntity yaw=%i\n", cent->currentState.number, cent->pe.torso.yawAngle);
	}
}





