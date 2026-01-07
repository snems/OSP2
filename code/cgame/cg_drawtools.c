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
// cg_drawtools.c -- helper functions called by cg_draw, cg_scoreboard, cg_info, etc
#include "cg_local.h"
#include "../qcommon/qcommon.h"


typedef struct OSP_ClientFont_t
{
	vmCvar_t* cvar;
	int modificationCount;
	int w;
	int h;
} OSP_ClientFont_t;

OSP_ClientFont_t OSP_ClientFontsTable[] =
{
	{&cf_Statusbar, -1, 0, 0},
	{&cf_Pickupstring, -1, 0, 0},
	{&cf_Teamchat, -1, 0, 0},
	{&cf_Teamoverlay, -1, 0, 0},
	{&cf_CrosshairNames, -1, 0, 0},
	{&cf_TeamCrosshairHealth, -1, 0, 0},
	{&cf_Vote, -1, 0, 0},
	{&cf_Following, -1, 0, 0},
	{&cf_AmmoStatusbar, -1, 0, 0},
	{&cf_timer, -1, 0, 0},
	{&cf_Fragmsg, -1, 0, 0},
	{&cf_Scores, -1, 0, 0},
	{&cf_WeaponSwitch, -1, 0, 0},
	{&cf_WeaponName, -1, 0, 0},
};

const int OSP_ClientFontsTableSize = sizeof(OSP_ClientFontsTable) / sizeof(OSP_ClientFont_t);

/*
================
CG_OSPGetClientFontSize

Get custom font for specific command
================
*/
qboolean CG_OSPGetClientFontSize(const vmCvar_t* cvar, int* w, int* h)
{
	int i;
	const char* cvar_value;

	for (i = 0; i < OSP_ClientFontsTableSize; ++i)
	{
		if (OSP_ClientFontsTable[i].cvar != cvar) continue;
		if (OSP_ClientFontsTable[i].modificationCount != cvar->modificationCount)
		{
			int new_h = 0;
			int new_w = 0;
			cvar_value = cvar->string;

			while (*cvar_value >= '0' && *cvar_value <= '9')
			{
				new_w = new_w * 10 + (int)(*cvar_value - '0');
				++cvar_value;
			}

			if (*cvar_value != 'x' && *cvar_value != '*')
			{
				break;
			}
			++cvar_value;
			while (*cvar_value >= '0' && *cvar_value <= '9')
			{
				new_h = new_h * 10 + (int)(*cvar_value - '0');
				++cvar_value;
			}
			OSP_ClientFontsTable[i].w = new_w;
			OSP_ClientFontsTable[i].h = new_h;
		}
		*w = OSP_ClientFontsTable[i].w;
		*h = OSP_ClientFontsTable[i].h;
		OSP_ClientFontsTable[i].modificationCount = cvar->modificationCount;
		return qtrue;
	}
	/* default 16x16 */
	*w = 16;
	*h = 16;
	return qfalse;
}


/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640(float* x, float* y, float* w, float* h)
{
	// scale for screen sizes
	if (x) *x *= cgs.screenXScale_Old;
	if (y) *y *= cgs.screenYScale_Old;
	if (w) *w *= cgs.screenXScale_Old;
	if (h) *h *= cgs.screenYScale_Old;
}
void CG_AdjustFrom640_Old(float* x, float* y, float* w, float* h, qboolean correctWide)
{
#if 0
	// adjust for wide screens
	if (cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640)
	{
		*x += 0.5 * (cgs.glconfig.vidWidth - (cgs.glconfig.vidHeight * 640 / 480));
	}
#endif
	// scale for screen sizes
	if (x) *x *= cgs.screenXScale_Old;
	if (y) *y *= cgs.screenYScale_Old;
	if (h) *h *= cgs.screenYScale_Old;

	if (!correctWide)
	{
		if (w) *w *= cgs.screenXScale_Old;
	}
	else
	{
		static qboolean koeffReady = qfalse;
		static float koeff;
		if (!koeffReady)
		{
			koeffReady = qtrue;
			koeff = cgs.screenXScale_Old * (cgs.glconfig.vidHeight * 640.0) / (cgs.glconfig.vidWidth * 480.0);
		}

		if (w) *w *= koeff;
	}
}

/*
================
CG_FillRect

Coordinates are 640*480 virtual values
=================
*/
void CG_FillRect(float x, float y, float width, float height, const float* color)
{
	trap_R_SetColor(color);

	CG_AdjustFrom640_Old(&x, &y, &width, &height, qfalse);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader);

	trap_R_SetColor(NULL);
}

