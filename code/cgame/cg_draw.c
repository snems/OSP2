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
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"
#include "cg_superhud.h"

int drawTeamOverlayModificationCount = -1;

int sortedTeamPlayers[TEAM_MAXOVERLAY];
int numSortedTeamPlayers;

char systemChat[256];
char teamChat1[256];
char teamChat2[256];

int frameTime = 0;

int wstatsWndId;
qboolean wstatsEnabled;
int statsInfo[24];
OSP_SlidingPrintContext_t ospPrintContext[16];

const char* monthName[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const char* dayOfWeekName[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

/*
==============
CG_DrawField

Draws large numbers for status bar and powerups
==============
*/
void CG_DrawField(int x, int y, int width, int value, int w, int h)
{
	char    num[16], *ptr;
	int     l;
	int     frame;

	if (width < 1)
	{
		return;
	}

	// draw number string
	if (width > 5)
	{
		width = 5;
	}

	switch (width)
	{
		case 1:
			value = value > 9 ? 9 : value;
			value = value < 0 ? 0 : value;
			break;
		case 2:
			value = value > 99 ? 99 : value;
			value = value < -9 ? -9 : value;
			break;
		case 3:
			value = value > 999 ? 999 : value;
			value = value < -99 ? -99 : value;
			break;
		case 4:
			value = value > 9999 ? 9999 : value;
			value = value < -999 ? -999 : value;
			break;
	}

	Com_sprintf(num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + w * (width - l);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr - '0';

		CG_DrawPicOld(x, y, w, h, cgs.media.numberShaders[frame]);
		x += w;
		ptr++;
		l--;
	}
}

void CG_OSPDrawField(int x, int y, int val)
{
	if (val >= 0 && val < 10)
	{
		CG_DrawField(x, y, 1, val, 32, 48);
	}
	else if ((val > -9 && val < 0) || (val >= 10 && val < 100))
	{
		CG_DrawField(x, y, 2, val, 32, 48);
	}
	else
	{
		CG_DrawField(x, y, 3, val, 32, 48);
	}
	return;
}

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles)
{
	refdef_t        refdef;
	refEntity_t     ent;

	if (!cg_draw3dIcons.integer || !cg_drawIcons.integer)
	{
		return;
	}

	CG_AdjustFrom640_Old(&x, &y, &w, &h, qfalse);

	memset(&refdef, 0, sizeof(refdef));

	memset(&ent, 0, sizeof(ent));
	AnglesToAxis(angles, ent.axis);
	VectorCopy(origin, ent.origin);
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;     // no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear(refdef.viewaxis);

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene(&ent);
	trap_R_RenderScene(&refdef);
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead(float x, float y, float w, float h, int clientNum, vec3_t headAngles)
{
	clipHandle_t    cm;
	clientInfo_t*    ci;
	float           len;
	vec3_t          origin;
	vec3_t          mins, maxs;

	ci = &cgs.clientinfo[ clientNum ];

	if (cg_draw3dIcons.integer)
	{
		cm = ci->headModel;

		if (cm)
		{
			// offset the origin y and z to center the head
			trap_R_ModelBounds(cm, mins, maxs);

			origin[2] = -0.5 * (mins[2] + maxs[2]);
			origin[1] = 0.5 * (mins[1] + maxs[1]);

			// calculate distance so the head nearly fills the box
			// assume heads are taller than wide
			len = 0.7 * (maxs[2] - mins[2]);
			origin[0] = len / 0.268;    // len / tan( fov/2 )

			VectorAdd(origin, ci->headOffset, origin);
			// allow per-model tweaking
			CG_Draw3DModel(x, y, w, h, ci->headModel, ci->headSkin, origin, headAngles);

		}
	}
	else if (cg_drawIcons.integer)
	{
		CG_DrawPicOld(x, y, w, h, ci->modelIcon);
	}

}

/*
================
CG_DrawFlagModel

Used for both the status bar and the scoreboard
================
*/
void CG_DrawFlagModel(float x, float y, float w, float h, int team, qboolean force2D)
{
	qhandle_t       cm;
	float           len;
	vec3_t          origin, angles;
	vec3_t          mins, maxs;
	qhandle_t       handle;

	if (!force2D && cg_draw3dIcons.integer)
	{

		VectorClear(angles);

		cm = cgs.media.redFlagModel;

		// offset the origin y and z to center the flag
		trap_R_ModelBounds(cm, mins, maxs);

		origin[2] = -0.5 * (mins[2] + maxs[2]);
		origin[1] = 0.5 * (mins[1] + maxs[1]);

		// calculate distance so the flag nearly fills the box
		// assume heads are taller than wide
		len = 0.5 * (maxs[2] - mins[2]);
		origin[0] = len / 0.268;    // len / tan( fov/2 )

		angles[YAW] = 60 * sin(cg.time / 2000.0);;

		if (team == TEAM_RED)
		{
			handle = cgs.media.redFlagModel;
		}
		else if (team == TEAM_BLUE)
		{
			handle = cgs.media.blueFlagModel;
		}
		else if (team == TEAM_FREE)
		{
			handle = cgs.media.neutralFlagModel;
		}
		else
		{
			return;
		}
		CG_Draw3DModel(x, y, w, h, handle, 0, origin, angles);
	}
	else if (cg_drawIcons.integer)
	{
		gitem_t* item;

		if (team == TEAM_RED)
		{
			item = BG_FindItemForPowerup(PW_REDFLAG);
		}
		else if (team == TEAM_BLUE)
		{
			item = BG_FindItemForPowerup(PW_BLUEFLAG);
		}
		else if (team == TEAM_FREE)
		{
			item = BG_FindItemForPowerup(PW_NEUTRALFLAG);
		}
		else
		{
			return;
		}
		if (item)
		{
			CG_DrawPicOld(x, y, w, h, cg_items[ ITEM_INDEX(item) ].icon);
		}
	}
}

/*
================
CG_DrawStatusBarHead

================
*/

static void CG_DrawStatusBarHead(float x)
{
	vec3_t      angles;
	float       size, stretch;
	float       frac;

	VectorClear(angles);

	if (cg.damageTime && cg.time - cg.damageTime < DAMAGE_TIME)
	{
		frac = (float)(cg.time - cg.damageTime) / DAMAGE_TIME;
		size = ICON_SIZE * 1.25 * (1.5 - frac * 0.5);

		stretch = size - ICON_SIZE * 1.25;
		// kick in the direction of damage
		x -= stretch * 0.5 + cg.damageX * stretch * 0.5;

		cg.headStartYaw = 180 + cg.damageX * 45;

		cg.headEndYaw = 180 + 20 * cos(crandom() * M_PI);
		cg.headEndPitch = 5 * cos(crandom() * M_PI);

		cg.headStartTime = cg.time;
		cg.headEndTime = cg.time + 100 + random() * 2000;
	}
	else
	{
		if (cg.time >= cg.headEndTime)
		{
			// select a new head angle
			cg.headStartYaw = cg.headEndYaw;
			cg.headStartPitch = cg.headEndPitch;
			cg.headStartTime = cg.headEndTime;
			cg.headEndTime = cg.time + 100 + random() * 2000;

			cg.headEndYaw = 180 + 20 * cos(crandom() * M_PI);
			cg.headEndPitch = 5 * cos(crandom() * M_PI);
		}

		size = ICON_SIZE * 1.25;
	}

	// if the server was frozen for a while we may have a bad head start time
	if (cg.headStartTime > cg.time)
	{
		cg.headStartTime = cg.time;
	}

	frac = (cg.time - cg.headStartTime) / (float)(cg.headEndTime - cg.headStartTime);
	frac = frac * frac * (3 - 2 * frac);
	angles[YAW] = cg.headStartYaw + (cg.headEndYaw - cg.headStartYaw) * frac;
	angles[PITCH] = cg.headStartPitch + (cg.headEndPitch - cg.headStartPitch) * frac;

	CG_DrawHead(x, 480 - size, size, size,
	            cg.snap->ps.clientNum, angles);
}

/*
================
CG_DrawStatusBarFlag

================
*/
static void CG_DrawStatusBarFlag(float x, int team)
{
	CG_DrawFlagModel(x, 480 - ICON_SIZE, ICON_SIZE, ICON_SIZE, team, qfalse);
}

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team)
{
	vec4_t      hcolor;

	hcolor[3] = alpha;
	if (team == TEAM_RED)
	{
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	}
	else if (team == TEAM_BLUE)
	{
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	}
	else
	{
		return;
	}
	trap_R_SetColor(hcolor);
	CG_DrawPicOld(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);
}

/*
================
CG_DrawStatusBar

================
*/
void CG_DrawStatusBar(void)
{
	int         color;
	centity_t*   cent;
	playerState_t*   ps;
	int         value;
	vec4_t      hcolor;
	vec3_t      angles;
	vec3_t      origin;
	static float colors[4][4] =
	{
//		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1} };
		{ 1.0f, 0.69f, 0.0f, 1.0f },    // normal
		{ 1.0f, 0.2f, 0.2f, 1.0f },     // low health
		{ 0.5f, 0.5f, 0.5f, 1.0f },     // weapon firing
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	};   // health > 100

	if (cg_drawStatus.integer == 0)
	{
		return;
	}

	// draw the team background
	CG_DrawTeamBackground(0, 420, 640, 60, 0.33f, cg.snap->ps.persistant[PERS_TEAM]);

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	VectorClear(angles);

	// draw any 3D icons first, so the changes back to 2D are minimized
	if (cg_draw3dIcons.integer && cg_drawIcons.integer &&
	        cent->currentState.weapon && cg_weapons[ cent->currentState.weapon ].ammoModel)
	{
		origin[0] = 70;
		origin[1] = 0;
		origin[2] = 0;
		angles[YAW] = 90 + 20 * sin(cg.time / 1000.0);
		CG_Draw3DModel(Q3_CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE,
		               cg_weapons[ cent->currentState.weapon ].ammoModel, 0, origin, angles);
	}

	CG_DrawStatusBarHead(185 + Q3_CHAR_WIDTH * 3 + TEXT_ICON_SPACE);

	if (cg.predictedPlayerState.powerups[PW_REDFLAG])
	{
		CG_DrawStatusBarFlag(185 + Q3_CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_RED);
	}
	else if (cg.predictedPlayerState.powerups[PW_BLUEFLAG])
	{
		CG_DrawStatusBarFlag(185 + Q3_CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_BLUE);
	}
	else if (cg.predictedPlayerState.powerups[PW_NEUTRALFLAG])
	{
		CG_DrawStatusBarFlag(185 + Q3_CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_FREE);
	}

	if (cg_draw3dIcons.integer && cg_drawIcons.integer &&
	        ps->stats[ STAT_ARMOR ])
	{
		origin[0] = 90;
		origin[1] = 0;
		origin[2] = -10;
		angles[YAW] = (cg.time & 2047) * 360 / 2048.0;
		CG_Draw3DModel(370 + Q3_CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 432.0, ICON_SIZE, ICON_SIZE,
		               cgs.media.armorModel[ps->stats[STAT_ARMOR_TYPE]], 0, origin, angles);
	}
	//
	// ammo
	//
	if (cent->currentState.weapon)
	{
		value = ps->ammo[cent->currentState.weapon];
		if (value > -1)
		{
			if (cg.predictedPlayerState.weaponstate == WEAPON_FIRING
			        && cg.predictedPlayerState.weaponTime > 100)
			{
				// draw as dark grey when reloading
				color = 2;  // dark grey
			}
			else
			{
				if (value > 0)
				{
					color = 0;  // green
				}
				else
				{
					color = 1;  // red
				}
			}
			trap_R_SetColor(colors[color]);

			CG_DrawField(0, 432, 3, value, 32, 48);
			trap_R_SetColor(NULL);

			// if we didn't draw a 3D icon, draw a 2D icon for ammo
			if (!cg_draw3dIcons.integer && cg_drawIcons.integer)
			{
				qhandle_t   icon;

				icon = cg_weapons[ cg.predictedPlayerState.weapon ].ammoIcon;
				if (icon)
				{
					CG_DrawPicOld(Q3_CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE, icon);
				}
			}
		}
	}

	//
	// health
	//
	value = ps->stats[STAT_HEALTH];
	if (value > 100)
	{
		trap_R_SetColor(colors[3]);          // white
	}
	else if (value > 25)
	{
		trap_R_SetColor(colors[0]);      // green
	}
	else if (value > 0)
	{
		color = (cg.time >> 8) & 1; // flash
		trap_R_SetColor(colors[color]);
	}
	else
	{
		trap_R_SetColor(colors[1]);      // red
	}

	// stretch the health up when taking damage
	CG_DrawField(185, 432, 3, value, 32, 48);
	CG_ColorForHealth(hcolor, NULL);
	trap_R_SetColor(hcolor);


	//
	// armor
	//
	value = ps->stats[STAT_ARMOR];
	if (value > 0)
	{
		trap_R_SetColor(colors[0]);
		CG_DrawField(370, 432, 3, value, 32, 48);
		trap_R_SetColor(NULL);
		// if we didn't draw a 3D icon, draw a 2D icon for armor
		if (!cg_draw3dIcons.integer && cg_drawIcons.integer)
		{
			CG_DrawPicOld(370 + Q3_CHAR_WIDTH * 3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE, cgs.media.armorIcon[ps->stats[STAT_ARMOR_TYPE]]);
		}

	}
}

/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

/*
================
CG_DrawAttacker

================
*/
float CG_DrawAttacker(float y)
{
	int         t;
	float       size;
	vec3_t      angles;
	const char*  info;
	const char*  name;
	int         clientNum;

	if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
	{
		return y;
	}

	if (!cg.attackerTime)
	{
		return y;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if (clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum)
	{
		return y;
	}

	t = cg.time - cg.attackerTime;
	if (t > ATTACKER_HEAD_TIME)
	{
		cg.attackerTime = 0;
		return y;
	}

	size = ICON_SIZE * 1.25;

	angles[PITCH] = 0;
	angles[YAW] = 180;
	angles[ROLL] = 0;
	CG_DrawHead(640 - size, y, size, size, clientNum, angles);

	info = CG_ConfigString(CS_PLAYERS + clientNum);
	name = Info_ValueForKey(info, "n");
	y += size;
	CG_DrawBigString(SCREEN_WIDTH - 1, y, name, 0.5f, DS_HRIGHT | DS_SHADOW, 0);

	return y + BIGCHAR_HEIGHT + 2;
}

/*
==================
CG_DrawSnapshot
==================
*/
float CG_DrawSnapshot(float y)
{
	char* s = va("time:%i snap:%i cmd:%i", cg.snap->serverTime, cg.latestSnapshotNum, cgs.serverCommandSequence);

	CG_DrawBigString(SCREEN_WIDTH - 1, y + 2, s, 1.0f, DS_HRIGHT | DS_SHADOW, 0);

	return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
CG_DrawFPS
==================
*/
#define FPS_FRAMES  4
float CG_DrawFPS(float y)
{
	char*        s;
	static int  previousTimes[FPS_FRAMES];
	static int  index;
	int     i, total;
	int     fps;
	static  int previous;
	int     t;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if (index > FPS_FRAMES)
	{
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for (i = 0 ; i < FPS_FRAMES ; i++)
		{
			total += previousTimes[i];
		}
		if (!total)
		{
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va("%ifps", fps);

		CG_DrawBigString(SCREEN_WIDTH - 5, y + 2, s, 1.0f, DS_HRIGHT | DS_SHADOW, 0);
	}

	return y + BIGCHAR_HEIGHT + 4;
}

/*
=================
CG_DrawTimer
=================
*/
float CG_DrawTimer(float y)
{
	char*        s;
	int         mins, seconds, tens;
	int         msec;
	int         w, h;
	const float   color[4] = {1.0, 1.0, 1.0, 1.0};

	msec = cg.time - cgs.levelStartTime;

	if (msec < 0) msec *= -1;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va("%i:%i%i", mins, tens, seconds);

	CG_OSPGetClientFontSize(&cf_timer, &w, &h);

	CG_FontSelect(0);
	CG_OSPDrawString(SCREEN_WIDTH - 5, y + 2, s, color, w, h, SCREEN_WIDTH, DS_HRIGHT | DS_SHADOW, NULL);

	return y + h + 4;
}

void CG_DrawTimer2(void)
{
	char*        s;
	int         mins, seconds, tens;
	int         msec;
	int         w, h;
	const float   color[4] = {1.0, 1.0, 1.0, 1.0};

	msec = cg.time - cgs.levelStartTime;

	if (msec < 0) msec *= -1;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;


	s = va("%i:%i%i", mins, tens, seconds);

	CG_OSPGetClientFontSize(&cf_timer, &w, &h);
	CG_FontSelect(0);
	CG_OSPDrawString(SCREEN_WIDTH / 2.0f, 2, s, color, w, h, SCREEN_WIDTH, DS_HCENTER | DS_SHADOW, NULL);
}


/*
=================
CG_DrawTeamOverlay
=================
*/

static float CG_DrawTeamOverlay(float y, qboolean right, qboolean upper)
{
	int x, w, h, xx;
	int i, j, len;
	const char* p;
	vec4_t      hcolor;
	int pwidth, lwidth;
	int plyrs;
	char st[16];
	clientInfo_t* ci;
	gitem_t* item;
	int ret_y, count;

	if (!cg_drawTeamOverlay.integer)
	{
		return y;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE)
	{
		return y; // Not on any team
	}

	plyrs = 0;

	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
	for (i = 0; i < count; i++)
	{
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if (ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM])
		{
			plyrs++;
			len = CG_DrawStrlen(ci->name);
			if (len > pwidth)
				pwidth = len;
		}
	}

	if (!plyrs)
		return y;

	if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;

	// max location name width
	lwidth = 0;
	for (i = 1; i < MAX_LOCATIONS; i++)
	{
		p = CG_ConfigString(CS_LOCATIONS + i);
		if (p && *p)
		{
			len = CG_DrawStrlen(p);
			if (len > lwidth)
				lwidth = len;
		}
	}

	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;

	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;

	if (right)
		x = 640 - w;
	else
		x = 0;

	h = plyrs * TINYCHAR_HEIGHT;

	if (upper)
	{
		ret_y = y + h;
	}
	else
	{
		y -= h;
		ret_y = y;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		hcolor[0] = 1.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 0.0f;
		hcolor[3] = 0.33f;
	}
	else     // if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
	{
		hcolor[0] = 0.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 1.0f;
		hcolor[3] = 0.33f;
	}
	trap_R_SetColor(hcolor);
	CG_DrawPicOld(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);

	for (i = 0; i < count; i++)
	{
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if (ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM])
		{

			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

			xx = x + TINYCHAR_WIDTH;

			CG_DrawStringExt(xx, y,
			                 ci->name, hcolor, qfalse, qfalse,
			                 TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);

			if (lwidth)
			{
				p = CG_ConfigString(CS_LOCATIONS + ci->location);
				if (!p || !*p)
					p = "unknown";
				len = CG_DrawStrlen(p);
				if (len > lwidth)
					len = lwidth;

//				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth +
//					((lwidth/2 - len/2) * TINYCHAR_WIDTH);
				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
				CG_DrawStringExt(xx, y,
				                 p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
				                 TEAM_OVERLAY_MAXLOCATION_WIDTH);
			}

			CG_GetColorForHealth(ci->health, ci->armor, hcolor, NULL);

			Com_sprintf(st, sizeof(st), "%3i %3i", ci->health, ci->armor);

			xx = x + TINYCHAR_WIDTH * 3 +
			     TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

			CG_DrawStringExt(xx, y,
			                 st, hcolor, qfalse, qfalse,
			                 TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);

			// draw weapon icon
			xx += TINYCHAR_WIDTH * 3;

			if (cg_weapons[ci->curWeapon].weaponIcon)
			{
				CG_DrawPicOld(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
				              cg_weapons[ci->curWeapon].weaponIcon);
			}
			else
			{
				CG_DrawPicOld(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
				              cgs.media.deferShader);
			}

			// Draw powerup icons
			if (right)
			{
				xx = x;
			}
			else
			{
				xx = x + w - TINYCHAR_WIDTH;
			}
			for (j = 0; j <= PW_NUM_POWERUPS; j++)
			{
				if (ci->powerups & (1 << j))
				{

					item = BG_FindItemForPowerup(j);

					if (item)
					{
						CG_DrawPicOld(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
						              trap_R_RegisterShader(item->icon));
						if (right)
						{
							xx -= TINYCHAR_WIDTH;
						}
						else
						{
							xx += TINYCHAR_WIDTH;
						}
					}
				}
			}

			y += TINYCHAR_HEIGHT;
		}
	}

	return ret_y;
//#endif
}


/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight(void)
{
	float   y;

	y = 0;

	if (cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1)
	{
		y = CG_DrawTeamOverlay(y, qtrue, qtrue);
	}
	if (cg_drawSnapshot.integer)
	{
		y = CG_DrawSnapshot(y);
	}
	if (cg_drawFPS.integer)
	{
		y = CG_DrawFPS(y);
	}
	if (cg_drawTimer.integer)
	{
		y = CG_DrawTimer(y);
	}
	if (cg_drawAttacker.integer)
	{
		y = CG_DrawAttacker(y);
	}

}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/

/*
=================
CG_DrawScores

Draw the small two score display
=================
*/
static float CG_DrawScores(float y)
{
	const char*  s;
	int         s1, s2, score;
	int         x, w;
	int         v;
	vec4_t      color;
	float       y1;
	gitem_t*     item;

	s1 = cgs.scores1;
	s2 = cgs.scores2;

	y -=  BIGCHAR_HEIGHT + 8;

	y1 = y;

	// draw from the right side to left
	if (cgs.gametype >= GT_TEAM)
	{
		x = 640;
		color[0] = 0.0f;
		color[1] = 0.0f;
		color[2] = 1.0f;
		color[3] = 0.33f;
		s = va("%2i", s2);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect(x, y - 4,  w, BIGCHAR_HEIGHT + 8, color);
		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
		{
			CG_DrawPicOld(x, y - 4, w, BIGCHAR_HEIGHT + 8, cgs.media.selectShader);
		}
		CG_DrawBigString(x + 4, y, s, 1.0f, DS_HLEFT | DS_SHADOW, 0);

		if (cgs.gametype == GT_CTF)
		{
			// Display flag status
			item = BG_FindItemForPowerup(PW_BLUEFLAG);

			if (item)
			{
				y1 = y - BIGCHAR_HEIGHT - 8;
				if (cgs.blueflag >= 0 && cgs.blueflag <= 2)
				{
					CG_DrawPicOld(x, y1 - 4, w, BIGCHAR_HEIGHT + 8, cgs.media.blueFlagShader[cgs.blueflag]);
				}
			}
		}
		color[0] = 1.0f;
		color[1] = 0.0f;
		color[2] = 0.0f;
		color[3] = 0.33f;
		s = va("%2i", s1);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect(x, y - 4,  w, BIGCHAR_HEIGHT + 8, color);
		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
		{
			CG_DrawPicOld(x, y - 4, w, BIGCHAR_HEIGHT + 8, cgs.media.selectShader);
		}
		CG_DrawBigString(x + 4, y, s, 1.0f, DS_HLEFT | DS_SHADOW, 0);

		if (cgs.gametype == GT_CTF)
		{
			// Display flag status
			item = BG_FindItemForPowerup(PW_REDFLAG);

			if (item)
			{
				y1 = y - BIGCHAR_HEIGHT - 8;
				if (cgs.redflag >= 0 && cgs.redflag <= 2)
				{
					CG_DrawPicOld(x, y1 - 4, w, BIGCHAR_HEIGHT + 8, cgs.media.redFlagShader[cgs.redflag]);
				}
			}
		}

		if (cgs.gametype >= GT_CTF)
		{
			v = cgs.capturelimit;
		}
		else
		{
			v = cgs.fraglimit;
		}
		if (v)
		{
			s = va("%2i", v);
			w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString(x + 4, y, s, 1.0f, DS_HLEFT | DS_SHADOW, 0);
		}

	}
	else
	{
		qboolean    spectator;

		x = 640;
		score = cg.snap->ps.persistant[PERS_SCORE];
		spectator = (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR);

		// always show your score in the second box if not in first place
		if (s1 != score)
		{
			s2 = score;
		}
		if (s2 != SCORE_NOT_PRESENT)
		{
			s = va("%2i", s2);
			w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
			x -= w;
			if (!spectator && score == s2 && score != s1)
			{
				color[0] = 1.0f;
				color[1] = 0.0f;
				color[2] = 0.0f;
				color[3] = 0.33f;
				CG_FillRect(x, y - 4,  w, BIGCHAR_HEIGHT + 8, color);
				CG_DrawPicOld(x, y - 4, w, BIGCHAR_HEIGHT + 8, cgs.media.selectShader);
			}
			else
			{
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect(x, y - 4,  w, BIGCHAR_HEIGHT + 8, color);
			}
			CG_DrawBigString(x + 4, y, s, 1.0f, DS_HLEFT | DS_SHADOW, 0);
		}

		// first place
		if (s1 != SCORE_NOT_PRESENT)
		{
			s = va("%2i", s1);
			w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
			x -= w;
			if (!spectator && score == s1)
			{
				color[0] = 0.0f;
				color[1] = 0.0f;
				color[2] = 1.0f;
				color[3] = 0.33f;
				CG_FillRect(x, y - 4,  w, BIGCHAR_HEIGHT + 8, color);
				CG_DrawPicOld(x, y - 4, w, BIGCHAR_HEIGHT + 8, cgs.media.selectShader);
			}
			else
			{
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect(x, y - 4,  w, BIGCHAR_HEIGHT + 8, color);
			}
			CG_DrawBigString(x + 4, y, s, 1.0f, DS_HLEFT | DS_SHADOW, 0);
		}

		if (cgs.fraglimit)
		{
			s = va("%2i", cgs.fraglimit);
			w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString(x + 4, y, s, 1.0f, DS_HLEFT | DS_SHADOW, 0);
		}

	}

	return y1 - 8;
}

/*
================
CG_DrawPowerups
================
*/
static float CG_DrawPowerups(float y)
{
	int     sorted[MAX_POWERUPS];
	int     sortedTime[MAX_POWERUPS];
	int     i, j, k;
	int     active;
	playerState_t*   ps;
	int     t;
	gitem_t* item;
	int     x;
	int     color;
	float   size;
	float   f;
	static float colors[2][4] =
	{
		{ 0.2f, 1.0f, 0.2f, 1.0f },
		{ 1.0f, 0.2f, 0.2f, 1.0f }
	};

	ps = &cg.snap->ps;

	if (ps->stats[STAT_HEALTH] <= 0)
	{
		return y;
	}

	// sort the list by time remaining
	active = 0;
	for (i = 0 ; i < MAX_POWERUPS ; i++)
	{
		if (!ps->powerups[ i ])
		{
			continue;
		}
		t = ps->powerups[ i ] - cg.time;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if (t < 0 || t > 999000)
		{
			continue;
		}

		// insert into the list
		for (j = 0 ; j < active ; j++)
		{
			if (sortedTime[j] >= t)
			{
				for (k = active - 1 ; k >= j ; k--)
				{
					sorted[k + 1] = sorted[k];
					sortedTime[k + 1] = sortedTime[k];
				}
				break;
			}
		}
		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}

	// draw the icons and timers
	x = 640 - ICON_SIZE - Q3_CHAR_WIDTH * 2;
	for (i = 0 ; i < active ; i++)
	{
		item = BG_FindItemForPowerup(sorted[i]);

		if (item)
		{

			color = 1;

			y -= ICON_SIZE;

			trap_R_SetColor(colors[color]);
			CG_DrawField(x, y, 2, sortedTime[ i ] / 1000, 32, 48);

			t = ps->powerups[ sorted[i] ];
			if (t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME)
			{
				trap_R_SetColor(NULL);
			}
			else
			{
				vec4_t    modulate;

				f = (float)(t - cg.time) / POWERUP_BLINK_TIME;
				f -= (int)f;
				modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;
				trap_R_SetColor(modulate);
			}

			if (cg.powerupActive == sorted[i] &&
			        cg.time - cg.powerupTime < PULSE_TIME)
			{
				f = 1.0 - (((float)cg.time - cg.powerupTime) / PULSE_TIME);
				size = ICON_SIZE * (1.0 + (PULSE_SCALE - 1.0) * f);
			}
			else
			{
				size = ICON_SIZE;
			}

			CG_DrawPicOld(640 - size, y + ICON_SIZE / 2 - size / 2,
			              size, size, trap_R_RegisterShader(item->icon));
		}
	}
	trap_R_SetColor(NULL);

	return y;
}

/*
=====================
CG_DrawLowerRight

=====================
*/
static void CG_DrawLowerRight(void)
{
	float   y;

	y = 480 - ICON_SIZE;

	if (cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 2)
	{
		y = CG_DrawTeamOverlay(y, qtrue, qfalse);
	}

	y = CG_DrawScores(y);
	(void)CG_DrawPowerups(y);
}

/*
===================
CG_DrawPickupItem
===================
*/
static int CG_DrawPickupItem(int y)
{
	int     value;
	float*   fadeColor;

	if (cg.snap->ps.stats[STAT_HEALTH] <= 0)
	{
		return y;
	}

	y -= ICON_SIZE;

	value = cg.itemPickup;
	if (value)
	{
		fadeColor = CG_FadeColor(cg.itemPickupTime, 3000);
		if (fadeColor)
		{
			CG_RegisterItemVisuals(value);
			trap_R_SetColor(fadeColor);
			CG_DrawPicOld(8, y, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon);
			CG_DrawBigString(ICON_SIZE + 16, y + (ICON_SIZE / 2 - BIGCHAR_HEIGHT / 2), bg_itemlist[ value ].pickup_name, fadeColor[0], DS_HCENTER | DS_SHADOW, 0);
			trap_R_SetColor(NULL);
		}
	}

	return y;
}

/*
=====================
CG_DrawLowerLeft

=====================
*/
static void CG_DrawLowerLeft(void)
{
	float   y = 480 - ICON_SIZE;

	if (cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 3)
	{
		(void)CG_DrawTeamOverlay(y, qfalse, qfalse);
	}

	(void)CG_DrawPickupItem(y);
}


//===========================================================================================

/*
=================
CG_DrawTeamInfo
=================
*/
static void CG_DrawTeamInfo(void)
{
	int w, h;
	int i, len;
	vec4_t      hcolor;
	int     chatHeight;

#define CHATLOC_Y 420 // bottom end
#define CHATLOC_X 0

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
		chatHeight = cg_teamChatHeight.integer;
	else
		chatHeight = TEAMCHAT_HEIGHT;
	if (chatHeight <= 0)
		return; // disabled

	if (cgs.teamLastChatPos != cgs.teamChatPos)
	{
		if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer)
		{
			cgs.teamLastChatPos++;
		}

		h = (cgs.teamChatPos - cgs.teamLastChatPos) * TINYCHAR_HEIGHT;

		w = 0;

		for (i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++)
		{
			len = CG_DrawStrlen(cgs.teamChatMsgs[i % chatHeight]);
			if (len > w)
				w = len;
		}
		w *= TINYCHAR_WIDTH;
		w += TINYCHAR_WIDTH * 2;

		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
		{
			hcolor[0] = 1.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		}
		else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
		{
			hcolor[0] = 0.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 1.0f;
			hcolor[3] = 0.33f;
		}
		else
		{
			hcolor[0] = 0.0f;
			hcolor[1] = 1.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		}

		trap_R_SetColor(hcolor);
		CG_DrawPicOld(CHATLOC_X, CHATLOC_Y - h, 640, h, cgs.media.teamStatusBar);
		trap_R_SetColor(NULL);

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--)
		{
			CG_DrawStringExt(CHATLOC_X + TINYCHAR_WIDTH,
			                 CHATLOC_Y - (cgs.teamChatPos - i)*TINYCHAR_HEIGHT,
			                 cgs.teamChatMsgs[i % chatHeight], hcolor, qfalse, qfalse,
			                 TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
		}
	}
}

/*
===================
CG_DrawHoldableItem
===================
*/
static void CG_DrawHoldableItem(void)
{
	int     value;

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if (value)
	{
		CG_RegisterItemVisuals(value);
		CG_DrawPicOld(640 - ICON_SIZE, (SCREEN_HEIGHT - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon);
	}

}


/*
===================
CG_DrawReward
===================
*/
void CG_DrawReward(void)
{
	float*   color;
	int     i;
	float   x, y, w, h;
	char    buf[32];

	if (!cg_drawRewards.integer)
	{
		return;
	}

	color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
	if (!color)
	{
		if (cg.rewardStack > 0)
		{
			for (i = 0; i < cg.rewardStack; i++)
			{
				cg.rewardSound[i] = cg.rewardSound[i + 1];
				cg.rewardShader[i] = cg.rewardShader[i + 1];
				cg.rewardCount[i] = cg.rewardCount[i + 1];
			}
			cg.rewardTime = cg.time;
			cg.rewardStack--;
			color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
			trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
		}
		else
		{
			return;
		}
	}

	trap_R_SetColor(color);

	w = ICON_SIZE - 4;
	h = ICON_SIZE - 4;
	y = 46;
	x = 320 - w / 2.0;
	CG_DrawPic(x, y, w, h, cg.rewardShader[0]);

	if (cg.rewardCount[0] > 1)
	{
		Com_sprintf(buf, sizeof(buf), "%d", cg.rewardCount[0]);
		CG_DrawString(x + w / 2.0f, y + h, buf, color, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 16,  DS_HCENTER | DS_PROPORTIONAL);
	}

	trap_R_SetColor(NULL);
}


/*
===============================================================================

LAGOMETER

===============================================================================
*/

lagometer_t     lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo(void)
{
	int         offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & (LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo(snapshot_t* snap)
{
	// dropped packet
	if (!snap)
	{
		lagometer.snapshotSamples[ lagometer.snapshotCount & (LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & (LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & (LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static float CG_DrawDisconnect(float pos)
{
	int         cmdNum;
	usercmd_t   cmd;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd(cmdNum, &cmd);
	if (cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time)    // special check for map_restart // bk 0102165 - FIXME
	{
		return pos;
	}

	// also add text in center of screen
	CG_DrawBigString(SCREEN_WIDTH / 2.0f, 100, "Connection Interrupted", 1.0f, DS_HCENTER | DS_SHADOW, 0);

	// blink the icon
	if ((cg.time >> 9) & 1)
	{
		return pos;
	}

	CG_DrawPicOld(640 - 48, pos, 48, 48, trap_R_RegisterShader("gfx/2d/net.tga"));
	return pos;
}

/*
==============
CG_DrawLagometer
==============
*/
float CG_DrawLagometer(float pos)
{
	int     a, x, i;
	float   v;
	float   ax, ay, aw, ah, mid, range;
	int     color;
	float   vscale;

	if (cg_drawPing.integer > 0)
	{
		pos = CG_OSPDrawPing(pos) - 2.0f;
	}

	x = 640 - 48;
	pos += 4.0f;

	trap_R_SetColor(NULL);
	CG_DrawPicOld(x, pos, 48, 48, cgs.media.lagometerShader);

	ax = x;
	ay = pos;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640_Old(&ax, &ay, &aw, &ah, qfalse);

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for (a = 0 ; a < aw ; a++)
	{
		i = (lagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if (v > 0)
		{
			if (color != 1)
			{
				color = 1;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
			}
			if (v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
		else if (v < 0)
		{
			if (color != 2)
			{
				color = 2;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_BLUE)]);
			}
			v = -v;
			if (v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for (a = 0 ; a < aw ; a++)
	{
		i = (lagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if (v > 0)
		{
			if (lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED)
			{
				if (color != 5)
				{
					color = 5;  // YELLOW for rate delay
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
				}
			}
			else
			{
				if (color != 3)
				{
					color = 3;
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_GREEN)]);
				}
			}
			v = v * vscale;
			if (v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
		else if (v < 0)
		{
			if (color != 4)
			{
				color = 4;      // RED for dropped snapshots
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_RED)]);
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	trap_R_SetColor(NULL);

	if (cg_nopredict.integer || cg_synchronousClients.integer)
	{
		CG_DrawBigString(ax, ay, "snc", 1.0f, DS_HLEFT | DS_SHADOW, 0);
	}

	pos += CG_DrawDisconnect(pos);

	return pos + 44.0f;
}



/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint(const char* str, int y, int charWidth)
{
	char*    s;

	Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while (*s)
	{
		if (*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}


/*
================================================================================

CROSSHAIR

================================================================================
*/

/*
=================
CG_ScanForCrosshairEntity
=================
*/
void CG_ScanForCrosshairEntity(void)
{
	trace_t     trace;
	vec3_t      start, end;
	int         content;
	vec3_t      l;

	VectorCopy(cg.refdef.vieworg, start);
	VectorMA(start, 131072, cg.refdef.viewaxis[0], end);

	CG_Trace(&trace, start, vec3_origin, vec3_origin, end,
	         cg.snap->ps.clientNum, CONTENTS_SOLID);

	VectorSubtract(cg.snap->ps.origin, trace.endpos, l);
	cgs.osp.crosshair.distance = VectorLength(l);

	CG_Trace(&trace, start, vec3_origin, vec3_origin, end,
	         cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY);
	if (trace.entityNum >= MAX_CLIENTS)
	{
		if (cgs.osp.gameTypeFreeze && trace.entityNum < MAX_GENTITIES)
		{
			centity_t* centp = &cg_entities[trace.entityNum];
			if (centp->currentState.powerups & (1 << PW_BATTLESUIT))
			{
				cg.crosshairClientNum = centp->currentState.otherEntityNum;
				cg.crosshairClientTime = cg.time;
			}
		}
		return;
	}

	// if the player is in fog, don't show it
	content = trap_CM_PointContents(trace.endpos, 0);
	if (content & CONTENTS_FOG)
	{
		return;
	}

	// if the player is invisible, don't show it
	if (cg_entities[ trace.entityNum ].currentState.powerups & (1 << PW_INVIS))
	{
		return;
	}

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}


/*
=====================
CG_DrawCrosshairNames
=====================
*/
void CG_DrawCrosshairNames(void)
{
	float*       color;
	char*        name;

	if (cg_drawCrosshair.integer == 0) return;
	if (cg_drawCrosshairNames.integer == 0) return;
	if (cg.renderingThirdPerson != 0) return;
	if (global_viewlistFirstOption > 1) return;

	CG_ScanForCrosshairEntity();
	if ((cg_drawCrosshairNames.integer == 2) &&
	        ((cg.snap->ps.persistant[PERS_TEAM] == TEAM_FREE) ||
	         (cg.snap->ps.persistant[PERS_TEAM] == cgs.clientinfo[cg.crosshairClientNum].team) ||
	         cgs.osp.gameTypeFreeze ||
	         (cgs.clientinfo[cg.snap->ps.clientNum].team != cgs.clientinfo[cg.crosshairClientNum].team)))
	{
		return;
	}

	color = CG_FadeColor(cg.crosshairClientTime, 1000);
	if (!color)
	{
		return;
	}
	name = cgs.clientinfo[ cg.crosshairClientNum ].name;
	CG_DrawBigString(SCREEN_WIDTH / 2.0f, 170, name, 0.5f * color[3], DS_HCENTER | DS_SHADOW, 0);
}


//==============================================================================

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void)
{
	char*    s;
	int     sec;

	if (!cgs.voteTime)
	{
		return;
	}

	// play a talk beep whenever it is modified
	if (cgs.voteModified)
	{
		cgs.voteModified = qfalse;
		if (!cg_noVoteBeep.integer)
		{
			trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
		}
	}

	sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;
	if (sec < 0)
	{
		sec = 0;
	}
	s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
	CG_DrawSmallString(0, 58, s, 1.0F, DS_HLEFT | DS_SHADOW, 0);
}

/*
=================
CG_DrawIntermission
=================
*/
qboolean CG_DrawIntermission(void)
{
	if (cg_q3compScoreboard.integer)
	{
		if (cgs.gametype >= GT_TEAM)
		{
			return CG_OSPDrawScoretable();
		}
	}
	return CG_DrawOldScoreboard();
}

void CG_OSPDrawIntermission()
{
	int i;

	if (cgs.gametype == GT_SINGLE_PLAYER)
	{
		CG_OSPDrawCenterString();
		return;
	}

	for (i = 0; i < cgs.osp.custom_gfx_number; ++i)
	{
		if (cgs.osp.custom_gfx[i][4] >= 0 &&
		        cgs.osp.custom_gfx[i][5] >= 0 &&
		        cgs.osp.custom_gfx[i][6] > 0 &&
		        cgs.osp.custom_gfx[i][7] > 0 &&
		        cgs.media.customShader[i])
		{
			CG_DrawPicOld(
			    (float)cgs.osp.custom_gfx[i][4],
			    (float)cgs.osp.custom_gfx[i][5],
			    (float)cgs.osp.custom_gfx[i][6],
			    (float)cgs.osp.custom_gfx[i][7],
			    cgs.media.customShader[i]);
		}
	}

	cg.scoreFadeTime = cg.time;
	cg.scoreBoardShowing = CG_DrawIntermission();

	if (cgs.osp.isOSPv1)
	{

		qtime_t qtime;
		char* str;
		trap_RealTime(&qtime);
		str = va("%02d:%02d:%02d - %s - %02d %s %d",
		         qtime.tm_hour,
		         qtime.tm_min,
		         qtime.tm_sec,
		         dayOfWeekName[qtime.tm_wday],
		         qtime.tm_mday,
		         monthName[qtime.tm_mon],
		         qtime.tm_year + 1900);

		CG_OSPDrawStringWithShadow(8, 462, str, 10, 10, colorWhite, 0);
		str = va("^B%s", CG_ConfigString(CS_OSP_VERSION_STR));
		CG_OSPDrawStringWithShadow(8, 450, str, 10, 10, colorYellow, 0);
	}
	CG_OSPSetColor(NULL);
	CG_DrawPicOld(590.0f, 430.f, 50.0f, 50.0f, cgs.media.blender180Shader);
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow(void)
{
	float       x;
	vec4_t      color;
	const char*  name;

	if (!(cg.snap->ps.pm_flags & PMF_FOLLOW))
	{
		return qfalse;
	}
	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

	CG_DrawBigString(SCREEN_WIDTH - 1, 24, "following", 1.0f, DS_HRIGHT | DS_SHADOW, 0);

	name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

	x = 0.5 * (640 - GIANT_WIDTH * CG_DrawStrlen(name));

	CG_DrawStringExt(x, 40, name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0);

	return qtrue;
}



/*
=================
CG_DrawAmmoWarning
=================
*/
void CG_DrawAmmoWarning(void)
{
	const char*  s;

	if (cg_drawAmmoWarning.integer == 0)
	{
		return;
	}

	if (!cg.lowAmmoWarning)
	{
		return;
	}

	if (cg.lowAmmoWarning == 2)
	{
		s = "OUT OF AMMO";
	}
	else
	{
		s = "LOW AMMO WARNING";
	}
	CG_DrawBigString(SCREEN_WIDTH / 2.0f, 64, s, 1.0f, DS_HCENTER | DS_SHADOW, 0);
}



/*
=================
CG_DrawWarmup
=================
*/
void CG_DrawWarmup(void)
{
	int sec = cg.warmup;

	if (sec == 0)
	{
		return;
	}
	else if (sec < 0)
	{
		const char* text = cgs.gametype != GT_TOURNAMENT ? "Waiting for Players" : "Waiting for Opponent";
		int textWidth = CG_DrawStrlen(text) * 14;
		CG_OSPDrawStringWithShadow(320 - (textWidth / 2), 24, va("^B%s", text), 14, 14, colorRed, 0);
		cg.warmupCount = 0;
	}
	else // warmup > 0
	{
		if (cgs.gametype == GT_TOURNAMENT)
		{
			const char* player1Name = NULL;
			const char* player2Name = NULL;
			int i;

			for (i = 0; i < cgs.maxclients; ++i)
			{
				if (cgs.clientinfo[i].name[0] && cgs.clientinfo[i].team == TEAM_FREE)
				{
					if (!player1Name)
					{
						player1Name = cgs.clientinfo[i].name;
					}
					else
					{
						player2Name = cgs.clientinfo[i].name;
						break;
					}
				}
			}
			if (player1Name && player2Name)
			{
				int len;
				int width;
				const char* text;
				text = va("%s^7 vs %s", player1Name, player2Name);

				len = CG_DrawStrlen(text);

				if (len > 20)
				{
					width = SCREEN_WIDTH / len;
				}
				else
				{
					width = 32;
				}

				CG_OSPDrawStringWithShadow((SCREEN_WIDTH - (len * width)) / 2, 20, text, width, (int)(1.5 * (float)width), colorWhite, 0);
			}
		}
		else
		{
			const char* text;
			int len;
			int width;

			if (cgs.osp.clanBaseTeamDM)
			{
				text = "ClanBase TeamDM";
			}
			else if (cgs.gametype == GT_FFA)
			{
				text = "Free for all";
			}
			else if (cgs.gametype == GT_TEAM)
			{
				if (cgs.osp.gameTypeFreeze == 0)
				{
					text = "Team Deathmatch";
				}
				else if (cgs.osp.gameTypeFreeze == 2)
				{

					text = "FreezeTag TDM (Vanilla)";
				}
				else
				{
					text = "FreezeTag TDM";
				}
			}
			else if (cgs.gametype == GT_CTF)
			{
				text = "Capture the Flag";
			}
			else if (cgs.gametype == GT_CA)
			{
				text = "Clan Arena";
			}
			else
			{
				text = "";
			}

			len = CG_DrawStrlen(text);
			if (len > 0x14)
			{
				width = SCREEN_WIDTH / len;
			}
			else
			{
				width = 32;
			}
			CG_DrawStringExt((SCREEN_WIDTH - (len * width)) / 2, 25, text, colorLtGrey, 0, 1, width, (int)(1.1 * (float)width), qfalse);
		}

		if (cg.showScores == 0)
		{
			int cw;
			float* color = colorWhite;
			const char* s;
			int w;

			sec = (sec - cg.time) / 1000;
			if (sec < 0)
			{
				cg.warmup = 0;
				sec = 0;
			}
			s = va("Starts in: %i", sec + 1);
			if (sec != cg.warmupCount)
			{
				cg.warmupCount = sec;
				switch (sec)
				{
					case 0:
						trap_S_StartLocalSound(cgs.media.count1Sound, CHAN_ANNOUNCER);
						break;
					case 1:
						trap_S_StartLocalSound(cgs.media.count2Sound, CHAN_ANNOUNCER);
						break;
					case 2:
						trap_S_StartLocalSound(cgs.media.count3Sound, CHAN_ANNOUNCER);
						break;
					default:
						break;
				}
			}

			switch (cg.warmupCount)
			{
				case 0:
					cw = 28;
					color = colorLtGrey;
					break;
				case 1:
					cw = 24;
					color = colorYellow;
					break;
				case 2:
					cw = 20;
					color = colorWhite;
					break;
				default:
					cw = 16;
					break;
			}

			w = CG_DrawStrlen(s);
			CG_DrawStringExt(320 - w * cw / 2, 70, s, color, qfalse, qtrue, cw, (int)(cw * 1.5), 0);
		}
	}
}

void CG_DrawWarmupShud(void)
{
	int sec = cg.warmup;

	if (sec == 0)
	{
		return;
	}
	else if (sec < 0)
	{
		cg.warmupCount = 0;
	}
	else // warmup > 0
	{
		if (cg.showScores == 0)
		{
			sec = (sec - cg.time) / 1000;
			if (sec < 0)
			{
				cg.warmup = 0;
				sec = 0;
			}
			if (sec != cg.warmupCount)
			{
				cg.warmupCount = sec;
				switch (sec)
				{
					case 0:
						trap_S_StartLocalSound(cgs.media.count1Sound, CHAN_ANNOUNCER);
						break;
					case 1:
						trap_S_StartLocalSound(cgs.media.count2Sound, CHAN_ANNOUNCER);
						break;
					case 2:
						trap_S_StartLocalSound(cgs.media.count3Sound, CHAN_ANNOUNCER);
						break;
					default:
						break;
				}
			}
		}
	}
}

static void CG_DrawTestFont(const char* font)
{
	const int offs_x = (SCREEN_WIDTH - 320) / 2;
	const int offs_y = (SCREEN_HEIGHT - 320) / 2;
	int x, y;
	char to_print[2];
	unsigned char c = 0;
	to_print[1] = 0;
	CG_FontSelect(CG_FontIndexFromName(font));
	for (y = 0; y < 16; ++y)
	{
		for (x = 0; x < 16; ++x)
		{
			to_print[0] = (char)c;
			c++;
			if (to_print[0] == 0) to_print[0] = ' ';
			CG_OSPDrawString(offs_x + x * 20, offs_y + y * 20, to_print, colorWhite, 16, 16, 4 * 16, DS_HCENTER | DS_PROPORTIONAL, NULL);
		}
	}

}

/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D(void)
{
	// if we are taking a levelshot for the menu, don't draw anything
	if (cg.levelShot)
	{
		return;
	}

	if (cg_draw2D.integer == 0)
	{
		return;
	}

	if (strlen(cgs.osp.testFont))
	{
		CG_DrawTestFont(cgs.osp.testFont);
		return;
	}

	if (cg_shud.integer)
	{
		CG_SHUDRoutine();
		CG_DrawWarmupShud();
		return;
	}

	if (cg_enableOSPHUD.integer)
	{
		CG_OSPHUDRoutine();
		return;
	}

	if (cg.snap->ps.pm_type == PM_INTERMISSION)
	{
		CG_OSPDrawIntermission();
		CG_DrawVote();
		return;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		CG_DrawCrosshair();
		CG_DrawCrosshairNames();
	}
	else
	{
		// don't draw any status if dead or the scoreboard is being explicitly shown
		if (!cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0)
		{
			CG_DrawStatusBar();
			CG_DrawAmmoWarning();
			CG_DrawCrosshair();
			CG_DrawCrosshairNames();
			CG_DrawWeaponSelect();
			CG_DrawHoldableItem();
			CG_DrawReward();
		}

		if (cgs.gametype >= GT_TEAM)
		{
			CG_DrawTeamInfo();
		}
	}

	CG_DrawVote();

	if (cg_lagometer.integer)
	{
		CG_DrawLagometer(428 - (cg_drawPing.integer > 0 ? 14 : 0));
	}
	CG_DrawUpperRight();
	CG_DrawLowerRight();
	CG_DrawLowerLeft();
	if (CG_DrawFollow() == qfalse)
	{
		CG_DrawWarmup();
	}
	cg.scoreBoardShowing = CG_DrawIntermission();
	if (cg.scoreBoardShowing == qfalse)
	{
		CG_OSPDrawCenterString();
	}
}


static void CG_DrawTourneyScoreboard(void)
{
	CG_DrawOldTourneyScoreboard();
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive(stereoFrame_t stereoView)
{
	float       separation;
	vec3_t      baseOrg;

	// optionally draw the info screen instead
	if (!cg.snap)
	{
		CG_DrawInformation();
		return;
	}

	// optionally draw the tournement scoreboard instead
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR &&
	        (cg.snap->ps.pm_flags & PMF_SCOREBOARD))
	{
		CG_DrawTourneyScoreboard();
		return;
	}

	switch (stereoView)
	{
		case STEREO_CENTER:
			separation = 0;
			break;
		case STEREO_LEFT:
			separation = -cg_stereoSeparation.value / 2;
			break;
		case STEREO_RIGHT:
			separation = cg_stereoSeparation.value / 2;
			break;
		default:
			separation = 0;
			CG_Error("CG_DrawActive: Undefined stereoView: %d", stereoView);
	}


	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy(cg.refdef.vieworg, baseOrg);
	if (separation != 0)
	{
		VectorMA(cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg);
	}

	// draw 3D view
	trap_R_RenderScene(&cg.refdef);

	// restore original viewpoint if running stereo
	if (separation != 0)
	{
		VectorCopy(baseOrg, cg.refdef.vieworg);
	}

	// draw status bar and other floating elements
	CG_Draw2D();
}



int CG_OSPDrawLeftSlidingWindow(float timeAppearance, float timeShow, float timeHide, float time3Sec, int numberOfLines, int sizeOfLine, int w, int h, char* text, int windowPosX, float* borderColor, float* bodyColor)
{
	OSP_SlidingPrintContext_t* target;
	int minTime;
	int minTimeIndex;
	int i; //14
	char* str; //1c
	char normalized_str[128]; //20
	int str_size; //c4
	minTimeIndex = -1;
	minTime = 9999999;
	i = 0;

	/* find free window */
	do
	{
		if (!ospPrintContext[i].hideBeforeCGTime)
		{
			minTimeIndex = i;
			break;
		}
		else if (ospPrintContext[i].hideBeforeCGTime < minTime)
		{
			minTimeIndex = i;
			minTime = ospPrintContext[i].hideBeforeCGTime;
		}
	}
	while (++i < 16);

	if (minTimeIndex == -1)
	{
		minTimeIndex = 0;
	}

	target = &ospPrintContext[minTimeIndex];
	target->windowPosX = windowPosX;
	target->charWidth = w;
	target->charHeight = h;
	target->timeAppearance = 1000.0 * timeAppearance;
	target->timeShow = 1000.0 * timeShow;
	target->timeHiding = 1000.0 * timeHide;

	if (borderColor)
	{
		VectorCopy(borderColor, target->borderColor);
	}
	else
	{
		VectorClear(target->borderColor);
	}
	target->borderColor[3] = 1.0;

	if (bodyColor)
	{
		VectorCopy(bodyColor, target->bodyColor);
	}
	else
	{
		target->bodyColor[0] = 0.7f;
		target->bodyColor[1] = 1.0f;
		target->bodyColor[2] = 0.6f;
	}
	target->bodyColor[3] = 0.45f;

	target->maxStringLen = 0;
	target->numberOfStrings = 0;

	/* copy text and update max string size */
	for (i = 0, str = text ; i < numberOfLines && target->numberOfStrings < 24; ++i, ++target->numberOfStrings, str += sizeOfLine)
	{
		if (!str) break;
		Q_strncpyz(&target->string[i][0], str, 128);
		CG_OSPNormalizeText(&target->string[i][0], strlen(&target->string[i][0]), normalized_str);
		str_size = strlen(normalized_str);
		if (str_size > target->maxStringLen)
		{
			target->maxStringLen = str_size;
		}
	}

	target->hideBeforeRealtime = trap_Milliseconds() + target->timeHiding + 1000 * time3Sec;
	target->hideBeforeCGTime = cg.time + target->timeHiding + 1000 * time3Sec;
	target->showFromCGTime = cg.time + 1000 * time3Sec;
	return minTimeIndex;
}


void CG_OSPDrawLeftSlidingWindowsRoutine(OSP_SlidingPrintContext_t* context)
{
	int time_u1;
	float calc_pos_x;
	float calc_pos_y;
	int i;
	time_u1 = context->hideBeforeRealtime - trap_Milliseconds();
	if (time_u1 < context->timeHiding - context->timeShow)
	{
		if (time_u1 < context->timeAppearance)
		{
			//after
			calc_pos_x = ((float)context->charWidth * context->maxStringLen + 3.0f) * ((float)context->timeAppearance - time_u1) / (float)context->timeAppearance;
		}
		else
		{
			//static
			calc_pos_x = 0;
		}
	}
	else
	{
		//before
		calc_pos_x = ((float)context->charWidth * context->maxStringLen + 3.0f) * ((float)time_u1 - context->timeHiding + context->timeShow) / (float)context->timeShow;
	}

	calc_pos_x = 6.0f - calc_pos_x;
	calc_pos_y = context->windowPosX - (float)context->numberOfStrings * (context->charHeight + 1);
// borders
	CG_OSPSetColor(context->borderColor);

//void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
	CG_DrawPicOld(
	    calc_pos_x,
	    calc_pos_y - 2.0f,
	    (float)(context->charWidth * context->maxStringLen + 3),
	    1.0f,
	    cgs.media.teamStatusBar);

	/* bottom */
	CG_DrawPicOld(
	    calc_pos_x,
	    calc_pos_y + ((float)context->charHeight + 1.0f) * (float)context->numberOfStrings + 1.0f,
	    (float)(context->charWidth * context->maxStringLen + 3),
	    1.0f,
	    cgs.media.teamStatusBar);

	/* left */
	CG_DrawPicOld(
	    calc_pos_x,
	    calc_pos_y - 1.0f,
	    1.0f,
	    (float)((context->charHeight + 1) * context->numberOfStrings + 2),
	    cgs.media.teamStatusBar);

	CG_DrawPicOld(
	    context->charWidth * context->maxStringLen + calc_pos_x + 2,
	    calc_pos_y - 1,
	    1.0f,
	    (context->charHeight + 1) * context->numberOfStrings + 2,
	    cgs.media.teamStatusBar);

	//fill window color
	CG_OSPSetColor(context->bodyColor);
	CG_OSPDrawPic(
	    calc_pos_x + 1.0f,
	    calc_pos_y - 1.0f,
	    (float)context->charWidth * context->maxStringLen + 1.0f,
	    (float)((context->charHeight + 1) * context->numberOfStrings + 2),
	    cgs.media.teamStatusBar);
	CG_OSPSetColor(NULL);

	for (i = 0; i < context->numberOfStrings; ++i)
	{
		CG_FontSelect(0);
		CG_OSPDrawString(calc_pos_x + 2,
		                 calc_pos_y + i * (context->charHeight + 1),
		                 &context->string[i][0],
		                 colorWhite,
		                 context->charWidth,
		                 context->charHeight,
		                 SCREEN_WIDTH,
		                 DS_HLEFT,
		                 NULL);
	}
}


void CG_OSPSetColor(vec4_t color)
{
	trap_R_SetColor(color);
	return;
}

void CG_OSPDrawPic(float x, float y, float w, float h, qhandle_t hShader)
{
	CG_AdjustFrom640_Old(&x, &y, &w, &h, qfalse);
	trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1.0f, 1.0f, hShader);
	return;
}

void CG_OSPDraw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t pos, vec3_t angles, vec3_t angles2)
{
	refdef_t rd;
	refEntity_t re;

	if (!cg_draw3dIcons.integer || !cg_drawIcons.integer)
	{
		return;
	}

	CG_AdjustFrom640_Old(&x, &y, &w, &h, qfalse);
	memset(&rd, 0, sizeof(rd));
	memset(&re, 0, sizeof(re));
	AnglesToAxis(angles, re.axis);

	memcpy(re.origin, pos, sizeof(vec3_t));
	re.hModel = model;
	re.customSkin = skin;
	re.renderfx = RF_NOSHADOW;     // no stencil shadows
	rd.rdflags = RDF_NOWORLDMODEL;

	AxisClear(rd.viewaxis);
	rd.fov_x = 30.0f;
	rd.fov_y = 30.0f;
	rd.x = x;
	rd.y = y;
	rd.width = w;
	rd.height = h;
	rd.time = cg.time;
	if (angles2)
	{
		float tmp;

		tmp = 255.0f * angles2[0];
		if (tmp >= (float)INT_MAX)
		{
			re.shaderRGBA[0] = -1 * (int)(tmp - (float)INT_MAX) ;
		}
		else
		{
			re.shaderRGBA[0] = (int)tmp;
		}

		tmp = 255.0f * angles2[1];
		if (tmp >= (float)INT_MAX)
		{
			re.shaderRGBA[1] = -1 * (int)(tmp - (float)INT_MAX);
		}
		else
		{
			re.shaderRGBA[1] = (int)tmp;
		}

		tmp = 255.0f * angles2[2];
		if (tmp >= (float)INT_MAX)
		{
			re.shaderRGBA[2] = -1 * (int)(tmp - (float)INT_MAX);
		}
		else
		{
			re.shaderRGBA[2] = (int)tmp;
		}
	}

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene(&re);
	trap_R_RenderScene(&rd);
}

void CG_OSPDrawCenterString(void)
{
	float* fadeColor;
	char* to_print;
	int y;
	if (!cg_drawCenterMessages.integer)
	{
		return;
	}
	if (!cg.centerPrintTime) return;
	fadeColor = CG_FadeColor(cg.centerPrintTime, 1000.0 * cg_centertime.value);


	if (!fadeColor) return;

	trap_R_SetColor(fadeColor);

	y = cg.centerPrintY - (cg.centerPrintLines * 8);
	for (to_print = cg.centerPrint; to_print < (to_print + sizeof(cg.centerPrint)) && *to_print != 0;)
	{
		int i;
		int space;
		char buf[48];

		space = 0;
		i = 0;
		do
		{
			if (to_print[i] == ' ')
			{
				space = i;
			}
			if (!to_print[i] || to_print[i] == '\n')
			{
				space = i;
				break;
			}
			buf[i]  = to_print[i];
		}
		while (++i < 40);

		if (space && space != i)
		{
			buf[space] = 0;
			to_print += space + 1;
		}
		else
		{
			buf[i] = 0;
			to_print += i + 1;
		}

		CG_FontSelect(0);
		CG_OSPDrawString(SCREEN_WIDTH / 2.0f,
		                 y,
		                 buf,
		                 fadeColor,
		                 cg.centerPrintCharWidth,
		                 1.5f * (float)cg.centerPrintCharWidth,
		                 SCREEN_WIDTH,
		                 DS_HCENTER | DS_PROPORTIONAL,
		                 NULL);

		y += 1.5f * (float)cg.centerPrintCharWidth;
	}
	trap_R_SetColor(NULL);
}

