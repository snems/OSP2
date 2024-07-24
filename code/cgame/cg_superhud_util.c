#include "./cg_superhud_private.h"
#include "cg_local.h"

typedef struct
{
	vec4_t bar1_value;
	vec4_t bar1_bottom;
	vec4_t bar2_value;
	vec4_t bar2_bottom;
} drawBarCoords_t;

static const float* CG_SHUDConfigPickColor(const superhudColor_t* in)
{
	clientInfo_t* ci;

	switch (in->type)
	{
		case SUPERHUD_COLOR_RGBA:
			return in->rgba;
		case SUPERHUD_COLOR_T:
			ci = &cgs.clientinfo[ cg.clientNum ];
			if (ci->rt == TEAM_RED)
			{
				return colorRed;
			}
			else if (ci->rt == TEAM_BLUE)
			{
				return colorBlue;
			}
			return colorRed;
		case SUPERHUD_COLOR_E:
			ci = &cgs.clientinfo[ cg.clientNum ];
			if (ci->rt == TEAM_RED)
			{
				return colorBlue;
			}
			else if (ci->rt == TEAM_BLUE)
			{
				return colorRed;
			}
			return colorBlue;
		case SUPERHUD_COLOR_I:
			return colorWhite;
	}
	return colorWhite;
}

static void CG_SHUDConfigDefaultsCheck(superhudConfig_t* config)
{
	if (!config->rect.isSet)
	{
		config->rect.value[0] = 0.0f;
		config->rect.value[1] = 0.0f;
		config->rect.value[2] = 320.0f;
		config->rect.value[3] = 12.0f;
		config->rect.isSet = qtrue;
	}

	if (!config->textAlign.isSet)
	{
		config->textAlign.value = SUPERHUD_ALIGNH_LEFT;
		config->textAlign.isSet = qtrue;
	}

	/* if fontsize wasn't set, find max size for rect */
	if (!config->fontsize.isSet)
	{
		config->fontsize.value[1] = config->rect.value[3];
		config->fontsize.value[0] = config->fontsize.value[1] / 1.618f;
		config->fontsize.isSet = qtrue;
	}

	if (!config->textStyle.isSet)
	{
		config->textStyle.value = 0; //drop shadow
		config->textStyle.isSet = qtrue;
	}

	if (!config->color.isSet)
	{
		Vector4Copy(colorWhite, config->color.value.rgba);
		config->color.value.type = SUPERHUD_COLOR_RGBA;
		config->color.isSet = qtrue;
	}
}

static void CG_SHUDTextMakeAdjustCoords(const superhudConfig_t* in, float *out_x, float *out_y)
{
	superhudAlignH_t h;
	superhudAlignV_t v;

	if (!in->rect.isSet)
	{
		return;
	}

	if (!in->alignH.isSet)
	{
		h = SUPERHUD_ALIGNH_LEFT;
	}
	else
	{
		h = in->alignH.value;
	}

	if (!in->alignV.isSet)
	{
		v = SUPERHUD_ALIGNV_CENTER;
	}
	else
	{
		v = in->alignV.value;
	}

	switch (h)
	{
    case SUPERHUD_ALIGNH_LEFT:
    	//allready x
			*out_x = in->rect.value[0];
    	break;
    case SUPERHUD_ALIGNH_CENTER:
    	// x + width/2
			*out_x = in->rect.value[0] + in->rect.value[2] / 2.0f;
			break;
    case SUPERHUD_ALIGNH_RIGHT:
    	// x + width
			*out_x = in->rect.value[0] + in->rect.value[2];
      break;
  }

	switch (v) 
	{
    case SUPERHUD_ALIGNH_LEFT:
			*out_y = in->rect.value[1];
      break;
    case SUPERHUD_ALIGNH_CENTER:
			*out_y = in->rect.value[1] + in->rect.value[3]/2.0f;
      break;
    case SUPERHUD_ALIGNH_RIGHT:
			*out_y = in->rect.value[1] + in->rect.value[3];
      break;
  }

}