/*
================
CG_DrawSides

Coords are virtual 640x480
================
*/
void CG_DrawSides(float x, float y, float w, float h, float size)
{
	CG_AdjustFrom640_Old(&x, &y, &w, &h, qfalse);
	size *= cgs.screenXScale_Old;
	trap_R_DrawStretchPic(x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_DrawStretchPic(x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
}

void CG_DrawTopBottom(float x, float y, float w, float h, float size)
{
	CG_AdjustFrom640_Old(&x, &y, &w, &h, qfalse);
	size *= cgs.screenYScale_Old;
	trap_R_DrawStretchPic(x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_DrawStretchPic(x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
}
/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawRect(float x, float y, float width, float height, float size, const float* color)
{
	trap_R_SetColor(color);

	CG_DrawTopBottom(x, y, width, height, size);
	CG_DrawSides(x, y, width, height, size);

	trap_R_SetColor(NULL);
}



/*
================
CG_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPicOld(float x, float y, float width, float height, qhandle_t hShader)
{
	CG_AdjustFrom640_Old(&x, &y, &width, &height, qfalse);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, hShader);
}

void CG_DrawPic(float x, float y, float width, float height, qhandle_t hShader)
{
	CG_AdjustFrom640(&x, &y, &width, &height);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, hShader);
}



/*
===============
CG_DrawChar

Coordinates and size in 640*480 virtual screen size
===============
*/
void CG_DrawChar(int x, int y, int width, int height, int ch)
{
	int row, col;
	float frow, fcol;
	float size;
	float   ax, ay, aw, ah;
	qhandle_t shader;

	ch &= 255;

	if (ch == ' ')
	{
		return;
	}

	ax = x;
	ay = y;
	aw = width;
	ah = height;
	CG_AdjustFrom640_Old(&ax, &ay, &aw, &ah, qfalse);

	row = ch >> 4;
	col = ch & 15;

	frow = row * 0.0625;
	fcol = col * 0.0625;
	size = 0.0625;

	shader = ch_3waveFont.integer ? cgs.media.charsetShader1 : cgs.media.charsetShader;

	trap_R_DrawStretchPic(ax, ay, aw, ah,
	                      fcol, frow,
	                      fcol + size, frow + size,
	                      shader);
}


/*
==================
CG_DrawStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void CG_DrawStringExt(int x, int y, const char* string, const float* setColor,
                      qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars)
{
	vec4_t      color;
	const char*  s;
	int         xx;
	int         cnt;

	if (maxChars <= 0)
		maxChars = 32767; // do them all!

	// draw the drop shadow
	if (shadow)
	{
		color[0] = color[1] = color[2] = 0;
		color[3] = setColor[3];
		trap_R_SetColor(color);
		s = string;
		xx = x;
		cnt = 0;
		while (*s && cnt < maxChars)
		{
			if (Q_IsColorString(s))
			{
				s += 2;
				continue;
			}
			CG_DrawChar(xx + 2, y + 2, charWidth, charHeight, *s);
			cnt++;
			xx += charWidth;
			s++;
		}
	}

	// draw the colored text
	s = string;
	xx = x;
	cnt = 0;
	trap_R_SetColor(setColor);
	while (*s && cnt < maxChars)
	{
		if (Q_IsColorString(s))
		{
			if (!forceColor)
			{
				memcpy(color, g_color_table[ColorIndex(*(s + 1))], sizeof(color));
				color[3] = setColor[3];
				trap_R_SetColor(color);
			}
			s += 2;
			continue;
		}
		CG_DrawChar(xx, y, charWidth, charHeight, *s);
		xx += charWidth;
		cnt++;
		s++;
	}
	trap_R_SetColor(NULL);
}

void CG_DrawBigString(int x, int y, const char* s, const float alpha, int flags, int font)
{
	float   color[4] = {1.0, 1.0, 1.0, 1.0};

	color[3] = alpha;

	CG_FontSelect(font);
	CG_OSPDrawString(x, y, s, color, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, SCREEN_WIDTH, flags, NULL);
}


void CG_DrawSmallString(int x, int y, const char* s, float alpha, int flags, int font)
{
	float   color[4] = {1.0, 1.0, 1.0, 1.0};

	color[3] = alpha;

	CG_FontSelect(font);
	CG_OSPDrawString(x, y, s, color, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, SCREEN_WIDTH, flags, NULL);
}

void CG_DrawSmallStringColor(int x, int y, const char* s, vec4_t color, int flags, int font)
{
	CG_FontSelect(font);
	CG_OSPDrawString(x, y, s, color, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, SCREEN_WIDTH, flags, NULL);
}

/*
 * Text compiler
 */
//Cut time related symbols like ^f ^F
void CG_OSPDrawStringPrepare(const char* from, char* to, int size)
{
	int printed = 0;
	int max = size - 1;

	if (!from || !to)
	{
		return;
	}

	while (*from && printed < max)
	{
		if (from[0] == '^' && from[1] != '^')
		{
			switch (from[1])
			{
				case 'f':
					from += 2; //skip ^f
					if ((cg.time & 0x3ff) >= 512)
					{
						while (*from && !(from[0] == '^' && (from[1] == 'N' || from[1] == 'F')))
						{
							++from;
						}
					}
					break;
				case 'F':
					from += 2; //skip ^F
					if ((cg.time & 0x3ff) < 512)
					{
						while (*from && !(from[0] == '^' && (from[1] == 'N' || from[1] == 'f')))
						{
							++from;
						}
					}
					break;
				default:
					break;
			}
		}

		*to = *from;
		++to;
		++from;
		++printed;
	}
	*to = 0; //eos
}


text_command_t* CG_CompileText(const char* in)
{
	int b;
	text_command_t* commands;
	text_command_t* result = NULL;
	char* text;
	char* dmem;
	int i = 0;
	int len;
	vec4_t back_color;
	qboolean back_color_was_set = qfalse;
	qboolean top_color_was_set = qfalse;
	float rc, gc, bc;
	unsigned int color_index;

	if (!in || *in == 0)
	{
		return NULL;
	}

	Vector4Copy(colorWhite, back_color);

	commands = Z_Malloc(sizeof(*commands) * OSP_TEXT_CMD_MAX) ;
	OSP_MEMORY_CHECK(commands);

	len = strlen(in) + 1;
	dmem = Z_Malloc(len);
	OSP_MEMORY_CHECK(dmem);
	text = dmem;

	CG_OSPDrawStringPrepare(in, text, len);

	while (*text)
	{
		if (text[0] == '^' && text[1])
		{
			switch (text[1])
			{
				case 'F':
				case 'f':
					//skip ^F and ^f
					text += 2;
					break;
				case 'B':
					if (!top_color_was_set && back_color_was_set)
					{
						commands[i].type = OSP_TEXT_CMD_TEXT_COLOR;
						VectorCopy(back_color, commands[i].value.color);
						++i;
					}
					b = cg.time & 0x7ff;
					if (b > 1024)
					{
						b = ~b & 0x7ff;
					}
					commands[i].type = OSP_TEXT_CMD_FADE;
					commands[i].value.fade =  b / 1463.0f + 0.3f;
					++i;
					text += 2;
					break;
				case 'b':
					if (!top_color_was_set && back_color_was_set)
					{
						commands[i].type = OSP_TEXT_CMD_TEXT_COLOR;
						VectorCopy(back_color, commands[i].value.color);
						++i;
					}
					b = cg.time & 0x7ff;
					if (b > 1024)
					{
						b = ~b & 0x7ff;
					}
					commands[i].type = OSP_TEXT_CMD_FADE;
					commands[i].value.fade = b / 1024.0f;
					++i;
					text += 2;
					break;
				case 'N':
				case 'n':
					//reset fade
					commands[i].type = OSP_TEXT_CMD_FADE;
					commands[i].value.fade = 1.0f;
					++i;

					//reset color
					if (!top_color_was_set && back_color_was_set)
					{
						commands[i].type = OSP_TEXT_CMD_TEXT_COLOR;
						VectorCopy(back_color, commands[i].value.color);
						++i;
					}
					else if (top_color_was_set && !back_color_was_set)
					{
						commands[i].type = OSP_TEXT_CMD_TEXT_COLOR;
						VectorCopy(colorWhite, commands[i].value.color);
						++i;
					}

					text += 2;
					break;
				case '^':
					commands[i].type = OSP_TEXT_CMD_CHAR;
					commands[i].value.character = '^';
					++i;
					text += 1;
					break;
				case 'X':
				case 'x':
					if (!CG_Hex16GetColor(&text[2], &rc))
					{
						text += 2;
						break;
					}
					if (!CG_Hex16GetColor(&text[4], &gc))
					{
						text += 2;
						break;
					}
					if (!CG_Hex16GetColor(&text[6], &bc))
					{
						text += 2;
						break;
					}
					back_color[0] = rc;
					back_color[1] = gc;
					back_color[2] = bc;

					commands[i].type = OSP_TEXT_CMD_SHADOW_COLOR;
					VectorCopy(back_color, commands[i].value.color);
					back_color_was_set = qtrue;

					++i;
					text += 8;
					break;
				default:
					if ((text[1] >= '0') && (text[1] <= '9'))
					{
						color_index = text[1] - 0x30;
						VectorCopy(g_color_table[color_index], commands[i].value.color); //-V557
						commands[i].type = OSP_TEXT_CMD_TEXT_COLOR;
						++i;
						//top_color_was_set = qtrue;
					}
					text += 2;
					break;
			}
		}
		else
		{
			commands[i].type = OSP_TEXT_CMD_CHAR;
			commands[i].value.character = text[0];
			++i;
			++text;
		}
	}
	commands[i++].type = OSP_TEXT_CMD_STOP;

	result = Z_Malloc(sizeof(text_command_t) * i);
	OSP_MEMORY_CHECK(result);

	memcpy(result, commands, sizeof(text_command_t) * i);

	Z_Free(dmem);
	Z_Free(commands);
	return result;
}

void CG_CompiledTextDestroy(text_command_t* root)
{
	if (root)
	{
		Z_Free(root);
	}
}

/*
 * New font renderer
 */
#define MAX_FONT_SHADERS 4

typedef struct
{
	float tc_prop[4];
	float tc_mono[4];
	float space1;
	float space2;
	float width;
} font_metric_t;

typedef struct
{
	const char* name;
	font_metric_t   metrics[256];
	qhandle_t       shader[ MAX_FONT_SHADERS ];
	int             shaderThreshold[ MAX_FONT_SHADERS ];
	int             shaderCount;
} font_t;

static font_t fonts[] = {{"id"}, {"idblock"}, {"sansman"}, {"cpma"}, {"m1rage"}, {"elite_emoji"}, {"diablo"}, {"eternal"}, {"qlnumbers"}, {"elite"}, {"elitebigchars"}};
static int fonts_num = sizeof(fonts) / sizeof(fonts[0]);
static const font_t* font = &fonts[0];
static const font_metric_t* metrics = &fonts[0].metrics[0];


void CG_FontSelect(int index)
{
	if (index < 0 || index >= fonts_num)
	{
		CG_Error("Requested nonexistent font number: %d\n", index);
	}

	font = &fonts[index];
	metrics = &font->metrics[0];
}

int CG_FontIndexFromName(const char* name)
{
	int index;
	for (index = 0; index < fonts_num; ++index)
	{
		if (Q_stricmp(name, fonts[index].name) == 0)
		{
			return index;
		}
	}
	return 0;
}


static qboolean CG_FileExist(const char* file)
{
	fileHandle_t    f;

	if (!file || !file[0])
		return qfalse;

	trap_FS_FOpenFile(file, &f, FS_READ);
	if (f == FS_INVALID_HANDLE)
		return qfalse;//-V1020
	else
	{
		trap_FS_FCloseFile(f);
		return qtrue;
	}
}


static void CG_LoadFont(font_t* fnt, const char* fontName)
{
	char buf[ 8000 ];
	fileHandle_t f;
	char* token, *text;
	float width, height, r_width, r_height;
	float char_width;
	float char_height;
	char shaderName[ MAX_FONT_SHADERS ][ MAX_QPATH ], tmpName[ MAX_QPATH ];
	int shaderCount;
	int shaderThreshold[ MAX_FONT_SHADERS ];
	font_metric_t* fm;
	int i, tmp, len, chars;
	float w1, w2;
	float s1, s2;
	float x0, y0;
	qboolean swapped;

	len = trap_FS_FOpenFile(fontName, &f, FS_READ);
	if (f == FS_INVALID_HANDLE)
	{
		CG_Printf(S_COLOR_YELLOW "CG_LoadFont: error opening %s\n", fontName);
		return;
	}

	if (len >= sizeof(buf))
	{
		CG_Printf(S_COLOR_YELLOW "CG_LoadFont: font file is too long: %i\n", len);
		len = sizeof(buf) - 1;
	}

	trap_FS_Read(buf, len, f);
	trap_FS_FCloseFile(f);
	buf[ len ] = '\0';

	shaderCount = 0;

	text = buf; // initialize parser
	COM_BeginParseSession(fontName);

	while (1)
	{
		token = COM_ParseExt(&text, qtrue);
		if (token[0] == '\0')
		{
			Com_Printf(S_COLOR_RED "CG_LoadFont: parse error.\n");
			return;
		}

		// font image
		if (strcmp(token, "img") == 0)
		{
			if (shaderCount >= MAX_FONT_SHADERS)
			{
				Com_Printf("CG_LoadFont: too many font images, ignoring.\n");
				SkipRestOfLine(&text);
				continue;
			}
			token = COM_ParseExt(&text, qfalse);
			if (!CG_FileExist(token))
			{
				Com_Printf("CG_LoadFont: font image '%s' doesn't exist.\n", token);
				return;
			}
			// save shader name
			Q_strncpyz(shaderName[ shaderCount ], token, sizeof(shaderName[ shaderCount ]));
			// get threshold
			token = COM_ParseExt(&text, qfalse);
			shaderThreshold[ shaderCount ] = atoi(token);

			//Com_Printf( S_COLOR_CYAN "img: %s, threshold: %i\n", shaderName[ shaderCount ], shaderThreshold[ shaderCount ] );
			shaderCount++;

			SkipRestOfLine(&text);
			continue;
		}

		// font parameters
		if (strcmp(token, "fnt") == 0)
		{
			token = COM_ParseExt(&text, qfalse);
			if (token[0] == '\0' || (width = atof(token)) <= 0.0)
			{
				Com_Printf("CG_LoadFont: error reading image width.\n");
				return;
			}
			r_width = 1.0 / width;

			token = COM_ParseExt(&text, qfalse);
			if (token[0] == '\0' || (height = atof(token)) <= 0.0)
			{
				Com_Printf("CG_LoadFont: error reading image height.\n");
				return;
			}
			r_height = 1.0 / height;

			token = COM_ParseExt(&text, qfalse);
			if (token[0] == '\0')
			{
				Com_Printf("CG_LoadFont: error reading char widht.\n");
				return;
			}
			char_width = atof(token);

			token = COM_ParseExt(&text, qfalse);
			if (token[0] == '\0')
			{
				Com_Printf("CG_LoadFont: error reading char height.\n");
				return;
			}
			char_height = atof(token);

			break; // parse char metrics
		}
	}

	if (shaderCount == 0)
	{
		Com_Printf("CG_LoadFont: no font images specified in %s.\n", fontName);
		return;
	}

	fm = fnt->metrics;

	chars = 0;
	for (;;)
	{
		// char index
		token = COM_ParseExt(&text, qtrue);
		if (!token[0])
			break;

		if (token[0] == '\'' && token[1] && token[2] == '\'')   // char code in form 'X'
			i = token[1] & 255;
		else // integer code
			i = atoi(token);

		if (i < 0 || i > 255)
		{
			CG_Printf(S_COLOR_RED "CG_LoadFont: bad char index %i.\n", i);
			return;
		}
		fm = fnt->metrics + i;

		// x0
		token = COM_ParseExt(&text, qfalse);
		if (!token[0])
		{
			CG_Printf(S_COLOR_RED "CG_LoadFont: error reading x0.\n");
			return;
		}
		x0 = atof(token);

		// y0
		token = COM_ParseExt(&text, qfalse);
		if (!token[0])
		{
			CG_Printf(S_COLOR_RED "CG_LoadFont: error reading y0.\n");
			return;
		}
		y0 = atof(token);

		// w1-offset
		token = COM_ParseExt(&text, qfalse);
		if (!token[0])
		{
			CG_Printf(S_COLOR_RED "CG_LoadFont: error reading x-offset.\n");
			return;
		}
		w1 = atof(token);

		// w2-offset
		token = COM_ParseExt(&text, qfalse);
		if (!token[0])
		{
			CG_Printf(S_COLOR_RED "CG_LoadFont: error reading x-length.\n");
			return;
		}
		w2 = atof(token);

		// space1
		token = COM_ParseExt(&text, qfalse);
		if (!token[0])
		{
			CG_Printf(S_COLOR_RED "CG_LoadFont: error reading space1.\n");
			return;
		}
		s1 = atof(token);

		// space2
		token = COM_ParseExt(&text, qfalse);
		if (!token[0])
		{
			CG_Printf(S_COLOR_RED "CG_LoadFont: error reading space2.\n");
			return;
		}
		s2 = atof(token);

		fm->tc_mono[0] = x0 * r_width;
		fm->tc_mono[1] = y0 * r_height;
		fm->tc_mono[2] = (x0 + char_width) * r_width;
		fm->tc_mono[3] = (y0 + char_height) * r_height;

		// proportional y-coords is matching with mono
		fm->tc_prop[1] = fm->tc_mono[1];
		fm->tc_prop[3] = fm->tc_mono[3];

		fm->width = w2 / char_width;
		fm->space1 = s1 / char_width;
		fm->space2 = (s2 + w2) / char_width;
		fm->tc_prop[0] = fm->tc_mono[0] + (w1 * r_width);
		fm->tc_prop[2] = fm->tc_prop[0] + (w2 * r_width);

		chars++;

		SkipRestOfLine(&text);
	}

	// sort images by threshold
	do
	{
		for (swapped = qfalse, i = 1 ; i < shaderCount; i++)
		{
			if (shaderThreshold[i - 1] > shaderThreshold[i])
			{
				tmp = shaderThreshold[i - 1];
				shaderThreshold[i - 1] = shaderThreshold[i];
				shaderThreshold[i] = tmp;
				strcpy(tmpName, shaderName[i - 1]);
				strcpy(shaderName[i - 1], shaderName[i]);
				strcpy(shaderName[i], tmpName);
				swapped = qtrue;
			}
		}
	}
	while (swapped);

	// always assume zero threshold for lowest-quality shader
	shaderThreshold[0] = 0;

	fnt->shaderCount = shaderCount;
	for (i = 0; i < shaderCount; i++)
	{
		fnt->shader[i] = trap_R_RegisterShaderNoMip(shaderName[i]);
		fnt->shaderThreshold[i] = shaderThreshold[i];
	}

	CG_Printf("Font '%s' loaded with %i chars and %i images\n", fontName, chars, shaderCount);
}


void CG_LoadFonts(void)
{
	CG_LoadFont(&fonts[0], "gfx/2d/bigchars.cfg");
	CG_LoadFont(&fonts[1], "gfx/2d/numbers.cfg");
	CG_LoadFont(&fonts[2], "gfx/2d/sansman.cfg");
	CG_LoadFont(&fonts[3], "gfx/2d/sansman.cfg"); // use sansman instead of cpma
	CG_LoadFont(&fonts[4], "gfx/2d/m1rage.cfg");
	CG_LoadFont(&fonts[5], "gfx/2d/elite_emoji.cfg");
	CG_LoadFont(&fonts[6], "gfx/2d/diablo.cfg");
	CG_LoadFont(&fonts[7], "gfx/2d/eternal.cfg");
	CG_LoadFont(&fonts[8], "gfx/2d/qlnumbers.cfg");
	CG_LoadFont(&fonts[9], "gfx/2d/Elite.cfg");
	CG_LoadFont(&fonts[10], "gfx/2d/EliteBigchars.cfg");
}


static float DrawStringLength(const char* string, float ax, float aw, float max_ax, int proportional)
{
	const font_metric_t* fm;
	//float         aw1;
	float           x_end;
	const byte*      s;
	float           xx;

	if (!string)
		return 0.0f;

	s = (const byte*)string;

	xx = ax;

	while (*s != '\0')
	{

		if (*s == Q_COLOR_ESCAPE && s[1] != '\0' && s[1] != '^')
		{
			//if ( !(flags & DS_SHOW_CODE) ) {
			s += 2;
			continue;
			//}
		}

		//fm = &font->metrics[ *s ];
		fm = &metrics[ *s ];
		if (proportional)
		{
			//aw1 = fm->width * aw;
			ax += fm->space1 * aw;          // add extra space if required by metrics
			x_end = ax + fm->space2 * aw;   // final position
		}
		else
		{
			//aw1 = aw;
			x_end = ax + aw;
		}

		if (x_end > max_ax)
			break;

		ax = x_end;
		s++;
	}

	return (ax - xx);
}

static float DrawCompiledStringLength(const text_command_t* cmd, float aw, int proportional)
{
	const font_metric_t* fm;
	float           x_end;
	float           ax = 0;
	int i;
	const text_command_t* curr;

	if (!cmd)
		return 0.0f;


	for (i = 0; i < OSP_TEXT_CMD_MAX; ++i)
	{
		curr = &cmd[i];

		if (curr->type == OSP_TEXT_CMD_CHAR)
		{
			fm = &metrics[(unsigned char)curr->value.character ];
			if (proportional)
			{
				ax += fm->space1 * aw;          // add extra space if required by metrics
				x_end = ax + fm->space2 * aw;   // final position
			}
			else
			{
				x_end = ax + aw;
			}

			if (x_end >= cgs.glconfig.vidWidth)
				break;

			ax = x_end;
		}
		else if (curr->type == OSP_TEXT_CMD_STOP)
		{
			break;
		}
	}

	return ax;
}

void CG_DrawString(float x, float y, const char* string, const vec4_t setColor, float charWidth, float charHeight, int maxChars, int flags)
{
	const font_metric_t* fm;
	const float*     tc; // texture coordinates for char
	float           ax, ay, aw, aw1, ah; // absolute positions/dimensions
	float           scale;
	float           x_end, xx;
	vec4_t          color;
	const byte*      s;
	float           xx_add, yy_add;
	float           max_ax;
	int             i;
	qhandle_t       sh;
	int             proportional;

	if (!string)
		return;

	s = (const byte*)string;

	ax = x * cgs.screenXScale + cgs.screenXBias;
	ay = y * cgs.screenYScale + cgs.screenYBias;

	aw = charWidth * cgs.screenXScale;
	ah = charHeight * cgs.screenYScale;

	if (maxChars <= 0)
	{
		max_ax = 9999999.0f;
	}
	else
	{
		max_ax = ax + aw * maxChars;
	}

	proportional = (flags & DS_PROPORTIONAL);

	if (flags & (DS_HCENTER | DS_HRIGHT))
	{
		if (flags & DS_HCENTER)
		{
			ax -= 0.5f * DrawStringLength(string, ax, aw, max_ax, proportional);
		}
		else
		{
			ax -= DrawStringLength(string, ax, aw, max_ax, proportional);
		}
	}

	if (flags & DS_VCENTER)
	{
		ay -= ah / 2;
	}
	else if (flags & DS_VTOP)
	{
		ay -= ah;
	}

	sh = font->shader[0]; // low-res shader by default

	if (flags & DS_SHADOW)
	{
		xx = ax;

		// calculate shadow offsets
		scale = charWidth * 0.075f; // charWidth/15
		xx_add = scale * cgs.screenXScale;
		yy_add = scale * cgs.screenYScale;

		color[0] = color[1] = color[2] = 0.0f;
		color[3] = setColor[3] * 0.5f;
		trap_R_SetColor(color);

		while (*s != '\0')
		{
			if (*s == Q_COLOR_ESCAPE && s[1] != '\0' && s[1] != '^')
			{
				//if ( !(options & DS_SHOW_CODE) ) {
				s += 2;
				continue;
				//}
			}
			//fm = &font->metrics[ *s ];
			fm = &metrics[ *s ];
			if (proportional)
			{
				tc = fm->tc_prop;
				aw1 = fm->width * aw;
				ax += fm->space1 * aw;          // add extra space if required by metrics
				x_end = ax + fm->space2 * aw;   // final position
			}
			else
			{
				tc = fm->tc_mono;
				aw1 = aw;
				x_end = ax + aw;
			}

			if (x_end > max_ax || ax >= cgs.glconfig.vidWidth)
				break;

			trap_R_DrawStretchPic(ax + xx_add, ay + yy_add, aw1, ah, tc[0], tc[1], tc[2], tc[3], sh);

			ax = x_end;
			s++;
		}

		// recover altered parameters
		s = (const byte*)string;
		ax = xx;
	}

	// select hi-res shader if accepted
	for (i = 1; i < font->shaderCount; i++)
	{
		if (ah >= font->shaderThreshold[i])
		{
			sh = font->shader[i];
		}
	}

	Vector4Copy(setColor, color);
	trap_R_SetColor(color);

	while (*s != '\0')
	{

		if (*s == Q_COLOR_ESCAPE && s[1] != '\0' && s[1] != '^')
		{
			if (!(flags & DS_FORCE_COLOR))
			{
				VectorCopy(g_color_table[ ColorIndex(s[1]) ], color);
				trap_R_SetColor(color);
			}
			//if ( !(options & DS_SHOW_CODE) ) {
			s += 2;
			continue;
			//}
		}

		//fm = &font->metrics[ *s ];
		fm = &metrics[ *s ];
		if (proportional)
		{
			tc = fm->tc_prop;
			aw1 = fm->width * aw;
			ax += fm->space1 * aw;          // add extra space if required by metrics
			x_end = ax + fm->space2 * aw;   // final position
		}
		else
		{
			tc = fm->tc_mono;
			aw1 = aw;
			x_end = ax + aw;
		}

		if (x_end > max_ax || ax >= cgs.glconfig.vidWidth)
			break;

		trap_R_DrawStretchPic(ax, ay, aw1, ah, tc[0], tc[1], tc[2], tc[3], sh);

		ax = x_end;
		s++;
	}

	//trap_R_SetColor( NULL );
}

/*
=================
CG_DrawStrlen

Returns character count, skiping color escape codes
=================
*/
int CG_DrawStrlen(const char* str)
{
	int result = 0;
	float color;
	int len;

	if (!str) return 0;

	len = strlen(str);

	while (len && *str)
	{
		if (*str == '^')
		{
			++str; //skip ^
			--len;
			if (len)
			{
				if (*str == 'X' || *str == 'x')
				{
					++str; //skip code X/x
					--len;
					if (len >= 6 &&
					        CG_Hex16GetColor(&str[0], &color) &&
					        CG_Hex16GetColor(&str[2], &color) &&
					        CG_Hex16GetColor(&str[4], &color))
					{
						str += 6; //skip RRGGBB
						len -= 6;
						continue;
					}
				}
				else if (*str != '^')
				{
					++str; //skip code X/x
					--len;
					continue;
				}
			}
		}
		++str;
		--len;
		++result;
	}
	return result;
}

/*
=============
CG_TileClearBox

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
static void CG_TileClearBox(int x, int y, int w, int h, qhandle_t hShader)
{
	float   s1, t1, s2, t2;

	s1 = x / 64.0;
	t1 = y / 64.0;
	s2 = (x + w) / 64.0;
	t2 = (y + h) / 64.0;
	trap_R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, hShader);
}



/*
==============
CG_TileClear

Clear around a sized down screen
==============
*/
void CG_TileClear(void)
{
	int     top, bottom, left, right;
	int     w, h;

	w = cgs.glconfig.vidWidth;
	h = cgs.glconfig.vidHeight;

	if (cg.refdef.x == 0 && cg.refdef.y == 0 &&
	        cg.refdef.width == w && cg.refdef.height == h)
	{
		return;     // full screen rendering
	}

	top = cg.refdef.y;
	bottom = top + cg.refdef.height - 1;
	left = cg.refdef.x;
	right = left + cg.refdef.width - 1;

	// clear above view screen
	CG_TileClearBox(0, 0, w, top, cgs.media.backTileShader);

	// clear below view screen
	CG_TileClearBox(0, bottom, w, h - bottom, cgs.media.backTileShader);

	// clear left of view screen
	CG_TileClearBox(0, top, left, bottom - top + 1, cgs.media.backTileShader);

	// clear right of view screen
	CG_TileClearBox(right, top, w - right, bottom - top + 1, cgs.media.backTileShader);
}



/*
================
CG_FadeColor
================
*/
float* CG_FadeColor(int startMsec, int totalMsec)
{
	static vec4_t       color;
	int         t;

	if (startMsec == 0)
	{
		return NULL;
	}

	t = cg.time - startMsec;

	if (t >= totalMsec)
	{
		return NULL;
	}

	// fade out
	if (totalMsec - t < FADE_TIME)
	{
		color[3] = (totalMsec - t) * 1.0 / FADE_TIME;
	}
	else
	{
		color[3] = 1.0;
	}
	color[0] = color[1] = color[2] = 1;

	return color;
}


/*
================
CG_TeamColor
================
*/
float* CG_TeamColor(int team)
{
	static vec4_t   red = {1, 0.2f, 0.2f, 1};
	static vec4_t   blue = {0.2f, 0.2f, 1, 1};
	static vec4_t   other = {1, 1, 1, 1};
	static vec4_t   spectator = {0.7f, 0.7f, 0.7f, 1};

	switch (team)
	{
		case TEAM_RED:
			return red;
		case TEAM_BLUE:
			return blue;
		case TEAM_SPECTATOR:
			return spectator;
		default:
			return other;
	}
}

/*
=================
CG_GetColorForHealthVQ3
=================
*/
void CG_GetColorForHealthVQ3(int health, int armor, vec4_t hcolor, char* ccolor)
{
	int     count;
	int     max;

	if (ccolor)
	{
		*ccolor = COLOR_WHITE;
	}

	if (!hcolor)
	{
		return;
	}

	// calculate the total points of damage that can
	// be sustained at the current health / armor level
	if (health <= 0)
	{
		VectorClear(hcolor);     // black
		hcolor[3] = 1;
		return;
	}
	count = armor;
	max = health * ARMOR_PROTECTION / (1.0 - ARMOR_PROTECTION);
	if (max < count)
	{
		count = max;
	}
	health += count;

	// set the color based on health
	hcolor[0] = 1.0;
	hcolor[3] = 1.0;
	if (health >= 100)
	{
		hcolor[2] = 1.0;
	}
	else if (health < 66)
	{
		hcolor[2] = 0;
	}
	else
	{
		hcolor[2] = (health - 66) / 33.0;
	}

	if (health > 60)
	{
		hcolor[1] = 1.0;
	}
	else if (health < 30)
	{
		hcolor[1] = 0;
	}
	else
	{
		hcolor[1] = (health - 30) / 30.0;
	}
}

/*
=================
CG_GetColorForHealth
=================
*/
void CG_GetColorForHealth(int health, int armor, vec4_t hcolor, char* ccolor)
{
	int     count;
	int     max;


	if (!cg_healthColorLevels.integer)
	{
		CG_GetColorForHealthVQ3(health, armor, hcolor, ccolor);
		return;
	}

	// calculate the total points of damage that can
	// be sustained at the current health / armor level
	if (health <= 0)
	{
		if (hcolor)
		{
			VectorClear(hcolor);     // black
			hcolor[3] = 1;
		}
		if (ccolor)
		{
			*ccolor = COLOR_BLACK;
		}
		return;
	}

	count = armor;
	max = health * ARMOR_PROTECTION / (1.0 - ARMOR_PROTECTION);
	if (max < count)
	{
		count = max;
	}
	health += count;

	if (health <= cg_healthLow.integer)
	{
		if (hcolor)
		{
			VectorCopy(colorRed, hcolor);
		}
		if (ccolor)
		{
			*ccolor = COLOR_RED;
		}
	}
	else if (health >= cg_healthMid.integer)
	{
		if (hcolor)
		{
			VectorCopy(colorWhite, hcolor);
		}
		if (ccolor)
		{
			*ccolor = COLOR_WHITE;
		}
	}
	else
	{
		if (hcolor)
		{
			VectorSet(hcolor, 1, 0.7, 0);
		}
		if (ccolor)
		{
			*ccolor = COLOR_YELLOW;
		}
	}

	if (hcolor)
	{
		hcolor[3] = 1.0;
	}
}

/*
=================
CG_ColorForHealth
=================
*/
void CG_ColorForHealth(vec4_t hcolor, char* color)
{

	CG_GetColorForHealth(cg.snap->ps.stats[STAT_HEALTH],
	                     cg.snap->ps.stats[STAT_ARMOR], hcolor, color);
}



void CG_OSPDrawPoly(float x, float y, float w, float h, vec4_t color)
{
	trap_R_SetColor(color);
	CG_AdjustFrom640_Old(&x, &y, &w, &h, qfalse);
	trap_R_DrawStretchPic(x, y, w, h, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_SetColor(colorWhite);
}


// bk001205 - code below duplicated in q3_ui/ui-atoms.c
// bk001205 - FIXME: does this belong in ui_shared.c?
// bk001205 - FIXME: HARD_LINKED flags not visible here
#ifndef Q3_STATIC // bk001205 - q_shared defines not visible here 
/*
=================
UI_DrawProportionalString2
=================
*/
static int  propMap[128][3] =
{
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

	{0, 0, PROP_SPACE_WIDTH},       // SPACE
	{11, 122, 7},   // !
	{154, 181, 14}, // "
	{55, 122, 17},  // #
	{79, 122, 18},  // $
	{101, 122, 23}, // %
	{153, 122, 18}, // &
	{9, 93, 7},     // '
	{207, 122, 8},  // (
	{230, 122, 9},  // )
	{177, 122, 18}, // *
	{30, 152, 18},  // +
	{85, 181, 7},   // ,
	{34, 93, 11},   // -
	{110, 181, 6},  // .
	{130, 152, 14}, // /

	{22, 64, 17},   // 0
	{41, 64, 12},   // 1
	{58, 64, 17},   // 2
	{78, 64, 18},   // 3
	{98, 64, 19},   // 4
	{120, 64, 18},  // 5
	{141, 64, 18},  // 6
	{204, 64, 16},  // 7
	{162, 64, 17},  // 8
	{182, 64, 18},  // 9
	{59, 181, 7},   // :
	{35, 181, 7},   // ;
	{203, 152, 14}, // <
	{56, 93, 14},   // =
	{228, 152, 14}, // >
	{177, 181, 18}, // ?

	{28, 122, 22},  // @
	{5, 4, 18},     // A
	{27, 4, 18},    // B
	{48, 4, 18},    // C
	{69, 4, 17},    // D
	{90, 4, 13},    // E
	{106, 4, 13},   // F
	{121, 4, 18},   // G
	{143, 4, 17},   // H
	{164, 4, 8},    // I
	{175, 4, 16},   // J
	{195, 4, 18},   // K
	{216, 4, 12},   // L
	{230, 4, 23},   // M
	{6, 34, 18},    // N
	{27, 34, 18},   // O

	{48, 34, 18},   // P
	{68, 34, 18},   // Q
	{90, 34, 17},   // R
	{110, 34, 18},  // S
	{130, 34, 14},  // T
	{146, 34, 18},  // U
	{166, 34, 19},  // V
	{185, 34, 29},  // W
	{215, 34, 18},  // X
	{234, 34, 18},  // Y
	{5, 64, 14},    // Z
	{60, 152, 7},   // [
	{106, 151, 13}, // '\'
	{83, 152, 7},   // ]
	{128, 122, 17}, // ^
	{4, 152, 21},   // _

	{134, 181, 5},  // '
	{5, 4, 18},     // A
	{27, 4, 18},    // B
	{48, 4, 18},    // C
	{69, 4, 17},    // D
	{90, 4, 13},    // E
	{106, 4, 13},   // F
	{121, 4, 18},   // G
	{143, 4, 17},   // H
	{164, 4, 8},    // I
	{175, 4, 16},   // J
	{195, 4, 18},   // K
	{216, 4, 12},   // L
	{230, 4, 23},   // M
	{6, 34, 18},    // N
	{27, 34, 18},   // O

	{48, 34, 18},   // P
	{68, 34, 18},   // Q
	{90, 34, 17},   // R
	{110, 34, 18},  // S
	{130, 34, 14},  // T
	{146, 34, 18},  // U
	{166, 34, 19},  // V
	{185, 34, 29},  // W
	{215, 34, 18},  // X
	{234, 34, 18},  // Y
	{5, 64, 14},    // Z
	{153, 152, 13}, // {
	{11, 181, 5},   // |
	{180, 152, 13}, // }
	{79, 93, 17},   // ~
	{0, 0, -1}      // DEL
};

static int propMapB[26][3] =
{
	{11, 12, 33},
	{49, 12, 31},
	{85, 12, 31},
	{120, 12, 30},
	{156, 12, 21},
	{183, 12, 21},
	{207, 12, 32},

	{13, 55, 30},
	{49, 55, 13},
	{66, 55, 29},
	{101, 55, 31},
	{135, 55, 21},
	{158, 55, 40},
	{204, 55, 32},

	{12, 97, 31},
	{48, 97, 31},
	{82, 97, 30},
	{118, 97, 30},
	{153, 97, 30},
	{185, 97, 25},
	{213, 97, 30},

	{11, 139, 32},
	{42, 139, 51},
	{93, 139, 32},
	{126, 139, 31},
	{158, 139, 25},
};

#define PROPB_GAP_WIDTH     4
#define PROPB_SPACE_WIDTH   12
#define PROPB_HEIGHT        36

/*
=================
UI_DrawBannerString
=================
*/
static void UI_DrawBannerString2(int x, int y, const char* str, vec4_t color)
{
	const char* s;
	unsigned char   ch; // bk001204 : array subscript
	float   ax;
	float   ay;
	float   aw;
	float   ah;
	float   frow;
	float   fcol;
	float   fwidth;
	float   fheight;

	// draw the colored text
	trap_R_SetColor(color);

	ax = x * cgs.screenXScale_Old + cgs.screenXBias_Old;
	ay = y * cgs.screenXScale_Old;

	s = str;
	while (*s)
	{
		ch = *s & 127;
		if (ch == ' ')
		{
			ax += ((float)PROPB_SPACE_WIDTH + (float)PROPB_GAP_WIDTH) * cgs.screenXScale_Old;
		}
		else if (ch >= 'A' && ch <= 'Z')
		{
			ch -= 'A';
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2] * cgs.screenXScale_Old;
			ah = (float)PROPB_HEIGHT * cgs.screenXScale_Old;
			trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, cgs.media.charsetPropB);
			ax += (aw + (float)PROPB_GAP_WIDTH * cgs.screenXScale_Old);
		}
		s++;
	}

	trap_R_SetColor(NULL);
}

void UI_DrawBannerString(int x, int y, const char* str, int style, vec4_t color)
{
	const char*     s;
	int             ch;
	int             width;
	vec4_t          drawcolor;

	// find the width of the drawn text
	s = str;
	width = 0;
	while (*s)
	{
		ch = *s;
		if (ch == ' ')
		{
			width += PROPB_SPACE_WIDTH;
		}
		else if (ch >= 'A' && ch <= 'Z')
		{
			width += propMapB[ch - 'A'][2] + PROPB_GAP_WIDTH;
		}
		s++;
	}
	width -= PROPB_GAP_WIDTH;

	switch (style & UI_FORMATMASK)
	{
		case UI_CENTER:
			x -= width / 2;
			break;

		case UI_RIGHT:
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if (style & UI_DROPSHADOW)
	{
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawBannerString2(x + 2, y + 2, str, drawcolor);
	}

	UI_DrawBannerString2(x, y, str, color);
}


int UI_ProportionalStringWidth(const char* str)
{
	const char*     s;
	int             ch;
	int             charWidth;
	int             width;

	s = str;
	width = 0;
	while (*s)
	{
		ch = *s & 127;
		charWidth = propMap[ch][2];
		if (charWidth != -1)
		{
			width += charWidth;
			width += PROP_GAP_WIDTH;
		}
		s++;
	}

	width -= PROP_GAP_WIDTH;
	return width;
}

static void UI_DrawProportionalString2(int x, int y, const char* str, vec4_t color, float sizeScale, qhandle_t charset)
{
	const char* s;
	unsigned char   ch; // bk001204 - unsigned
	float   ax;
	float   ay;
	float   aw;
	float   ah;
	float   frow;
	float   fcol;
	float   fwidth;
	float   fheight;

	// draw the colored text
	trap_R_SetColor(color);

	ax = x * cgs.screenXScale_Old + cgs.screenXBias_Old;
	ay = y * cgs.screenXScale_Old;

	s = str;
	while (*s)
	{
		ch = *s & 127;
		if (ch == ' ')
		{
			aw = (float)PROP_SPACE_WIDTH * cgs.screenXScale_Old * sizeScale;
		}
		else if (propMap[ch][2] != -1)
		{
			fcol = (float)propMap[ch][0] / 256.0f;
			frow = (float)propMap[ch][1] / 256.0f;
			fwidth = (float)propMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)propMap[ch][2] * cgs.screenXScale_Old * sizeScale;
			ah = (float)PROP_HEIGHT * cgs.screenXScale_Old * sizeScale;
			trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, charset);
		}
		else
		{
			aw = 0;
		}

		ax += (aw + (float)PROP_GAP_WIDTH * cgs.screenXScale_Old * sizeScale);
		s++;
	}

	trap_R_SetColor(NULL);
}

/*
=================
UI_ProportionalSizeScale
=================
*/
float UI_ProportionalSizeScale(int style)
{
	if (style & UI_SMALLFONT)
	{
		return 0.75;
	}

	return 1.00;
}


/*
=================
UI_DrawProportionalString
=================
*/
void UI_DrawProportionalString(int x, int y, const char* str, int style, vec4_t color)
{
	vec4_t  drawcolor;
	int     width;
	float   sizeScale;

	sizeScale = UI_ProportionalSizeScale(style);

	switch (style & UI_FORMATMASK)
	{
		case UI_CENTER:
			width = UI_ProportionalStringWidth(str) * sizeScale;
			x -= width / 2;
			break;

		case UI_RIGHT:
			width = UI_ProportionalStringWidth(str) * sizeScale;
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if (style & UI_DROPSHADOW)
	{
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x + 2, y + 2, str, drawcolor, sizeScale, cgs.media.charsetProp);
	}

	if (style & UI_INVERSE)
	{
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x, y, str, drawcolor, sizeScale, cgs.media.charsetProp);
		return;
	}

	if (style & UI_PULSE)
	{
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x, y, str, color, sizeScale, cgs.media.charsetProp);

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin(cg.time / (float)PULSE_DIVISOR);
		UI_DrawProportionalString2(x, y, str, drawcolor, sizeScale, cgs.media.charsetPropGlow);
		return;
	}

	UI_DrawProportionalString2(x, y, str, color, sizeScale, cgs.media.charsetProp);
}

