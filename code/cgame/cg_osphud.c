#include "cg_local.h"

#define OSPHUD_TEAMOVERLAY_STR_SIZE 128
typedef struct
{
	int powerupOffsetChar;
	int nameOffsetChar;
	int healthOffsetChar;
	int armorOffsetChar;
	int ammoOffsetChar;
	int locationOffsetChar;
	int overlayWidthChar;
	int strX;
	int nameLenChar;
	char str[OSPHUD_TEAMOVERLAY_STR_SIZE];
} teamOverlay_t;

teamOverlay_t teamOverlay;
int teamOverlayWidth = 0;

int modChCountTeamOverlay;
int modCfCountTeamOverlay;
int modCgMaxLocationWidth;
int modCgDrawTeamOverlay;

qboolean teamOverlayUnkFlag;
int teamOverlayPosY;

static vec4_t colorTeamOverlay = { 0, 0, 0, 0.33f };

static qboolean weaponBarDrew;

typedef struct
{
	int num;
	int time;
} powerUpInfo_t;

static vec4_t wepSwitchColor1 = { 0.1f, 0.1f, 0.5f, 0.5f };
static vec4_t wepSwitchColor2 = { 1.0f, 0.75f, 1.0f, 1.0f };
static int wepSwitch67[16];
static vec4_t healthColors67[3] = { { 1.0f, 0.7f, 0.0f, 1.0f }, { 1.0f, 0.2f, 0.2f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
static vec4_t powerupColors67[2] = {{0.2f, 1.0f, 0.2f, 1.0f}, {1.0f, 0.2f, 0.2f, 1.0f}};

static void CG_OSPPrepareTeamOverlay(int w, qboolean right) ;
static void CG_OSPDrawPlayerIcon(float x, float y, float w, float h, int attacker, float* angles, float* color2);
static int CG_OSPGetPresistantTeam(void);


static void CG_OSPDrawSpectator(void)
{
	int persOpt;
	if (global_viewlistFirstOption) return;
	persOpt = CG_OSPGetPresistantTeam();

	CG_OSPDrawString(SCREEN_WIDTH / 2.0f, 440, "^BSPECTATOR", colorYellow, 18, 18, SCREEN_WIDTH, DS_HCENTER | DS_SHADOW, NULL);
	if (cgs.gametype != 1 && (cgs.osp.gameTypeFreeze != 0 || persOpt == 3))
	{
		if (cgs.gametype >= GT_TEAM && (cgs.osp.gameTypeFreeze == OSP_SERVER_MODE_VQ3 || persOpt == 3))
		{
			CG_DrawBigString(SCREEN_WIDTH / 2.0f, 460, "press ESC and use the START menu to play", 1.0f, DS_HCENTER | DS_SHADOW, 0);
		}
	}
	else
	{
		CG_DrawBigString(SCREEN_WIDTH / 2.0f, 460, "Waiting to play", 1.0f, DS_HCENTER | DS_SHADOW, 0);
	}
}

static void CG_OSPDrawStatusBarString(int x, int y, int len, int w, int h, int val)
{
	char buf[16];
	int bufLen;
	char* ptr;

	if (len < 1) return;
	if (len > 5) len = 5;

	switch (len)
	{
		case 1:
			if (val > 9) val = 9;
			if (val < 0) val = 0;
			break;
		case 2:
			if (val > 99) val = 99;
			if (val < -9) val = -9;
			break;
		case 3:
			if (val > 999) val = 999;
			if (val < -99) val = -99;
			break;
		case 4:
			if (val > 9999) val = 9999;
			if (val < -999) val = -999;
			break;
		case 5:
			if (val > 99999) val = 99999;
			if (val < -9999) val = -9999;
			break;
	}


	Com_sprintf(buf, 16, "%i", val);
	bufLen = strlen(buf);

	if (bufLen > len) bufLen = len;

	x += w * (len - bufLen) + 2;

	for (ptr = buf; *ptr != 0; ++ptr, --bufLen)
	{
		int shaderIndex;
		if (*ptr == '-')
		{
			shaderIndex = 10;
		}
		else
		{
			shaderIndex = *ptr - '0';
		}
		CG_DrawPicOld(x, y, w, h, cgs.media.numberShaders[shaderIndex]);
		x += w;
	}
}

static float CG_OSPDrawStatusBar(float arg)
{
	playerState_t* ps;
	int fontW;
	int fontH;
	int offsetW;
	int color;
	int value;
	vec4_t      hcolor;
	centity_t* cent;

	static float colors[4][4] =
	{
		{ 1.0f, 0.69f, 0.0f, 1.0f },    // normal
		{ 1.0f, 0.2f, 0.2f, 1.0f },     // low health
		{ 0.5f, 0.5f, 0.5f, 1.0f },     // weapon firing
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	};   // health > 100
	//
	if (cg_drawStatus.integer == 0)
	{
		return arg;
	}
	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;
	CG_OSPGetClientFontSize(&cf_Statusbar, &fontW, &fontH);

	//
	// flags
	//
	value = ps->ammo[cent->currentState.weapon];
	if (value < 0)
	{
		offsetW = -2 * fontW;
	}
	else if (value < 10)
	{
		offsetW = ~fontW;
	}
	else if (value < 100)
	{
		offsetW = ~fontW / 2;
	}
	else
	{
		offsetW = 0;
	}
	arg -= fontH;

	if (ch_TeamBackground.integer)
	{
		CG_DrawTeamBackground(0, arg, 640, fontH, 0.33f, cg.snap->ps.persistant[PERS_TEAM]);
	}
	if (cg.predictedPlayerState.powerups[PW_REDFLAG])
	{
		CG_DrawFlagModel((float)(((320 - (12 * fontW / (float)2)) + offsetW) + fontW * 8), arg, (float)fontW, (float)fontH, 1, 0);
	}
	else if (cg.predictedPlayerState.powerups[PW_BLUEFLAG])
	{
		CG_DrawFlagModel((float)(((320 - ((12 * fontW) / (float)2)) + offsetW) + (fontW * 8)), arg, (float)fontW, (float)fontH, 2, 0);
	}
	//
	// ammo
	//
	if (cent->currentState.weapon)
	{
		qhandle_t   icon;
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
			CG_OSPDrawStatusBarString((320 - (12 * fontW / (float)2) + offsetW), arg, 3, fontW, fontH, value);

			trap_R_SetColor(NULL);

			icon = cg_weapons[ cg.predictedPlayerState.weapon ].ammoIcon;
			if (icon)
			{
				CG_DrawPicOld((float)(((320 - ((12 * fontW) / (float)2)) + offsetW) + (3 * fontW)), arg, fontW, fontH, icon);
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
	CG_OSPDrawStatusBarString((((320 - ((12 * fontW) / 2)) + offsetW) + (5 * fontW)), (int)arg, 3, fontW, fontH, value);
	CG_ColorForHealth(hcolor, NULL);
	trap_R_SetColor(hcolor);

	//
	// armor
	//
	value = ps->stats[STAT_ARMOR];
	if (value > 0)
	{
		trap_R_SetColor(colors[0]);
		CG_OSPDrawStatusBarString((((320 - ((12 * fontW) / 2)) + offsetW) + (9 * fontW)), (int)arg, 3, fontW, fontH, value);
		trap_R_SetColor(NULL);

		if (cgs.osp.server_mode & OSP_SERVER_MODE_PROMODE)
		{
			CG_DrawPicOld((float)((((320 - (12 * fontW / (float)2) + offsetW) + 9 * fontW) + 3 * fontW) + 3), 452, 24, 24, cgs.media.armorIcon[ps->stats[STAT_ARMOR_TYPE]]);
		}
	}

	trap_R_SetColor(NULL);
	return arg;
}

static void CG_OSPDrawStatusBarAmmo1(float y)
{
	int ammoW;
	int ammoH;
	int statusH;
	int statusW;
	int weaponHave;
	int weaponIndex;
	int i;
	float h;

	static struct
	{
		unsigned char hi;
		unsigned char mid;
		unsigned char low;
	} weaponRanges[16] =
	{
		{0, 0, 0},      // empty
		{0, 0, 0},      // Gauntlet
		{200, 100, 50}, // Machinegun
		{40, 20, 10},   // Shotgun
		{40, 20, 10},   // Grenade
		{40, 20, 10},   // Rocket
		{200, 100, 50}, // Shaft
		{40, 20, 10},   // Railgun
		{200, 100, 50}, // Plasma
		{100, 50, 25},  // BFG
	};
	static vec4_t color[3] =
	{
		{ 0.0f, 1.0f, 0.0f, 1.0f },
		{ 0.6f, 0.6f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f, 1.0f }
	};
	float wHi;

	CG_OSPGetClientFontSize(&cf_Statusbar, &statusW, &statusH);
	CG_OSPGetClientFontSize(&cf_AmmoStatusbar, &ammoW, &ammoH);
	weaponHave = cg.snap->ps.stats[STAT_WEAPONS];
	i = 0;
	weaponIndex = 2;


	do
	{
		if ((weaponHave & (1 << weaponIndex)))
		{
			int ammo;
			ammo = cg.snap->ps.ammo[weaponIndex];
			CG_DrawPicOld(i * (ammoW + 2), y - (ammoH + 2), ammoW, ammoH, cg_weapons[weaponIndex].ammoIcon);

			if (weaponIndex == cg.weaponSelect)
			{
				CG_DrawPicOld((float)(i * (ammoW + 2)) - 2, y - (ammoH + 4), ammoW + 4, ammoH + 4, cgs.media.selectShader);
			}
			wHi = ammo / (float)weaponRanges[weaponIndex].hi;

			if (wHi > 1.0f)
			{
				wHi = 1.0f;
			}

			if (ammo > weaponRanges[weaponIndex].mid)
			{
				color[0][3] = 1.0;
				trap_R_SetColor(color[0]);
			}
			else if (ammo > weaponRanges[weaponIndex].low)
			{
				int tmp;
				tmp = cg.time & 0x7ff;
				if (tmp > 1024)
				{
					tmp = ~tmp + 2047;
				}
				color[1][3] = tmp / 1463.0f + 0.3f;
				trap_R_SetColor(color[1]);
			}
			else
			{
				int tmp;
				tmp = cg.time & 0x3ff;
				if (tmp > 512)
				{
					tmp = ~tmp + 1023;
				}
				color[2][3] = tmp / 731.0f + 0.3f;
				trap_R_SetColor(color[2]);
			}
			h = (float)(wHi * (float)(statusH - (ammoH + 0x4)));
			CG_DrawPicOld((float)(i * (ammoW + 2)), ((y - (ammoH + 4)) - h), ammoW, h, cgs.media.teamStatusBar);
			trap_R_SetColor(NULL);
			++i;
		}
		++weaponIndex;
	}
	while (weaponIndex < 16);
}

static void CG_OSPDrawStatusBarAmmo2(float y)
{
	int weaponIndex;
	int ammoW;
	int ammoH;
	int posX;
	float posY;
	int i;
	float koeffH;
	int weaponHave;
	int statusH;
	int statusW;
	char* str;
	CG_OSPGetClientFontSize(&cf_Statusbar, &statusW, &statusH);
	CG_OSPGetClientFontSize(&cf_AmmoStatusbar, &ammoW, &ammoH);
	weaponHave = cg.snap->ps.stats[STAT_WEAPONS];
	i = 0;
	koeffH = (float)statusH / (float)ammoH;
	weaponIndex = 2;
	do
	{
		if (weaponHave & (1 << weaponIndex))
		{
			posX = (int)(i / koeffH) * 6 * ammoW;
			posY = (int)(y - (koeffH - (i % (int)koeffH)) * ammoH);
			if (weaponIndex == cg.weaponSelect)
			{
				trap_R_SetColor(colorMdGrey);
				CG_DrawPicOld(posX, posY, 5 * ammoW, ammoH, cgs.media.teamStatusBar);
			}
			CG_DrawPicOld(posX, posY, ammoW, ammoH, cg_weapons[weaponIndex].ammoIcon);
			str = va("%3i", cg.snap->ps.ammo[weaponIndex]);
			CG_OSPDrawStringOld(posX + ammoW * 2, posY, str, ammoW, ammoH, NULL, 0, 0);
			++i;
		}
		++weaponIndex;
	}
	while (weaponIndex < 16);
	trap_R_SetColor(NULL);
}


static void CG_OSPDrawCPMStatusbar0Attacker(float x)
{
	float size = 0;
	float tmp;
	vec3_t angles = {0, 0, 0};

	if (cg.damageTime != 0)
	{
		if (((float)cg.time - cg.damageTime) < 500.0f)
		{
			size = 60.0f * (1.5f - (0.5f * (((float)cg.time - cg.damageTime) / 500.0f)));
			x -= (0.5f * (size - 60.0f)) + (0.5f * (cg.damageX * (size - 60.0f)));

			cg.headStartYaw = (45.0f * cg.damageX) + 180.0f;
			cg.headEndYaw = 20.0f * cos((M_PI * (2.0f * (((float)(rand() & 0x7fff) / 32767.0f) - 0.5f)))) + 180.0f;
			cg.headEndPitch = 5.0f * cos((M_PI * (2.0f * (((float)(rand() & 0x7fff) / 32767.0f) - 0.5f))));
			cg.headStartTime = cg.time;
			cg.headEndTime = (int)((float)(cg.time + 100) + (2000.0f * ((float)(rand() & 0x7fff) / 32767.0f)));
		}
	}
	else
	{
		if (cg.time >= cg.headEndTime)
		{
			cg.headStartYaw = cg.headEndYaw;
			cg.headStartPitch = cg.headEndPitch;
			cg.headStartTime = cg.headEndTime;

			cg.headEndTime = (int)((float)(cg.time + 0x64) + (2000.0f * ((float)(rand() & 0x7fff) / 32767.0f)));
			cg.headEndYaw = (20.0f * cos((M_PI * (2.0f * (((float)(rand() & 0x7fff) / 32767.0f) - 0.5f))))) + 180.0f;
			cg.headEndPitch = 5.0f * cos((M_PI * (2.0f * (((float)(rand() & 0x7fff) / 32767.0f) - 0.5f))));
		}
		size = 60.0f;
	}
	if (cg.headStartTime > cg.time)
	{
		cg.headStartTime = cg.time;
	}

	tmp = (float)(cg.time - cg.headStartTime) / (float)(cg.headEndTime - cg.headStartTime);
	tmp = (tmp * tmp) * (3.0f - (2.0f * tmp));

	angles[1] = cg.headStartYaw + ((cg.headEndYaw - cg.headStartYaw) * tmp);
	angles[0] = cg.headStartPitch + ((cg.headEndPitch - cg.headStartPitch) * tmp);

	CG_OSPDrawPlayerIcon(x,
	                     480.0f - size,
	                     size,
	                     size,
	                     cg.snap->ps.clientNum,
	                     angles,
	                     cgs.clientinfo[cg.clientNum].colors.railCore);
}

static void CG_OSPDrawCPMStatusbar0(void)
{
	int health;
	playerState_t* ps;
	centity_t* es;
	vec3_t angles = {0, 0, 0};
	vec3_t pos;
	vec4_t color;
	qhandle_t model;
	int weapon;
	int armor;
	qhandle_t icon;
	static vec4_t colorCPMStatusbar0_1[2] = {{ 1.0f, 0.7f, 0.0f, 1.0f }, { 1.0f, 0.2f, 0.2f, 1.0f }};
	static vec4_t colorCPMStatusbar0_2 = { 1.0f, 1.0f, 1.0f, 1.0f };

	es = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	weapon = es->currentState.weapon;
	if (weapon)
	{
		icon = cg_weapons[weapon].ammoModel;
		if (icon)
		{
			pos[0] = 70.0f;
			pos[1] = 0;
			pos[2] = 0;
			angles[1] = (20.0f * sin((float)cg.time / 1000.0f)) + 90.0f;
			CG_OSPDraw3DModel(100.0f, 432.0f, 48.0f, 48.0f, icon, 0, pos, angles, NULL);
		}
	}

	CG_OSPDrawCPMStatusbar0Attacker(285.0f);
	if (cg.predictedPlayerState.powerups[PW_REDFLAG])
	{
		CG_DrawFlagModel(333.0f, 432.0f, 48.0f, 48.0f, TEAM_RED, qfalse);
	}
	else if (cg.predictedPlayerState.powerups[PW_BLUEFLAG])
	{
		CG_DrawFlagModel(333.0f, 432.0f, 48.0f, 48.0f, TEAM_BLUE, qfalse);
	}

	if (ps->stats[STAT_ARMOR])
	{
		model = cgs.media.armorModel[ps->stats[STAT_ARMOR_TYPE]];
		pos[0] = 90.0f;
		pos[1] = 0;
		pos[2] = -10.0f;
		angles[1] = ((float)(0x168 * (cg.time & 0x7ff)) / 2048.0f);
		CG_OSPDraw3DModel(470.0f, 432.0f, 48.0f, 48.0f, model, 0, pos, angles, NULL);
	}

	if (es->currentState.weapon)
	{
		health = ps->ammo[es->currentState.weapon];
		if (health > -1)
		{
			if (health > 0)
			{
				trap_R_SetColor(colorCPMStatusbar0_1[0]);
			}
			else
			{
				trap_R_SetColor(colorCPMStatusbar0_1[1]);
			}
			CG_DrawField(0, 432, 3, health, 32, 48);
			trap_R_SetColor(NULL);
			if (cg_draw3dIcons.integer == 0)
			{
				if (cg_drawIcons.integer)
				{
					icon = cg_weapons[cg.predictedPlayerState.weapon].weaponIcon;
					if (icon)
					{
						CG_DrawPicOld(100.0f, 432.0f, 48.0f, 48.0f, icon);
					}
				}
			}
		}
	}

	health = ps->stats[STAT_HEALTH];
	if (health > 100)
	{
		trap_R_SetColor(colorCPMStatusbar0_2);
	}
	else if (health > 30)
	{
		trap_R_SetColor(colorCPMStatusbar0_1[0]);
	}
	else if (health > 0)
	{
		trap_R_SetColor(colorCPMStatusbar0_1[(cg.time >> 0x8) & 1]);
	}
	else
	{
		trap_R_SetColor(colorCPMStatusbar0_1[0]);
	}

	CG_DrawField(185, 432, 3, health, 32, 48);
	CG_ColorForHealth(color, NULL);
	trap_R_SetColor(color);

	armor = ps->stats[STAT_ARMOR];
	if (armor > 0)
	{
		trap_R_SetColor(colorCPMStatusbar0_1[0]);
		CG_DrawField(370, 432, 3, armor, 32, 48);
		trap_R_SetColor(NULL);
		if (!cg_draw3dIcons.integer && cg_drawIcons.integer)
		{
			CG_DrawPicOld(470.0f, 432.0f, 48.0f, 48.0f, cgs.media.armorIcon[ps->stats[STAT_ARMOR_TYPE]]);
		}
	}
}

static void CG_OSPDrawCPMStatusbarsBar(int value, qhandle_t icon, int size, int direction, float* color)
{
	int x;
	int tmp_x;

	trap_R_SetColor(color);

	if (direction)
	{
		CG_DrawField(265, size + 6, 3, value, 16, 24);
		x = 267;
	}
	else
	{
		CG_DrawField(323, size + 6, 3, value, 16, 24);
		x = 373;
	}

	trap_R_SetColor(NULL);

	tmp_x = x;
	if (ch_graphs.integer != 0)
	{
		if (direction != 0)
		{
			tmp_x -= 202;
		}
		else
		{
			tmp_x += 2;
		}
		CG_FillRect(tmp_x, size + 16 + 2, 200.0f, 1.0f, colorWhite);
	}

	color[3] = 0.5f;

	if (value && ch_graphs.integer)
	{
		int w;
		if (value > 100)
		{
			w = 100 << 1;
		}
		else
		{
			w = value << 1;
		}

		tmp_x = x;
		if (direction != 0)
		{
			tmp_x -= w + 2;
		}
		else
		{
			tmp_x += 2;
		}
		CG_FillRect(tmp_x, size, w, 16.0f, color);

		if (value > 100)
		{
			w = (value << 1) - 200;
			tmp_x = x;
			if (direction != 0)
			{
				tmp_x -= w + 2;
			}
			else
			{
				tmp_x += 2;
			}
			size += 21;
			CG_FillRect(tmp_x, size, w, 16.0f, color);
		}
		if (direction != 0)
		{
			tmp_x -= 20;
		}
		else
		{
			tmp_x += w + 4.0f;
		}
		if ((icon > 0) && (cg_drawIcons.integer != 0))
		{
			CG_DrawPicOld(tmp_x, size, 16.0f, 16.0f, icon);
		}
	}
	else if (icon > 0)
	{
		if (cg_drawIcons.integer != 0)
		{
			tmp_x = x;
			if (direction != 0)
			{
				tmp_x -= 22;
			}
			else
			{
				tmp_x += 6;
			}
			CG_DrawPicOld((float)tmp_x, (float)size, 16.0f, 16.0f, icon);
		}
	}
	color[3] = 1.0f;
}

static void CG_OSPDrawCPMStatusbarBars(void)
{
	int health;
	int armor;
	qhandle_t icon;
	int colorIndex;

	static vec4_t CPMStatusBarsColors[4] =
	{
		{1.0f, 0.7f, 0.0f, 1.0f},
		{1.0f, 0.2f, 0.2f, 1.0f},
		{0.5f, 0.5f, 0.5f, 1.0f},
		{1.0f, 1.0f, 1.0f, 1.0f}
	};


	health = cg.snap->ps.stats[STAT_HEALTH];
	if ((health <= 30) && (cg.time % 1000 >= 500))
	{
		icon = -1;
	}
	else
	{
		icon = cgs.clientinfo[cg.snap->ps.clientNum].modelIcon;
	}

	if (health > 100)
	{
		colorIndex = 3;
	}
	else if (health > 30)
	{
		colorIndex = 0;
	}
	else
	{
		colorIndex = 1;
	}

	CG_OSPDrawCPMStatusbarsBar(health, icon, 432, 1, CPMStatusBarsColors[colorIndex]);


	armor = cg.snap->ps.stats[STAT_ARMOR];
	if (armor > 100)
	{
		colorIndex = 3;
	}
	else
	{
		colorIndex = 0;
	}

	icon = cgs.media.armorIcon[cg.snap->ps.stats[STAT_ARMOR_TYPE]];

	CG_OSPDrawCPMStatusbarsBar(armor, icon, 432, 0, CPMStatusBarsColors[colorIndex]);
	CG_FillRect(320.0f, 432.0f, 1.0f, 32.0f, CPMStatusBarsColors[3]);
}

static void CG_OSPDrawCPMStatusbar3Bars(int health, int armor, qhandle_t iconModel, qhandle_t iconArmor, int x, int y, float* colorHealth, float* colorArmor)
{
	int tmpX;
	float color[4];

	color[2] = 1.0f;
	color[1] = 1.0f;
	color[0] = 1.0f;
	color[3] = 0.7f;
	if (ch_graphs.integer != 0)
	{
		CG_FillRect((float)(x - 0x66), (float)(y - 2), 1.0f, 40.0f, color);
	}
	tmpX = x;
	colorHealth[3] = 0.5f;
	if (health != 0)
	{
		if (ch_graphs.integer != 0)
		{
			tmpX -= health + 2;
			CG_FillRect(tmpX, y, health, 16.0f, colorHealth);
			tmpX -= 20;
			if (iconModel > 0)
			{
				if (cg_drawIcons.integer != 0)
				{
					CG_DrawPicOld(tmpX, y, 16.0f, 16.0f, iconModel);
				}
			}
		}
	}
	else
	{
		if (iconModel > 0)
		{
			if (cg_drawIcons.integer != 0)
			{
				tmpX -= 11;
				CG_DrawPicOld(tmpX, y, 16.0f, 16.0f, iconModel);
			}
		}
	}
	colorHealth[3] = 1.0f;

	trap_R_SetColor(colorHealth);
	CG_DrawField(x, y, 3, health, 10, 15);
	trap_R_SetColor(NULL);

	tmpX = x;
	colorHealth[3] = 0.5f;
	y += 21;

	if (armor)
	{
		if (ch_graphs.integer)
		{
			tmpX -= armor + 2;
			CG_FillRect(tmpX, y, armor, 16.0f, colorArmor);
			tmpX -= 20;
			if ((iconModel > 0) && (cg_drawIcons.integer != 0))
			{
				CG_DrawPicOld((float)tmpX, (float)y, 16.0f, 16.0f, iconArmor);
			}
		}
	}
	else if ((iconArmor > 0) && cg_drawIcons.integer)
	{
		tmpX -= 11;
		CG_DrawPicOld(tmpX, y, 16.0f, 16.0f, iconArmor);
	}

	colorHealth[3] = 1.0f;
	trap_R_SetColor(colorArmor);
	CG_DrawField(x, y, 3, armor, 10, 15);
	trap_R_SetColor(NULL);
}

static void CG_OSPDrawCPMStatusbar3(void)
{
	int health;
	int armor;
	qhandle_t iconModel;
	qhandle_t iconArmor;
	int healthColorIndex;
	int armorColorIndex;
	centity_t* cent;

	static vec4_t CPMStatusBar3Colors[4] =
	{
		{1.0f, 0.7f, 0.0f, 1.0f},
		{1.0f, 0.2f, 0.2f, 1.0f},
		{0.5f, 0.5f, 0.5f, 1.0f},
		{1.0f, 1.0f, 1.0f, 1.0f}
	};

	health = cg.snap->ps.stats[STAT_HEALTH];
	if ((health <= 30) && (cg.time % 1000 >= 500))
	{
		iconModel = -1;
	}
	else
	{
		iconModel = cgs.clientinfo[cg.snap->ps.clientNum].modelIcon;
	}

	if (health > 100)
	{
		healthColorIndex = 3;
	}
	else if (health > 30)
	{
		healthColorIndex = 0;
	}
	else
	{
		healthColorIndex = 1;
	}

	armor = cg.snap->ps.stats[STAT_ARMOR];
	if (health > 100)
	{
		armorColorIndex = 3;
	}
	else
	{
		armorColorIndex = 0;
	}

	iconArmor = cgs.media.armorIcon[cg.snap->ps.stats[STAT_ARMOR_TYPE]];

	CG_OSPDrawCPMStatusbar3Bars(health, armor, iconModel, iconArmor, 225, 432, CPMStatusBar3Colors[healthColorIndex], CPMStatusBar3Colors[armorColorIndex]);

	cent = &cg_entities[cg.snap->ps.clientNum];

	if ((ch_weaponList.integer != 3) && cent->currentState.weapon)
	{
		playerState_t* ps;
		qhandle_t ammoModel;
		int ammo;
		int ammoColorIndex;

		ammoModel = cg_weapons[cent->currentState.weapon].ammoModel;
		ps = &cg.snap->ps;

		if (ammoModel)
		{
			vec3_t pos1 = {70.0f, 0.0f, 0.0f};
			vec3_t pos2 = {0.0f, 0.0f, 0.0f};

			pos2[1] = 20.0f * sin(((float)cg.time / 1000.0f)) + 90.0f;

			CG_OSPDraw3DModel(380.0f, 427.0f, 48.0f, 48.0f, ammoModel, 0, pos1, pos2, NULL);
		}

		ammo = ps->ammo[cent->currentState.weapon];
		if (ammo > -1)
		{
			qhandle_t ammoIcon;
			if (ammo > 0)
			{
				ammoColorIndex = 0;
			}
			else
			{
				ammoColorIndex = 1;
			}

			trap_R_SetColor(CPMStatusBar3Colors[ammoColorIndex]);
			CG_DrawField(280, 427, 3, ammo, 32, 48);
			trap_R_SetColor(NULL);

			if (cg_draw3dIcons.integer == 0)
			{
				if (cg_drawIcons.integer != 0)
				{
					ammoIcon = cg_weapons[cg.predictedPlayerState.weapon].ammoIcon;
					if (ammoIcon)
					{
						CG_DrawPicOld(380.0f, 427.0f, 48.0f, 48.0f, ammoIcon);
					}
				}
			}
		}
	}
}

static void CG_OSPDrawCPMStatusbar4Bars(int x, int y, int value, qhandle_t icon, vec4_t color)
{
	int reminder;
	int offset;

	if (cg_drawIcons.integer != 0)
	{
		y -= 32;
		if (icon > 0)
		{
			CG_DrawPicOld(x, y, 32.0f, 32.0f, icon);
		}
		y -= 5;
	}
	if (value >= 0)
	{
		y -= 16;
		trap_R_SetColor(color);
		CG_DrawField(x + 2, y + 1, 3, value, 10, 15);
		trap_R_SetColor(NULL);
		color[3] = 0.5f;

		if (value > 150)
		{
			reminder = value - 150;
			if (ch_graphs.integer != 0)
			{
				offset = reminder << 1;
				CG_FillRect(x - 2, y - offset, 8.0f, offset, color);
			}
			value = 150;
		}

		if (value > 100)
		{
			reminder = value - 100;
			if (ch_graphs.integer != 0)
			{
				offset = reminder << 1;
				CG_FillRect(x + 7, y - offset, 8.0f, offset, color);
			}
			value = 100;
		}

		if (value > 50)
		{
			reminder = value - 50;
			if (ch_graphs.integer != 0)
			{
				offset = reminder << 1;
				CG_FillRect(x + 17, y - offset, 8.0f, offset, color);
			}
			value = 50;
		}

		if (value != 0)
		{
			if (ch_graphs.integer != 0)
			{
				offset = value << 1;
				CG_FillRect(x + 26, y - offset, 8.0f, offset, color);
			}
		}
		color[3] = 1.0f;
	}
}

static void CG_OSPDrawCPMStatusbar4(void)
{
	int value;
	qhandle_t icon;
	int colorIndex;
	playerState_t* ps;
	centity_t* cent;
	static vec4_t CPMStatusBar4Colors[4] =
	{
		{1.0f, 0.7f, 0.0f, 1.0f},
		{1.0f, 0.2f, 0.2f, 1.0f},
		{0.5f, 0.5f, 0.5f, 1.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
	};

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	value = cg.snap->ps.stats[STAT_HEALTH];
	if ((value <= 30) && (cg.time % 1000 >= 500))
	{
		icon = -1;
	}
	else
	{
		icon = cgs.clientinfo[cg.snap->ps.clientNum].modelIcon;
	}

	if (value > 100)
	{
		colorIndex = 3;
	}
	else if (value > 30)
	{
		colorIndex = 0;
	}
	else
	{
		colorIndex = 1;
	}

	CG_OSPDrawCPMStatusbar4Bars(5, 470, value, icon, CPMStatusBar4Colors[colorIndex]);

	value = cg.snap->ps.stats[STAT_ARMOR];
	if (value > 100)
	{
		colorIndex = 3;
	}
	else
	{
		colorIndex = 0;
	}

	icon = cgs.media.armorIcon[ps->stats[STAT_ARMOR_TYPE]];

	CG_OSPDrawCPMStatusbar4Bars(62, 470, value, icon, CPMStatusBar4Colors[colorIndex]);

	if ((ch_weaponList.integer != 3) && (ch_weaponList.integer != 1))
	{
		value = ps->ammo[cent->currentState.weapon];
		if (value > -1)
		{
			if (value > 0)
			{
				colorIndex = 0;
			}
			else
			{
				colorIndex = 1;
			}
		}

		icon = cg_weapons[cg.predictedPlayerState.weapon].ammoIcon;

		CG_OSPDrawCPMStatusbar4Bars(119, 470, value, icon, CPMStatusBar4Colors[colorIndex]);
	}
}

static void CG_OSPDrawCPMStatusbar5Bars(int value, qhandle_t icon, int y, int x, float* color, qboolean flag)
{
	vec4_t color2;
	int offsetX;
	int tmpX;

	color2[3] = 1.0f;
	color2[2] = 1.0f;
	color2[1] = 1.0f;
	color2[0] = 1.0f;
	tmpX = x - 48;
	if (value > -1)
	{
		trap_R_SetColor(color);
		CG_DrawField(tmpX, y + 6, 3, value, 16, 24);
		trap_R_SetColor(NULL);
	}

	offsetX = tmpX - 3;

	if (ch_graphs.integer != 0)
	{
		if (value > -1)
		{
			tmpX = offsetX - 98;
			CG_FillRect(tmpX, y + 16 + 2, 96.0f, 1.0f, color2);
		}
	}
	else
	{
		if ((icon > 0) && cg_drawIcons.integer)
		{
			tmpX = offsetX - 16;
			CG_DrawPicOld(tmpX, y + 8 + 3, 16.0f, 16.0f, icon);
		}
		return;
	}

	color[3] = 0.5f;
	if (flag && value > 200)
	{
		value = 200;
	}

	if (value > 100)
	{
		float w;
		w = value - 100;
		value = 100;
		tmpX = (int)((float)offsetX - w);
		CG_FillRect((float)tmpX, (float)(y + 16 + 4 + 1), w, 16.0f, color);
		if ((icon > 0) && (cg_drawIcons.integer != 0))
		{
			tmpX -= 20;
			if (tmpX < (offsetX - 100))
			{
				tmpX = offsetX - 100;
			}
			CG_DrawPicOld((float)tmpX, (float)(y + 16 + 4 + 1), 16.0f, 16.0f, icon);
			icon = 0;
		}
	}

	if (value != 0)
	{
		tmpX = offsetX - value;
		CG_FillRect(tmpX, y, value, 16.0f, color);
		if ((icon > 0) && cg_drawIcons.integer)
		{
			tmpX -= 20;
			if (tmpX < (offsetX - 100))
			{
				tmpX = offsetX - 100;
			}
			CG_DrawPicOld(tmpX, y, 16.0f, 16.0f, icon);
		}
	}
	else
	{
		if ((icon > 0) && cg_drawIcons.integer)
		{
			tmpX = offsetX - 20;
			CG_DrawPicOld(tmpX, y, 16.0f, 16.0f, icon);
		}
	}

	color[3] = 1.0f;
}

static void CG_OSPDrawCPMStatusbar5(void)
{
	int value;
	qhandle_t icon;
	playerState_t* ps;
	centity_t* cent;
	int colorIndex;
	int y;

	static vec4_t CPMStatusBar5Colors[4] =
	{
		{1.0f, 0.7f, 0.0f, 1.0f},
		{1.0f, 0.2f, 0.2f, 1.0f},
		{0.5f, 0.5f, 0.5f, 1.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
	};

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	value = cg.snap->ps.stats[STAT_HEALTH];
	if ((value <= 30) && (cg.time % 1000 >= 500))
	{
		icon = -1;
	}
	else
	{
		icon = cgs.clientinfo[cg.snap->ps.clientNum].modelIcon;
	}

	if (value > 100)
	{
		colorIndex = 3;
	}
	else if (value > 30)
	{
		colorIndex = 0;
	}
	else
	{
		colorIndex = 1;
	}
	if (ch_graphs.integer == 0)
	{
		y = 275;
	}
	else
	{
		y = 220;
	}
	CG_OSPDrawCPMStatusbar5Bars(value, icon, 432, y, CPMStatusBar5Colors[colorIndex], qfalse);

	value = cg.snap->ps.stats[STAT_ARMOR];
	if (value > 100)
	{
		colorIndex = 3;
	}
	else
	{
		colorIndex = 0;
	}

	icon = cgs.media.armorIcon[ps->stats[STAT_ARMOR_TYPE]];
	if (ch_graphs.integer == 0)
	{
		y = 350;
	}
	else
	{
		y = 390;
	}
	CG_OSPDrawCPMStatusbar5Bars(value, icon, 432, y, CPMStatusBar5Colors[colorIndex], qfalse);

	value = ps->ammo[cent->currentState.weapon];
	if (value > -1)
	{
		if (value > 0)
		{
			colorIndex = 0;
		}
		else
		{
			colorIndex = 1;
		}
	}

	icon = cg_weapons[cg.predictedPlayerState.weapon].ammoIcon;
	if (ch_graphs.integer == 0)
	{
		y = 425;
	}
	else
	{
		y = 560;
	}
	CG_OSPDrawCPMStatusbar5Bars(value, icon, 432, y, CPMStatusBar5Colors[colorIndex], qtrue);

	if (ch_graphs.integer != 0)
	{
		CG_FillRect(230.0f, 432.0f, 1.0f, 35.0f, CPMStatusBar5Colors[3]);
		CG_FillRect(400.0f, 432.0f, 1.0f, 35.0f, CPMStatusBar5Colors[3]);
	}
	else
	{
		CG_FillRect(280.0f, 432.0f, 1.0f, 35.0f, CPMStatusBar5Colors[3]);
		CG_FillRect(355.0f, 432.0f, 1.0f, 35.0f, CPMStatusBar5Colors[3]);
	}
}

static void CG_OSPHUDDrawFlag(int x, int team)
{
	if (ch_drawHealthMeters.integer)
	{
		CG_DrawFlagModel(x, 427.0f, 48.0f, 48.0f, team, qfalse);
	}
	else
	{
		CG_DrawFlagModel(x, 432.0f, 48.0f, 48.0f, team, qfalse);
	}
}

static void CG_OSPDrawQ3CompWeapon1(int x, int y, int weapon, int num, vec4_t color, char* str, int max)
{
	int tmpY;
	vec4_t colorTmp;
	centity_t* cent;
	int w;
	int len;
	char* strTmp;

	cent = &cg_entities[cg.snap->ps.clientNum];

	tmpY = y + 2;

	if (cent->currentState.weapon == weapon)
	{
		CG_DrawRect(x, y, 120.0f, 12.0f, 1.0f, colorWhite);
		color[3] = 0.9f;
	}
	else
	{
		color[3] = 0.25f;
	}

	if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << weapon))
	{
		if (num != 0)
		{
			Vector4Copy(color, colorTmp);
			w = (int)((120.0f / max) * (float)num);
			if (w > 120)
			{
				w = 120;
			}
		}
		else
		{
			Vector4Copy(colorBlack, colorTmp);
			w = 120;
		}
		CG_FillRect(x, y, w, 12.0f, colorTmp);
	}
	CG_DrawStringExt(x + 1, tmpY, str, colorWhite, 0, 0, 8, 8, 18);

	tmpY = y;
	strTmp = va("%d", num);
	len = CG_DrawStrlen(strTmp);

	CG_DrawStringExt(x + 120 - (12 * len), tmpY, strTmp, colorWhite, 0, 0, 12, 12, 18);
}

static void CG_OSPDrawQ3CompWeapon2(int x, int y, int weapon, int num, float* color, char* str, int max)
{
	centity_t* cent;

	cent = &cg_entities[cg.snap->ps.clientNum];

	if (cent->currentState.weapon == weapon)
	{
		CG_DrawRect(x - 1, y - 1, 54.0f, 14.0f, 1.0f, colorWhite);
		color[3] = 1.0f;
	}
	else
	{
		color[3] = 0.7f;
	}

	if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << weapon))
	{
		qhandle_t icon;
		icon = cg_weapons[weapon].weaponIcon;
		if (icon)
		{
			CG_DrawPicOld(x, y, 12.0f, 12.0f, icon);
		}
	}

	CG_DrawStringExt(x + 16, y, va("%d", num), color, 0, 1, 12, 12, 18);
}

static void CG_OSPDrawQ3CompStatusbar(void)
{
	playerState_t* ps;
	centity_t* cent;
	vec3_t pos1;
	vec3_t pos2;
	float flagPos;
	int w;
	int h;
	int colorIndex;
	int w_tmp;
	int posX;
	int posY;
	vec4_t railColor;
	vec4_t shaftColor = { 0.8f, 0.8f, 0.8f, 0.5f };
	int health;
	int armor;

	static vec4_t Q3CompStatusBarColors[4] =
	{
		{1.0f, 0.69f, 0.0f, 1.0f},
		{1.0f, 0.2f, 0.2f, 1.0f},
		{0.5f, 0.5f, 0.5f, 1.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
	};


	if (cg_drawStatus.integer == 0)
	{
		return;
	}

	if (ch_TeamBackground.integer != 0)
	{
		CG_DrawTeamBackground(0, 420, 640, 60, 0.33f, cg.snap->ps.persistant[PERS_TEAM]);
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	pos1[0] = 0.0f;
	pos1[1] = 0.0f;
	pos1[2] = 0.0f;

	if (ch_StatusbarFormat.integer == 10 || ch_StatusbarFormat.integer == 12)
	{
		int weapon;
		qhandle_t icon;
		weapon = cent->currentState.weapon;

		if (weapon)
		{
			icon = cg_weapons[weapon].ammoModel;
			if (icon)
			{
				pos2[0] = 70.0f;
				pos2[1] = 0.0f;
				pos2[2] = 0.0f;

				pos1[1] = (20.0f * sin(((float)cg.time / 1000.0f))) + 90.0f;

				if (ch_StatusbarFormat.integer == 10)
				{
					CG_Draw3DModel(100.0f, 432.0f, 48.0f, 48.0f, icon, 0, pos2, pos1);
				}
				else
				{
					CG_Draw3DModel(105.0f, 432.0f, 48.0f, 48.0f, icon, 0, pos2, pos1);
				}
			}
		}
	}

	if (ch_drawHealthMeters.integer != 0)
	{
		flagPos = 415;
	}
	else
	{
		flagPos = 350;
	}
	if (cg.predictedPlayerState.powerups[PW_REDFLAG] != 0)
	{
		CG_OSPHUDDrawFlag(flagPos, TEAM_RED);
	}
	else if (cg.predictedPlayerState.powerups[PW_BLUEFLAG] != 0)
	{
		CG_OSPHUDDrawFlag(flagPos, TEAM_BLUE);
	}
	else if (cg.predictedPlayerState.powerups[PW_NEUTRALFLAG] != 0)
	{
		CG_OSPHUDDrawFlag(flagPos, TEAM_FREE);
	}

	CG_OSPGetClientFontSize(&cf_Statusbar, &w, &h);
	w_tmp = 3 * w;

	if (ch_StatusbarFormat.integer != 9)
	{
		if (cent->currentState.weapon)
		{
			qhandle_t icon;
			int ammo;
			ammo = ps->ammo[cent->currentState.weapon];
			if (ammo > -1)
			{
				if ((cg.predictedPlayerState.weaponstate == 3) && (cg.predictedPlayerState.weaponTime > 100))
				{
					colorIndex = 2;
				}
				else if (ammo > 0)
				{
					colorIndex = 0;
				}
				else
				{
					colorIndex = 1;
				}

				trap_R_SetColor(Q3CompStatusBarColors[colorIndex]);

				if (ch_StatusbarFormat.integer == 10)
				{
					CG_DrawField(96 - w_tmp, 476 - h, 3, ammo, w, h);
					trap_R_SetColor(NULL);

					if ((cg_draw3dIcons.integer == 0) && cg_drawIcons.integer)
					{
						icon = cg_weapons[cg.predictedPlayerState.weapon].weaponIcon;
						if (icon)
						{
							CG_DrawPicOld(w_tmp + 4, 476 - h, h, w, icon);
						}
					}
				}
				else
				{
					if (ch_StatusbarFormat.integer == 12)
					{
						if ((cg_draw3dIcons.integer == 0) && cg_drawIcons.integer)
						{
							icon = cg_weapons[cg.predictedPlayerState.weapon].weaponIcon;
							if (icon)
							{
								CG_DrawPicOld(111.0f, 438.0f, w, h, icon);
							}
						}
					}
					CG_DrawField(236 - w_tmp, 476 - h, 3, ammo, w, h);
					trap_R_SetColor(NULL);
				}
			}
		}
	}
//-----

	if (ch_StatusbarFormat.integer == 9)
	{
		posX = 5;
		posY = 466;

		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_BFG, ps->ammo[WP_BFG], colorBlue, "BFG", 50.0f);
		posY -= 14;

		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_GRENADE_LAUNCHER, ps->ammo[WP_GRENADE_LAUNCHER], colorGreen, "Grenades", 20.0f);
		posY -= 14;

		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_SHOTGUN, ps->ammo[WP_SHOTGUN], colorLtGrey, "Shotgun", 20.0f);
		posY -= 14;

		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_MACHINEGUN, ps->ammo[WP_MACHINEGUN], colorYellow, "Machinegun", 100.0f);
		posX += 126;
		posY = 466;

		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_PLASMAGUN, ps->ammo[WP_PLASMAGUN], colorMagenta, "Plasma", 100.0f);
		posY -= 14;

		Vector4Copy(colorGreen, railColor);
		railColor[0] += 0.4f;
		railColor[3] = 0.5f;

		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_RAILGUN, ps->ammo[WP_RAILGUN], railColor, "Railgun", 20.0f);
		posY -= 14;

		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_LIGHTNING, ps->ammo[WP_LIGHTNING], shaftColor, "Lightning", 200.0f);
		posY -= 14;

		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_ROCKET_LAUNCHER, ps->ammo[WP_ROCKET_LAUNCHER], colorRed, "Rockets", 20.0f);
	}
	else if (ch_StatusbarFormat.integer == 11)
	{
		posX = 5;
		posY = 466;

		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_PLASMAGUN, ps->ammo[WP_PLASMAGUN], colorMagenta, "Plasma", 100.0f);
		posY -= 14;

		Vector4Copy(colorGreen, railColor);
		railColor[0] += 0.4f;
		railColor[3] = 0.5f;

		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_RAILGUN, ps->ammo[WP_RAILGUN], railColor, "Railgun", 20.0f);
		posY -= 14;
		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_LIGHTNING, ps->ammo[WP_LIGHTNING], shaftColor, "Lightning", 200.0f);
		posY -= 14;
		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_ROCKET_LAUNCHER, ps->ammo[WP_ROCKET_LAUNCHER], colorRed, "Rockets", 20.0f);
		posY -= 14;
		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_GRENADE_LAUNCHER, ps->ammo[WP_GRENADE_LAUNCHER], colorGreen, "Grenades", 20.0f);
		posY -= 14;
		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_SHOTGUN, ps->ammo[WP_SHOTGUN], colorLtGrey, "Shotgun", 20.0f);
		posY -= 14;
		CG_OSPDrawQ3CompWeapon1(posX, posY, WP_MACHINEGUN, ps->ammo[WP_MACHINEGUN], colorYellow, "Machinegun", 100.0f);
	}
	else if (ch_StatusbarFormat.integer == 12)
	{
		posX = 5;
		posY = 466;
		CG_OSPDrawQ3CompWeapon2(posX, posY, WP_BFG, ps->ammo[WP_BFG], colorBlue, "BFG", 50.0f);
		posY -= 14;
		CG_OSPDrawQ3CompWeapon2(posX, posY, WP_GRENADE_LAUNCHER, ps->ammo[WP_GRENADE_LAUNCHER], colorGreen, "Grenades", 20.0f);
		posY -= 14;
		CG_OSPDrawQ3CompWeapon2(posX, posY, WP_SHOTGUN, ps->ammo[WP_SHOTGUN], colorLtGrey, "Shotgun", 20.0f);
		posY -= 14;
		CG_OSPDrawQ3CompWeapon2(posX, posY, WP_MACHINEGUN, ps->ammo[WP_MACHINEGUN], colorYellow, "Machinegun", 100.0f);
		posX += 54;
		posY = 466;
		CG_OSPDrawQ3CompWeapon2(posX, posY, WP_PLASMAGUN, ps->ammo[WP_PLASMAGUN], colorMagenta, "Plasma", 100.0f);
		posY -= 14;

		Vector4Copy(colorGreen, railColor);
		railColor[0] += 0.4f;
		railColor[3] = 0.5f;

		CG_OSPDrawQ3CompWeapon2(posX, posY, WP_RAILGUN, ps->ammo[WP_RAILGUN], railColor, "Railgun", 20.0f);
		posY -= 14;
		CG_OSPDrawQ3CompWeapon2(posX, posY, WP_LIGHTNING, ps->ammo[WP_LIGHTNING], shaftColor, "Lightning", 200.0f);
		posY -= 14;
		CG_OSPDrawQ3CompWeapon2(posX, posY, WP_ROCKET_LAUNCHER, ps->ammo[WP_ROCKET_LAUNCHER], colorRed, "Rockets", 20.0f);
	}

	health = ps->stats[STAT_HEALTH];
	if (ch_drawHealthMeters.integer)
	{
		vec4_t rectColor;
		int wBar;
		const char* str;
		int len;

		Vector4Copy(colorBlue, rectColor);
		rectColor[3] = 0.3f;

		if (health > 0x64)
		{
			wBar = 150;
			CG_FillRect(260.0f, 424.0f, (float)wBar, 24.0f, rectColor);
			wBar = (int)(1.5f * (float)(health - 100));
			rectColor[3] = 0.7f;
		}
		else if (health < 0)
		{
			wBar = 0;
		}
		else
		{
			(wBar = (int)(1.5f * (float)health));
		}
		CG_FillRect(260.0f, 424.0f, (float)wBar, 24.0f, rectColor);
		CG_DrawStringExt(260, 430, "Health", colorWhite, 0, 0, 10, 12, 0);
		str = va("%d", health);
		len = CG_DrawStrlen(str);
		CG_DrawStringExt((410 - (len * 16)), 426, str, colorWhite, 0, 0, 16, 22, 0);
	}
	else
	{
		vec4_t colorForHealth;
		if (health > 0x64)
		{
			trap_R_SetColor(Q3CompStatusBarColors[3]);
		}
		else if (health > 0x19)
		{
			trap_R_SetColor(Q3CompStatusBarColors[0]);
		}
		else if (health > 0)
		{
			trap_R_SetColor(Q3CompStatusBarColors[(cg.time >> 8) & 1]);
		}
		else
		{
			trap_R_SetColor(Q3CompStatusBarColors[1]);
		}

		CG_DrawField(354 - w_tmp, 476 - h, 3, health, w, h);
		CG_ColorForHealth(colorForHealth, NULL);
		trap_R_SetColor(colorForHealth);
	}
	//--
	armor = ps->stats[STAT_ARMOR];
	if (ch_drawHealthMeters.integer)
	{
		vec4_t colorArmor;
		int wBar;
		const char* str;
		int len;

		Vector4Copy(colorGreen, colorArmor);

		colorArmor[3] = 0.3f;

		if (armor > 100)
		{
			wBar = 150;
			CG_FillRect(260.0f, 452.0f, wBar, 24.0f, colorArmor);
			wBar = (int)(1.5f * (float)(armor - 100));
			colorArmor[3] = 0.7f;
		}
		else if (armor < 0)
		{
			wBar = 0;
		}
		else
		{
			wBar = (int)(1.5f * (float)armor);
		}

		CG_FillRect(260.0f, 452.0f, wBar, 24.0f, colorArmor);
		CG_DrawStringExt(0x104, 0x1cb, "Armor", colorWhite, 0, 0, 10, 12, 0);
		str = va("%d", armor);
		len = CG_DrawStrlen(str);
		CG_DrawStringExt(410 - (len * 16), 454, str, colorWhite, 0, 0, 16, 22, 0);
	}
	else if (armor > 0)
	{
		trap_R_SetColor(Q3CompStatusBarColors[0]);
		CG_DrawField(486 - w_tmp, 476 - h, 3, armor, w, h);
		if ((cgs.osp.server_mode & 0x2) != 0)
		{
			qhandle_t icon;
			icon = cgs.media.armorIcon[ps->stats[STAT_ARMOR_TYPE]];
			if (icon)
			{
				CG_DrawPicOld((float)(((486 - w_tmp) + (3 * w)) + 3), 452.0f, 24.0f, 24.0f, icon);
			}
		}
		trap_R_SetColor(NULL);
	}
}