void CG_SHUDTextMakeContext(const superhudConfig_t* in, superhudTextContext_t* out)
{
	superhudConfig_t config;
	memset(out, 0, sizeof(*out));
	memcpy(&config, in, sizeof(config));

	CG_SHUDConfigDefaultsCheck(&config);

	CG_SHUDTextMakeAdjustCoords(in, &out->textX, &out->textY);

  switch (config.textAlign.value)
	{
		default:
		case SUPERHUD_ALIGNH_LEFT:
			out->flags |= DS_LEFT;
			break;
		case SUPERHUD_ALIGNH_CENTER:
			out->flags |= DS_CENTER;
			break;
		case SUPERHUD_ALIGNH_RIGHT:
			out->flags |= DS_RIGHT;
			break;
	}

	out->fontW = config.fontsize.value[0];
	out->fontH = config.fontsize.value[1];

	if (!config.monospace.isSet)
	{
		out->flags |= DS_PROPORTIONAL;
	}
	if (config.textStyle.value & 1)
	{
		out->flags |= DS_SHADOW;
	}

	out->fontIndex = CG_FontIndexFromName(config.font.isSet ? config.font.value : "sansman");

	Vector4Copy(CG_SHUDConfigPickColor(&config.color.value), out->color);
	Vector4Copy(out->color, out->color_origin);
}

void CG_SHUDDrawMakeContext(const superhudConfig_t* in, superhudDrawContext_t* out)
{
	superhudConfig_t config;
	memset(out, 0, sizeof(*out));
	memcpy(&config, in, sizeof(config));

	CG_SHUDConfigDefaultsCheck(&config);

	out->x = config.rect.value[0];
	out->y = config.rect.value[1];
	out->w = config.rect.value[2];
	out->h = config.rect.value[3];

	CG_AdjustFrom640(&out->x, &out->y, &out->w, &out->h);

	Vector4Copy(CG_SHUDConfigPickColor(&config.color.value), out->color);
	Vector4Copy(out->color, out->color_origin);
}

void CG_SHUDBarMakeContext(const superhudConfig_t* in, superhudBarContext_t* out, float max)
{
	float x = 0, y = 0;
	superhudConfig_t config;
	memset(out, 0, sizeof(*out));
	memcpy(&config, in, sizeof(config));

	CG_SHUDConfigDefaultsCheck(&config);

	if (!config.direction.isSet)
	{
		config.direction.isSet = qtrue;
		config.direction.value = SUPERHUD_DIR_LEFT_TO_RIGHT;
	}

	out->direction = config.direction.value;

	x = config.rect.value[0];
	y = config.rect.value[1];

	if (config.doublebar.isSet)
	{
		static const float bar_gap = 4;
		out->two_bars = qtrue;
		//split into two bars
		if (out->direction == SUPERHUD_DIR_LEFT_TO_RIGHT || out->direction == SUPERHUD_DIR_RIGHT_TO_LEFT)
		{
			//split horizontal
			const float bar_height = config.rect.value[3] / 2 - bar_gap / 2;
			const float bar_width = config.rect.value[2];

			out->bar[0][0] = x;//x
			out->bar[0][1] = y;//y
			out->bar[0][2] = bar_width;//w
			out->bar[0][3] = bar_height;   // height is half of rect and minus half of the gap between two bars
			//
			out->bar[1][0] = x;//x
			out->bar[1][1] = y + bar_height + bar_gap;//y starts after first bar and gap
			out->bar[1][2] = bar_width;//w
			out->bar[1][3] = bar_height;//h height is same as in first bar
			CG_AdjustFrom640(&out->bar[1][0], &out->bar[1][1], &out->bar[1][2], &out->bar[1][3]);
			CG_AdjustFrom640(&out->bar[0][0], &out->bar[0][1], &out->bar[0][2], &out->bar[0][3]);
			out->max = out->bar[1][2];
			out->koeff = 2 * out->bar[1][2] / max;
		}
		else
		{
			//split vertical
			const float bar_height = config.rect.value[3];
			const float bar_width = config.rect.value[2] / 2 - bar_gap / 2;

			out->bar[0][0] = x;//x
			out->bar[0][1] = y;//y
			out->bar[0][2] = bar_width;//w
			out->bar[0][3] = bar_height;   //h
			//
			out->bar[1][0] = x + bar_width + bar_gap;//x
			out->bar[1][1] = y;//y
			out->bar[1][2] = bar_width;//w
			out->bar[1][3] = bar_height;//h
			CG_AdjustFrom640(&out->bar[1][0], &out->bar[1][1], &out->bar[1][2], &out->bar[1][3]);
			CG_AdjustFrom640(&out->bar[0][0], &out->bar[0][1], &out->bar[0][2], &out->bar[0][3]);
			out->max = out->bar[1][3];
			out->koeff = 2 * out->bar[1][3] / max;
		}
	}
	else
	{
		// single bar
		out->bar[0][0] = x;
		out->bar[0][1] = y;
		out->bar[0][2] = config.rect.value[2];
		out->bar[0][3] = config.rect.value[3];
		CG_AdjustFrom640(&out->bar[0][0], &out->bar[0][1], &out->bar[0][2], &out->bar[0][3]);
		if (out->direction == SUPERHUD_DIR_LEFT_TO_RIGHT || out->direction == SUPERHUD_DIR_RIGHT_TO_LEFT)
		{
			out->max = out->bar[0][2]; // max / width
		}
		else
		{
			out->max = out->bar[0][3]; // max / height
		}
		out->koeff = out->max / max;
	}

	Vector4Copy(CG_SHUDConfigPickColor(&config.color.value), out->color_top);
	Vector4Copy(CG_SHUDConfigPickColor(&config.color.value), out->color_top_origin);
	if (config.bgcolor.isSet)
	{
		Vector4Copy(config.bgcolor.value, out->color_back);
	}
	else
	{
		Vector4Set(out->color_back, 0, 0, 0, 0);
	}
}