#endif // Q3STATIC

static qboolean CG_OSPCharHexToInt(char c, int* out)
{
	if (c >= '0' && c <= '9')
	{
		*out = c - '0';
		return qtrue;
	}
	if (c >= 'a' && c <= 'f')
	{
		*out = c - 'a' + 10;
		return qtrue;
	}
	if (c >= 'A' && c <= 'F')
	{
		*out = c - 'A' + 10;
		return qtrue;
	}
	return qfalse;
}

qboolean CG_Hex16GetColor(const char* str, float* color)
{
	int d1;
	int d2;
	int color_int;
	if (!str) return qfalse;

	if (!CG_OSPCharHexToInt(str[0], &d1))
	{
		return  qfalse;
	}

	if (!CG_OSPCharHexToInt(str[1], &d2))
	{
		return  qfalse;
	}

	color_int = d1 * 16 + d2;

	*color = (float)color_int;
	*color /= 255.0f;

	return qtrue;
}

int CG_OSPDrawStringOld(int x, int y, const char* str, int charWidth, int charHeight, vec4_t const colors, int maxChars, qboolean drawShadow)
{
	vec4_t lTextColor;
	float defaultBright;
	float char_w_f;
	float char_h_f;
	const char* str_ptr;
	int next_x;
	int printed;
	const static vec4_t textColor = { 1.0, 1.0, 1.0, 1.0};

	Vector4Copy(textColor, lTextColor);
	char_w_f = (float)charWidth * cgs.screenXScale_Old;
	char_h_f = (float)charHeight * cgs.screenYScale_Old;
	str_ptr = str;
	next_x = x;
	printed = 0;

	if (colors)
	{
		Vector4Copy(colors, lTextColor);
	}

	trap_R_SetColor(lTextColor);

	defaultBright = lTextColor[3];
	if (maxChars == 0)
	{
		maxChars = 32768;
	}

	while (str_ptr && *str_ptr != 0)
	{
		if (str_ptr[0] == '^' && str_ptr[1] != '^')
		{
			/*
			^B and ^b - make text blink
			^F        - makes the text display only at the first half of a second
			^f        - makes the text display only at the last half of a second
			^N        - resets control stuff like ^B, ^F, ^f
			^Xrrggbb  - allows the user to define the color using the rgb values (hex)
			            (e.g. ^XFF0000 is red, ^X777777 some kind of grey).
			*/
			char control_char;
			control_char = str_ptr[1];
			str_ptr += 2;

			switch (control_char)
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
					if (!drawShadow)
					{
						unsigned int color_index;
						color_index = (control_char - 0x30) % 10;
						VectorCopy(g_color_table[color_index], lTextColor);
						trap_R_SetColor(lTextColor);
					}
					break;
				case 'N':
				case 'n':
					lTextColor[3] = defaultBright;
					trap_R_SetColor(lTextColor);
					break;
				case 'B':
				{
					int b;
					b = cg.time & 0x7ff;
					if (b > 1024)
					{
						b = ~b & 0x7ff;
					}
					lTextColor[3] = b / 1463.0f + 0.3f;
					trap_R_SetColor(lTextColor);
				}
				break;
				case 'b':
				{
					int b;
					b = cg.time & 0x7ff;
					if (b > 1024)
					{
						b = ~b & 0x7ff;
					}
					lTextColor[3] = b / 1024.0f;
					trap_R_SetColor(lTextColor);
				}
				break;
				case 'f':
					if ((cg.time & 0x3ff) >= 512)
					{
						while (str_ptr && *str_ptr && !(str_ptr[0] == '^' && (str_ptr[1] == 'N' || str_ptr[1] == 'F')))
						{
							++str_ptr;
						}
					}
					break;
				case 'F':
					if ((cg.time & 0x3ff) < 512)
					{
						while (str_ptr && *str_ptr && !(str_ptr[0] == '^' && (str_ptr[1] == 'N' || str_ptr[1] == 'f')))
						{
							++str_ptr;
						}
					}
					break;
				case 'X':
				case 'x':
				{
					float r, g, b;
					if (!CG_Hex16GetColor(&str_ptr[0], &r))
					{
						break;
					}
					if (!CG_Hex16GetColor(&str_ptr[2], &g))
					{
						break;
					}
					if (!CG_Hex16GetColor(&str_ptr[4], &b))
					{
						break;
					}

					lTextColor[0] = r;
					lTextColor[1] = g;
					lTextColor[2] = b;
					if (drawShadow)
					{
						trap_R_SetColor(lTextColor);
					}
					str_ptr += 6;
				}
				break;
				default:
					break;
			}
		}
		else
		{
			if (++printed > maxChars)
			{
				return next_x - x;
			}
			if (*str_ptr != ' ')
			{
				float t1 = 0.0625f * (float)(*str_ptr >> 4);
				float s1 = 0.0625f * (float)(*str_ptr & 0x0f);
				qhandle_t shader;

				shader = ch_3waveFont.integer ? cgs.media.charsetShader1 : cgs.media.charsetShader;
				trap_R_DrawStretchPic(
				    (float)next_x * cgs.screenXScale_Old,
				    (float)y * cgs.screenYScale_Old,
				    char_w_f,
				    char_h_f,
				    s1,
				    t1,
				    s1 + 0.0625f,
				    t1 + 0.0625f,
				    shader);
			}
			next_x += charWidth;
			++str_ptr;
		}
	}
	trap_R_SetColor(colorWhite);
	return next_x - x;
}