static float CG_OSPDrawTeamInfo(float y)
{
	int i;
	int maxChars;
	int chatHeight;
	int h;
	int w;
	vec4_t hcolor;
	float chatY;
	int chatYOffset;

	if (cg_teamChatHeight.integer <= 0)
	{
		return y;
	}

	if (cg_teamChatHeight.integer < 8)
	{
		chatHeight = cg_teamChatHeight.integer;
	}
	else
	{
		chatHeight = 8;
	}

	CG_OSPGetClientFontSize(&cf_Teamchat, &w, &h);

	if (cgs.teamLastChatPos != cgs.teamChatPos)
	{
		if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer)
		{
			cgs.teamLastChatPos++;
		}

		chatYOffset = (cgs.teamChatPos - cgs.teamLastChatPos) * h;

		if (ch_TeamBackground.integer != 0)
		{
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
			CG_DrawPicOld(0, y - chatYOffset, 0x44200000, chatYOffset, cgs.media.teamStatusBar);
			trap_R_SetColor(NULL);
		}
		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--)
		{
			maxChars = (SCREEN_WIDTH - (cg_drawTeamOverlay.integer == 2 ? teamOverlayWidth : 0)) / w;

			if (ch_InverseTeamChat.integer != 0)
			{
				chatY = y - (float)((((chatHeight + 1) - cgs.teamChatPos) + i) * h);
			}
			else
			{
				chatY = y - (float)((cgs.teamChatPos - i) * h);
			}
			CG_OSPDrawStringOld(w, chatY, cgs.teamChatMsgs[i % chatHeight], w, h, NULL, maxChars, 0);
		}

	}
	return y - (float)(chatHeight * h);
}