qboolean CG_SHUDIsTimeOut(const superhudConfig_t* cfg, int startTime)
{
	if (!startTime)
	{
		return qtrue;
	}
	if (cfg->time.isSet)
	{
		if ((cg.time - startTime) < cfg->time.value)
		{
			//fade time is not started;
			return qfalse;
		}
	}
	return qtrue;
}

 /*
 	*  Затухание
 	*  возвращает qfalse если элемент потух
 	*/
qboolean CG_SHUDGetFadeColor(const vec4_t from_color, vec4_t out, const superhudConfig_t* cfg, int startTime)
{
	int time = 0;

	Vector4Copy(from_color, out);

	if (!CG_SHUDIsTimeOut(cfg, startTime))
	{
		return qtrue;	
	}

	if (cfg->time.isSet)
	{
		time = cfg->time.value;
	}

	if (cfg->fade.isSet)
	{
		int fadetime;
		float fadedelay = SUPERHUD_DEFAULT_FADEDELAY;

		fadetime = cg.time - startTime - time;

		if (cfg->fadedelay.isSet)
		{
			fadedelay = (float)cfg->fadedelay.value;
		}

		if (fadetime > 0 && fadetime < fadedelay)
		{
			vec4_t tmpfade;
			float k = (float)fadetime/fadedelay;
			Vector4Copy(cfg->fade.value , tmpfade);
			Vector4Subtract(tmpfade, from_color, tmpfade);
			Vector4MA(from_color, k, tmpfade, out);
			return qtrue;
		}
		else
		{
			return qfalse;
		}
	}

	return qfalse;
}

void CG_SHUDTextPrint(const char* text, const superhudTextContext_t* ctx)
{
	CG_FontSelect(ctx->fontIndex);
	CG_OSPDrawString(ctx->textX,
	                 ctx->textY,
	                 text,
	                 ctx->color,
	                 ctx->fontW,
	                 ctx->fontH,
	                 ctx->maxchars,
	                 ctx->flags);
}


static void CG_SHUDBarPreparePrintLTR(const superhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	if (ctx->two_bars)
	{
		float width;

		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		Vector4Copy(ctx->bar[1], coords->bar2_value);
		Vector4Copy(ctx->bar[1], coords->bar2_bottom);
		//bar1
		width = value * ctx->koeff;
		if (width > ctx->max)
		{
			width = ctx->max;
		}
		if (width < 0)
		{
			width = 0;
		}
		coords->bar1_value[2] = width;

		//bar2
		width = value * ctx->koeff;
		width -= ctx->max;

		if (width > ctx->max)
		{
			width = ctx->max;
		}
		if (width < 0)
		{
			width = 0;
		}

		coords->bar2_value[2] = width;
	}
	else
	{
		float width;
		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		width = value * ctx->koeff;
		if (width > ctx->max)
		{
			width = ctx->max;
		}
		coords->bar1_value[2] = width;
	}
}

