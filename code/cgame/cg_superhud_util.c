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
			return colorWhite;
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
			return colorWhite;
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

void CG_SHUDTextMakeContext(const superhudConfig_t* in, superhudTextContext_t* out)
{
	superhudConfig_t config;
	memset(out, 0, sizeof(*out));
	memcpy(&config, in, sizeof(config));

	CG_SHUDConfigDefaultsCheck(&config);

	switch (config.textAlign.value)
	{
		default:
		case SUPERHUD_ALIGNH_LEFT:
			out->textX = config.rect.value[0];
			out->textY = config.rect.value[1];
			out->flags |= DS_LEFT;
			break;
		case SUPERHUD_ALIGNH_CENTER:
			out->textX = config.rect.value[0] + config.rect.value[2] / 2.0f;
			out->textY = config.rect.value[1];
			out->flags |= DS_CENTER;
			break;
		case SUPERHUD_ALIGNH_RIGHT:
			out->textX = config.rect.value[0] + config.rect.value[2];
			out->textY = config.rect.value[1];
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
}

void CG_SHUDBarMakeContext(const superhudConfig_t* in, superhudBarContext_t* out, float max)
{
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

			out->bar[0][0] = config.rect.value[0];//x
			out->bar[0][1] = config.rect.value[1];//y
			out->bar[0][2] = bar_width;//w
			out->bar[0][3] = bar_height;   // height is half of rect and minus half of the gap between two bars
			//
			out->bar[1][0] = config.rect.value[0];//x
			out->bar[1][1] = config.rect.value[1] + bar_height + bar_gap;//y starts after first bar and gap
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

			out->bar[0][0] = config.rect.value[0];//x
			out->bar[0][1] = config.rect.value[1];//y
			out->bar[0][2] = bar_width;//w
			out->bar[0][3] = bar_height;   //h
			//
			out->bar[1][0] = config.rect.value[0] + bar_width + bar_gap;//x
			out->bar[1][1] = config.rect.value[1];//y
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
		out->bar[0][0] = config.rect.value[0];
		out->bar[0][1] = config.rect.value[1];
		out->bar[0][2] = config.rect.value[2];
		out->bar[0][3] = config.rect.value[3];
		CG_AdjustFrom640(&out->bar[0][0], &out->bar[0][1], &out->bar[0][2], &out->bar[0][3]);
		if (out->direction == SUPERHUD_DIR_LEFT_TO_RIGHT || out->direction == SUPERHUD_DIR_RIGHT_TO_LEFT)
		{
			out->max = config.rect.value[2]; // max / width
		}
		else
		{
			out->max = config.rect.value[3]; // max / height
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
	team_t our_team = TEAM_FREE;
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
	return our_team;
}



