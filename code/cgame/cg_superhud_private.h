#ifndef CG_SUPERHUD_PRIVATE_H
#define CG_SUPERHUD_PRIVATE_H

#include "cg_local.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SUPERHUD_DEFAULT_FADEDELAY 1000.0

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
	struct
	{
		int value;
		qboolean isSet;
	} visiblity;
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


#define SE_IM         0x00000001 // available in intermission view
#define SE_TEAM_ONLY  0x00000002 // team only
#define SE_SPECT      0x00000004 // available in spectator and not folowing view
#define SE_DEAD       0x00000008 // available if dead or freeze
#define SE_SCORES     0x00000010 // available if scores visible

typedef struct superHUDConfigElement_s
{
	const char* name;
	int visibility;
	void* (*create)(const superhudConfig_t* config);
	void (*routine)(void* context);
	void (*destroy)(void* context);
	void* context;
	int order;
} superHUDConfigElement_t;

typedef struct superhudElementDictMember_s
{
	const superHUDConfigElement_t* element;
	struct superhudElementDictMember_s* next;
}
superhudElementDictMember_t;

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

#define SHUD_ELEMENT_INIT(E, CFG)                     \
    do{                                                 \
        E = Z_Malloc(sizeof(*E));                         \
        OSP_MEMORY_CHECK(E);                              \
        memset(E, 0, sizeof(*E));                         \
        memcpy(&E->config, CFG, sizeof(element->config)); \
    }while(0)

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

void* CG_SHUDElementFPSCreate(const superhudConfig_t* config);
void CG_SHUDElementFPSRoutine(void* context);
void CG_SHUDElementFPSDestroy(void* context);

void* CG_SHUDElementSBHCCreate(const superhudConfig_t* config);
void CG_SHUDElementSBHCRoutine(void* context);
void CG_SHUDElementSBHCDestroy(void* context);

void* CG_SHUDElementSBHBCreate(const superhudConfig_t* config);
void CG_SHUDElementSBHBRoutine(void* context);
void CG_SHUDElementSBHBDestroy(void* context);

void* CG_SHUDElementSBHICreate(const superhudConfig_t* config);
void CG_SHUDElementSBHIRoutine(void* context);
void CG_SHUDElementSBHIDestroy(void* context);

void* CG_SHUDElementSBACCreate(const superhudConfig_t* config);
void CG_SHUDElementSBACRoutine(void* context);
void CG_SHUDElementSBACDestroy(void* context);

void* CG_SHUDElementSBABCreate(const superhudConfig_t* config);
void CG_SHUDElementSBABRoutine(void* context);
void CG_SHUDElementSBABDestroy(void* context);

void* CG_SHUDElementSBAICreate(const superhudConfig_t* config);
void CG_SHUDElementSBAIRoutine(void* context);
void CG_SHUDElementSBAIDestroy(void* context);

void* CG_SHUDElementSBAmBCreate(const superhudConfig_t* config);
void CG_SHUDElementSBAmBRoutine(void* context);
void CG_SHUDElementSBAmBDestroy(void* context);

void* CG_SHUDElementSBAmCCreate(const superhudConfig_t* config);
void CG_SHUDElementSBAmCRoutine(void* context);
void CG_SHUDElementSBAmCDestroy(void* context);

void* CG_SHUDElementSBAmICreate(const superhudConfig_t* config);
void CG_SHUDElementSBAmIRoutine(void* context);
void CG_SHUDElementSBAmIDestroy(void* context);

void* CG_SHUDElementTargetNameCreate(const superhudConfig_t* config);
void CG_SHUDElementTargetNameRoutine(void* context);
void CG_SHUDElementTargetNameDestroy(void* context);

void* CG_SHUDElementTargetStatusCreate(const superhudConfig_t* config);
void CG_SHUDElementTargetStatusRoutine(void* context);
void CG_SHUDElementTargetStatusDestroy(void* context);

