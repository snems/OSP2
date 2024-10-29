
#include "cg_local.h"

#define CG_CROSSHAIR_DECOR_PULSE 1
#define CG_CROSSHAIR_DECOR_SHOW  2
#define CG_CROSSHAIR_DECOR_COLOR 4

static void CG_DrawCrosshairSpeed(void)
{
	static const vec4_t crosshairSpeedColor = { 1.0f, 1.0f, 1.0f, 0.4f };
	const char* speed_str = va("<%dups>", (int)cg.xyspeed);
	CG_OSPDrawStringOld((320 - 5 * strlen(speed_str)),
	                    225.0f - 0.5f * cg_crosshairSize.value,
	                    speed_str,
	                    10,
	                    10,
	                    crosshairSpeedColor,
	                    0,
	                    0);
}

static qhandle_t CG_CrosshairGetShader(void)
{
	qhandle_t shader;
	int crosshair;

	switch (cg.predictedPlayerState.weapon)
	{
		case WP_GAUNTLET:
			crosshair = cg_drawCrosshairGauntlet.integer;
			break;
		case WP_MACHINEGUN:
			crosshair = cg_drawCrosshairMachinegun.integer;
			break;
		case WP_SHOTGUN:
			crosshair = cg_drawCrosshairShotgun.integer;
			break;
		case WP_GRENADE_LAUNCHER:
			crosshair = cg_drawCrosshairGrenadeLauncher.integer;
			break;
		case WP_ROCKET_LAUNCHER:
			crosshair = cg_drawCrosshairRocketLauncher.integer;
			break;
		case WP_LIGHTNING:
			if (cg_lightningHideCrosshair.integer && cg.predictedPlayerState.eFlags & EF_FIRING) return 0;
			crosshair = cg_drawCrosshairLightning.integer;
			break;
		case WP_RAILGUN:
			crosshair = cg_drawCrosshairRailgun.integer;
			break;
		case WP_PLASMAGUN:
			crosshair = cg_drawCrosshairPlasmagun.integer;
			break;
		case WP_BFG:
			crosshair = cg_drawCrosshairBFG.integer;
			break;
		default:
			crosshair = cg_drawCrosshair.integer;
	}

	if (crosshair == -1)
	{
		crosshair = cg_drawCrosshair.integer;
	}

	if (!crosshair)
	{
		return 0;
	}

	shader = ch_crosshair45.integer ? cgs.media.crosshairShader45[crosshair % cgs.media.numberOfCrosshairs] : cgs.media.crosshairShader[crosshair % cgs.media.numberOfCrosshairs];

	return shader;
}

static float CG_GetCrosshairDistanceScale(void)
{
	float scale = 1.0f;
	if (ch_crosshairAutoScale.integer)
	{
		const float distance = 2000;
		const float startFrom = 200;
		const float scaleRange = 0.40f;

		if (cgs.osp.crosshair.distance > startFrom)
		{
			float mult = 1.0f - scaleRange;

			if (cgs.osp.crosshair.distance < startFrom + distance)
			{
				float d = (cgs.osp.crosshair.distance - startFrom);
				float s = scaleRange / distance;
				mult = 1.0f - (d * s);
			}

			scale = mult;
		}
	}

	return scale;
}

static qhandle_t CG_GetCrosshairDecorShader(void)
{
	qhandle_t shader;
	int decor = ch_crosshairDecor.integer;
	shader = ch_crosshairDecor45.integer ? cgs.media.crosshairDecorShader45[decor % cgs.media.numberOfCrosshairDecors] : cgs.media.crosshairDecorShader[decor % cgs.media.numberOfCrosshairDecors];

	return shader;
}

static float CG_CrosshairGetHitVisibilty(float visible_opaque, float period)
{
	int time = (float)(cg.time - cgs.osp.lastHitTime);
	if ((time > 0) && (time < period))
	{
		float result = visible_opaque;
		result -= (visible_opaque / period) * time;
		return result;
	}
	return 0.0f;
}

static float CG_CrosshairGetPulseScaler(float pulseTime, float scale, int eventTime)
{
	int time;
	float result = 1.0f;
	float tmp = 0.0f;
	time = (float)(cg.time - eventTime);
	if ((time > 0) && (time < pulseTime))
	{
		tmp = (float)time;
		tmp /= pulseTime;
		result += tmp * (scale - 1.0f);
	}
	return result;
}


static void CG_CrosshairGetSize(float* w, float* h)
{
	float size = CG_GetCrosshairDistanceScale() * cg_crosshairSize.value;
	*h = size;
	*w = size;
	if (cg_crosshairPulse.integer != 0)
	{
		float k;
		k = CG_CrosshairGetPulseScaler(200.0f, 2.0f, cg.itemPickupBlendTime);
		*w *= k;
		*h *= k;
	}
	else if (ch_crosshairAction.integer & CG_CROSSHAIR_DECOR_PULSE)
	{
		float k;
		k = CG_CrosshairGetPulseScaler(ch_crosshairActionTime.integer, ch_crosshairActionScale.value, cgs.osp.lastHitTime);
		*w *= k;
		*h *= k;
	}
}