static void CG_SHUDBarPreparePrintRTL(const superhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	//Just make left-to-right and mirror it
	CG_SHUDBarPreparePrintLTR(ctx, value, coords);

	coords->bar1_value[0] = coords->bar1_value[0] + ctx->bar[0][2] - coords->bar1_value[2]; //x = x + max_width - width
	coords->bar2_value[0] = coords->bar2_value[0] + ctx->bar[1][2] - coords->bar2_value[2]; //x = x + max_width - width
}

static void CG_SHUDBarPreparePrintTTB(const superhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	if (ctx->two_bars)
	{
		float height;

		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		Vector4Copy(ctx->bar[1], coords->bar2_value);
		Vector4Copy(ctx->bar[1], coords->bar2_bottom);
		//bar1
		height = value * ctx->koeff;
		if (height > ctx->max)
		{
			height = ctx->max;
		}
		if (height < 0)
		{
			height = 0;
		}
		coords->bar1_value[3] = height;

		//bar2
		height = value * ctx->koeff;
		height -= ctx->max;

		if (height > ctx->max)
		{
			height = ctx->max;
		}
		if (height < 0)
		{
			height = 0;
		}

		coords->bar2_value[3] = height;
	}
	else
	{
		float height;
		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		height = value * ctx->koeff;
		if (height > ctx->max)
		{
			height = ctx->max;
		}
		if (height < 0)
		{
			height = 0;
		}
		coords->bar1_value[2] = height;
	}
}

static void CG_SHUDBarPreparePrintBTT(const superhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	//Just make top to bottom and mirror it
	CG_SHUDBarPreparePrintTTB(ctx, value, coords);

	coords->bar1_value[1] += ctx->bar[0][3] - coords->bar1_value[3]; //y = y + max_height - height
	coords->bar2_value[1] += ctx->bar[1][3] - coords->bar2_value[3]; //y = y + max_height - height
}

void CG_SHUDBarPrint(const superhudBarContext_t* ctx, float value)
{
	drawBarCoords_t coords;

	memset(&coords, 0, sizeof(coords));

	switch (ctx->direction)
	{
		default:
		case SUPERHUD_DIR_LEFT_TO_RIGHT:
			CG_SHUDBarPreparePrintLTR(ctx, value, &coords);
			break;
		case SUPERHUD_DIR_RIGHT_TO_LEFT:
			CG_SHUDBarPreparePrintRTL(ctx, value, &coords);
			break;
		case SUPERHUD_DIR_TOP_TO_BOTTOM:
			CG_SHUDBarPreparePrintTTB(ctx, value, &coords);
			break;
		case SUPERHUD_DIR_BOTTOM_TO_TOP:
			CG_SHUDBarPreparePrintBTT(ctx, value, &coords);
			break;
	}

	trap_R_SetColor(ctx->color_back);
	trap_R_DrawStretchPic(coords.bar1_bottom[0], coords.bar1_bottom[1], coords.bar1_bottom[2], coords.bar1_bottom[3],
	                      0, 0, 0, 0,
	                      cgs.media.whiteShader);
	if (ctx->two_bars)
	{
		trap_R_DrawStretchPic(coords.bar2_bottom[0], coords.bar2_bottom[1], coords.bar2_bottom[2], coords.bar2_bottom[3],
		                      0, 0, 0, 0,
		                      cgs.media.whiteShader);
	}
	trap_R_SetColor(ctx->color_top);
	trap_R_DrawStretchPic(coords.bar1_value[0], coords.bar1_value[1], coords.bar1_value[2], coords.bar1_value[3],
	                      0, 0, 0, 0,
	                      cgs.media.whiteShader);
	if (ctx->two_bars)
	{
		trap_R_DrawStretchPic(coords.bar2_value[0], coords.bar2_value[1], coords.bar2_value[2], coords.bar2_value[3],
		                      0, 0, 0, 0,
		                      cgs.media.whiteShader);
	}
	trap_R_SetColor(ctx->color_top_origin);
	trap_R_DrawStretchPic(coords.bar1_value[0], coords.bar1_value[1], coords.bar1_value[2], coords.bar1_value[3],
	                      0, 0, 0, 0,
	                      cgs.media.whiteShader);
	if (ctx->two_bars)
	{
		trap_R_DrawStretchPic(coords.bar2_value[0], coords.bar2_value[1], coords.bar2_value[2], coords.bar2_value[3],
		                      0, 0, 0, 0,
		                      cgs.media.whiteShader);
	}
	trap_R_SetColor(NULL);
}

