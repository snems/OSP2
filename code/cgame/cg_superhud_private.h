#ifndef CG_SUPERHUD_PRIVATE_H
#define CG_SUPERHUD_PRIVATE_H

#include "cg_local.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	SUPERHUD_ALIGNH_LEFT,
	SUPERHUD_ALIGNH_CENTER,
	SUPERHUD_ALIGNH_RIGHT,
} superhudAlignH_t;

typedef enum
{
	SUPERHUD_ALIGNV_TOP,
	SUPERHUD_ALIGNV_CENTER,
	SUPERHUD_ALIGNV_BOTTOM,
} superhudAlignV_t;

typedef enum
{
	SUPERHUD_COLOR_RGBA,
	SUPERHUD_COLOR_T,
	SUPERHUD_COLOR_E,
	SUPERHUD_COLOR_I,
} superhudColorType_t;

typedef struct
{
	superhudColorType_t type;
	vec4_t rgba;
} superhudColor_t;

typedef enum
{
	SUPERHUD_DIR_LEFT_TO_RIGHT,
	SUPERHUD_DIR_RIGHT_TO_LEFT,
	SUPERHUD_DIR_TOP_TO_BOTTOM,
	SUPERHUD_DIR_BOTTOM_TO_TOP,
} superhudDirection_t;

typedef enum
{
	SUPERHUD_ITTEAM_BLUE,
	SUPERHUD_ITTEAM_RED,
	SUPERHUD_ITTEAM_NEUTRAL,
	SUPERHUD_ITTEAM_OWN,
	SUPERHUD_ITTEAM_ENEMY,
} superhudItTeam_t;

typedef struct
{
	struct
	{
		superhudAlignH_t value;
		qboolean isSet;
	} alignH;
	struct
	{
		superhudAlignV_t value;
		qboolean isSet;
	} alignV;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} angles;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} bgcolor;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} hlcolor;
	struct
	{
		superhudColor_t value;
		qboolean isSet;
	} color;
	struct
	{
		superhudDirection_t value;
		qboolean isSet;
	} direction;
	struct
	{
		qboolean isSet;
	} doublebar;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} fade;
	struct
	{
		int value;
		qboolean isSet;
	} fadedelay;
	struct
	{
		qboolean isSet;
	} fill;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} font;
	struct
	{
		vec2_t value;
		qboolean isSet;
	} fontsize;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} image;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} imagetc;
	struct
	{
		superhudItTeam_t value;
		qboolean isSet;
	} itTeam;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} margins;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} model;
	struct
	{
		qboolean isSet;
	} monospace;
	struct
	{
		vec3_t value;
		qboolean isSet;
	} offset;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} rect;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} text;
	struct
	{
		superhudAlignH_t value;
		qboolean isSet;
	} textAlign;
	struct
	{
		vec2_t value;
		qboolean isSet;
	} textOffset;
	struct
	{
		int value;
		qboolean isSet;
	} textStyle;
	struct
	{
		int value;
		qboolean isSet;
	} time;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} visflags;
	struct
	{
		int value;
		qboolean isSet;
	} hlsize;
} superhudConfig_t;

typedef superhudConfig_t superhudElementDefault_t;

typedef enum
{
	SUPERHUD_ELEMENT_TYPE_DEFAULTS,
}
superhudElementType_t;

typedef struct
{
	union
	{
		superhudElementDefault_t def; //todo remove it, default have no content
	} value;
	superhudElementType_t type;
} superhudElementContent_t;

typedef struct configFileLine_s
{
	char* line;
	int size;
	int line_number;
	struct configFileLine_s* next;
} configFileLine_t;

typedef struct configFileInfo_s
{
	int pos; /* position in line */
	configFileLine_t* root;
	configFileLine_t* last_line;
} configFileInfo_t;

typedef enum
{
	SUPERHUD_CONFIG_OK,
	SUPERHUD_CONFIG_UNEXPECTED_CHARACTER,
	SUPERHUD_CONFIG_END_OF_FILE,
	SUPERHUD_CONFIG_END_OF_ELEMENT,
	SUPERHUD_CONFIG_WRONG_ELEMENT_NAME,
	SUPERHUD_CONFIG_WRONG_COMMAND_NAME,
	SUPERHUD_CONFIG_LOST_ELEMENT_BODY,
} superhudConfigParseStatus_t;