static int CG_OSPGetPresistantTeam(void)
{
	/* В оригинале именно так, надо решить нужна ли эта обертка вообще */
	if (!CG_OSPIsGameTypeCA(cgs.gametype))
	{
		return cg.snap->ps.persistant[PERS_TEAM];
	}
	return cg.snap->ps.persistant[PERS_TEAM];
}

static void CG_OSPTeamOverlayReset(int w, qboolean right, const char* message)
{
	trap_Cvar_Set("ch_Teamoverlay", "p12n h/aw l");
	CG_Printf("%s\n", message);
	CG_OSPPrepareTeamOverlay(w, right);
}

static void CG_OSPPrepareTeamOverlay(int w, qboolean right)
{
	char* teamOverlayStr;
	int i;
	char teamOverlayChar;
	qboolean isNamePresent = qfalse;
	char string[OSPHUD_TEAMOVERLAY_STR_SIZE];
	int nameLen = 0;
	int numberOfNames = 0;
	int numberOfPowerups = 0;
	int numberOfHealths = 0;
	int numberOfArmors = 0;
	int numberOfAmmos = 0;
	int numberOfLocations = 0;


	memset(string, ' ', 128);
	string[127] = 0;
	memset(&teamOverlay, 0, sizeof(teamOverlay));

	if (strlen(ch_Teamoverlay.string) > 20)
	{
		CG_OSPTeamOverlayReset(w, right, "Rejected ch_Teamoverlay setting, too long");
		return;
	}

	for (i = 0, teamOverlayStr = ch_Teamoverlay.string; *teamOverlayStr ; ++teamOverlayStr)
	{
		teamOverlayChar = *teamOverlayStr;
		switch (teamOverlayChar)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				nameLen = 10 * nameLen + (teamOverlayChar - '0');
				break;
			case 'n':
			case 'N':
				teamOverlay.nameOffsetChar = i;
				teamOverlay.nameLenChar = nameLen ? nameLen : 12;
				i += teamOverlay.nameLenChar * w;
				isNamePresent = qtrue;
				++numberOfNames;
				break;
			case 'p':
			case 'P':
				teamOverlay.powerupOffsetChar = i;
				i +=  w;
				++numberOfPowerups;
				break;
			case 'h':
			case 'H':
				teamOverlay.healthOffsetChar = i;
				i += 3 * w;
				++numberOfHealths;
				break;
			case 'a':
			case 'A':
				teamOverlay.armorOffsetChar = i;
				i += 3 * w;
				++numberOfArmors;
				break;
			case 'w':
			case 'W':
				teamOverlay.ammoOffsetChar = i;
				i += w;
				++numberOfAmmos;
				break;
			case 'l':
			case 'L':
				teamOverlay.locationOffsetChar = i;
				i += cg_MaxlocationWidth.integer * w;
				++numberOfLocations;
				break;
			case ' ':
				i += w;
				break;
			default:
				string[i / w] = teamOverlayChar;
				i += w;
				break;
		}
	}

	if (numberOfNames > 1 || numberOfPowerups > 1 || numberOfHealths > 1 || numberOfArmors > 1 || numberOfAmmos > 1 || numberOfLocations > 1)
	{
		CG_OSPTeamOverlayReset(w, right, "Rejected ch_Teamoverlay setting, used an item more than once");
		return;
	}

	if (isNamePresent == qfalse)
	{
		CG_OSPTeamOverlayReset(w, right, "Rejected ch_Teamoverlay setting - useless");
		return;
	}

	string[i / w] = 0;

	/* remove leading and tailing spaces */
	{
		char* ptr = &string[0];
		char* start;

		while (*ptr && *ptr != ' ')
		{
			++ptr;
		}
		teamOverlay.strX = (ptr - &string[0]) * w;
		start = ptr;

		ptr += strlen(ptr) - 1;
		while (ptr != start && *ptr == ' ')
		{
			*ptr = 0;
			--ptr;
		}
		Q_strncpyz(teamOverlay.str, start, OSPHUD_TEAMOVERLAY_STR_SIZE);
	}
	teamOverlay.overlayWidthChar = i;
	teamOverlayWidth = i;

	if (numberOfPowerups)
	{
		teamOverlay.powerupOffsetChar += right ? 640 - i : 0;
		if (teamOverlay.powerupOffsetChar == 0)
		{
			++teamOverlay.powerupOffsetChar;
		}
	}

	if (numberOfNames)
	{
		teamOverlay.nameOffsetChar += right ? 640 - i : 0;
		if (teamOverlay.nameOffsetChar == 0)
		{
			++teamOverlay.nameOffsetChar;
		}
	}

	if (numberOfHealths)
	{
		teamOverlay.healthOffsetChar += right ? 640 - i : 0;
		if (teamOverlay.healthOffsetChar == 0)
		{
			++teamOverlay.healthOffsetChar;
		}
	}

	if (numberOfArmors)
	{
		teamOverlay.armorOffsetChar += right ? 640 - i : 0;
		if (teamOverlay.armorOffsetChar == 0)
		{
			++teamOverlay.armorOffsetChar;
		}
	}

	if (numberOfAmmos)
	{
		teamOverlay.ammoOffsetChar += right ? 640 - i : 0;
		if (teamOverlay.ammoOffsetChar == 0)
		{
			++teamOverlay.ammoOffsetChar;
		}
	}

	if (numberOfLocations)
	{
		teamOverlay.locationOffsetChar += right ? 640 - i : 0;
		if (teamOverlay.locationOffsetChar == 0)
		{
			++teamOverlay.locationOffsetChar;
		}
	}

	if (teamOverlay.strX)
	{
		teamOverlay.strX += right ? 640 - i : 0;
		if (teamOverlay.strX == 0)
		{
			++teamOverlay.strX;
		}
	}
}