team_t CG_SHUDGetOurActiveTeam(void)
{
	const qboolean is_freeze = CG_OSPIsGameTypeFreeze();

	team_t our_team = TEAM_FREE;
	if (is_freeze)
	{
		our_team = cgs.clientinfo[cg.clientNum].rt;
	}
	else
	{
		our_team = cgs.clientinfo[cg.clientNum].team;
	}

	if (our_team == TEAM_SPECTATOR)
	{
		if (is_freeze)
		{
			our_team = cgs.clientinfo[cg.snap->ps.clientNum].rt;
		}
		else
		{
			our_team = cgs.clientinfo[cg.snap->ps.clientNum].team;
		}
	}
	return our_team;
}

qboolean CG_SHUDFill(const superhudConfig_t* cfg)
{
	float x,y,w,h;
	if (!cfg->fill.isSet || !cfg->rect.isSet)
	{
		return qfalse;
	}
	
	x = cfg->rect.value[0];
	y = cfg->rect.value[1];
	w = cfg->rect.value[2];
	h = cfg->rect.value[3];

	CG_AdjustFrom640(&x, &y, &w, &h);
	if (cfg->bgcolor.isSet)
	{
		trap_R_SetColor(cfg->bgcolor.value);
		trap_R_DrawStretchPic(x, y, w, h, 0, 0, 0, 0, cgs.media.whiteShader);
		trap_R_SetColor(NULL);
		return qtrue;
	}
	if (cfg->image.isSet)
	{
		const float *color = colorWhite;
		qhandle_t image = trap_R_RegisterShader(cfg->image.value);
		if (!image)
		{
			return qfalse;
		}
		if (cfg->color.isSet)
		{
			color = CG_SHUDConfigPickColor(&cfg->color.value);
		}
		trap_R_SetColor(color);
		trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, image);
		trap_R_SetColor(NULL);
		return qtrue;
	}
	return qfalse;
}