typedef struct superHUDConfigElement_s
{
	const char* name;
	const int order;
	void* (*create)(superhudConfig_t* config);
	void (*routine)(void* context);
	void (*destroy)(void* context);
	void* context;
	struct superHUDConfigElement_s* next;
} superHUDConfigElement_t;

typedef struct superhudElement_s
{
	superHUDConfigElement_t element;
	superhudConfig_t config;
	superhudElementContent_t content;
	struct superhudElement_s* next;
}
superhudElement_t;

typedef struct superHUDConfigCommand_s
{
	const char* name;
	superhudConfigParseStatus_t (*parse)(configFileInfo_t* finfo, superhudConfig_t* config);
	struct superHUDConfigCommand_s* next;
} superHUDConfigCommand_t;

typedef struct
{
	const superHUDConfigCommand_t* item;
	superhudConfigParseStatus_t status;
} superhudConfigParseCommand_t;

typedef struct
{
	const superHUDConfigElement_t* item;
	superhudConfigParseStatus_t status;
} superhudConfigParseElement_t;

void CG_SHUDParserInit(void);
const superHUDConfigElement_t* CG_SHUDFindConfigElementItem(const char* name);
const superHUDConfigCommand_t* CG_SHUDFindConfigCommandItem(const char* name);

qboolean CG_SHUDFileInfoInit(configFileInfo_t* info, const char* fileContent);
void CG_SHUDFileInfoTeardown(configFileInfo_t* cfi);

qboolean CG_SHUDFileInfoGoToChar(configFileInfo_t* cfi, char to, qboolean next);
void CG_SHUDFileInfoSkipSpaces(configFileInfo_t* cfi);
qboolean CG_SHUDFileInfoSkipCommandEnd(configFileInfo_t* cfi);
const superhudConfigParseElement_t CG_SHUDFileInfoGetElementItem(configFileInfo_t* cfi);
const superhudConfigParseCommand_t CG_SHUDFileInfoGetCommandItem(configFileInfo_t* cfi);

void* CG_SHUDElementFPSCreate(superhudConfig_t* config);
void CG_SHUDElementFPSRoutine(void* context);
void CG_SHUDElementFPSDestroy(void* context);

void* CG_SHUDElementSBHCCreate(superhudConfig_t* config);
void CG_SHUDElementSBHCRoutine(void* context);
void CG_SHUDElementSBHCDestroy(void* context);

void* CG_SHUDElementSBHBCreate(superhudConfig_t* config);
void CG_SHUDElementSBHBRoutine(void* context);
void CG_SHUDElementSBHBDestroy(void* context);

void* CG_SHUDElementSBHICreate(superhudConfig_t* config);
void CG_SHUDElementSBHIRoutine(void* context);
void CG_SHUDElementSBHIDestroy(void* context);

void* CG_SHUDElementSBACCreate(superhudConfig_t* config);
void CG_SHUDElementSBACRoutine(void* context);
void CG_SHUDElementSBACDestroy(void* context);

void* CG_SHUDElementSBABCreate(superhudConfig_t* config);
void CG_SHUDElementSBABRoutine(void* context);
void CG_SHUDElementSBABDestroy(void* context);

void* CG_SHUDElementSBAICreate(superhudConfig_t* config);
void CG_SHUDElementSBAIRoutine(void* context);
void CG_SHUDElementSBAIDestroy(void* context);

void* CG_SHUDElementSBAmBCreate(superhudConfig_t* config);
void CG_SHUDElementSBAmBRoutine(void* context);
void CG_SHUDElementSBAmBDestroy(void* context);

void* CG_SHUDElementSBAmCCreate(superhudConfig_t* config);
void CG_SHUDElementSBAmCRoutine(void* context);
void CG_SHUDElementSBAmCDestroy(void* context);

void* CG_SHUDElementSBAmICreate(superhudConfig_t* config);
void CG_SHUDElementSBAmIRoutine(void* context);
void CG_SHUDElementSBAmIDestroy(void* context);

void* CG_SHUDElementTargetNameCreate(superhudConfig_t* config);
void CG_SHUDElementTargetNameRoutine(void* context);
void CG_SHUDElementTargetNameDestroy(void* context);

void* CG_SHUDElementTargetStatusCreate(superhudConfig_t* config);
void CG_SHUDElementTargetStatusRoutine(void* context);
void CG_SHUDElementTargetStatusDestroy(void* context);