void* CG_SHUDElementVMWCreate(const superhudConfig_t* config);
void CG_SHUDElementVMWRoutine(void* context);
void CG_SHUDElementVMWDestroy(void* context);

void* CG_SHUDElementAttackerNameCreate(const superhudConfig_t* config);
void CG_SHUDElementAttackerNameRoutine(void* context);
void CG_SHUDElementAttackerNameDestroy(void* context);

void* CG_SHUDElementAttackerIconCreate(const superhudConfig_t* config);
void CG_SHUDElementAttackerIconRoutine(void* context);
void CG_SHUDElementAttackerIconDestroy(void* context);

void* CG_SHUDElementFragMessageCreate(const superhudConfig_t* config);
void CG_SHUDElementFragMessageRoutine(void* context);
void CG_SHUDElementFragMessageDestroy(void* context);

void* CG_SHUDElementRankMessageCreate(const superhudConfig_t* config);
void CG_SHUDElementRankMessageRoutine(void* context);
void CG_SHUDElementRankMessageDestroy(void* context);

void* CG_SHUDElementNGPCreate(const superhudConfig_t* config);
void CG_SHUDElementNGPRoutine(void* context);
void CG_SHUDElementNGPDestroy(void* context);

void* CG_SHUDElementNGCreate(const superhudConfig_t* config);
void CG_SHUDElementNGRoutine(void* context);
void CG_SHUDElementNGDestroy(void* context);

void* CG_SHUDElementDecorCreate(const superhudConfig_t* config);
void CG_SHUDElementDecorRoutine(void* context);
void CG_SHUDElementDecorDestroy(void* context);

void* CG_SHUDElementPlayerSpeedCreate(const superhudConfig_t* config);
void CG_SHUDElementPlayerSpeedRoutine(void* context);
void CG_SHUDElementPlayerSpeedDestroy(void* context);

void* CG_SHUDElementLocalTimeCreate(const superhudConfig_t* config);
void CG_SHUDElementLocalTimeRoutine(void* context);
void CG_SHUDElementLocalTimeDestroy(void* context);

void* CG_SHUDElementAmmoMessageCreate(const superhudConfig_t* config);
void CG_SHUDElementAmmoMessageRoutine(void* context);
void CG_SHUDElementAmmoMessageDestroy(void* context);

void* CG_SHUDElementChat1Create(const superhudConfig_t* config);
void* CG_SHUDElementChat2Create(const superhudConfig_t* config);
void* CG_SHUDElementChat3Create(const superhudConfig_t* config);
void* CG_SHUDElementChat4Create(const superhudConfig_t* config);
void* CG_SHUDElementChat5Create(const superhudConfig_t* config);
void* CG_SHUDElementChat6Create(const superhudConfig_t* config);
void* CG_SHUDElementChat7Create(const superhudConfig_t* config);
void* CG_SHUDElementChat8Create(const superhudConfig_t* config);
void* CG_SHUDElementChat9Create(const superhudConfig_t* config);
void* CG_SHUDElementChat10Create(const superhudConfig_t* config);
void* CG_SHUDElementChat11Create(const superhudConfig_t* config);
void* CG_SHUDElementChat12Create(const superhudConfig_t* config);
void* CG_SHUDElementChat13Create(const superhudConfig_t* config);
void* CG_SHUDElementChat14Create(const superhudConfig_t* config);
void* CG_SHUDElementChat15Create(const superhudConfig_t* config);
void* CG_SHUDElementChat16Create(const superhudConfig_t* config);
void CG_SHUDElementChatRoutine(void* context);
void CG_SHUDElementChatDestroy(void* context);

void* CG_SHUDElementSpecMessageCreate(const superhudConfig_t* config);
void CG_SHUDElementSpecMessageRoutine(void* context);
void CG_SHUDElementSpecMessageDestroy(void* context);

