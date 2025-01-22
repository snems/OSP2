#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"


typedef enum
{
	SHUD_ELEMENT_TEMPACC_CURRENT,
	SHUD_ELEMENT_TEMPACC_LAST,
}
shudElementTempAccType_t;

typedef struct
{
	float accuracy;
	float currentAccuracy;
	float lastAccuracy;
	int totalAmmoUsed;
	int lastTotalAmmoUsed;
	int totalHits;
	int lastTotalHits;
} shudElementTempAccCounters_t;

static shudElementTempAccCounters_t tempAccCounters;

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
	shudElementTempAccType_t type;
}
shudElementTempAcc_t;

static void* CG_SHUDElementTempAccCreate(const superhudConfig_t* config, shudElementTempAccType_t type)
{
	shudElementTempAcc_t* element;

	SHUD_ELEMENT_INIT(element, config);

	if (!element->config.color.isSet)
	{
		element->config.color.isSet = qtrue;
		element->config.color.value.type = SUPERHUD_COLOR_RGBA;
		Vector4Copy(config->color.value.rgba, element->config.color.value.rgba);
	}
	if (!element->config.text.isSet)
	{
		element->config.text.isSet = qtrue;
		Q_strncpyz(element->config.text.value, "%i", sizeof(element->config.text.value));
	}

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	element->type = type;

	return element;
}

void* CG_SHUDElementTempAccCurrentCreate(const superhudConfig_t* config)
{
	return CG_SHUDElementTempAccCreate(config, SHUD_ELEMENT_TEMPACC_CURRENT);
}

void* CG_SHUDElementTempAccLastCreate(const superhudConfig_t* config)
{
	return CG_SHUDElementTempAccCreate(config, SHUD_ELEMENT_TEMPACC_LAST);
}

void CG_SHUDElementTempAccRoutine(void* context)
{
	shudElementTempAcc_t* element = (shudElementTempAcc_t*)context;
	float accuracy;
	char accuracyStr[32];
	vec4_t color;

	superhudGlobalContext_t* ctx = CG_SHUDGetContext();

	CG_SHUDEventTempAccUpdateAndCalc(&tempAccCounters);

	switch (element->type)
	{
		case SHUD_ELEMENT_TEMPACC_CURRENT:
			accuracy = tempAccCounters.currentAccuracy;
			break;
		case SHUD_ELEMENT_TEMPACC_LAST:
			accuracy = tempAccCounters.lastAccuracy;
			break;
		default:
			return;
	}

	if (accuracy == 0)
	{
		element->ctx.text = "";
	}
	else
	{
		Com_sprintf(accuracyStr, sizeof(accuracyStr), "%.0f%%", accuracy);
		element->ctx.text = va("%s", accuracyStr);
	}

	if (element->config.style.isSet)
	{
		CG_SHUDEStylesTempAcc_Color(color, element->config.style.value, &element->config, accuracy);
	}
	else
	{
		Vector4Copy(element->config.color.value.rgba, color);
	}

	Vector4Copy(color, element->config.color.value.rgba);

	CG_SHUDFill(&element->config);
	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDEventTempAccUpdateAndCalc(shudElementTempAccCounters_t* counters)
{
	static float lastAccuracy = 0.0f;
	static int lastHitCount = 0;
	static int lastFireTime = 0;
	static int lastWeapon = WP_NONE;
	static int detectedDamage = 0;
	static int currentHits;
	static int lastAmmoCount = 0;
	int weapon = cg.snap->ps.weapon;
	int newHits;
	int currentAmmoCount;
	int ammoUsed;

	if ((weapon != WP_LIGHTNING && lastWeapon == WP_LIGHTNING) || (cg.time - lastFireTime > 1000) || (cg.snap->ps.stats[STAT_HEALTH] <= 0))
	{
		if (counters->totalAmmoUsed > 0 || counters->totalHits > 0)
		{
			lastAccuracy = counters->totalHits > 0 ? ((float)counters->totalHits / (float)counters->totalAmmoUsed) * 100.0f : 0.0f;

			counters->lastAccuracy = (lastAccuracy != 0.0f) ? lastAccuracy : counters->lastAccuracy;
			counters->lastTotalAmmoUsed = counters->totalAmmoUsed;
			counters->lastTotalHits = counters->totalHits;

			counters->totalAmmoUsed = 0;
			counters->totalHits = 0;
			counters->accuracy = 0.0f;
			counters->currentAccuracy = 0.0f;
		}
		lastHitCount = cg.snap->ps.persistant[PERS_HITS];
		detectedDamage = 0;

		if (lastWeapon != weapon)
		{
			lastAmmoCount = cg.snap->ps.ammo[weapon];
		}
	}

	lastWeapon = weapon;

	if (weapon != WP_LIGHTNING)
	{
		return;
	}

	if (cg.snap->ps.weaponstate == WEAPON_FIRING)
	{
		lastFireTime = cg.time;

		currentAmmoCount = cg.snap->ps.ammo[weapon];

		if (currentAmmoCount < lastAmmoCount || (lastAmmoCount == currentAmmoCount && counters->totalAmmoUsed == 0))
		{
			ammoUsed = lastAmmoCount - currentAmmoCount;
			counters->totalAmmoUsed += ammoUsed;
			lastAmmoCount = currentAmmoCount;

			currentHits = cg.snap->ps.persistant[PERS_HITS];
			if (currentHits > lastHitCount)
			{
				int delta = currentHits - lastHitCount;

				if (detectedDamage == 0)
				{
					detectedDamage = delta;
				}

				newHits = detectedDamage > 0 ? delta / detectedDamage : 1;

				counters->totalHits += newHits;
				lastHitCount = currentHits;
			}

			counters->accuracy = counters->totalHits > 0 ? ((float)counters->totalHits / (float)counters->totalAmmoUsed) * 100.0f : 0.0f;
			counters->currentAccuracy = counters->accuracy;
			if (lastAccuracy != 0.0f)
			{
				counters->lastAccuracy = lastAccuracy;
			}
		}
	}

}

static void CG_SHUDEStylesTempAcc_Color(vec4_t color, int style, const superhudConfig_t* config, float accuracy)
{
	if (style == 1)
	{
		const vec4_t colors[] =
		{
			{1.0f, 0.0f, 0.0f, 1.0f}, // < 30%
			{1.0f, 1.0f, 0.0f, 1.0f}, // < 50%
			{0.0f, 1.0f, 0.0f, 1.0f}, // < 60%
			{1.0f, 0.0f, 1.0f, 1.0f}  // >= 60%
		};

		int index = (accuracy < 30.0f) ? 0 :
		            (accuracy < 50.0f) ? 1 :
		            (accuracy < 60.0f) ? 2 : 3;

		Vector4Copy(colors[index], color);
	}
	else
	{
		Vector4Copy(config->color.value.rgba, color);
	}
}

void CG_SHUDElementTempAccDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