static float CG_OSPDrawTeamOverlay(float y, qboolean right, qboolean upper)
{
	clientInfo_t* ci;
	int w;
	int h;
	char buf[16];
	int persistantTeam;
	int x;
	int offsetY;
	int i;
	vec4_t healthColor;


	persistantTeam = CG_OSPGetPresistantTeam();

	if ((persistantTeam != TEAM_RED  && persistantTeam != TEAM_BLUE) || numSortedTeamPlayers == 0)
	{
		return y;
	}

	CG_OSPGetClientFontSize(&cf_Teamoverlay, &w, &h);

	if (cf_Teamoverlay.modificationCount != modCfCountTeamOverlay ||
	        ch_Teamoverlay.modificationCount != modChCountTeamOverlay ||
	        cg_MaxlocationWidth.modificationCount != modCgMaxLocationWidth ||
	        cg_drawTeamOverlay.modificationCount != modCgDrawTeamOverlay
	   )
	{
		modChCountTeamOverlay = ch_Teamoverlay.modificationCount;
		modCfCountTeamOverlay = cf_Teamoverlay.modificationCount;
		modCgMaxLocationWidth = cg_MaxlocationWidth.modificationCount;
		modCgDrawTeamOverlay = cg_drawTeamOverlay.modificationCount;
		CG_OSPPrepareTeamOverlay(w, right);
		teamOverlayUnkFlag = qtrue;
	}

	teamOverlayUnkFlag = qfalse; //-V519
	if (right)
	{
		x = 640 - teamOverlay.overlayWidthChar;
	}
	else
	{
		x = 0;
	}
	offsetY = numSortedTeamPlayers * h;
	if (upper)
	{
		teamOverlayPosY = (int)(y + (float)offsetY);
	}
	else
	{
		y = (y - (float)offsetY);
		teamOverlayPosY = (int)y;
	}

	if (persistantTeam == TEAM_RED)
	{
		colorTeamOverlay[0] = 1.0f;
		colorTeamOverlay[2] = 0.0f;
	}
	else
	{
		colorTeamOverlay[0] = 0.0f;
		colorTeamOverlay[2] = 1.0f;
	}

	CG_OSPSetColor(colorTeamOverlay);
	CG_OSPDrawPic(x, y, (float)teamOverlay.overlayWidthChar, offsetY, cgs.media.teamStatusBar);
	CG_OSPSetColor(NULL);

	for (i = 0; i < numSortedTeamPlayers; ++i)
	{
		ci = &cgs.clientinfo[sortedTeamPlayers[i]];
		if (ci->infoValid)
		{
			if (ci->team != persistantTeam && ci->rt != persistantTeam)
			{
				continue;
			}

			if (teamOverlay.nameOffsetChar)
			{
				CG_OSPDrawStringOld(teamOverlay.nameOffsetChar, (int)y, ci->name, w, h, NULL, teamOverlay.nameLenChar, 0);
			}

			if (teamOverlay.healthOffsetChar || teamOverlay.armorOffsetChar)
			{
				CG_GetColorForHealth(ci->health, ci->armor, healthColor, NULL);
			}

			if (teamOverlay.healthOffsetChar)
			{
				Com_sprintf(buf, 16, "%3i", ci->health);
				CG_OSPDrawStringOld(teamOverlay.healthOffsetChar, (int)y, buf, w, h, healthColor, 0, 0);
			}

			if (teamOverlay.armorOffsetChar)
			{
				Com_sprintf(buf, 16, "%3i", ci->armor);
				CG_OSPDrawStringOld(teamOverlay.armorOffsetChar, (int)y, buf, w, h, healthColor, 0, 0);
			}
			if (teamOverlay.strX)
			{
				CG_OSPDrawStringOld(teamOverlay.strX, (int)y, teamOverlay.str, w, h, NULL, 0, 0);
			}
			if (teamOverlay.ammoOffsetChar)
			{
				if (cg_weapons[ci->curWeapon].ammoIcon)
				{
					CG_OSPDrawPic((float)teamOverlay.ammoOffsetChar, y, (float)w, (float)h, cg_weapons[ci->curWeapon].ammoIcon);
				}
				else
				{
					CG_OSPDrawPic((float)teamOverlay.ammoOffsetChar, y, (float)w, (float)h, cgs.media.deferShader);
				}
			}
			if (cg_MaxlocationWidth.integer != 0)
			{
				const char* location = NULL;
				if (teamOverlay.locationOffsetChar != 0)
				{
					if (customLocationsEnabled != 0)
					{
						location = CG_CustomLocationsGetName(ci->customLocation);
					}
					else
					{
						if (ch_ColorLocations.integer != 0 && cgs.gametype == GT_CTF)
						{
							location = CG_GetCTFLocation(ci->location);
						}
						else
						{
							location = CG_ConfigString(CS_LOCATIONS + ci->location);
						}
					}
					if (!location || *location == 0)
					{
						location = "unknown";
					}
					CG_OSPDrawStringOld(teamOverlay.locationOffsetChar, (int)y, location, w, h, NULL, cg_MaxlocationWidth.integer, 0);
				}
			}
			if (teamOverlay.powerupOffsetChar)
			{
				int k = 0;
				int px = teamOverlay.powerupOffsetChar;
				gitem_t* gi;
				do
				{
					if (cgs.osp.gameTypeFreeze && ci->health <= 0)
					{
						CG_OSPDrawPic((float)px, y, (float)w, (float)h, cgs.media.noammoShader);
						px -= w;
						break;
					}
					else if (ci->powerups & (1 << k))
					{
						gi = BG_FindItemForPowerup(k);
						if (gi)
						{
							CG_OSPDrawPic((float)px, y, (float)w, (float)h, trap_R_RegisterShader(gi->icon));
							px -= w;
						}
					}
				}
				while (++k < 16);
			}
			y += (float)h;
		}
	}

	return (float)teamOverlayPosY;
}