void* CG_SHUDElementVMWCreate(superhudConfig_t* config);
void CG_SHUDElementVMWRoutine(void* context);
void CG_SHUDElementVMWDestroy(void* context);

void* CG_SHUDElementAttackerNameCreate(superhudConfig_t* config);
void CG_SHUDElementAttackerNameRoutine(void* context);
void CG_SHUDElementAttackerNameDestroy(void* context);

void* CG_SHUDElementAttackerIconCreate(superhudConfig_t* config);
void CG_SHUDElementAttackerIconRoutine(void* context);
void CG_SHUDElementAttackerIconDestroy(void* context);

void* CG_SHUDElementFragMessageCreate(superhudConfig_t* config);
void CG_SHUDElementFragMessageRoutine(void* context);
void CG_SHUDElementFragMessageDestroy(void* context);

void* CG_SHUDElementRankMessageCreate(superhudConfig_t* config);
void CG_SHUDElementRankMessageRoutine(void* context);
void CG_SHUDElementRankMessageDestroy(void* context);

void* CG_SHUDElementNGPCreate(superhudConfig_t* config);
void CG_SHUDElementNGPRoutine(void* context);
void CG_SHUDElementNGPDestroy(void* context);

void* CG_SHUDElementNGCreate(superhudConfig_t* config);
void CG_SHUDElementNGRoutine(void* context);
void CG_SHUDElementNGDestroy(void* context);

void* CG_SHUDElementDecorCreate(superhudConfig_t* config);
void CG_SHUDElementDecorRoutine(void* context);
void CG_SHUDElementDecorDestroy(void* context);

void* CG_SHUDElementPlayerSpeedCreate(superhudConfig_t* config);
void CG_SHUDElementPlayerSpeedRoutine(void* context);
void CG_SHUDElementPlayerSpeedDestroy(void* context);

void* CG_SHUDElementLocalTimeCreate(superhudConfig_t* config);
void CG_SHUDElementLocalTimeRoutine(void* context);
void CG_SHUDElementLocalTimeDestroy(void* context);

void* CG_SHUDElementAmmoMessageCreate(superhudConfig_t* config);
void CG_SHUDElementAmmoMessageRoutine(void* context);
void CG_SHUDElementAmmoMessageDestroy(void* context);

void* CG_SHUDElementChat1Create(superhudConfig_t* config);
void* CG_SHUDElementChat2Create(superhudConfig_t* config);
void* CG_SHUDElementChat3Create(superhudConfig_t* config);
void* CG_SHUDElementChat4Create(superhudConfig_t* config);
void* CG_SHUDElementChat5Create(superhudConfig_t* config);
void* CG_SHUDElementChat6Create(superhudConfig_t* config);
void* CG_SHUDElementChat7Create(superhudConfig_t* config);
void* CG_SHUDElementChat8Create(superhudConfig_t* config);
void CG_SHUDElementChatRoutine(void* context);
void CG_SHUDElementChatDestroy(void* context);

void* CG_SHUDElementSpecMessageCreate(superhudConfig_t* config);
void CG_SHUDElementSpecMessageRoutine(void* context);
void CG_SHUDElementSpecMessageDestroy(void* context);

void* CG_SHUDElementFollowMessageCreate(superhudConfig_t* config);
void CG_SHUDElementFollowMessageRoutine(void* context);
void CG_SHUDElementFollowMessageDestroy(void* context);

void* CG_SHUDElementGameTimeCreate(superhudConfig_t* config);
void CG_SHUDElementGameTimeRoutine(void* context);
void CG_SHUDElementGameTimeDestroy(void* context);

void* CG_SHUDElementItemPickupCreate(superhudConfig_t* config);
void CG_SHUDElementItemPickupRoutine(void* context);
void CG_SHUDElementItemPickupDestroy(void* context);

void* CG_SHUDElementItemPickupIconCreate(superhudConfig_t* config);
void CG_SHUDElementItemPickupIconRoutine(void* context);
void CG_SHUDElementItemPickupIconDestroy(void* context);

void* CG_SHUDElementFlagStatusNMECreate(superhudConfig_t* config);
void* CG_SHUDElementFlagStatusOWNCreate(superhudConfig_t* config);
void CG_SHUDElementFlagStatusRoutine(void* context);
void CG_SHUDElementFlagStatusDestroy(void* context);

#define SUPERHUD_UPDATE_TIME 50