void* CG_SHUDElementFollowMessageCreate(const superhudConfig_t* config);
void CG_SHUDElementFollowMessageRoutine(void* context);
void CG_SHUDElementFollowMessageDestroy(void* context);

void* CG_SHUDElementGameTimeCreate(const superhudConfig_t* config);
void CG_SHUDElementGameTimeRoutine(void* context);
void CG_SHUDElementGameTimeDestroy(void* context);

void* CG_SHUDElementItemPickupCreate(const superhudConfig_t* config);
void CG_SHUDElementItemPickupRoutine(void* context);
void CG_SHUDElementItemPickupDestroy(void* context);

void* CG_SHUDElementItemPickupIconCreate(const superhudConfig_t* config);
void CG_SHUDElementItemPickupIconRoutine(void* context);
void CG_SHUDElementItemPickupIconDestroy(void* context);

void* CG_SHUDElementFlagStatusNMECreate(const superhudConfig_t* config);
void* CG_SHUDElementFlagStatusOWNCreate(const superhudConfig_t* config);
void CG_SHUDElementFlagStatusRoutine(void* context);
void CG_SHUDElementFlagStatusDestroy(void* context);

#define SUPERHUD_UPDATE_TIME 50

void* CG_SHUDElementPwTime1Create(const superhudConfig_t* config);
void* CG_SHUDElementPwTime2Create(const superhudConfig_t* config);
void* CG_SHUDElementPwTime3Create(const superhudConfig_t* config);
void* CG_SHUDElementPwTime4Create(const superhudConfig_t* config);
void* CG_SHUDElementPwTime5Create(const superhudConfig_t* config);
void* CG_SHUDElementPwTime6Create(const superhudConfig_t* config);
void* CG_SHUDElementPwTime7Create(const superhudConfig_t* config);
void* CG_SHUDElementPwTime8Create(const superhudConfig_t* config);
void* CG_SHUDElementPwIcon1Create(const superhudConfig_t* config);
void* CG_SHUDElementPwIcon2Create(const superhudConfig_t* config);
void* CG_SHUDElementPwIcon3Create(const superhudConfig_t* config);
void* CG_SHUDElementPwIcon4Create(const superhudConfig_t* config);
void* CG_SHUDElementPwIcon5Create(const superhudConfig_t* config);
void* CG_SHUDElementPwIcon6Create(const superhudConfig_t* config);
void* CG_SHUDElementPwIcon7Create(const superhudConfig_t* config);
void* CG_SHUDElementPwIcon8Create(const superhudConfig_t* config);
void CG_SHUDElementPwRoutine(void* context);
void CG_SHUDElementPwDestroy(void* context);

void* CG_SHUDElementNameNMECreate(const superhudConfig_t* config);
void* CG_SHUDElementNameOWNCreate(const superhudConfig_t* config);
void CG_SHUDElementNameRoutine(void* context);
void CG_SHUDElementNameDestroy(void* context);

void* CG_SHUDElementScoreNMECreate(const superhudConfig_t* config);
void* CG_SHUDElementScoreOWNCreate(const superhudConfig_t* config);
void* CG_SHUDElementScoreMAXCreate(const superhudConfig_t* config);
void CG_SHUDElementScoreRoutine(void* context);
void CG_SHUDElementScoreDestroy(void* context);

void* CG_SHUDElementRewardIconCreate(const superhudConfig_t* config);
void* CG_SHUDElementRewardCountCreate(const superhudConfig_t* config);
void CG_SHUDElementRewardRoutine(void* context);
void CG_SHUDElementRewardDestroy(void* context);

void* CG_SHUDElementTeamCountOWNCreate(const superhudConfig_t* config);
void* CG_SHUDElementTeamCountNMECreate(const superhudConfig_t* config);
void CG_SHUDElementTeamCountRoutine(void* context);
void CG_SHUDElementTeamCountDestroy(void* context);