static float CG_OSPDrawWeaponBarOld(float pos_y)
{
	int weaponNum;
	int w;
	int x;
	int h;
	int y;
	int weapons;
	int numberOfWeapons = 0;
	float* color;

	if (ch_Weaponswitch.integer == 1 || ch_Weaponswitch.integer == 9)
	{
		color = CG_FadeColor(cg.weaponSelectTime, 1400);
		if (color == NULL)
		{
			return pos_y;
		}
	}
	else if (ch_Weaponswitch.integer == 2)
	{
		color = colorWhite;
	}
	else
	{
		return pos_y;
	}

	CG_OSPGetClientFontSize(&cf_WeaponSwitch, &w, &h);

	trap_R_SetColor(color);

	cg.itemPickupTime = 0;
	weapons = cg.snap->ps.stats[STAT_WEAPONS];

	for (weaponNum = 1; weaponNum < 16; ++weaponNum)
	{
		if (weapons & (1 << weaponNum))
		{
			++numberOfWeapons;
		}
	}

	x = 320 - ((numberOfWeapons * (w + 4)) / 2);
	y = (int)((pos_y - (float)h) - 4.0f);
	for (weaponNum = 1; weaponNum < 16; ++weaponNum)
	{
		if (weapons & (1 << weaponNum))
		{
			CG_RegisterWeapon(weaponNum);
			CG_DrawPicOld(x, y, w, h, cg_weapons[weaponNum].weaponIcon);
			if (weaponNum == cg.weaponSelect)
			{
				CG_DrawPicOld(x - 2, y - 2, w + 4, h + 4, cgs.media.selectShader);
			}
			if (cg.snap->ps.ammo[weaponNum] == 0)
			{
				CG_DrawPicOld((float)x, (float)y, (float)w, (float)h, cgs.media.noammoShader);
			}
			x += w + 4;
		}
	}


	if (ch_Weaponswitch.integer == 1 && cg_weapons[cg.weaponSelect].weaponIcon && CG_OSPGetClientFontSize(&cf_WeaponName, &w, &h))
	{
		const char* name = cg_weapons[cg.weaponSelect].item->pickup_name;
		if (name)
		{
			x = (640 - (CG_DrawStrlen(name) * w)) / 2;
			CG_OSPDrawStringOld(x, y - h - 0x2, name, w, h, color, 0, 0);
			pos_y -= h;
		}
	}

	trap_R_SetColor(NULL);

	return ((pos_y - (float)h) - 4);
}

static void CG_OSPDrawWeaponBar_4_5(float pos_y)
{
	int weaponNum;
	int h;
	int y;
	int x;
	int w;
	int statWeapons;
	int numberOfWeapons;
	float* color = colorWhite;
	char* str;

	if (ch_Weaponswitch.integer == 2)
	{
		(cg.itemPickupTime = 0);
	}

	statWeapons = cg.snap->ps.stats[STAT_WEAPONS];

	numberOfWeapons = 0;

	for (weaponNum = 1; weaponNum < 16; ++weaponNum)
	{
		if ((statWeapons & (1 << weaponNum)) != 0)
		{
			++numberOfWeapons;
		}
	}

	CG_OSPGetClientFontSize(&cf_WeaponSwitch, &w, &h);

	x = 6.0f;
	y = ((((pos_y - 64.0f) / 2.0f) + 64.0f) - (float)((numberOfWeapons * (h + 8)) / 2.0f));


	if (ch_Weaponswitch.integer == 4)
	{
		color = CG_FadeColor(cg.weaponSelectTime, 1400);
		if (color == NULL) return;
	}
	else if (ch_Weaponswitch.integer == 5)
	{
		color = colorWhite;
	}

	trap_R_SetColor(color);
	statWeapons = cg.snap->ps.stats[STAT_WEAPONS];
	for (weaponNum = 1; weaponNum < 16; ++weaponNum)
	{
		if (statWeapons & (1 << weaponNum))
		{
			CG_RegisterWeapon(weaponNum);
			CG_DrawPicOld(x, y, (float)w, (float)h, cg_weapons[weaponNum].weaponIcon);
			if (weaponNum == cg.weaponSelect)
			{
				CG_DrawPicOld(x - 4.0f, y - 4.0f, w + 8, h + 8, cgs.media.selectShader);
			}
			if (cg.snap->ps.ammo[weaponNum] == 0)
			{
				CG_DrawPicOld(x, y, (float)w, (float)h, cgs.media.noammoShader);
			}
			if (ch_Weaponswitch.integer == 5)
			{
				if (cg.snap->ps.ammo[weaponNum] != -1)
				{
					str = va("%3i", cg.snap->ps.ammo[weaponNum]);
					CG_OSPDrawStringOld(x + w + 8, y + ((h + 8) / 2 - 8), str, 8, 8, NULL, 0, 0);
				}
			}
			y += h + 8;
		}
	}

	trap_R_SetColor(NULL);
}

float CG_OSPDrawPickupItem(float y)
{
	int h;
	int w;
	if (cg.snap->ps.stats[STAT_HEALTH] <= 0)
	{
		return y;
	}
	CG_OSPGetClientFontSize(&cf_Pickupstring, &w, &h);
	y -= h;

	if (cg.itemPickup)
	{
		float* color;
		color = CG_FadeColor(cg.itemPickupTime, 3000);
		if (color)
		{
			CG_RegisterItemVisuals(cg.itemPickup);
			trap_R_SetColor(color);
			CG_DrawPicOld(8.0f, y, w, (float)h, cg_items[cg.itemPickup].icon);
			CG_OSPDrawStringOld(w + 8 + (w / 3), y, bg_itemlist[cg.itemPickup].pickup_name, w, h, color, 0, 0);
			trap_R_SetColor(NULL);
		}
	}
	return y;
}

/*
 * ch_drawFlagNames <0-5> (default = 0)
 *  0 - default like id
 *  1 - draw flag carrier's name (but no icons)
 *  2 - draw flag carrier's name and the icons
 *  3 - draw flag carrier's name (but no icon) in the lower right
 *      corner (Q3comp statusbars only)
 *  4 - draw flag carrier's name and icon in the lower right
 *      corner (Q3comp Statusbars only)
 */