float CG_OSPDrawStringLengthNew(const char* string, float ax, float aw, float max_ax, int proportional)
{
	const font_metric_t* fm;
	float           x_end;
	const byte*      s;
	float           xx;

	if (!string)
		return 0.0f;

	s = (const byte*)string;

	xx = ax;

	while (*s != '\0')
	{

		if (*s == Q_COLOR_ESCAPE && s[1] != '\0' && s[1] != '^')
		{
			switch (s[1])
			{
				case 'X':
				case 'x':
				{
					float r, g, b;
					if (!CG_Hex16GetColor((const char*)&s[2], &r))
					{
						break;
					}
					if (!CG_Hex16GetColor((const char*)&s[4], &g))
					{
						break;
					}
					if (!CG_Hex16GetColor((const char*)&s[6], &b))
					{
						break;
					}
				}
				s += 8;
				break;
				default:
					s += 2;
					break;
			}
			continue;
		}

		fm = &metrics[ *s ];
		if (proportional)
		{
			ax += fm->space1 * aw;          // add extra space if required by metrics
			x_end = ax + fm->space2 * aw;   // final position
		}
		else
		{
			x_end = ax + aw;
		}

		if (x_end > max_ax)
			break;

		ax = x_end;
		s++;
	}

	return (ax - xx);
}