void* CG_SHUDElementPwTime1Create(superhudConfig_t* config);
void* CG_SHUDElementPwTime2Create(superhudConfig_t* config);
void* CG_SHUDElementPwTime3Create(superhudConfig_t* config);
void* CG_SHUDElementPwTime4Create(superhudConfig_t* config);
void* CG_SHUDElementPwTime5Create(superhudConfig_t* config);
void* CG_SHUDElementPwTime6Create(superhudConfig_t* config);
void* CG_SHUDElementPwTime7Create(superhudConfig_t* config);
void* CG_SHUDElementPwTime8Create(superhudConfig_t* config);
void* CG_SHUDElementPwIcon1Create(superhudConfig_t* config);
void* CG_SHUDElementPwIcon2Create(superhudConfig_t* config);
void* CG_SHUDElementPwIcon3Create(superhudConfig_t* config);
void* CG_SHUDElementPwIcon4Create(superhudConfig_t* config);
void* CG_SHUDElementPwIcon5Create(superhudConfig_t* config);
void* CG_SHUDElementPwIcon6Create(superhudConfig_t* config);
void* CG_SHUDElementPwIcon7Create(superhudConfig_t* config);
void* CG_SHUDElementPwIcon8Create(superhudConfig_t* config);
void CG_SHUDElementPwRoutine(void* context);
void CG_SHUDElementPwDestroy(void* context);

void* CG_SHUDElementNameNMECreate(superhudConfig_t* config);
void* CG_SHUDElementNameOWNCreate(superhudConfig_t* config);
void CG_SHUDElementNameRoutine(void* context);
void CG_SHUDElementNameDestroy(void* context);

void* CG_SHUDElementScoreNMECreate(superhudConfig_t* config);
void* CG_SHUDElementScoreOWNCreate(superhudConfig_t* config);
void* CG_SHUDElementScoreMAXCreate(superhudConfig_t* config);
void CG_SHUDElementScoreRoutine(void* context);
void CG_SHUDElementScoreDestroy(void* context);

void* CG_SHUDElementRewardIconCreate(superhudConfig_t* config);
void* CG_SHUDElementRewardCountCreate(superhudConfig_t* config);
void CG_SHUDElementRewardRoutine(void* context);
void CG_SHUDElementRewardDestroy(void* context);

/*
 * cg_superhud_util.c
 */
typedef struct
{
	float textX;
	float textY;
	float fontW;
	float fontH;
	int flags;
	vec4_t color;
	int maxchars;
	int fontIndex;
} superhudTextContext_t;

typedef struct
{
	float x;
	float y;
	float w;
	float h;
	qhandle_t image;
	vec4_t color;
} superhudDrawContext_t;

typedef struct
{
	superhudDirection_t direction;
	float max; // maximum coord for bar
	float koeff; //multiplier
	vec4_t bar[2]; // coord of two bars
	vec4_t color_top_origin; // color of bar
	vec4_t color_top; // color of bar
	vec4_t color_back; // color of background
	qboolean two_bars; // one or two bars
} superhudBarContext_t;

void CG_SHUDBarMakeContext(const superhudConfig_t* in, superhudBarContext_t* out, float max);
void CG_SHUDDrawMakeContext(const superhudConfig_t* in, superhudDrawContext_t* out);
void CG_SHUDTextMakeContext(const superhudConfig_t* in, superhudTextContext_t* out);
void CG_SHUDTextPrint(const char* text, const superhudTextContext_t* pos);
void CG_SHUDBarPrint(const superhudBarContext_t* ctx, float value);
team_t CG_SHUDGetOurActiveTeam(void);

typedef struct
{
	char message[MAX_SAY_TEXT];
	int time;
} superhudChatEntry_t;

#define SHUD_MAX_CHAT_LINES 8
#define SHUD_MAX_POWERUPS 8

typedef struct
{
	struct
	{
		int time;
		char message[256];
	} fragmessage;
	struct
	{
		int time;
		char message[256];
	} rankmessage;
	struct
	{
		superhudChatEntry_t line[SHUD_MAX_CHAT_LINES];
		unsigned int index;
	} chat;
	struct superhudPowerupsCache_t
	{
		struct superhudPowerupElement_t
		{
			int time;
			int powerup;
		} element[SHUD_MAX_POWERUPS];
		int numberOfActive;
		int lastUpdateTime;
	} powerupsCache;
} superhudGlobalContext_t;

superhudGlobalContext_t* CG_SHUDGetContext(void);

#ifdef __cplusplus
}
#endif

#endif
