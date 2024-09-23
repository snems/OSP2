
#include "cg_local.h"

#define CG_CROSSHAIR_DECOR_PULSE 1
#define CG_CROSSHAIR_DECOR_SHOW  2
#define CG_CROSSHAIR_DECOR_COLOR 4

static void CG_DrawCrosshairSpeed (void)
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

static qhandle_t CG_GetCrosshairShader(void)
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

	shader = cg_crosshair45.integer ? cgs.media.crosshairShader45[crosshair % cgs.media.numberOfCrosshairs] : cgs.media.crosshairShader[crosshair % cgs.media.numberOfCrosshairs];

  return shader;
}

static qhandle_t CG_GetCrosshairDecorShader(void)
{
  qhandle_t shader;
  int decor = cg_crosshairDecor.integer;
	shader = cg_crosshairDecor45.integer ? cgs.media.crosshairDecorShader45[decor % cgs.media.numberOfCrosshairDecors] : cgs.media.crosshairDecorShader[decor % cgs.media.numberOfCrosshairDecors];

  return shader;
}

static void CG_GetCrosshairSize(float *w, float *h)
{
	*h = cg_crosshairSize.value;
	*w = cg_crosshairSize.value;
	if (cg_crosshairPulse.integer != 0)
	{
		int time;
		time = (float)(cg.time - cg.itemPickupBlendTime);
		if ((time > 0) && (time < 200.0f))
		{
			float tmp;
			tmp = (float)time;
			tmp /= 200.0f;
			tmp += 1.0f;
			*w *= tmp;
			*h *= tmp;
		}
	}
}

static void CG_GetCrosshairDecorSize(float *w, float *h)
{
  CG_GetCrosshairSize(w, h);
}

static float CG_GetCrosshairOpaque(void)
{
  float value;

  value = Com_Clamp(0, 1, cg_crosshairOpaque.value);

  return value;
}

static float CG_GetCrosshairDecorOpaque(void)
{
  float value;

  value = Com_Clamp(0, 1, cg_crosshairDecorOpaque.value);

  return value;
}

static void CG_GetCrosshairColor(vec4_t hcolor)
{
	if (cg_crosshairHealth.integer != 0)
	{
		CG_ColorForHealth(hcolor, NULL);
	}
	else
	{
	  //todo это надо заменить на новый декодер цвета
		if (ch_CrosshairColor.integer > 0)
		{
			hcolor[0] = ch_CrosshairColor.integer & 1 ? 1.0f : 0;
			hcolor[1] = ch_CrosshairColor.integer & 2 ? 1.0f : 0;
			hcolor[2] = ch_CrosshairColor.integer & 4 ? 1.0f : 0;
			hcolor[3] = 1.0f;
		}
		else 
		{
		  VectorCopy(colorRed, hcolor);
		}
	}
	hcolor[3] = CG_GetCrosshairOpaque();
}

static float CG_CrosshairGetHitPulse(float visible_opaque, float period)
{
	int time = (float)(cg.time - cgs.osp.lastHitTime);
	Com_Clamp(200, 1000, period);
	if ((time > 0) && (time < period))
	{
    float result = visible_opaque;
	  result -= (visible_opaque/period) * time;
	  return result;
	}
	return 0.0f;
}

static void CG_GetCrosshairDecorColor(vec4_t hcolor)
{
  CG_GetCrosshairColor(hcolor);
  hcolor[3] = CG_GetCrosshairDecorOpaque();

  if (cg_crosshairDecorAction.integer & CG_CROSSHAIR_DECOR_SHOW)
  {
    hcolor[3] = CG_CrosshairGetHitPulse(hcolor[3], cg_crosshairDecorActionTime.value);
  }
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
	if (cg_crosshairDecor.integer && (decorShader = CG_GetCrosshairDecorShader()))
	{
	  CG_GetCrosshairDecorColor(decorColor);
	  CG_GetCrosshairDecorSize(&w, &h);
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
	if (cg_drawCrosshair.integer && (crosshairShader = CG_GetCrosshairShader()))
	{
	  CG_GetCrosshairColor(crosshairColor);
	  CG_GetCrosshairSize(&w, &h);

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