static float GetSymbolSize(char sym, qboolean proportional, float charWidth)
{
	const font_metric_t* fm;
	fm = &metrics[(unsigned char)sym];

	if (proportional)
	{
		return (fm->space1 + fm->space2) * charWidth;
	}

	return charWidth;
}

static int CompiledStringSize(const text_command_t* cmd)
{
	int size = 0;
	if (!cmd)
	{
		return 0;
	}

	for (size = 0; size < OSP_TEXT_CMD_MAX && cmd[size].type != OSP_TEXT_CMD_STOP; ++size);

	return size;
}

static float RestrictCompiledString(text_command_t* cmd, float charWidth, qboolean proportional, float toWidth)
{
	const font_metric_t* fm;
	float           x_end;
	float           ax = 0;
	int i;
	int size;
	text_command_t* curr;
	qboolean restricted = qfalse;

	size = CompiledStringSize(cmd);

	if (!cmd || toWidth == 0)
		return 0;

	for (i = 0; i < size; ++i)
	{
		curr = &cmd[i];

		if (curr->type == OSP_TEXT_CMD_CHAR)
		{
			fm = &metrics[(unsigned char)curr->value.character ];
			if (proportional)
			{
				ax += fm->space1 * charWidth;          // add extra space if required by metrics
				x_end = ax + fm->space2 * charWidth;   // final position
			}
			else
			{
				x_end = ax + charWidth;
			}

			ax = x_end;
			if (ax >= toWidth)
			{
				restricted = qtrue;
				break;
			}
		}
		else if (curr->type == OSP_TEXT_CMD_STOP)
		{
			break;
		}
	}

	/* replace tail with "..." */
	if (restricted)
	{
		int replacedWithDots;
		float three_dot_size = GetSymbolSize('.', proportional, charWidth) * 3;
		float erased_size;
		int erased_cmds;

		/* first, erase characters is enough to insert "..." */
		for (erased_size = 0, erased_cmds = 0; (i > 0) && (erased_size < three_dot_size || erased_cmds < 4); --i, ++erased_cmds)
		{
			curr = &cmd[i];
			if (curr->type == OSP_TEXT_CMD_CHAR)
			{
				erased_size += GetSymbolSize(curr->value.character, proportional, charWidth);
			}
		}

		/* second, insert "..." */
		for (replacedWithDots = 0; (i < (size - 1)) && (replacedWithDots < 3); ++i)
		{
			curr = &cmd[i];
			curr->type = OSP_TEXT_CMD_CHAR;
			curr->value.character = '.';
			++replacedWithDots;
		}

		/* set stop of text */
		cmd[i].type = OSP_TEXT_CMD_STOP;
	}

	return ax;
}