static void CG_CrosshairDecorGetSize(float* w, float* h)
{
	float size = CG_GetCrosshairDistanceScale() * ch_crosshairDecorSize.value;
	*h = size;
	*w = size;

	if (ch_crosshairDecorAction.integer & CG_CROSSHAIR_DECOR_PULSE)
	{
		float k;
		k = CG_CrosshairGetPulseScaler(ch_crosshairDecorActionTime.integer, ch_crosshairDecorActionScale.value, cgs.osp.lastHitTime);
		*w *= k;
		*h *= k;
	}
}

static float CG_CrosshairGetOpaque(void)
{
	float value;

	value = 1.0f - ch_crosshairOpaque.value;
	if (ch_crosshairAction.integer & CG_CROSSHAIR_DECOR_SHOW)
	{
		value = CG_CrosshairGetHitVisibilty(value, ch_crosshairActionTime.value);
	}

	return value;
}

static float CG_CrosshairDecorGetOpaque(void)
{
	float value;

	value = 1.0f - ch_crosshairDecorOpaque.value;
	if (ch_crosshairDecorAction.integer & CG_CROSSHAIR_DECOR_SHOW)
	{
		value = CG_CrosshairGetHitVisibilty(value, ch_crosshairDecorActionTime.value);
	}

	return value;
}

static void CG_CrosshairGetHitColor(const vec4_t base_color, const vec4_t hit_color, float period, vec4_t result)
{
	int time = (float)(cg.time - cgs.osp.lastHitTime);
	if ((time > 0) && (time < period))
	{
		VectorCopy(hit_color, result);
	}
	else
	{
		VectorCopy(base_color, result);
	}
}

static void CG_CrosshairGetColor(vec4_t hcolor)
{
	if (cg_crosshairHealth.integer != 0)
	{
		CG_ColorForHealth(hcolor, NULL);
	}
	else if (ch_crosshairAction.integer & CG_CROSSHAIR_DECOR_COLOR)
	{
		CG_CrosshairGetHitColor(cgs.osp.crosshair.color, cgs.osp.crosshair.actionColor, ch_crosshairActionTime.integer, hcolor);
	}
	else
	{
		VectorCopy(cgs.osp.crosshair.color, hcolor);
	}

	hcolor[3] = CG_CrosshairGetOpaque();
}


static void CG_CrosshairDecorGetColor(vec4_t hcolor)
{
	if (ch_crosshairDecorAction.integer & CG_CROSSHAIR_DECOR_COLOR)
	{
		CG_CrosshairGetHitColor(cgs.osp.crosshair.decorColor, cgs.osp.crosshair.decorActionColor, ch_crosshairDecorActionTime.integer, hcolor);
	}
	else
	{
		VectorCopy(cgs.osp.crosshair.decorColor, hcolor);
	}

	hcolor[3] = CG_CrosshairDecorGetOpaque();

}

void CG_DrawCrosshair(void)
{
	float x, y, w, h;
	qhandle_t crosshairShader;
	vec4_t crosshairColor;
	qhandle_t decorShader;
	vec4_t decorColor;


	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		return;
	}

	if (cg.renderingThirdPerson)
	{
		return;
	}

	if (global_viewlistFirstOption > 1)
	{
		return;
	}

	if (ch_drawSpeed.integer == 2)
	{
		CG_DrawCrosshairSpeed();
	}


	//Draw decor
	if (ch_crosshairDecor.integer && (decorShader = CG_GetCrosshairDecorShader()))
	{
		CG_CrosshairDecorGetColor(decorColor);
		CG_CrosshairDecorGetSize(&w, &h);
		x = (float)cg_crosshairX.integer;
		y = (float)cg_crosshairY.integer;
		CG_AdjustFrom640_Old(&x, &y, &w, &h, cg_crosshairAspectRatioFix.integer != 0);


		trap_R_SetColor(decorColor);
		trap_R_DrawStretchPic(
		    x + (float)cg.refdef.x + (0.5f * ((float)cg.refdef.width - w)),
		    y + (float)cg.refdef.y + (0.5f * ((float)cg.refdef.height - h)),
		    w,
		    h,
		    0,
		    0,
		    1,
		    1,
		    decorShader);
	}


	//Draw crosshair
	if (cg_drawCrosshair.integer && (crosshairShader = CG_CrosshairGetShader()))
	{
		CG_CrosshairGetColor(crosshairColor);
		CG_CrosshairGetSize(&w, &h);

		x = (float)cg_crosshairX.integer;
		y = (float)cg_crosshairY.integer;
		CG_AdjustFrom640_Old(&x, &y, &w, &h, cg_crosshairAspectRatioFix.integer != 0);


		trap_R_SetColor(crosshairColor);
		trap_R_DrawStretchPic(
		    x + (float)cg.refdef.x + (0.5f * ((float)cg.refdef.width - w)),
		    y + (float)cg.refdef.y + (0.5f * ((float)cg.refdef.height - h)),
		    w,
		    h,
		    0,
		    0,
		    1,
		    1,
		    crosshairShader);
	}

	trap_R_SetColor(0);
}