qboolean shudElementCompileTeamOverlayConfig(const char *configString, int width, int maxLocation, shudTeamOverlay_t *configOut, const char **err_message)
{
	const char* teamOverlayStr;
	int total_len;
	char teamOverlayChar;
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
	memset(configOut, 0, sizeof(*configOut));

	if (strlen(configString) > 20)
	{
		*err_message = "Rejected ch_Teamoverlay setting, too long";
		return qfalse;
	}
	configOut->ammoLenChar = 1;
	configOut->powerupLenChar = 1;
	configOut->healthLenChar = 3;
	configOut->armorLenChar = 3;
	configOut->locationLenChar = maxLocation;

	for (total_len = 0, teamOverlayStr = configString; *teamOverlayStr ; ++teamOverlayStr)
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
				configOut->nameOffsetChar = total_len;
				configOut->nameLenChar = nameLen ? nameLen : 12;
				total_len += configOut->nameLenChar;
				configOut->isNameEnabled = qtrue;
				++numberOfNames;
				break;
			case 'p':
			case 'P':
				configOut->powerupOffsetChar = total_len;
				total_len +=  configOut->powerupLenChar;
				configOut->isPowerupEnabled = qtrue;
				++numberOfPowerups;
				break;
			case 'h':
			case 'H':
				configOut->healthOffsetChar = total_len;
				total_len += configOut->healthLenChar;
				configOut->isHealthEnabled = qtrue;
				++numberOfHealths;
				break;
			case 'a':
			case 'A':
				configOut->armorOffsetChar = total_len;
				total_len += configOut->armorLenChar;
				configOut->isArmorEnabled = qtrue;
				++numberOfArmors;
				break;
			case 'w':
			case 'W':
				configOut->ammoOffsetChar = total_len;
				total_len += configOut->ammoLenChar;
				configOut->isAmmoEnabled = qtrue;
				++numberOfAmmos;
				break;
			case 'l':
			case 'L':
				configOut->locationOffsetChar = total_len;
				total_len += configOut->locationLenChar;
				configOut->isLocationEnabled = qtrue;
				++numberOfLocations;
				break;
			case ' ':
				total_len += 1;
				break;
			default:
				string[total_len] = teamOverlayChar;
				total_len += 1;
				break;
		}
	}

	if (numberOfNames > 1 || numberOfPowerups > 1 || numberOfHealths > 1 || numberOfArmors > 1 || numberOfAmmos > 1 || numberOfLocations > 1)
	{
		*err_message = "Rejected ch_Teamoverlay setting, used an item more than once";
		return qfalse;
	}

	if (configOut->isNameEnabled == qfalse)
	{
		*err_message = "Rejected ch_Teamoverlay setting - useless";
		return qfalse;
	}

	string[total_len] = 0;

	/* remove leading and tailing spaces */
	{
		char* ptr = &string[0];
		char* start;

		while (*ptr && *ptr != ' ')
		{
			++ptr;
		}
		configOut->strX = (ptr - &string[0]);
		start = ptr;

		ptr += strlen(ptr) - 1;
		while (ptr != start && *ptr == ' ')
		{
			*ptr = 0;
			--ptr;
		}
		Q_strncpyz(configOut->str, start, OSPHUD_TEAMOVERLAY_STR_SIZE);
	}
	configOut->overlayWidthChar = total_len;
	teamOverlayWidth = total_len;

	configOut->powerupOffsetPix = configOut->powerupOffsetChar * width;
	//CG_AdjustFrom640(&configOut->powerupOffsetPix, NULL, NULL, NULL);
	configOut->powerupLenPix = configOut->powerupLenChar * width;
	CG_AdjustFrom640(&configOut->powerupLenPix, NULL, NULL, NULL);

	configOut->nameOffsetPix = configOut->nameOffsetChar * width;
	//CG_AdjustFrom640(&configOut->nameOffsetPix, NULL, NULL, NULL);
	configOut->nameLenPix = configOut->nameLenChar * width;
	CG_AdjustFrom640(&configOut->nameLenPix, NULL, NULL, NULL);

	configOut->healthOffsetPix = configOut->healthOffsetChar * width;
	//CG_AdjustFrom640(&configOut->healthOffsetPix, NULL, NULL, NULL);
	configOut->healthLenPix = configOut->healthLenChar * width;
	CG_AdjustFrom640(&configOut->healthLenPix, NULL, NULL, NULL);

	configOut->armorOffsetPix = configOut->armorOffsetChar * width;
	//CG_AdjustFrom640(&configOut->armorOffsetPix, NULL, NULL, NULL);
	configOut->armorLenPix = configOut->armorLenChar * width;
	CG_AdjustFrom640(&configOut->armorLenPix, NULL, NULL, NULL);

	configOut->ammoOffsetPix = configOut->ammoOffsetChar * width;
	//CG_AdjustFrom640(&configOut->ammoOffsetPix, NULL, NULL, NULL);
	configOut->ammoLenPix = configOut->ammoLenChar * width;
	CG_AdjustFrom640(&configOut->ammoLenPix, NULL, NULL, NULL);

	configOut->locationOffsetPix = configOut->locationOffsetChar * width;
	//CG_AdjustFrom640(&configOut->locationOffsetPix, NULL, NULL, NULL);
	configOut->locationLenPix = configOut->locationLenChar * width;
	CG_AdjustFrom640(&configOut->locationLenPix, NULL, NULL, NULL);

	configOut->overlayWidthPix = configOut->overlayWidthChar * width;
	CG_AdjustFrom640(&configOut->overlayWidthPix, NULL, NULL, NULL);

	return qtrue;
}