void* CG_SHUDElementTeam1Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam2Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam3Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam4Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam5Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam6Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam7Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam8Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam9Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam10Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam11Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam12Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam13Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam14Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam15Create(const superhudConfig_t* config);
void* CG_SHUDElementTeam16Create(const superhudConfig_t* config);
void CG_SHUDElementTeamRoutine(void* context);
void CG_SHUDElementTeamDestroy(void* context);

void* CG_SHUDElementWeaponListCreate(const superhudConfig_t* config);
void CG_SHUDElementWeaponListRoutine(void* context);
void CG_SHUDElementWeaponListDestroy(void* context);


/*
 * cg_superhud_util.c
 */
typedef union
{
	struct
	{
		float x;
		float y;
		float w;
		float h;
	} named;
	vec4_t arr;
} superhudCoord_t;

typedef struct
{
	superhudCoord_t coord;
	int flags;
	vec4_t color_origin;
	vec4_t color;
	int maxchars;
	int fontIndex;
	const char* text;
} superhudTextContext_t;

typedef struct
{
	superhudCoord_t coord;
	superhudCoord_t coordPicture;
	qhandle_t image;
	vec4_t color;
	vec4_t color_origin;
} superhudDrawContext_t;

typedef struct
{
	superhudDirection_t direction;
	float max; // maximum coord for bar
	float koeff; //multiplier
	vec4_t bar[2]; // coord of two bars
	vec4_t color_top; // color of bar
	vec4_t color_back; // color of background
	qboolean two_bars; // one or two bars
} superhudBarContext_t;

#define OSPHUD_TEAMOVERLAY_STR_SIZE 128
typedef struct
{
	int powerupOffsetChar;
	int powerupLenChar;
	float powerupOffsetPix;
	float powerupLenPix;

	int nameOffsetChar;
	int nameLenChar;
	float nameOffsetPix;
	float nameLenPix;

	int healthAndArmorOffsetChar;
	int healthAndArmorLenChar;
	float healthAndArmorOffsetPix;
	float healthAndArmorLenPix;

	int weaponOffsetChar;
	int weaponLenChar;
	float weaponOffsetPix;
	float weaponLenPix;

	int locationOffsetChar;
	int locationLenChar;
	float locationOffsetPix;
	float locationLenPix;

	int overlayWidthChar;
	float overlayWidthPix;
} shudTeamOverlay_t;

void CG_SHUDBarMakeContext(const superhudConfig_t* in, superhudBarContext_t* out, float max);
void CG_SHUDTextMakeContext(const superhudConfig_t* in, superhudTextContext_t* out);
void CG_SHUDDrawStretchPic(superhudCoord_t coord, const superhudCoord_t coordPicture, const float* color, qhandle_t shader);
void CG_SHUDDrawMakeContext(const superhudConfig_t* cfg, superhudDrawContext_t* out);

void CG_SHUDTextPrint(const superhudConfig_t* cfg, superhudTextContext_t* pos);
void CG_SHUDDrawStretchPicCtx(const superhudConfig_t* cfg, superhudDrawContext_t* out);
void CG_SHUDBarPrint(const superhudConfig_t* cfg, superhudBarContext_t* ctx, float value);
qboolean CG_SHUDFill(const superhudConfig_t* cfg);

team_t CG_SHUDGetOurActiveTeam(void);
qboolean CG_SHUDGetFadeColor(const vec4_t from_color, vec4_t out, const superhudConfig_t* cfg, int startTime);
void CG_SHUDFillWithColor(const superhudCoord_t* coord, const float* color);
void CG_SHUDElementCompileTeamOverlayConfig(int fontWidth, shudTeamOverlay_t* configOut);

typedef struct
{
	char message[MAX_SAY_TEXT];
	int time;
} superhudChatEntry_t;

#define SHUD_MAX_CHAT_LINES 16
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
void CG_SHUDAvailableElementsInit(void);
const superHUDConfigElement_t* CG_SHUDAvailableElementsGet(void);


#ifdef __cplusplus
}
#endif

#endif