static float CG_OSPHUDDrawScores(float y)
{
	int fontScoresH;
	int fontScoresW;
	int s1;
	int s2;
	int fontStatusbarH;
	int fontStatusbarW;
	float x;
	float y1; //40
	const char* s; //30
	int len; //9c
	int w; //2c
	//
	int score;

	vec4_t colorScores[3] =
	{
		{0.0f, 0.0f, 1.0f, 0.33}, //3aa0
		{1.0f, 0.0f, 0.0f, 0.33}, //3ab0
		{1.0f, 1.0f, 1.0f, 0.33}, //3ac0
	};

	CG_OSPGetClientFontSize(&cf_Scores, &fontScoresW, &fontScoresH);

	s1 = cgs.scores1;
	s2 = cgs.scores2;

	if ((ch_StatusbarFormat.integer >= 9) && (ch_StatusbarFormat.integer <= 12))
	{
		if ((cg_lagometer.integer != 1) && (cg_drawTeamOverlay.integer != 2))
		{
			if (cgs.gametype == GT_CTF && !ch_drawFlagNames.integer)
			{
				x = SCREEN_WIDTH;
				y = (float)(480 - (fontScoresH + 8));

			}
			else
			{
				CG_OSPGetClientFontSize(&cf_Statusbar, &fontStatusbarW, &fontStatusbarH);
				x = SCREEN_WIDTH;
				y = (float)((472 - fontStatusbarH) - fontScoresH);
			}
		}
		else
		{
			if ((cgs.gametype != GT_TEAM) && (cgs.osp.gameTypeFreeze == 0) && (CG_OSPIsGameTypeCA(cgs.gametype) == 0) && (cgs.gametype != GT_CTF) && ch_drawFlagNames.integer)
			{
				x = SCREEN_WIDTH;
				y -= 8.0f;
			}
			else
			{
				x = 584;
				y = (float)(480 - (fontScoresH + 8));
			}
		}
	}
	else
	{
		x = SCREEN_WIDTH;
		y -= (float)(fontScoresH + 10);
	}

	y1 = y;
	/* Team games */
	if (cgs.gametype >= GT_TEAM)
	{
		qboolean haveFlag = qfalse;

		s = NULL;
		if (cgs.osp.gameTypeFreeze == 0)
		{
			s =  va("%2i", s2);
		}
		else
		{
			s = va("(%i) %2i", cgs.osp.osp_teamcount2, s2);
		}

		len = CG_DrawStrlen(s);
		w = len * fontScoresW + 8;
		x -=  w + 2;

		if (cgs.osp.gameTypeFreeze != 0)
		{
			if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_BLUE)
			{
				CG_OSPDrawPoly(x - 1, y - 5.0f, w + 2, fontScoresH + 8 + 2, colorScores[2]);
			}
		}
		CG_OSPDrawPoly(x, y - 4.0f, w, fontScoresH + 8, colorScores[0]);

		if (cgs.osp.gameTypeFreeze == 0)
		{
			if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
			{
				CG_OSPDrawPic(x, y - 4.0f, w, fontScoresH + 8, cgs.media.selectShader);
			}
		}
		CG_OSPDrawStringWithShadow(x + 4, y, s, fontScoresW, fontScoresH, NULL, 0);
		if (cgs.gametype == GT_CTF)
		{
			gitem_t* item;

			item = BG_FindItemForPowerup(PW_BLUEFLAG);
			if (item)
			{
				y1 = y - (float)fontScoresH - 8.0f;
				if ((cgs.blueflag >= 0) && (cgs.blueflag <= 2))
				{
					if (!ch_drawFlagNames.integer)
					{
						CG_DrawPicOld(x, y1 - 4.0f, w, fontScoresH + 8, cgs.media.blueFlagShader[cgs.blueflag]);
					}
					else
					{

						if (ch_drawFlagNames.integer == 2 || ch_drawFlagNames.integer == 4)
						{
							x -= (fontScoresW << 1) + 8;
							CG_DrawPicOld(x, y - 4.0f, w, fontScoresH + 8, cgs.media.blueFlagShader[cgs.blueflag]);
						}

						switch (cgs.blueflag)
						{
							default:
							case 0:
								break;
							case 1:
								//22189
								if (cgs.osp.flagStatus != 0)
								{
									s = cgs.clientinfo[cgs.osp.blueflag].name;
									haveFlag = qtrue;
								}
								else
								{
									int i;
									for (i = 0; i < cgs.maxclients; ++i)
									{
										s = cgs.clientinfo[i].name;
										if (cgs.clientinfo[i].powerups & (1 << PW_BLUEFLAG))
										{
											haveFlag = qtrue;
											break;
										}
									}
								}

								if (haveFlag && s)
								{
									int wstr;
									len = CG_DrawStrlen(s + 4);
									wstr = (len * fontScoresW) / 2.0f + 10;
									if (wstr > 120)
									{
										wstr = 120;
									}
									CG_OSPDrawStringWithShadow(x - wstr, y + 4.0f, s + 4, fontScoresW / 2, (3 * fontScoresH) / 4, colorWhite, 15);
								}
								else
								{
									len = CG_DrawStrlen("Taken");
									CG_OSPDrawStringWithShadow(x - (((len * fontScoresW) / 2.0f) + 10), y + 4.0f, "Taken", fontScoresW / 2, 3 * fontScoresH / 4, colorWhite, 15);
								}
								break;
							case 2:
								len = CG_DrawStrlen("dropped");
								CG_OSPDrawStringWithShadow(x - (((3 * len * fontScoresW) / 4.0f) + 10), y + 4.0f, "^xff8000^b^f^3DROPPED^F^7   FLAG", (3 * fontScoresW) / 4, (3 * fontScoresH) / 4, colorWhite, 15);
								break;
						}
					}
				}
			}
		}

		if (cgs.osp.gameTypeFreeze == 0)
		{
			s =  va("%2i", s1);
		}
		else
		{
			s = va("(%i) %2i", cgs.osp.osp_teamcount1, s1);
		}

		len = CG_DrawStrlen(s);
		w = len * fontScoresW + 8;

		if ((cgs.gametype == GT_CTF) && (ch_drawFlagNames.integer == 0))
		{
			x -= w;
		}
		else if (cgs.gametype == GT_CTF && (ch_drawFlagNames.integer == 2 || ch_drawFlagNames.integer == 4))
		{
			x += w;
			y -= fontScoresH + 8;
		}
		else
		{
			if (ch_StatusbarFormat.integer <= 9 && cgs.gametype != GT_CTF)
			{
				x -= w;
			}
			else
			{
				y -= fontScoresH + 8;
			}
		}

		if (cgs.osp.gameTypeFreeze)
		{
			if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_RED)
			{
				CG_OSPDrawPoly(x - 1, y - 5.0f, w + 2, fontScoresH + 8 + 2, colorScores[2]);
			}
		}

		CG_OSPDrawPoly(x, y - 4.0f, w, fontScoresH + 8, colorScores[1]);
		if (cgs.osp.gameTypeFreeze == 0)
		{
			if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
			{
				CG_OSPDrawPic(x, y - 4.0f, w, fontScoresH + 8, cgs.media.selectShader);
			}
		}

		CG_OSPDrawStringWithShadow(x + 4, y, s, fontScoresW, fontScoresH, NULL, 0);

		if (cgs.gametype == GT_CTF)
		{
			gitem_t* item;//60
			item = BG_FindItemForPowerup(PW_REDFLAG);

			if (item)
			{
				y1 = y - fontScoresH - 8.0f;
				if ((cgs.redflag >= 0) && (cgs.redflag <= 2))
				{
					if (!ch_drawFlagNames.integer)
					{
						CG_DrawPicOld(x, y1 - 4.0f, w, fontScoresH + 8, cgs.media.redFlagShader[cgs.redflag]);
					}
					else
					{
						if (ch_drawFlagNames.integer == 2 || ch_drawFlagNames.integer == 4)
						{
							x -= (fontScoresW << 1) + 8;
							CG_DrawPicOld(x, y - 4.0f, w, fontScoresH + 8, cgs.media.redFlagShader[cgs.redflag]);
						}

						switch (cgs.redflag)
						{
							default:
							case 0:
								break;
							case 1:
								haveFlag = qfalse;
								if (cgs.osp.flagStatus != 0)
								{
									s = cgs.clientinfo[cgs.osp.redflag].name;
									haveFlag = qtrue;
								}
								else
								{
									int i;
									for (i = 0; i < cgs.maxclients; ++i)
									{
										s = cgs.clientinfo[i].name;
										if (cgs.clientinfo[i].powerups & (1 << PW_REDFLAG))
										{
											haveFlag = qtrue;
											break;
										}
									}
								}

								if (haveFlag && s)
								{
									int wstr; // 80
									len = CG_DrawStrlen(s + 4);
									wstr = (len * fontScoresW) / 2.0f + 10;
									if (wstr > 120)
									{
										wstr = 120;
									}
									CG_OSPDrawStringWithShadow(x - wstr, y + 4.0f, s + 4, fontScoresW / 2, (3 * fontScoresH) / 4, colorWhite, 15);
								}
								else
								{
									len = CG_DrawStrlen("Taken");
									CG_OSPDrawStringWithShadow(x - (((len * fontScoresW) / 2.0f) + 10), y + 4.0f, "Taken", fontScoresW / 2, 3 * fontScoresH / 4, colorWhite, 15);
								}
								break;
							case 2:
								//22254
								len = CG_DrawStrlen("dropped");
								CG_OSPDrawStringWithShadow(x - (((3 * len * fontScoresW) / 4.0f) + 10), y + 4.0f, "^xff8000^b^f^3DROPPED^F^7   FLAG", (3 * fontScoresW) / 4, (3 * fontScoresH) / 4, colorWhite, 15);
								break;
						}
					}
				}
			}
		}
		if (((cgs.gametype == GT_CTF) && (!ch_drawFlagNames.integer)) ||
		        ((cgs.gametype != GT_CTF) && (ch_StatusbarFormat.integer < 9)))
		{
			int limit;

			if (cgs.gametype == GT_CTF)
			{
				limit = cgs.capturelimit;
			}
			else
			{
				limit = cgs.fraglimit;
			}
			if (limit)
			{
				s = va("%2i", limit);
				len = CG_DrawStrlen(s);
				x -= (len * fontScoresW) + 8;

				CG_OSPDrawStringWithShadow(x + 4, y, s, fontScoresW, fontScoresH, NULL, 0);
			}
		}

	}
	/* FFA, 1vs1 or single player games */
	else
	{
		qboolean    spectator;
		x = SCREEN_WIDTH;

		y -= fontScoresH + 8;
		y1 = y;

		fontStatusbarH = fontStatusbarW;
		score = cg.snap->ps.persistant[PERS_SCORE];
		spectator = (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR);

		// always show your score in the second box if not in first place
		if (s1 != score && spectator == 0)
		{
			s2 = score;
		}
		if (s2 != SCORE_NOT_PRESENT)
		{
			vec4_t color;
			s = va("%2i", s2);
			len = CG_DrawStrlen(s);
			w = len * fontScoresW + 8;
			x -= w;

			color[0] = 1.0f;
			color[1] = 0.0f;
			color[2] = 0.0f;
			color[3] = 0.33f;

			CG_OSPDrawPoly(x, y - 4.0f, w, fontScoresH + 8, color);
			if (!spectator && (score == s2) && (score != s1))
			{
				CG_OSPDrawPic(x, y - 4.0f, w, fontScoresH + 8, cgs.media.selectShader);
			}
			CG_OSPDrawStringWithShadow(x + 4, y, s, fontScoresW, fontScoresH, NULL, 0);
		}

		// first place
		if (s1 != SCORE_NOT_PRESENT)
		{
			vec4_t color;
			s = va("%2i", s1);
			w = CG_DrawStrlen(s) * fontScoresW + 8;
			x -= w;

			color[0] = 0.0f;
			color[1] = 0.0f;
			color[2] = 1.0f;
			color[3] = 0.33f;

			CG_OSPDrawPoly(x, y - 4.0f, w, fontScoresH + 8, color);
			if (!spectator && score == s1)
			{
				CG_OSPDrawPic(x, y - 4.0f, w, fontScoresH + 8, cgs.media.selectShader);
			}
			CG_OSPDrawStringWithShadow(x + 4, y, s, fontScoresW, fontScoresH, NULL, 0);
		}

		if (cgs.fraglimit != 0)
		{
			s = va("%2i", cgs.fraglimit);
			w = CG_DrawStrlen(s) * fontScoresW + 8;
			x -= w;
			CG_OSPDrawStringWithShadow(x + 4, y, s, fontScoresW, fontScoresH, NULL, 0);
		}

	}

	return y1 - 8;
}

static float CG_OSPDrawHoldableItem(float y)
{
	int hi = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if (hi)
	{
		CG_RegisterItemVisuals(hi);
		y -= 48.0f;
		CG_DrawPicOld(592.0f, y, 48.0f, 48.0f, cg_items[hi].icon);
	}
	return y;
}

static float CG_OSPDrawPowerups(float x, float y)
{
	int     sorted[MAX_POWERUPS];
	int     sortedTime[MAX_POWERUPS];
	int     i, j, k;
	int     active;
	playerState_t*   ps;
	int     t;
	gitem_t* item;
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

			CG_DrawPicOld(x + 64.0f, y + ICON_SIZE / 2.0f - size / 2.0f,
			              size, size, trap_R_RegisterShader(item->icon));
		}
	}
	trap_R_SetColor(NULL);

	return y;
}

static float CG_OSPDrawWeaponSelect(float x, float y, const float* color)
{
	int weaponNum;
	int h;
	int w;
	int statWeapons;

	CG_OSPGetClientFontSize(&cf_WeaponSwitch, &w, &h);
	trap_R_SetColor(color);

	if (ch_Weaponswitch.integer == 2)
	{
		(cg.itemPickupTime = 0);
	}

	statWeapons = cg.snap->ps.stats[STAT_WEAPONS];

	for (weaponNum = 1; weaponNum < 16; ++weaponNum)
	{
		if (statWeapons & (1 << weaponNum))
		{
			CG_RegisterWeapon(weaponNum);
			CG_DrawPicOld(x, y, w, h, cg_weapons[weaponNum].weaponIcon);
			if (weaponNum == cg.weaponSelect)
			{
				CG_DrawPicOld(x - 2, y - 2, w + 4, w + 4, cgs.media.selectShader);
			}
			if (cg.snap->ps.ammo[weaponNum] == 0)
			{
				CG_DrawPicOld(x, y, (float)w, (float)h, cgs.media.noammoShader);
			}
			y += h + 2.0f;
		}
	}

	trap_R_SetColor(0);
	return y;
}

static float CG_OSPDrawWeaponBar_2_3(float pos_y)
{
	int h;
	int w;
	float* color;
	int weapons;
	int weaponNum;
	int powerupNum;
	int powerupTime;
	int numberOfPowerups = 0;
	int numberOfWeapons;
	int y;

	CG_OSPGetClientFontSize(&cf_WeaponSwitch, &w, &h);

	for (powerupNum = 0 ; powerupNum < 16; ++powerupNum)
	{
		powerupTime = cg.snap->ps.powerups[powerupNum];

		if (powerupTime)
		{
			powerupTime -= cg.time;
			if (powerupTime >= 0 && powerupTime <= 999000)
			{
				++numberOfPowerups;
			}
		}
	}

	weapons = cg.snap->ps.stats[STAT_WEAPONS];
	numberOfWeapons = 0;

	for (weaponNum = 1; weaponNum < 16; ++ weaponNum)
	{
		if (weapons & (1 << weaponNum))
		{
			++numberOfWeapons;
		}
	}

	if (ch_Weaponswitch.integer == 2)
	{
		color = CG_FadeColor(cg.weaponSelectTime, 1400);
		if (color == NULL)
		{
			return CG_OSPDrawPowerups(528.0f, pos_y);
		}
	}
	else if (ch_Weaponswitch.integer == 0x3)
	{
		color = colorWhite;
	}
	else
	{
		return CG_OSPDrawPowerups(528.0f, pos_y);
	}

	y = (((pos_y - 64.0f) / 2.0f) + 64.0f) - (float)(numberOfWeapons * ((5 * h) / 8.0f));
	CG_OSPDrawWeaponSelect(640 - (5 * w) / 4.0f, y, color);

	if ((float)((y + (numberOfWeapons * ((5 * h) / 4.0f))) + (48 * numberOfPowerups)) > pos_y)
	{
		CG_OSPDrawPowerups(640 - (5 * w) / 4.0f - 48 - 64, pos_y);
	}
	else
	{
		CG_OSPDrawPowerups(528.0f, pos_y);
	}

	return pos_y;
}

float CG_OSPDrawPing(float y)
{
	int ping;
	int color;

	if (cg.demoPlayback != 0) return y;

	if (cg.time > cgs.osp.pingPrintTime)
	{
		color = 7;
		if (cgs.osp.pingCount == 0)
		{
			ping = 0;
		}
		else
		{
			ping = (int)(cgs.osp.pingMs);
		}
		if (ping > 999)
		{
			ping = 999;
		}
		cgs.osp.pingPrintTime = cg.time + 1000;
		if (cg_drawPing.integer > 1)
		{
			if (ping > 40)
			{
				if (ping < 70)
				{
					color = 2;
				}
				else if (ping < 120)
				{
					color = 3;
				}
				else if (ping < 190)
				{
					color = 8;
				}
				else if (ping < 400)
				{
					color = 6;
				}
				else
				{
					color = 1;
				}
			}
		}
		Q_strncpyz(cgs.osp.pingString, va("^%d%dms", color, ping), 16);
		cgs.osp.pingStringPositionX = ((635 - (12 * CG_DrawStrlen(cgs.osp.pingString)))) + (cg_lagometer.integer > 0 ? 4 : 0);
	}
	CG_OSPDrawStringWithShadow(cgs.osp.pingStringPositionX, y + 4.0f, cgs.osp.pingString, 12, 12, colorWhite, 0);
	return y + 16.0f;
}

static float CG_OSPDrawSplitter(float x, float y)
{
	CG_DrawPicOld(x, y, 76.0f, 1.0f, cgs.media.whiteShader);
	return y + 4.0f;
}

static float CG_OSPDrawCustomPowerup(float x, float y, int powerup, int blink, qhandle_t shader)
{
	if (BG_FindItemForPowerup(powerup))
	{
		if (blink == 0 || (cg.time % 1000) < 500)
		{
			CG_DrawPicOld(x, y, 24.0f, 24.0f, shader);
		}

		if (cg.predictedPlayerState.powerups[powerup])
		{
			CG_DrawPicOld(x, y, 24.0f, 24.0f, cgs.media.selectShader);
		}
	}
	return y;
}

static void CG_OSPDrawPlayerIcon(float x, float y, float w, float h, int attacker, float* angles, float* color2)
{
	clientInfo_t* ci = &cgs.clientinfo[attacker];

	if (cg_draw3dIcons.integer)
	{
		if (ci->headModel)
		{
			vec3_t  pos;  //30
			vec3_t  mins; //3c
			vec3_t  maxs; //48
			trap_R_ModelBounds(ci->headModel, mins, maxs);

			pos[2] = -0.5f * (mins[2] + maxs[2]);
			pos[1] = 0.5f * (mins[1] + maxs[1]);
			pos[0] = (0.7f * (maxs[2] - mins[2])) / 0.268f;

			pos[0] += ci->headOffset[0];
			pos[1] += ci->headOffset[1];
			pos[2] += ci->headOffset[2];

			CG_OSPDraw3DModel(x, y, w, h, ci->headModel, ci->headSkin, pos, angles, color2);
		}
	}
	else if (cg_drawIcons.integer)
	{
		CG_DrawPicOld(x, y, w, h, ci->modelIcon);
	}
}

static float CG_OSPDrawWeaponSwitchAttacker(float y)
{
	int attacker;
	char* str;
	vec3_t color;

	if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
	{
		return y;
	}
	if (cg.attackerTime == 0)
	{
		return y;
	}

	attacker = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if (attacker < 0 || attacker > 64 || attacker == cg.snap->ps.clientNum)
	{
		return y;
	}

	if ((cg.time - cg.attackerTime) > 10000)
	{
		cg.attackerTime = 0;
		return y;
	}

	color[0] = 0;
	color[1] = 180.0f;
	color[2] = 0;

	CG_OSPDrawPlayerIcon(640.0f - 60.0f, y, 60.0f, 60.0f, attacker, color, cgs.clientinfo[attacker].colors.railCore);

	str = Info_ValueForKey(CG_ConfigString(attacker + CS_PLAYERS), "n");
	y += 60.0f;
	CG_DrawBigString(SCREEN_WIDTH - 1, y, str, 0.5f, DS_HRIGHT | DS_SHADOW, 0);

	return (y + 16.0f) + 2.0f;
}