int CG_OSPDrawStringLenPix(const char* string, float charWidth, int flags, int toWidth)
{
	int rez;
	text_command_t* text_commands;

	if (!string)
	{
		return 0;
	}

	text_commands = CG_CompileText(string);
	if (!text_commands)
	{
		return 0;
	}
	RestrictCompiledString(text_commands, charWidth, flags & DS_PROPORTIONAL, toWidth);
	rez = DrawCompiledStringLength(text_commands, charWidth, flags & DS_PROPORTIONAL);
	CG_CompiledTextDestroy(text_commands);
	return rez;
}



void CG_OSPDrawString(float x, float y, const char* string, const vec4_t setColor, float charWidth, float charHeight, int maxWidth, int flags, vec4_t background)
{
	const font_metric_t* fm;
	const float*     tc; // texture coordinates for char
	float           ax, ay, aw, aw1, ah; // absolute positions/dimensions
	float           x_end, xx;
	float           fade = 1.0f;
	vec4_t          color;
	float           xx_add, yy_add;
	int             i;
	qhandle_t       sh;
	int             proportional;
	text_command_t* text_commands;
	text_command_t* curr;
	float           expectedLenght = 0;

	if (!string)
		return;


	text_commands = CG_CompileText(string);
	if (!text_commands)
	{
		return;
	}

	CG_AdjustFrom640(&x, &y, &charWidth, &charHeight);

	ax = x;
	ay = y;

	aw = charWidth;
	ah = charHeight;

	proportional = (flags & DS_PROPORTIONAL);

	{
		float mw = maxWidth;
		CG_AdjustFrom640(NULL, NULL, &mw, NULL);
		RestrictCompiledString(text_commands, aw, proportional, mw);
	}

	if (background || (flags & (DS_HCENTER | DS_HRIGHT)))
	{
		expectedLenght = DrawCompiledStringLength(text_commands, aw, proportional);
	}


	if (flags & (DS_HCENTER | DS_HRIGHT))
	{
		if (flags & DS_HCENTER)
		{
			ax -= 0.5f * expectedLenght;
		}
		else
		{
			ax -= expectedLenght;
		}
	}

	if (flags & DS_VCENTER)
	{
		ay -= ah / 2;
	}
	else if (flags & DS_VTOP)
	{
		ay -= ah;
	}

	sh = font->shader[0]; // low-res shader by default

	// select hi-res shader if accepted
	for (i = 1; i < font->shaderCount; i++)
	{
		if (ah >= font->shaderThreshold[i])
		{
			sh = font->shader[i];
		}
	}

	if (background)
	{
		trap_R_SetColor(background);
		trap_R_DrawStretchPic(ax, ay, expectedLenght, ah, 0, 0, 0, 0, cgs.media.whiteShader);
		trap_R_SetColor(colorWhite);
	}

	if (flags & DS_SHADOW)
	{
		xx = ax;

		// calculate shadow offsets
		yy_add = xx_add = charWidth / 10.0f;

		VectorCopy(colorBlack, color);
		color[3] = fade;
		trap_R_SetColor(color);

		for (i = 0; i < OSP_TEXT_CMD_MAX && text_commands[i].type != OSP_TEXT_CMD_STOP; ++i)
		{
			curr = &text_commands[i];
			switch (curr->type)
			{
				case OSP_TEXT_CMD_CHAR:
					fm = &metrics[(unsigned char)curr->value.character ];
					if (proportional)
					{
						tc = fm->tc_prop;
						aw1 = fm->width * aw;
						ax += fm->space1 * aw;          // add extra space if required by metrics
						x_end = ax + fm->space2 * aw;   // final position
					}
					else
					{
						tc = fm->tc_mono;
						aw1 = aw;
						x_end = ax + aw;
					}

					if (ax >= cgs.glconfig.vidWidth)
						break;

					trap_R_DrawStretchPic(ax + xx_add, ay + yy_add, aw1, ah, tc[0], tc[1], tc[2], tc[3], sh);

					ax = x_end;
					break;
				case OSP_TEXT_CMD_SHADOW_COLOR:
					VectorCopy(curr->value.color, color);
					color[3] = fade;
					if (setColor && color[3] > setColor[3])
					{
						color[3] = setColor[3];
					}
					trap_R_SetColor(color);
					break;
				case OSP_TEXT_CMD_FADE:
					fade = curr->value.fade;
					color[3] = fade;
					if (setColor && color[3] > setColor[3])
					{
						color[3] = setColor[3];
					}
					trap_R_SetColor(color);
					break;
				case OSP_TEXT_CMD_STOP:
				case OSP_TEXT_CMD_TEXT_COLOR:
					break;
			}
		}

		// recover altered parameters
		ax = xx;
	}


	if (setColor)
	{
		Vector4Copy(setColor, color);
	}
	trap_R_SetColor(color);
	fade = 1.0f;

	for (i = 0; i < OSP_TEXT_CMD_MAX && text_commands[i].type != OSP_TEXT_CMD_STOP; ++i)
	{
		curr = &text_commands[i];
		switch (curr->type)
		{
			case OSP_TEXT_CMD_CHAR:
			{
				//avoid compiler bug
				//after cast curr->value.character to unsigned char it still could be less than zero
				int index = curr->value.character;
				if (index < 0) index += 256;
				fm = &metrics[(unsigned char)index ];
			}
			if (proportional)
			{
				tc = fm->tc_prop;
				aw1 = fm->width * aw;
				ax += fm->space1 * aw;          // add extra space if required by metrics
				x_end = ax + fm->space2 * aw;   // final position
			}
			else
			{
				tc = fm->tc_mono;
				aw1 = aw;
				x_end = ax + aw;
			}

			if (ax >= cgs.glconfig.vidWidth)
				break;

			trap_R_DrawStretchPic(ax, ay, aw1, ah, tc[0], tc[1], tc[2], tc[3], sh);

			ax = x_end;
			break;
			case OSP_TEXT_CMD_TEXT_COLOR:
				VectorCopy(curr->value.color, color);
				color[3] = fade;
				if (setColor && color[3] > setColor[3])
				{
					color[3] = setColor[3];
				}
				trap_R_SetColor(color);
				break;
			case OSP_TEXT_CMD_FADE:
				fade = curr->value.fade;
				color[3] = fade;
				if (setColor && color[3] > setColor[3])
				{
					color[3] = setColor[3];
				}
				trap_R_SetColor(color);
				break;
			case OSP_TEXT_CMD_SHADOW_COLOR:
			case OSP_TEXT_CMD_STOP:
				break;
		}
	}

	CG_CompiledTextDestroy(text_commands);

	trap_R_SetColor(NULL);
}

int CG_OSPDrawStringWithShadow(int x, int y, const char* str, int charWidth, int charHeight, const vec4_t color, int maxChars)
{
	int shift_x;
	int shift_y;
	vec4_t shadow;
	const static vec4_t textShadowColor = { 0.0, 0.0, 0.0, 1.0};


	memcpy(shadow, textShadowColor, sizeof(shadow));

	if (color)
	{
		shadow[3] = color[3];
	}
	else
	{
		shadow[3] = 1.0;
	}

	shift_x = charWidth > 12 ? 2 : 1;
	shift_y = charHeight > 12 ? 2 : 1;


	CG_OSPDrawStringOld(x + shift_x, y + shift_y, str, charWidth, charHeight, shadow, maxChars, qtrue);
	return CG_OSPDrawStringOld(x, y, str, charWidth, charHeight, color, maxChars, qfalse);
}