static float CG_OSPDrawFlags67(float x, float y)
{
	if (cgs.gametype == GT_CTF)
	{
		if (cgs.blueflag >= 0 && cgs.blueflag <= 2)
		{
			CG_OSPDrawCustomPowerup((x + 4.0f), y, PW_BLUEFLAG, cgs.blueflag, cgs.media.blueFlagShader[cgs.blueflag]);
			CG_OSPDrawCustomPowerup(((x + 20.0f) + 24.0f), y, PW_REDFLAG, cgs.redflag, cgs.media.redFlagShader[cgs.redflag]);
			y = CG_OSPDrawSplitter(x, y + 28.0f);
		}
		else
		{
			if (cg.predictedPlayerState.powerups[PW_BLUEFLAG] != 0)
			{
				CG_OSPDrawCustomPowerup((x + 4.0f), y, 8, 1, cgs.media.blueFlagShader[1]);
				y = CG_OSPDrawSplitter(x, y + 28.0f);
			}
			else
			{
				if (cg.predictedPlayerState.powerups[PW_REDFLAG] != 0)
				{
					CG_OSPDrawCustomPowerup((x + 4.0f), y, 7, 1, cgs.media.redFlagShader[1]);
					y = CG_OSPDrawSplitter(x, y + 28.0f);
				}
			}
		}
	}
	return y;
}

static float CG_OSPDrawWeapons67(float x, float y)
{
	int weaponNum;
	int holdableItem;
	int ammo;
	int size;
	float w;
	float h;
	vec4_t color;
	qhandle_t item;

	for (weaponNum = 1; weaponNum < 16; ++weaponNum)
	{
		if (weaponNum != 10)
		{
			if (((cg.snap->ps.stats[STAT_WEAPONS] & (1 << weaponNum)) || ch_weaponListDrawAll.integer) && cg_weapons[weaponNum].weaponIcon)
			{
				ammo = cg.snap->ps.ammo[weaponNum];
				color[0] = 1.0f;
				if (ammo < 10)
				{
					color[2] = 0.3f;
					color[1] = 0.3f;
				}
				else
				{
					color[2] = 1.0f;
					color[1] = 1.0f;
				}

				item = cg_weapons[weaponNum].weaponIcon;
				if (cg.weaponSelect == weaponNum)
				{
					size = 24;
					w = 16.0f;
					h = 16.0f;
					wepSwitch67[weaponNum] = 1;
					if (ch_weaponListBackground.integer != 0)
					{
						CG_FillRect(x, y, 82.0f, 24.0f, wepSwitchColor1);
					}
				}
				else
				{
					size = 16;
					w = 8.0f;
					h = 8.0f;
					if (wepSwitch67[weaponNum] == 0)
					{
						if ((cg.snap->ps.stats[STAT_WEAPONS] & (1 << weaponNum)) != 0)
						{
							wepSwitch67[weaponNum] = 900;
						}
					}
					if (wepSwitch67[weaponNum] > 1)
					{
						statsInfo[OSP_STATS_UNKNOWN1] = wepSwitch67[weaponNum] % 300;
						wepSwitchColor2[2] = statsInfo[OSP_STATS_UNKNOWN1] / 600.0f;
						wepSwitchColor2[3] = statsInfo[OSP_STATS_UNKNOWN1] / 300.0f;

						if (ch_weaponListFlash.integer != 0)
						{
							CG_FillRect(x, y, 76.0f, 16.0f, wepSwitchColor2);
						}
						wepSwitch67[weaponNum] -= frameTime;
						if (wepSwitch67[weaponNum] < 1)
						{
							wepSwitch67[weaponNum] = 1;
						}
					}
				}
				CG_DrawPicOld(x + 4, y, size, size, item);

				if ((ammo == 0) && ((ch_weaponListDrawAll.integer != 0) || ((cg.snap->ps.stats[STAT_WEAPONS] & (1 << weaponNum)) != 0)))
				{
					CG_DrawPicOld(x + 4.0f, y, size, size, cgs.media.noammoShader);
				}

				if (cg.weaponSelect == weaponNum)
				{
					color[3] = 1.0f;
				}
				else
				{
					color[3] = 0.5f;
				}

				if (ammo >= 0)
				{
					char str[16];
					Com_sprintf(str, 10, "%i", ammo);
					CG_DrawStringExt((int)((x + 8.0f) + (float)size), (int)(y + ((float)(size / 2.0f) - (h / 2.0f))), str, color, 0, 1, w, h, 3);
				}
				y += size + 4;
			}

			if ((cg.snap->ps.stats[STAT_WEAPONS] & (1 << weaponNum)) == 0)
			{
				wepSwitch67[weaponNum] = 0;
			}
		}
	}

	holdableItem = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if (holdableItem)
	{
		CG_RegisterItemVisuals(holdableItem);
		CG_DrawPicOld(x + 4, y, 16.0f, 16.0f, cg_items[holdableItem].icon);
		y += 20;
	}

	return CG_OSPDrawSplitter(x, y);
}

static float CG_OSPDrawHealthArmor67(float x, float y)
{
	int health;
	int armor;

	if (ch_statusbarStyle.integer != 4)
	{
		return y;
	}

	health = cg.snap->ps.stats[STAT_HEALTH];
	if (cg_drawIcons.integer != 0)
	{
		if ((health > 30) || ((cg.time % 1000) < 500))
		{
			CG_DrawPicOld(x + 4.0f, y, 24.0f, 24.0f, cgs.clientinfo[cg.snap->ps.clientNum].modelIcon);
		}
	}
	if (health > 100)
	{
		trap_R_SetColor(healthColors67[2]);
	}
	else if (health > 30)
	{
		trap_R_SetColor(healthColors67[0]);
	}
	else
	{
		trap_R_SetColor(healthColors67[1]);
	}

	CG_DrawField(x + 28.0f, y, 3, health, 16, 24);
	y += 28.0f;
	if (cg_drawIcons.integer)
	{
		CG_DrawPicOld(x + 4.0f, y, 24.0f, 24.0f, cgs.media.armorIcon[cg.snap->ps.stats[STAT_ARMOR_TYPE]]);
	}

	armor = cg.snap->ps.stats[STAT_ARMOR];
	if (armor > 100)
	{
		trap_R_SetColor(healthColors67[2]);
	}
	else
	{
		trap_R_SetColor(healthColors67[0]);
	}

	CG_DrawField(x + 28.0f, y, 3, armor, 16, 24);
	trap_R_SetColor(NULL);
	y += 28.0f;
	return CG_OSPDrawSplitter(x, y);
}

static int Powerup67Compare(const void* a, const void* b)
{
	powerUpInfo_t* sa, *sb;

	sa = (powerUpInfo_t*)a;
	sb = (powerUpInfo_t*)b;

	if (sa->time < sb->time)
	{
		return -1;
	}
	if (sa->time > sb->time)
	{
		return 1;
	}
	return 0;
}

static float CG_OSPDrawPowerup67(float x, float y)
{
	int powerupTime;
	int i;
	playerState_t* ps;

	gitem_t* item;
	float beat;
	char buf[10];
	float tmp;

	powerUpInfo_t pinfo[16];
	int numberOfPowerups = 0;

	ps = &cg.snap->ps;

	if (ps->stats[STAT_HEALTH] <= 0)
	{
		return y;
	}

	/* make powerups list */
	for (i = 0; i < 16; ++i)
	{
		if (ps->powerups[i])
		{
			powerupTime = ps->powerups[i] - cg.time;
			if ((powerupTime < 0) || (powerupTime > 999000))
			{
				continue;
			}

			pinfo[numberOfPowerups].num = i;
			pinfo[numberOfPowerups].time = powerupTime;
			++numberOfPowerups;
		}
	}

	/* sort powerups */

	qsort(&pinfo, numberOfPowerups, sizeof(powerUpInfo_t), Powerup67Compare);

	/* draw powerups */
	for (i = 0; i < numberOfPowerups; ++i)
	{
		item = BG_FindItemForPowerup(pinfo[i].num);
		beat = 24.0f;
		if (cg.powerupActive == pinfo[i].num)
		{
			int t = cg.time - cg.powerupTime;
			if (t < 200)
			{
				tmp = 1.0f - (((float)t) / 200.0f);
				beat = 24.0f * ((0.5f * tmp) + 1.0f);
			}
		}

		CG_DrawPicOld(x + 4.0f, y - ((beat - 24.0f) / 2.0f), beat, beat, trap_R_RegisterShader(item->icon));
		Com_sprintf(buf, 10, "%i", pinfo[i].time / 1000);

		CG_DrawStringExt(x + 24.0f, y + 4.0f, buf, powerupColors67[1], 0, 1, 16, 16, 2);
		y += 28.0f;
	}

	if (numberOfPowerups)
	{
		y += CG_OSPDrawSplitter(x, y);
	}

	return y;
}

static float CG_OSPDrawWeaponBar_6(void)
{
	int x = 4.0f;
	float y;
	y = 48.0f;
	y = CG_OSPDrawSplitter(x, y);
	y = CG_OSPDrawFlags67(x, y);
	y = CG_OSPDrawWeapons67(x, y);
	y = CG_OSPDrawHealthArmor67(x, y);
	y = CG_OSPDrawPowerup67(x, y);
	weaponBarDrew = qtrue;
	return y;
}

static float CG_OSPDrawWeaponBar_7(float y)
{
	float x = 559.0f;

	if (y < 48.0f)
	{
		y = 48.0f;
	}

	y = CG_OSPDrawSplitter(x, y);
	y = CG_OSPDrawFlags67(x, y);
	y = CG_OSPDrawWeapons67(x, y);
	y = CG_OSPDrawHealthArmor67(x, y);
	y = CG_OSPDrawPowerup67(x, y);

	if (cg_drawAttacker.integer)
	{
		y = CG_OSPDrawWeaponSwitchAttacker(y);
	}
	weaponBarDrew = qtrue;
	return y;
}

static void CG_OSPDrawWeaponBarCPM1(float y)
{
	int weaponIndex;
	int ammo;
	int x = 320;
	int size;
	int w;
	int h;
	qhandle_t item;
	vec4_t color;

	for (weaponIndex = 1; weaponIndex < 16; ++weaponIndex)
	{
		if (weaponIndex == 10) continue;

		if ((cg.predictedPlayerState.stats[STAT_WEAPONS] & (1 << weaponIndex)) || (ch_weaponListDrawAll.integer && cg_weapons[weaponIndex].item))
		{
			x -= 24;
		}

	}

	x -= 20;

	for (weaponIndex = 1; weaponIndex < 16; ++weaponIndex)
	{
		if (weaponIndex == 10) continue;
		if ((cg.predictedPlayerState.stats[STAT_WEAPONS] & (1 << weaponIndex)) || (ch_weaponListDrawAll.integer && cg_weapons[weaponIndex].item))
		{
			ammo = cg.predictedPlayerState.ammo[weaponIndex];
			color[0] = 1.0f;
			if (ammo < 10)
			{
				color[2] = 0.3f;
				color[1] = 0.3f;
			}
			else
			{
				color[2] = 1.0f;
				color[1] = 1.0f;
			}
			item = cg_weapons[weaponIndex].weaponIcon;
			if (cg.weaponSelect == weaponIndex)
			{
				size = 24;
				w = 16.0f;
				h = 16.0f;
				if (ch_weaponListBackground.integer != 0)
				{
					CG_FillRect(x - 4, y, ((float)size + 5 + (3.0f * w)) + 4.0f, 24.0f, wepSwitchColor1);
				}
				wepSwitch67[weaponIndex] = 1;
			}
			else
			{
				size = 16;
				w = 8.0f;
				h = 8.0f;

				if (wepSwitch67[weaponIndex] == 0)
				{
					if ((cg.predictedPlayerState.stats[STAT_WEAPONS] & (1 << weaponIndex)) != 0)
					{
						wepSwitch67[weaponIndex] = 900;
					}
				}

				if (wepSwitch67[weaponIndex] > 1)
				{
					statsInfo[OSP_STATS_UNKNOWN1] = wepSwitch67[weaponIndex] % 300;
					wepSwitchColor2[2] = statsInfo[OSP_STATS_UNKNOWN1] / 600.0f;
					wepSwitchColor2[3] = statsInfo[OSP_STATS_UNKNOWN1] / 300.0f;

					if (ch_weaponListFlash.integer != 0)
					{
						CG_FillRect(x - 2, y - 2, ((float)size + (3.0f * w)) + 4.0f, 20.0f, wepSwitchColor2);
					}
					wepSwitch67[weaponIndex] -= frameTime;
					if (wepSwitch67[weaponIndex] < 1)
					{
						wepSwitch67[weaponIndex] = 1;
					}
				}
			}
			CG_DrawPicOld(x, y, size, size, item);

			if ((ammo == 0) && ((ch_weaponListDrawAll.integer != 0) || ((cg.predictedPlayerState.stats[STAT_WEAPONS] & (1 << weaponIndex)) != 0)))
			{
				CG_DrawPicOld(x, y, size, size, cgs.media.noammoShader);
			}

			if (cg.weaponSelect == weaponIndex)
			{
				color[3] = 1.0f;
			}
			else
			{
				color[3] = 0.5f;
			}
			if (ammo >= 0)
			{
				char str[16];
				Com_sprintf(str, 16, "%i", ammo);
				CG_DrawStringExt(((x + 0x4) + size), (int)((float)y + ((float)(size / 2.0f) - (h / 2.0f))), str, color, 0, 1, w, h, 3);
			}
			x = (int)((float)x + (((float)size + (3.0f * w)) + 8.0f));
		}
		if ((cg.predictedPlayerState.stats[2] & (1 << weaponIndex)) == 0)
		{
			wepSwitch67[weaponIndex] = 0;
		}
	}
}

static void CG_OSPDrawWeaponBarCPM2(float x, float y)
{
	int weaponIndex;
	int size;
	float color[4];
	int ammo;
	int w;
	int line = 0;
	qhandle_t item;
	int h;
	char str[16];
	int tmp_x = x;

	for (weaponIndex = 1; weaponIndex < 16; ++weaponIndex)
	{
		if (weaponIndex == 10) continue;

		if ((cg.predictedPlayerState.stats[STAT_WEAPONS] & (1 << weaponIndex)) || (ch_weaponListDrawAll.integer && cg_weapons[weaponIndex].item))
		{
			if (line == 0x5)
			{
				y += 28;
				x = tmp_x;
				line = 0;
			}

			ammo = cg.predictedPlayerState.ammo[weaponIndex];
			color[0] = 1.0f;
			if (ammo < 10)
			{
				color[2] = 0.3f;
				color[1] = 0.3f;
			}
			else
			{
				color[2] = 1.0f;
				color[1] = 1.0f;
			}
			item = cg_weapons[weaponIndex].weaponIcon;

			if (cg.weaponSelect == weaponIndex)
			{
				size = 24;
				w = 16.0f;
				h = 16.0f;
				if (ch_weaponListBackground.integer != 0)
				{
					CG_FillRect(x - 4, y, ((float)size + 5 + (3.0f * w)) + 4.0f, 24.0f, wepSwitchColor1);
				}
				wepSwitch67[weaponIndex] = 1;
			}
			else
			{
				size = 16;
				w = 8.0f;
				h = 8.0f;

				if (wepSwitch67[weaponIndex] == 0)
				{
					if ((cg.predictedPlayerState.stats[STAT_WEAPONS] & (1 << weaponIndex)) != 0)
					{
						wepSwitch67[weaponIndex] = 900;
					}
				}

				if (wepSwitch67[weaponIndex] > 1)
				{
					statsInfo[OSP_STATS_UNKNOWN1] = wepSwitch67[weaponIndex] % 300;
					wepSwitchColor2[2] = statsInfo[OSP_STATS_UNKNOWN1] / 600.0f;
					wepSwitchColor2[3] = statsInfo[OSP_STATS_UNKNOWN1] / 300.0f;

					if (ch_weaponListFlash.integer != 0)
					{
						CG_FillRect(x - 2, y - 2, ((float)size + (3.0f * w)) + 4.0f, 20.0f, wepSwitchColor2);
					}
					wepSwitch67[weaponIndex] -= frameTime;
					if (wepSwitch67[weaponIndex] < 1)
					{
						wepSwitch67[weaponIndex] = 1;
					}
				}
			}
			CG_DrawPicOld((float)x, (float)y, size, size, item);

			if ((ammo == 0) && ((ch_weaponListDrawAll.integer != 0) || ((cg.predictedPlayerState.stats[STAT_WEAPONS] & (1 << weaponIndex)) != 0)))
			{
				CG_DrawPicOld(x, y, size, size, cgs.media.noammoShader);
			}

			if (cg.weaponSelect == weaponIndex)
			{
				color[3] = 1.0f;
			}
			else
			{
				color[3] = 0.5f;
			}
			if (ammo >= 0)
			{
				Com_sprintf(str, 16, "%i", ammo);
				CG_DrawStringExt(x + 4 + size, (int)((float)y + ((float)(size / 2.0f) - (h / 2.0f))), str, color, 0, 1, w, h, 3);
			}
			x += ((float)size + (3.0f * w)) + 8.0f;
			++line;
		}

		if ((cg.predictedPlayerState.stats[2] & (1 << weaponIndex)) == 0)
		{
			wepSwitch67[weaponIndex] = 0;
		}
	}

}

static float CG_OSPDrawSpeed(float y)
{
	const char* str = va("%dups", (int)cg.xyspeed);
	CG_DrawBigString(SCREEN_WIDTH - 1, y + 2.0f, str, 1.0f, DS_HRIGHT | DS_SHADOW, 0);
	return y + 20.0f;
}

static void CG_OSPDrawCrosshairNames(void)
{
	float* color;
	float delta_time;
	clientInfo_t* ci;
	int w;
	int h;

	if (cg_drawCrosshair.integer == 0) return;
	if (cg_drawCrosshairNames.integer == 0) return;
	if (cg.renderingThirdPerson != 0) return;
	if (global_viewlistFirstOption > 1) return;

	CG_ScanForCrosshairEntity();

	color = CG_FadeColor(cg.crosshairClientTime, 1000);
	if (color == NULL)
	{
		trap_R_SetColor(NULL);
		return;
	}

	ci = &cgs.clientinfo[cg.crosshairClientNum];


	CG_OSPGetClientFontSize(&cf_CrosshairNames, &w, &h);
	delta_time = (float)(cg.time - cg.crosshairClientTime);
	if ((1000.0f - delta_time) < 200.0)
	{
		w = ((1000.0f - delta_time) * w) / 200.0f;
		h = ((1000.0f - delta_time) * h) / 200.0f;
	}

	if ((ci->team == cg.snap->ps.persistant[PERS_TEAM]) || cg_drawCrosshairNames.integer != 2 || (cgs.osp.gameTypeFreeze && ci->team == cgs.clientinfo[cg.snap->ps.clientNum].team))
	{
		if (ch_CrosshairNamesLeft.integer == 0)
		{
			float len;
			len = (float)(CG_DrawStrlen(ci->name) * w);

			CG_OSPDrawStringOld((int)(320.0f - (len / 2.0f)), 170, ci->name, w, h, NULL, 0, 0);

			if (ci->team != TEAM_FREE && ci->team == cg.snap->ps.persistant[PERS_TEAM] && ch_TeamCrosshairHealth.integer != 0 && !(cg.snap->ps.pm_flags & PMF_FOLLOW))
			{
				char str2[64];
				int w2;
				int h2;
				int len2;
				vec4_t color2;

				CG_OSPGetClientFontSize(&cf_TeamCrosshairHealth, &w2, &h2);
				CG_GetColorForHealth(ci->health, ci->armor, color2, NULL);
				Com_sprintf(str2, 64, "%i/%i", ci->health, ci->armor);
				len2 = CG_DrawStrlen(str2);
				color2[3] = color[3];
				CG_OSPDrawStringOld((int)(320.0f - ((float)(len2 * w2) / 2.0f)), h + 170, str2, w2, h2, color2, 0, 0);
			}
		}
		else
		{
			int wStatusChar;
			int hStatusChar;
			int wTeamChar;
			int hTeamChar;
			float y;
			char str2[64];
			int w2;
			int h2;
			int lenName;
			int lenStr;
			float nameW;
			float healthW;
			vec4_t color2;
			CG_OSPGetClientFontSize(&cf_Statusbar, &wStatusChar, &hStatusChar);
			CG_OSPGetClientFontSize(&cf_Teamchat, &wTeamChar, &hTeamChar);

			wTeamChar = wStatusChar;

			y = 480 - hStatusChar - (cg_teamChatHeight.integer * hTeamChar) - h;
			if (ci->team && ci->team == cg.snap->ps.persistant[PERS_TEAM] && ch_TeamCrosshairHealth.integer != 0 && !(cg.snap->ps.pm_flags & PMF_FOLLOW))
			{
				int x_name;
				int x_health;
				CG_OSPGetClientFontSize(&cf_TeamCrosshairHealth, &w2, &h2);
				y -= h2;
				Com_sprintf(str2, 64, "%i/%i", ci->health, ci->armor);
				lenName = CG_DrawStrlen(ci->name);
				nameW = (float)(lenName * w);
				lenStr = CG_DrawStrlen(str2);
				healthW = lenStr * w2;
				if (nameW > healthW)
				{
					x_name = 0;
					x_health = nameW / 2.0f - healthW / 2.0f;
				}
				else
				{
					x_name = healthW / 2.0f - nameW / 2.0f;
					x_health = 0;
				}
				CG_OSPDrawStringOld(x_name, y, ci->name, w, h, color, 0, 0);
				CG_GetColorForHealth(ci->health, ci->armor, color2, NULL);
				color2[3] = 0.5f * color[3];
				CG_OSPDrawStringOld(x_health, y + h, str2, w2, h2, color2, 0, 0);
			}
			else
			{
				CG_OSPDrawStringOld(0, y, ci->name, w, h, color, 0, 0);
			}
		}
		trap_R_SetColor(NULL);
	}
}

static void CG_OSPDrawSpectatorInfo(void)
{
	int w;
	int h;
	int len;
	const char* str;
	if (!cg.demoPlayback)
	{
		if (!(cg.snap->ps.pm_flags & PMF_FOLLOW))
		{
			return;
		}
		if (cg.snap->ps.pm_type == PM_FREEZE || cg.intermissionStarted || cg.snap->ps.clientNum == cg.clientNum)
		{
			return;
		}
	}
	else if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR)
	{
		return;
	}

	CG_OSPGetClientFontSize(&cf_Following, &w, &h);

	str = "Following";
	len = CG_OSPGetNormalizedStringSize(str);
	CG_OSPDrawStringOld(320 - (len * w / 2), 24, str, w, h, NULL, 0, 0);

	str = cgs.clientinfo[cg.snap->ps.clientNum].name;
	len = CG_OSPGetNormalizedStringSize(str);
	CG_OSPDrawStringOld(320 - (len * w / 2), h + 24, str, w, h, NULL, 0, 0);
}

static void CG_OSPDrawVote(void)
{
	int w;
	int h;
	int time;
	char* votestr;
	int nsize;
	int posx;

	if (cgs.voteTime == 0) return;

	CG_OSPGetClientFontSize(&cf_Vote, &w, &h);
	if (cgs.voteModified)
	{
		cgs.voteModified = 0;
		if (!cg_noVoteBeep.integer)
		{
			trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
		}
	}

	time = (30000 - (cg.time - cgs.voteTime)) / 1000;

	if (time < 0)
	{
		time = 0;
	}
	votestr = va("VOTE(%i):%s yes(F1):%i no(F2):%i", time, &cgs.voteString, cgs.voteYes, cgs.voteNo);
	nsize = CG_OSPGetNormalizedStringSize(votestr);

	posx = 320 - (nsize * w) / 2;
	if (posx < 0)
	{
		posx = 0;
	}

	CG_OSPDrawStringOld(posx, 58, votestr, w, h, NULL, 0, qfalse);
}


void CG_OSPHUDRoutine(void)
{
	qboolean isTeamGame;
	qboolean isSpectator;
	qboolean isAlive;
	float drew = 0;
	float pos = 0;
	float pos1 = 0;

	isTeamGame = cgs.gametype >= GT_TEAM ? qtrue : qfalse;

	if (cg.snap->ps.pm_type == PM_INTERMISSION)
	{
		CG_OSPDrawIntermission();
		CG_OSPDrawVote();
		return;
	}

	{
		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
		{
			isSpectator = qtrue;
		}
		else
		{
			isSpectator = qfalse;
		}

		if (cg.snap->ps.stats[STAT_HEALTH] > 0 && !isSpectator)
		{
			isAlive = qtrue;
		}
		else
		{
			isAlive = qfalse;
		}

		if (isSpectator)
		{
			CG_OSPDrawSpectator();
		}

		drew = 480.0f;

		if (isAlive)
		{
			switch (ch_StatusbarFormat.integer)
			{
				default:
				case 0:
					CG_DrawStatusBar();
					drew = 432.0f;
					break;
				case 1:
					drew = CG_OSPDrawStatusBar(480.0f);
					break;
				case 2:
					drew = CG_OSPDrawStatusBar(480.0f);
					CG_OSPDrawStatusBarAmmo1(480.0);
					break;
				case 3:
					drew = CG_OSPDrawStatusBar(480.0f);
					CG_OSPDrawStatusBarAmmo2(480.0);
					break;
				case 4:
					CG_OSPDrawCPMStatusbar0();
					break;
				case 5:
					CG_OSPDrawCPMStatusbarBars();
					break;
				case 6:
					CG_OSPDrawCPMStatusbar3();
					break;
				case 7:
					CG_OSPDrawCPMStatusbar4();
					break;
				case 8:
					CG_OSPDrawCPMStatusbar5();
					break;
				case 9:
				case 10:
				case 11:
				case 12:
					CG_OSPDrawQ3CompStatusbar();
					drew = 424.0f;
					break;
			}
		}

		if (cg_lagometer.integer == 1)
		{
			CG_DrawLagometer((float)(428 - (cg_drawPing.integer > 0 ? 14 : 0)));
		}
		if (drew < 432.0)
		{
			pos = drew;
		}
		else
		{
			pos = 432.0;
		}

		if (cg_lagometer.integer == 1 && cg_drawPing.integer > 0)
		{
			pos -= 14;
		}

		pos1 = pos;
		if (isTeamGame)
		{
			pos = CG_OSPDrawTeamInfo(pos);
		}

		if (isTeamGame && cg_drawTeamOverlay.integer == 2)
		{
			if (isAlive || ((CG_OSPIsGameTypeCA(cgs.gametype) || cgs.osp.gameTypeFreeze) && CG_OSPGetPresistantTeam() != TEAM_SPECTATOR))
			{
				pos1 = CG_OSPDrawTeamOverlay(pos, 1, 0);
				pos = pos1;
			}
		}
		if (isAlive)
		{
			if (isTeamGame != 0)
			{
				if (cg_drawTeamOverlay.integer == 3)
				{
					pos = CG_OSPDrawTeamOverlay(pos1, 0, 0);
				}
			}

			if (ch_Weaponswitch.integer == 0 || ch_Weaponswitch.integer == 1)
			{
				CG_OSPDrawWeaponBarOld(pos);
			}
			else if (ch_Weaponswitch.integer == 4 || ch_Weaponswitch.integer == 5)
			{
				CG_OSPDrawWeaponBar_4_5(pos);
			}
			else if (ch_Weaponswitch.integer == 9)
			{
				CG_OSPDrawWeaponBarOld(380.0);
			}

			if (ch_Pickupitem.integer)
			{
				pos = CG_OSPDrawPickupItem(pos);
			}
		}
		else if (isTeamGame && (cg_drawTeamOverlay.integer == 3 && CG_OSPIsGameTypeCA(cgs.gametype)) && cgs.osp.gameTypeFreeze != 0)
		{
			if (CG_OSPGetPresistantTeam() != TEAM_SPECTATOR)
			{
				pos = CG_OSPDrawTeamOverlay(pos, 0, 0);
			}
		}

		pos = CG_OSPHUDDrawScores(pos1);
		if (ch_StatusbarFormat.integer >= 9)
		{
			float tmp;
			if (pos > pos1)
			{
				tmp = pos1;
			}
			else
			{
				tmp = pos;
			}
			pos = tmp - 3;
		}
		if (isSpectator == 0)
		{
			pos = CG_OSPDrawHoldableItem(pos);
		}
		if (isAlive && ch_Weaponswitch.integer != 6 && ch_Weaponswitch.integer != 7)
		{
			CG_OSPDrawWeaponBar_2_3(pos);
		}
		pos = 0;
		if (isTeamGame)
		{
			if (cg_drawTeamOverlay.integer == 1 && isAlive &&
			        ((CG_OSPIsGameTypeCA(cgs.gametype) || cgs.osp.gameTypeFreeze) && CG_OSPGetPresistantTeam() != TEAM_SPECTATOR))
			{
				pos = CG_OSPDrawTeamOverlay(pos, 1, 1);
			}
		}
		if (cg_drawSnapshot.integer != 0)
		{
			pos = CG_DrawSnapshot(pos);
		}
		if (cg_drawFPS.integer != 0)
		{
			pos = CG_DrawFPS(pos);
		}
		if (cg_drawTimer.integer == 2)
		{
			CG_DrawTimer2();
		}
		else if (cg_drawTimer.integer != 0)
		{
			pos = CG_DrawTimer(pos);
		}
		if (ch_drawSpeed.integer == 1)
		{
			pos = CG_OSPDrawSpeed(pos);
		}
		if (cg_lagometer.integer > 1)
		{
			pos = CG_DrawLagometer(pos);
		}
		else
		{
			if (cg_lagometer.integer <= 0)
			{
				if (cg_drawPing.integer != 0)
				{
					pos = CG_OSPDrawPing(pos);
				}
			}
		}
		if (isAlive != 0)
		{
			if (ch_Weaponswitch.integer >= 6)
			{
				if (ch_Weaponswitch.integer == 6)
				{
					if (ch_StatusbarFormat.integer != 7)
					{
						CG_OSPDrawWeaponBar_6();
					}
				}
				else if (ch_Weaponswitch.integer == 7)
				{
					CG_OSPDrawWeaponBar_7(48.0f);
				}
				else
				{
					switch (ch_StatusbarFormat.integer)
					{
						case 4:
						case 5:
							CG_OSPDrawWeaponBarCPM1(380);
							weaponBarDrew = qfalse;
							break;
						case 6:
							CG_OSPDrawWeaponBarCPM2(285, 425);
							weaponBarDrew = qfalse;
							break;
						case 7:
							if (ch_Weaponswitch.integer == 8)
							{
								CG_OSPDrawWeaponBarCPM2(110, 425);
								weaponBarDrew = qfalse;
							}
							break;
					}
				}
			}
		}
		if (cg_drawAttacker.integer != 0 && (ch_Weaponswitch.integer != 7 || !isAlive))
		{
			pos = CG_DrawAttacker(pos);
		}
		if (isAlive != 0)
		{
			CG_DrawAmmoWarning();
		}

		CG_DrawCrosshair();
		CG_OSPDrawCrosshairNames();
		CG_DrawReward();
		CG_OSPDrawVote();
		CG_OSPDrawSpectatorInfo();
		CG_DrawWarmup();

		if (CG_DrawIntermission() == 0)
		{
			CG_OSPDrawCenterString();
		}
	}
}


