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
#ifndef CG_LOCAL_H
#define CG_LOCAL_H
#include "../qcommon/q_shared.h"
#include "tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NYAN_POINT()      CG_Printf( "%s:%d: DEBUG point reached\n", __FILE__, __LINE__)
#define NYAN_INT(VALUE)   CG_Printf( "%s:%d: %s = %i\n", __FILE__, __LINE__, #VALUE, (int)VALUE)
#define NYAN_FLOAT(VALUE) CG_Printf( "%s:%d: %s = %f\n", __FILE__, __LINE__, #VALUE, (float)VALUE)
#define NYAN_VEC4(VALUE) CG_Printf( "%s:%d: %s = %f,%f,%f,%f\n", __FILE__, __LINE__, #VALUE, VALUE[0], VALUE[1], VALUE[2], VALUE[3])
#define NYAN_MSG(VALUE)   CG_Printf( "%s:%d: %s\n", __FILE__, __LINE__, VALUE)

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#define POWERUP_BLINKS      5

#define POWERUP_BLINK_TIME  1000
#define FADE_TIME           200
#define PULSE_TIME          200
#define DAMAGE_DEFLECT_TIME 100
#define DAMAGE_RETURN_TIME  400
#define DAMAGE_TIME         500
#define LAND_DEFLECT_TIME   150
#define LAND_RETURN_TIME    300
#define STEP_TIME           200
#define DUCK_TIME           100
#define PAIN_TWITCH_TIME    200
#define WEAPON_SELECT_TIME  1400
#define ITEM_SCALEUP_TIME   1000
#define ITEM_BLOB_TIME      200
#define MUZZLE_FLASH_TIME   20
#define SINK_TIME           1000        // time for fragments to sink into ground before going away
#define ATTACKER_HEAD_TIME  10000
#define REWARD_TIME         3000

#define PULSE_SCALE         1.5         // amount to scale up the icons when activating

#define MAX_STEP_CHANGE     32

#define MAX_VERTS_ON_POLY   10
#define MAX_MARK_POLYS      256

#define STAT_MINUS          10  // num frame for '-' stats digit

#define ICON_SIZE           48
#define Q3_CHAR_WIDTH       32
#define Q3_CHAR_HEIGHT      48
#define TEXT_ICON_SPACE     4

#define TEAMCHAT_WIDTH      80
#define TEAMCHAT_HEIGHT     8

// very large characters
#define GIANT_WIDTH         32
#define GIANT_HEIGHT        48

#define NUM_CROSSHAIRS      80
#define NUM_DECORS          33

#define TEAM_OVERLAY_MAXNAME_WIDTH  12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH  16

#define DEFAULT_MODEL           "sarge"

#define DEFAULT_TEAM_MODEL      "sarge"
#define DEFAULT_TEAM_HEAD       "sarge"

#define DEFAULT_REDTEAM_NAME        "Stroggs"
#define DEFAULT_BLUETEAM_NAME       "Pagans"

typedef enum
{
	FOOTSTEP_NORMAL,
	FOOTSTEP_BOOT,
	FOOTSTEP_FLESH,
	FOOTSTEP_MECH,
	FOOTSTEP_ENERGY,
	FOOTSTEP_METAL,
	FOOTSTEP_SPLASH,

	FOOTSTEP_TOTAL
} footstep_t;

typedef enum
{
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
} impactSound_t;

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct
{
	int         oldFrame;
	int         oldFrameTime;       // time when ->oldFrame was exactly on

	int         frame;
	int         frameTime;          // time when ->frame will be exactly on

	float       backlerp;

	float       yawAngle;
	qboolean    yawing;
	float       pitchAngle;
	qboolean    pitching;

	int         animationNumber;    // may include ANIM_TOGGLEBIT
	animation_t* animation;
	int         animationTime;      // time when the first frame of the animation will be exact
} lerpFrame_t;


typedef struct
{
	lerpFrame_t     legs, torso, flag;
	int             painTime;
	int             painDirection;  // flip from 0 to 1
	int             lightningFiring;

	// railgun trail spawning
	vec3_t          railgunImpact;
	qboolean        railgunFlash;

	// machinegun spinning
	float           barrelAngle;
	int             barrelTime;
	qboolean        barrelSpinning;
} playerEntity_t;

//=================================================



// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s
{
	entityState_t   currentState;   // from cg.frame
	entityState_t   nextState;      // from cg.nextFrame, if available
	qboolean        interpolate;    // true if next is valid to interpolate to
	qboolean        currentValid;   // true if cg.frame holds this entity

	int             muzzleFlashTime;    // move to playerEntity?
	int             previousEvent;
	int             teleportFlag;

	int             trailTime;      // so missile trails can handle dropped initial packets
	int             dustTrailTime;
	int             miscTime;

	int             snapShotTime;   // last time this entity was found in a snapshot

	playerEntity_t  pe;

	int             errorTime;      // decay the error from this time
	vec3_t          errorOrigin;
	vec3_t          errorAngles;

	qboolean        extrapolated;   // false if origin / angles is an interpolation
	vec3_t          rawOrigin;
	vec3_t          rawAngles;

	vec3_t          beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t          lerpOrigin;
	vec3_t          lerpAngles;
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef struct markPoly_s
{
	struct markPoly_s*   prevMark, *nextMark;
	int         time;
	qhandle_t   markShader;
	qboolean    alphaFade;      // fade alpha instead of rgb
	float       color[4];
	poly_t      poly;
	polyVert_t  verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum
{
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_SCOREPLUM,
} leType_t;

typedef enum
{
	LEF_PUFF_DONT_SCALE  = 0x0001,          // do not scale size over time
	LEF_TUMBLE           = 0x0002,          // tumble over time, used for ejecting shells
	LEF_SOUND1           = 0x0004,          // sound 1 for kamikaze
	LEF_SOUND2           = 0x0008           // sound 2 for kamikaze
} leFlag_t;

typedef enum
{
	LEMT_NONE,
	LEMT_BURN,
	LEMT_BLOOD
} leMarkType_t;         // fragment local entities can leave marks on walls

typedef enum
{
	LEBS_NONE,
	LEBS_BLOOD,
	LEBS_BRASS
} leBounceSoundType_t;  // fragment local entities can make sounds on impacts

typedef struct localEntity_s
{
	struct localEntity_s*    prev, *next;
	leType_t        leType;
	int             leFlags;

	int             startTime;
	int             endTime;
	int             fadeInTime;

	float           lifeRate;           // 1.0 / (endTime - startTime)

	trajectory_t    pos;
	trajectory_t    angles;

	float           bounceFactor;       // 0.0 = no bounce, 1.0 = perfect

	float           color[4];

	float           radius;

	float           light;
	vec3_t          lightColor;

	leMarkType_t        leMarkType;     // mark to leave on fragment impact
	leBounceSoundType_t leBounceSoundType;

	refEntity_t     refEntity;
} localEntity_t;

//======================================================================


typedef struct
{
	int             client;
	int             score;
	int             ping;
	int             time;
	int             scoreFlags;
	int             powerUps;
	int             accuracy;
	int             impressiveCount;
	int             excellentCount;
	int             guantletCount;
	int             defendCount;
	int             assistCount;
	int             captures;
	qboolean    perfect;
	int             team;
} score_t;

typedef struct playerColors_s
{

	vec3_t head;
	vec3_t torso;
	vec3_t legs;

	vec3_t railCore;
	vec3_t railRings;
	vec3_t frozen;

} playerColors_t;

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define MAX_CUSTOM_SOUNDS   32

typedef struct
{
	qboolean        infoValid;

	char            name_original[MAX_QPATH];
	char            name_clean[MAX_QPATH];
	char            name_codes[MAX_QPATH * 2];
	char            name[MAX_QPATH];
	qboolean        nameIsInvisible;
	team_t          team;

	unsigned short  xid; //crc32 like in xq3e
	unsigned char   xidStr[5];

	int             botSkill;       // 0 = not bot, 1-5 = bot

	playerColors_t  colors;

	int             score;          // updated by score servercmds
	int             location;       // location index for team mode
	int             health;         // you only get this info about your teammates
	int             armor;
	int             curWeapon;
	int             unknown;
	int             handicap;
	int             wins, losses;   // in tourney mode

	int             teamTask;       // task in teamplay (offence/defence)
	qboolean        teamLeader;     // true when this is a team leader

	int             powerups;       // so can display quad/flag status

	int             medkitUsageTime;
	int             invulnerabilityStartTime;
	int             invulnerabilityStopTime;

	int             breathPuffTime;

	char            modelName[MAX_QPATH];
	char            skinName[MAX_QPATH];
	qboolean        isPmSkin;
	char            headModelName[MAX_QPATH];
	char            headSkinName[MAX_QPATH];
	char            redTeam[MAX_TEAMNAME];
	char            blueTeam[MAX_TEAMNAME];

	qboolean        newAnims;       // true if using the new mission pack animations
	qboolean        fixedlegs;      // true if legs yaw is always the same as torso yaw
	qboolean        fixedtorso;     // true if torso never changes yaw

	vec3_t          headOffset;     // move head in icon views
	footstep_t      footsteps;
	gender_t        gender;         // from model

	qhandle_t       legsModel;
	qhandle_t       legsSkin;

	qhandle_t       torsoModel;
	qhandle_t       torsoSkin;

	qhandle_t       headModel;
	qhandle_t       headSkin;

	qhandle_t       modelIcon;

	animation_t     animations[MAX_TOTALANIMATIONS];

	sfxHandle_t     sounds[MAX_CUSTOM_SOUNDS];
	vec3_t          customLocation;
	team_t          rt;
	team_t          st;
} clientInfo_t;


// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s
{
	qboolean        registered;
	gitem_t*         item;

	qhandle_t       handsModel;         // the hands don't actually draw, they just position the weapon
	qhandle_t       weaponModel;
	qhandle_t       barrelModel;
	qhandle_t       flashModel;

	vec3_t          weaponMidpoint;     // so it will rotate centered instead of by tag

	float           flashDlight;
	vec3_t          flashDlightColor;
	sfxHandle_t     flashSound[4];      // fast firing weapons randomly choose

	qhandle_t       weaponIcon;
	qhandle_t       ammoIcon;

	qhandle_t       ammoModel;

	qhandle_t       missileModel;
	sfxHandle_t     missileSound;
	void (*missileTrailFunc)(centity_t*, const struct weaponInfo_s* wi);
	float           missileDlight;
	vec3_t          missileDlightColor;
	int             missileRenderfx;

	void (*ejectBrassFunc)(centity_t*);

	float           trailRadius;
	float           wiTrailTime;

	sfxHandle_t     readySound;
	sfxHandle_t     firingSound;
	qboolean        loopFireSound;
	vec3_t          explosionDlightColor;
} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct
{
	qboolean        registered;
	qhandle_t       models[MAX_ITEM_MODELS];
	qhandle_t       icon;
} itemInfo_t;


typedef struct
{
	int             itemNum;
} powerupInfo_t;


#define MAX_SKULLTRAIL      10

typedef struct
{
	vec3_t positions[MAX_SKULLTRAIL];
	int numpositions;
} skulltrail_t;


#define MAX_REWARDSTACK     10
#define MAX_SOUNDBUFFER     20

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS    16
#define NUM_SAVED_STATES (CMD_BACKUP + 2)

typedef struct
{
	int         clientFrame;        // incremented each frame

	int         clientNum;

	qboolean    demoPlayback;
	qboolean    levelShot;          // taking a level menu screenshot
	qboolean    loading;            // don't defer players at initial startup
	qboolean    intermissionStarted;    // don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int         latestSnapshotNum;  // the number of snapshots the client system has received
	int         latestSnapshotTime; // the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t*  snap;              // cg.snap->serverTime <= cg.time
	snapshot_t*  nextSnap;          // cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t  activeSnapshots[2];

	float       frameInterpolation; // (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean    thisFrameTeleport;
	qboolean    nextFrameTeleport;

	int         frametime;      // cg.time - cg.oldTime

	int         time;           // this is the time value that the client
	// is rendering at.
	int         oldTime;        // time at last frame, used for missile trails and prediction checking

	int         physicsTime;    // either cg.snap->time or cg.nextSnap->time

	int         timelimitWarnings;  // 5 min, 1 min, overtime
	int         fraglimitWarnings;

	qboolean    mapRestart;         // set on a map restart to set back the weapon

	qboolean    renderingThirdPerson;       // during deaths, chasecams, etc

	// prediction state
	qboolean    hyperspace;             // true if prediction has hit a trigger_teleport
	playerState_t   predictedPlayerState;
	centity_t       predictedPlayerEntity;
	qboolean    validPPS;               // clear until the first call to CG_PredictPlayerState
	int         predictedErrorTime;
	vec3_t      predictedError;

	int         eventSequence;
	int         predictableEvents[MAX_PREDICTED_EVENTS];

	float       stepChange;             // for stair up smoothing
	int         stepTime;

	float       duckChange;             // for duck viewheight smoothing
	int         duckTime;

	float       landChange;             // for landing hard
	int         landTime;

	// input state sent to server
	int         weaponSelect;

	// auto rotating items
	vec3_t      autoAngles;
	vec3_t      autoAxis[3];
	vec3_t      autoAnglesFast;
	vec3_t      autoAxisFast[3];

	// view rendering
	refdef_t    refdef;
	vec3_t      refdefViewAngles;       // will be converted to refdef.viewaxis

	// zoom key
	qboolean    zoomed;
	int         zoomTime;
	float       zoomSensitivity;

	// information screen text during loading
	char        infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	int         scoresRequestTime;
	int         numScores;
	int         selectedScore;
	int         teamScores[2];
	score_t     scores[MAX_CLIENTS];
	qboolean    showScores;
	qboolean    scoreBoardShowing;
	int         scoreFadeTime;
	char        killerName[MAX_NAME_LENGTH];
	char        spectatorList[MAX_STRING_CHARS];        // list of names
	int         spectatorLen;                                               // length of list
	float       spectatorWidth;                                         // width in device units
	int         spectatorTime;                                          // next time to offset
	int         spectatorPaintX;                                        // current paint x
	int         spectatorPaintX2;                                       // current paint x
	int         spectatorOffset;                                        // current offset from start
	int         spectatorPaintLen;                                  // current offset from start

	// skull trails
	skulltrail_t    skulltrails[MAX_CLIENTS];

	// centerprinting
	int         centerPrintTime;
	int         centerPrintCharWidth;
	int         centerPrintY;
	char        centerPrint[1024];
	int         centerPrintLines;

	// low ammo warning state
	int         lowAmmoWarning;     // 1 = low, 2 = empty

	// kill timers for carnage reward
	int         lastKillTime;

	// crosshair client ID
	int         crosshairClientNum;
	int         crosshairClientTime;

	// powerup active flashing
	int         powerupActive;
	int         powerupTime;

	// attacking player
	int         attackerTime;
	int         voiceTime;

	// reward medals
	int         rewardStack;
	int         rewardTime;
	int         rewardCount[MAX_REWARDSTACK];
	qhandle_t   rewardShader[MAX_REWARDSTACK];
	qhandle_t   rewardSound[MAX_REWARDSTACK];

	// sound buffer mainly for announcer sounds
	int         soundBufferIn;
	int         soundBufferOut;
	int         soundTime;
	qhandle_t   soundBuffer[MAX_SOUNDBUFFER];

	// warmup countdown
	int         warmup;
	int         warmupCount;

	//==========================

	int         itemPickup;
	int         itemPickupTime;
	int         itemPickupBlendTime;    // the pulse around the crosshair is timed seperately

	int         weaponSelectTime;
	int         weaponAnimation;
	int         weaponAnimationTime;

	// blend blobs
	float       damageTime;
	float       damageX, damageY, damageValue;

	// status bar head
	float       headYaw;
	float       headEndPitch;
	float       headEndYaw;
	int         headEndTime;
	float       headStartPitch;
	float       headStartYaw;
	int         headStartTime;

	// view movement
	float       v_dmg_time;
	float       v_dmg_pitch;
	float       v_dmg_roll;

	vec3_t      kick_angles;    // weapon kicks
	vec3_t      kick_origin;

	// temp working variables for player view
	float       bobfracsin;
	int         bobcycle;
	float       xyspeed;
	int     nextOrbitTime;

	//qboolean cameraMode;      // if rendering from a loaded camera


	// development tool
	refEntity_t     testModelEntity;
	char            testModelName[MAX_QPATH];
	qboolean        testGun;

	int         lastPredictedCommand;
	int         lastServerTime;
	playerState_t savedPmoveStates[NUM_SAVED_STATES];
	int         stateHead, stateTail;

} cg_t;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct
{
	qhandle_t   charsetShader1;
	qhandle_t   charsetShader;
	qhandle_t   charsetProp;
	qhandle_t   charsetPropGlow;
	qhandle_t   charsetPropB;
	qhandle_t   whiteShader;

	qhandle_t   redCubeModel;
	qhandle_t   blueCubeModel;
	qhandle_t   redCubeIcon;
	qhandle_t   blueCubeIcon;
	qhandle_t   redFlagModel;
	qhandle_t   blueFlagModel;
	qhandle_t   neutralFlagModel;
	qhandle_t   redFlagShader[3];
	qhandle_t   blueFlagShader[3];
	qhandle_t   flagShader[4];

	qhandle_t   flagPoleModel;
	qhandle_t   flagFlapModel;

	qhandle_t   redFlagFlapSkin;
	qhandle_t   blueFlagFlapSkin;
	qhandle_t   neutralFlagFlapSkin;

	qhandle_t   redFlagBaseModel;
	qhandle_t   blueFlagBaseModel;
	qhandle_t   neutralFlagBaseModel;

	qhandle_t   armorModel[3];
	qhandle_t   armorIcon[3];

	qhandle_t   teamStatusBar;

	qhandle_t   deferShader;

	// gib explosions
	qhandle_t   gibAbdomen;
	qhandle_t   gibArm;
	qhandle_t   gibChest;
	qhandle_t   gibFist;
	qhandle_t   gibFoot;
	qhandle_t   gibForearm;
	qhandle_t   gibIntestine;
	qhandle_t   gibLeg;
	qhandle_t   gibSkull;
	qhandle_t   gibBrain;

	qhandle_t   smoke2;

	qhandle_t   machinegunBrassModel;
	qhandle_t   shotgunBrassModel;

	qhandle_t   railRingsShader;
	qhandle_t   railRingsShaderNoPicMip;
	qhandle_t   railCoreShader;
	qhandle_t   railCoreShaderNoPicMip;

	qhandle_t   lightningShader; //probably removed
	qhandle_t   firstPersonGun;

	qhandle_t   unk_media_60;
	qhandle_t   unk_media_61;
	qhandle_t   unk_media_62;
	qhandle_t   unk_media_63;

#define LIGHTNING_NUMBER_OF_SHADERS 16
#define LIGHTNING_DEFAULT_SHADER 0
	qhandle_t   lightningBolt[LIGHTNING_NUMBER_OF_SHADERS];
	qhandle_t   lightningBoltNoPicMip[LIGHTNING_NUMBER_OF_SHADERS];

	qhandle_t   friendShader;
	qhandle_t   frozenFoeTagShader;
	qhandle_t   frozenShader;

	qhandle_t   balloonShader;
	qhandle_t   connectionShader;

	qhandle_t   selectShader;
	qhandle_t   viewBloodShader;
	qhandle_t   damageIndicatorCenter;
	qhandle_t   tracerShader;
	qhandle_t   crosshairShader[NUM_CROSSHAIRS];
	qhandle_t   crosshairShader45[NUM_CROSSHAIRS];
	qhandle_t   crosshairDecorShader[NUM_DECORS];
	qhandle_t   crosshairDecorShader45[NUM_DECORS];
	int         numberOfCrosshairs;
	int         numberOfCrosshairDecors;
	qhandle_t   lagometerShader;
	qhandle_t   backTileShader;
	qhandle_t   noammoShader;

	qhandle_t   smokePuffShader;
	qhandle_t   smokePuffNoPicMipShader;
	qhandle_t   smokePuffRageProShader;
	qhandle_t   shotgunSmokePuffShader;
	qhandle_t   shotgunSmokePuffNoPicMipShader;
	qhandle_t   plasmaBallShader;
	qhandle_t   plasmaBallNoPicMipShader;
	qhandle_t   plasmaNewBallShader;
	qhandle_t   plasmaNewBallNoPicMipShader;
	qhandle_t   grenadeCPMANoPicMipShader;
	qhandle_t   grenadeCPMAModel;
	qhandle_t   waterBubbleShader;
	qhandle_t   bloodTrailShader;

	qhandle_t   numberShaders[11];

	qhandle_t   shadowMarkShader;

	qhandle_t   botSkillShaders[5];

	// wall mark shaders
	qhandle_t   wakeMarkShader;
	qhandle_t   bloodMarkShader;
	qhandle_t   bulletMarkShader;
	qhandle_t   bulletMarkNoPicMipShader;
	qhandle_t   burnMarkShader;
	qhandle_t   burnMarkNoPicMipShader;
	qhandle_t   holeMarkShader;
	qhandle_t   holeMarkNoPicMipShader;
	qhandle_t   energyMarkShader;
	qhandle_t   energyMarkNoPicMipShader;

	// powerup shaders
	qhandle_t freezeShader;
	qhandle_t freezeMarkShader;
	qhandle_t   quadShader;
	qhandle_t   redQuadShader;
	qhandle_t   quadWeaponShader;
	qhandle_t   invisShader;
	qhandle_t   regenShader;
	qhandle_t   battleSuitShader;
	qhandle_t   battleWeaponShader;
	qhandle_t   hastePuffShader;
	qhandle_t   redKamikazeShader;
	qhandle_t   blueKamikazeShader;

	// weapon effect models
	qhandle_t   bulletFlashModel;
	qhandle_t   ringFlashModel;
	qhandle_t   dishFlashModel;
	qhandle_t   lightningExplosionModel;

	// weapon effect shaders
	qhandle_t   railExplosionShader;
	qhandle_t   railExplosionShaderNoPicMip;
	qhandle_t   plasmaExplosionShader;
	qhandle_t   plasmaExplosionShaderNoPicMip;
	qhandle_t   plasmaNewExplosionShader;
	qhandle_t   plasmaNewExplosionShaderNoPicMip;
	qhandle_t   bulletExplosionShader;
	qhandle_t   bulletExplosionShaderNoPicMip;
	qhandle_t   rocketExplosionShader;
	qhandle_t   rocketExplosionShaderNoPicMip;
	qhandle_t   grenadeExplosionShader;
	qhandle_t   grenadeExplosionShaderNoPicMip;
	qhandle_t   bfgExplosionShader;
	qhandle_t   bfgExplosionShaderNoPicMip;
	qhandle_t   bloodExplosionShader;
	qhandle_t   bloodExplosionNoPicMipShader;

	// special effects models
	qhandle_t   teleportEffectModel;
	qhandle_t   teleportEffectShader;
	qhandle_t   invulnerabilityPowerupModel;

	// scoreboard headers
	qhandle_t   scoreboardName;
	qhandle_t   scoreboardPing;
	qhandle_t   scoreboardScore;
	qhandle_t   scoreboardTime;

	// medals shown during gameplay
	qhandle_t   medalImpressive;
	qhandle_t   medalExcellent;
	qhandle_t   medalGauntlet;
	qhandle_t   medalDefend;
	qhandle_t   medalAssist;
	qhandle_t   medalCapture;
	qhandle_t   blender180Shader;

	//superhud
	qhandle_t   obituariesGrenadeDirect;
	qhandle_t   obituariesRocketDirect;
	qhandle_t   obituariesBFGDirect;
	qhandle_t   obituariesDrowned;
	qhandle_t   obituariesMelted;
	qhandle_t   obituariesLava;
	qhandle_t   obituariesTelefrag;
	qhandle_t   obituariesFallenCrashed;
	qhandle_t   obituariesFalling;
	qhandle_t   obituariesSkull;

	// sounds
	sfxHandle_t quadSound;
	sfxHandle_t tracerSound;
	sfxHandle_t selectSound;
	sfxHandle_t useNothingSound;
	sfxHandle_t wearOffSound;
	sfxHandle_t footsteps[FOOTSTEP_TOTAL][4];
	sfxHandle_t sfx_lghit1;
	sfxHandle_t sfx_lghit2;
	sfxHandle_t sfx_lghit3;
	sfxHandle_t sfx_ric1;
	sfxHandle_t sfx_ric2;
	sfxHandle_t sfx_ric3;
	sfxHandle_t sfx_railg;
	sfxHandle_t sfx_rockexp;
	sfxHandle_t sfx_plasmaexp;
	sfxHandle_t gibSound;
	sfxHandle_t gibBounce1Sound;
	sfxHandle_t gibBounce2Sound;
	sfxHandle_t gibBounce3Sound;
	sfxHandle_t teleInSound;
	sfxHandle_t teleOutSound;
	sfxHandle_t noAmmoSound;
	sfxHandle_t lowAmmoSound;
	sfxHandle_t respawnSound;
	sfxHandle_t talkSound;
	sfxHandle_t landSound;
	sfxHandle_t fallSound;
	sfxHandle_t jumpPadSound;

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;
	sfxHandle_t suddenDeathSound;

	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;

	sfxHandle_t hitLowestSound;
	sfxHandle_t hitLowSound;
	sfxHandle_t hitSound;
	sfxHandle_t hitSounds[4];
	sfxHandle_t hitHighSound;
	sfxHandle_t fragSound;
	sfxHandle_t hitSoundHighArmor;
	sfxHandle_t hitSoundLowArmor;
	sfxHandle_t hitTeamSound;
	sfxHandle_t gotDamageSound;
	sfxHandle_t impressiveSound;
	sfxHandle_t excellentSound;
	sfxHandle_t deniedSound;
	sfxHandle_t humiliationSound;
	sfxHandle_t assistSound;
	sfxHandle_t defendSound;
	sfxHandle_t firstImpressiveSound;
	sfxHandle_t firstExcellentSound;
	sfxHandle_t firstHumiliationSound;

	sfxHandle_t takenLeadSound;
	sfxHandle_t tiedLeadSound;
	sfxHandle_t lostLeadSound;

	sfxHandle_t voteNow;
	sfxHandle_t votePassed;
	sfxHandle_t voteFailed;

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

	sfxHandle_t flightSound;
	sfxHandle_t medkitSound;

	sfxHandle_t weaponHoverSound;

	// teamplay sounds
	sfxHandle_t captureAwardSound;
	sfxHandle_t redScoredSound;
	sfxHandle_t blueScoredSound;
	sfxHandle_t redLeadsSound;
	sfxHandle_t blueLeadsSound;
	sfxHandle_t teamsTiedSound;

	sfxHandle_t captureYourTeamSound;
	sfxHandle_t captureOpponentSound;
	sfxHandle_t returnYourTeamSound;
	sfxHandle_t returnOpponentSound;
	sfxHandle_t takenYourTeamSound;
	sfxHandle_t takenOpponentSound;

	sfxHandle_t redFlagReturnedSound;
	sfxHandle_t blueFlagReturnedSound;
	sfxHandle_t neutralFlagReturnedSound;
	sfxHandle_t enemyTookYourFlagSound;
	sfxHandle_t enemyTookTheFlagSound;
	sfxHandle_t yourTeamTookEnemyFlagSound;
	sfxHandle_t yourTeamTookTheFlagSound;
	sfxHandle_t youHaveFlagSound;
	sfxHandle_t yourBaseIsUnderAttackSound;
	sfxHandle_t holyShitSound;

	// tournament sounds
	sfxHandle_t count3Sound;
	sfxHandle_t count2Sound;
	sfxHandle_t count1Sound;
	sfxHandle_t countFightSound;
	sfxHandle_t countPrepareSound;

	qhandle_t cursor;
	qhandle_t selectCursor;
	qhandle_t sizeCursor;
	qhandle_t customShader[16];

	sfxHandle_t regenSound;
	sfxHandle_t protectSound;
	sfxHandle_t n_healthSound;
	sfxHandle_t hgrenb1aSound;
	sfxHandle_t hgrenb2aSound;
	sfxHandle_t wstbimplSound;
	sfxHandle_t wstbimpmSound;
	sfxHandle_t wstbimpdSound;
	sfxHandle_t wstbactvSound;

} cgMedia_t;

typedef struct osp_decal_s
{
	int v1;
	int v2;
	int v3;
	int v4;
	float v5;
	float v6;
	float v7;
	float v8;
	float v9;
	float v10;
	float v11;
} osp_decal_t;

typedef struct playerColorsOverride_s
{
	qboolean isRailColorSet;
	qboolean isModelColorSet;
	qboolean isFrozenColorSet;
} playerColorsOverride_t;

#define  OSP_SERVER_MODE_VQ3      0
#define  OSP_SERVER_MODE_PROMODE  1
#define  OSP_SERVER_MODE_CQ3      2
#define  OSP_SERVER_MODE_PROMODE_OPT1      0x11
#define  OSP_SERVER_MODE_PROMODE_OPT2      0x21

#define  CS_OSP_CUSTOM_CLIENT_DEFAULT 47
#define  CS_OSP_CUSTOM_CLIENT2_DEFAULT 0

typedef struct cgs_osp_s
{
	qhandle_t configString_0x368;
	fileHandle_t logFileHandle;
	int unknown_variable_329;
	int server_mode;
	vec3_t rail_color;
	int custom_client;
	int custom_client_2;
	int allow_pmove;
	int serverConfigMinimumTimenudge;
	int serverConfigMaximumTimenudge;
	int serverConfigMinimumMaxpackets;
	int serverConfigMaximumMaxpackets;
	int nextCvarsUpdateTime;
	int cvarsUpdateDelay;
	qboolean isOSPv1;
	int stats_mask;
	int stats_shift;
	qhandle_t quadSoundTime;

	int numberOfStringsMotd;
	char motd[8][64];

	char testFont[MAX_QPATH];
	int custom_gfx_number;
	int custom_gfx[16][8];
	qboolean clanBaseTeamDM;
	int gameTypeFreeze;
	qhandle_t flagStatus;
	qhandle_t redflag;
	qhandle_t blueflag;
	qhandle_t unknown_variable_609;
	qhandle_t decals_number;
	qhandle_t unknown_variable_611;
	int decalOSPShaderCopyCount;
	osp_decal_t decals[32];
	qhandle_t blender180ShaderCopy[17];
	qhandle_t decalOSPShader[17];
	qhandle_t unknown_variable_999;
	int osp_teamcount1;
	int osp_teamcount2;
	qhandle_t unknown_variable_1002;
	qhandle_t pingPrintTime;
	float pingMs;
	int pingCount;
	char pingString[16];
	qhandle_t pingStringPositionX;
	qhandle_t unknown_variable_1011;
	qhandle_t hboxShader;
	qhandle_t hboxShader_nocull;
	playerColors_t myColors;
	playerColors_t teamColors;
	playerColorsOverride_t teamColorsOverride;
	playerColors_t enemyColors;
	playerColorsOverride_t enemyColorsOverride;
	int lastHitTime;
	struct
	{
		vec4_t color;
		vec4_t actionColor;
		vec4_t decorColor;
		vec4_t decorActionColor;
		int distance;
	} crosshair;
	struct
	{
		qboolean forceChat;
		qboolean key[4];
	} shud;
} cgs_osp_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct
{
	gameState_t     gameState;          // gamestate from server
	glconfig_t      glconfig;           // rendering configuration
	float           screenXScale_Old;       // derived from glconfig
	float           screenYScale_Old;
	float           screenXBias_Old;

	float                 screenXScale;
	float                 screenYScale;

	float                 screenXBias;
	float                 screenYBias;

	float                 screenXmin;
	float                 screenXmax;

	float                 screenYmin;
	float                 screenYmax;

	int             serverCommandSequence;  // reliable command stream counter
	int             processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean        localServer;        // detected on startup by checking sv_running

	// parsed from serverinfo
	gametype_t      gametype;
	int             dmflags;
	int             teamflags;
	int             fraglimit;
	int             capturelimit;
	int             timelimit;
	int             maxclients;
	char            mapname[MAX_QPATH];
	char            redTeam[MAX_QPATH];
	char            blueTeam[MAX_QPATH];

	int             voteTime;
	int             voteYes;
	int             voteNo;
	qboolean        voteModified;           // beep whenever changed
	char            voteString[MAX_STRING_TOKENS];

	int             teamVoteTime[2];
	int             teamVoteYes[2];
	int             teamVoteNo[2];
	qboolean        teamVoteModified[2];    // beep whenever changed
	char            teamVoteString[2][MAX_STRING_TOKENS];

	int             levelStartTime;

	int             scores1, scores2;       // from configstrings
	int             redflag, blueflag;      // flag status from configstrings
	int             flagStatus;

	qboolean  newHud;

	//
	// locally derived information from gamestate
	//
	qhandle_t       gameModels[MAX_MODELS];
	sfxHandle_t     gameSounds[MAX_SOUNDS];

	int             numInlineModels;
	qhandle_t       inlineDrawModel[MAX_MODELS];
	vec3_t          inlineModelMidpoints[MAX_MODELS];

	clientInfo_t    clientinfo[MAX_CLIENTS];

	// teamchat width is *3 because of embedded color codes
	char            teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH * 3 + 1];
	int             teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int             teamChatPos;
	int             teamLastChatPos;

	int cursorX;
	int cursorY;
	qboolean eventHandling;
	qboolean mouseCaptured;
	qboolean sizingHud;
	void* capturedItem;
	qhandle_t activeCursor;

	// orders
	int currentOrder;
	qboolean orderPending;
	int orderTime;
	int acceptOrderTime;
	int acceptTask;
	int acceptLeader;

	// media
	cgMedia_t       media;
	cgs_osp_t osp;
} cgs_t;

//==============================================================================

extern  cgs_t           cgs;
extern  cg_t            cg;
extern  centity_t       cg_entities[MAX_GENTITIES];
extern  weaponInfo_t    cg_weapons[MAX_WEAPONS];
extern  itemInfo_t      cg_items[MAX_ITEMS];
extern  markPoly_t      cg_markPolys[MAX_MARK_POLYS];

extern vmCvar_t           osp_client;
extern vmCvar_t           osp_hidden;
extern vmCvar_t           osp_debug;
extern vmCvar_t           cg_autoswitch;
extern vmCvar_t           cg_drawGun;
extern vmCvar_t           cg_zoomFov;
extern vmCvar_t           cg_zoomTime;
extern vmCvar_t           cg_zoomSensitivityAuto;
extern vmCvar_t           cg_zoomSensitivityMultiplier;
extern vmCvar_t           cg_fov;
extern vmCvar_t           cg_viewsize;
extern vmCvar_t           cg_stereoSeparation;
extern vmCvar_t           cg_shadows;
extern vmCvar_t           cg_gibs;
extern vmCvar_t           cg_draw2D;
extern vmCvar_t           cg_drawStatus;
extern vmCvar_t           cg_drawTimer;
extern vmCvar_t           cg_drawFPS;
extern vmCvar_t           cg_drawSnapshot;
extern vmCvar_t           cg_draw3dIcons;
extern vmCvar_t           cg_drawIcons;
extern vmCvar_t           cg_drawAmmoWarning;
extern vmCvar_t           cg_drawAttacker;
extern vmCvar_t           cg_drawCrosshair;
extern vmCvar_t           cg_drawCrosshairGauntlet;
extern vmCvar_t           cg_drawCrosshairMachinegun;
extern vmCvar_t           cg_drawCrosshairShotgun;
extern vmCvar_t           cg_drawCrosshairGrenadeLauncher;
extern vmCvar_t           cg_drawCrosshairRocketLauncher;
extern vmCvar_t           cg_drawCrosshairLightning;
extern vmCvar_t           cg_drawCrosshairRailgun;
extern vmCvar_t           cg_drawCrosshairPlasmagun;
extern vmCvar_t           cg_drawCrosshairBFG;
extern vmCvar_t           cg_drawCrosshairNames;
extern vmCvar_t           cg_drawRewards;
extern vmCvar_t           cg_crosshairSize;
extern vmCvar_t           cg_crosshairAspectRatioFix;
extern vmCvar_t           cg_crosshairHealth;
extern vmCvar_t           cg_crosshairX;
extern vmCvar_t           cg_crosshairY;
extern vmCvar_t           cg_brassTime;
extern vmCvar_t           cg_simpleItems;
extern vmCvar_t           cg_addMarks;
extern vmCvar_t           cg_lagometer;
extern vmCvar_t           cg_railTrailTime;
extern vmCvar_t           cg_gun_x;
extern vmCvar_t           cg_gun_y;
extern vmCvar_t           cg_gun_z;
extern vmCvar_t           cg_centertime;
extern vmCvar_t           cg_runpitch;
extern vmCvar_t           cg_runroll;
extern vmCvar_t           cg_bobup;
extern vmCvar_t           cg_bobpitch;
extern vmCvar_t           cg_bobroll;
extern vmCvar_t           cg_swingSpeed;
extern vmCvar_t           cg_animSpeed;
extern vmCvar_t           cg_debugAnim;
extern vmCvar_t           cg_debugPosition;
extern vmCvar_t           cg_debugEvents;
extern vmCvar_t           cg_errorDecay;
extern vmCvar_t           cg_nopredict;
extern vmCvar_t           cg_noPlayerAnims;
extern vmCvar_t           cg_showmiss;
extern vmCvar_t           cg_footsteps;
extern vmCvar_t           cg_tracerChance;
extern vmCvar_t           cg_tracerWidth;
extern vmCvar_t           cg_tracerLength;
extern vmCvar_t           cg_thirdPersonRange;
extern vmCvar_t           cg_thirdPersonAngle;
extern vmCvar_t           cg_thirdPerson;
extern vmCvar_t           cg_teamChatTime;
extern vmCvar_t           cg_teamChatHeight;
extern vmCvar_t           cg_forceModel;
extern vmCvar_t           cg_predictItems;
extern vmCvar_t           cg_deferPlayers;
extern vmCvar_t           cg_drawTeamOverlay;
extern vmCvar_t           teamoverlay;
extern vmCvar_t           cg_stats;
extern vmCvar_t           cg_drawFriend;
extern vmCvar_t           cg_buildScript;
extern vmCvar_t           cg_paused;
extern vmCvar_t           com_blood;
extern vmCvar_t           cg_synchronousClients;
extern vmCvar_t           cg_cameraOrbit;
extern vmCvar_t           cg_cameraOrbitDelay;
extern vmCvar_t           cg_timescaleFadeEnd;
extern vmCvar_t           cg_timescaleFadeSpeed;
extern vmCvar_t           cg_timescale;
extern vmCvar_t           cg_scorePlums;
extern vmCvar_t           cg_smoothClients;
extern vmCvar_t           cg_cameraMode;
extern vmCvar_t           pmove_fixed;
extern vmCvar_t           pmove_msec;
extern vmCvar_t           cg_noTaunt;
extern vmCvar_t           cg_noProjectileTrail;
extern vmCvar_t           cg_oldRail;
extern vmCvar_t           cg_oldRocket;
extern vmCvar_t           cg_oldPlasma;
extern vmCvar_t           cg_altGrenades;
extern vmCvar_t           cg_altLightning;
extern vmCvar_t           cg_altPlasma;
extern vmCvar_t           cg_ammoCheck;
extern vmCvar_t           cg_autoAction;
extern vmCvar_t           cg_clientLog;
extern vmCvar_t           cg_crosshairPulse;
extern vmCvar_t           cg_customLoc;
extern vmCvar_t           cg_damageSound;
extern vmCvar_t           cg_damageDraw;
extern vmCvar_t           cg_damageIndicatorScale;
extern vmCvar_t           cg_damageIndicatorOpaque;
extern vmCvar_t           cg_damageKick;
extern vmCvar_t           cg_deadBodyFilter;
extern vmCvar_t           cg_deadBodyBlack;
extern vmCvar_t           cg_deadBodyInvisible;
extern vmCvar_t           cg_drawDecals;
extern vmCvar_t           cg_drawPing;
extern vmCvar_t           cg_enableOSPHUD;
extern vmCvar_t           cg_shud;
extern vmCvar_t           cg_enableBreath;
extern vmCvar_t           cg_enemyColors;
extern vmCvar_t           cg_enemyModel;
extern vmCvar_t           cg_teamColors;
extern vmCvar_t           cg_teamModel;
extern vmCvar_t           cg_execVstr;
extern vmCvar_t           cg_fallKick;
extern vmCvar_t           cg_followkiller;
extern vmCvar_t           cg_followpowerup;
extern vmCvar_t           cg_followviewcam;
extern vmCvar_t           cg_forceColors;
extern vmCvar_t           cg_lightningImpact;
extern vmCvar_t           cg_MaxlocationWidth;
extern vmCvar_t           cg_muzzleFlash;
extern vmCvar_t           cg_nochatbeep;
extern vmCvar_t           cg_noTeamChatBeep;
extern vmCvar_t           cg_NoMOTDDisplayDuringDemo;
extern vmCvar_t           cg_nomip;
extern vmCvar_t           cg_oversampleMouse;
extern vmCvar_t           cg_statScrollTime;
extern vmCvar_t           cg_q3compScoreboard;
extern vmCvar_t           cg_scoreTransparency;
extern vmCvar_t           cg_showPlayerLean;
extern vmCvar_t           cg_smoke_sg;
extern vmCvar_t           cg_smokegrowth_gl;
extern vmCvar_t           cg_smokegrowth_rl;
extern vmCvar_t           cg_smokeradius_gl;
extern vmCvar_t           cg_smokeradius_rl;
extern vmCvar_t           cg_smokedensity_gl;
extern vmCvar_t           cg_smokedensity_rl;
extern vmCvar_t           cg_swapSkins;
extern vmCvar_t           cg_teamRails;
extern vmCvar_t           cg_trueLightning;
extern vmCvar_t           cg_useScreenShotJPEG;
extern vmCvar_t           ch_3waveFont;
extern vmCvar_t           ch_ColorLocations;
extern vmCvar_t           cg_CrosshairColor;
extern vmCvar_t           cf_CrosshairNames;
extern vmCvar_t           ch_CrosshairNamesLeft;
extern vmCvar_t           ch_crosshairTeamInfo;
extern vmCvar_t           ch_drawHealthMeters;
extern vmCvar_t           ch_drawFlagNames;
extern vmCvar_t           ch_drawPickup;
extern vmCvar_t           ch_drawSpeed;
extern vmCvar_t           ch_TeamBackground;
extern vmCvar_t           cf_Following;
extern vmCvar_t           ch_FilterLocationsTeamchat;
extern vmCvar_t           ch_fragMessage;
extern vmCvar_t           ch_file;
extern vmCvar_t           cf_Fragmsg;
extern vmCvar_t           ch_graphs;
extern vmCvar_t           ch_InverseTeamChat;
extern vmCvar_t           ch_Obituaries;
extern vmCvar_t           ch_Pickupitem;
extern vmCvar_t           cf_Pickupstring;
extern vmCvar_t           ch_recordMessage;
extern vmCvar_t           cf_Scores;
extern vmCvar_t           cf_Statusbar;
extern vmCvar_t           ch_StatusbarFormat;
extern vmCvar_t           ch_statusbarStyle;
extern vmCvar_t           ch_teamBackground;
extern vmCvar_t           cf_Teamchat;
extern vmCvar_t           ch_TeamchatOnly;
extern vmCvar_t           ch_TeamCrosshairHealth;
extern vmCvar_t           cf_TeamCrosshairHealth;
extern vmCvar_t           ch_Teamoverlay;
extern vmCvar_t           cf_Teamoverlay;
extern vmCvar_t           cf_Vote;
extern vmCvar_t           ch_weaponList;
extern vmCvar_t           ch_weaponListBackground;
extern vmCvar_t           ch_weaponListDrawAll;
extern vmCvar_t           ch_weaponListFlash;
extern vmCvar_t           ch_Weaponswitch;
extern vmCvar_t           cf_WeaponSwitch;
extern vmCvar_t           cf_WeaponName;
extern vmCvar_t           cf_AmmoStatusbar;
extern vmCvar_t           cf_timer;
extern vmCvar_t           s_ambient;
extern vmCvar_t           cl_maxpackets;
extern vmCvar_t           cl_timenudge;
extern vmCvar_t           snaps;
extern vmCvar_t           r_shownormals;
extern vmCvar_t           r_fullbright;
extern vmCvar_t           r_lodCurveError;
extern vmCvar_t           r_showtris;
extern vmCvar_t           r_subdivisions;
extern vmCvar_t           r_znear;
extern vmCvar_t           handicap;
extern vmCvar_t           m_pitch;
extern vmCvar_t           com_maxfps;
extern vmCvar_t           cg_gun_frame;
extern vmCvar_t           cg_noLeadSounds;
extern vmCvar_t           cg_fragSound;
extern vmCvar_t           cg_lightningHide;
extern vmCvar_t           cg_lightningHideCrosshair;
extern vmCvar_t           cg_lightningSilent;
extern vmCvar_t           cg_delag;
extern vmCvar_t           cg_drawHitBox;
extern vmCvar_t           cg_projectileNudge;
extern vmCvar_t           cg_hideScores;

extern vmCvar_t           cg_playerModelColors;
extern vmCvar_t           cg_playerRailColors;
extern vmCvar_t           cg_playerFrozenColor;

extern vmCvar_t           cg_teamModelColors;
extern vmCvar_t           cg_teamRailColors;
extern vmCvar_t           cg_teamFrozenColor;
extern vmCvar_t           cg_teamFrozenFoe;

extern vmCvar_t           cg_enemyModelColorsUnique;
extern vmCvar_t           cg_enemyModelColors;
extern vmCvar_t           cg_enemyRailColors;
extern vmCvar_t           cg_enemyFrozenColor;

extern vmCvar_t           cg_spectGlow;
extern vmCvar_t           cg_spectOrigModel;
extern vmCvar_t           cg_hitSounds;
extern vmCvar_t           cg_playersXID;

extern vmCvar_t           cg_chatEnable;
extern vmCvar_t           cg_shudChatEnable;

extern vmCvar_t           cg_healthMid;
extern vmCvar_t           cg_healthLow;
extern vmCvar_t           cg_healthColorLevels;

extern vmCvar_t           ch_crosshairDecor;
extern vmCvar_t           ch_crosshair45;
extern vmCvar_t           ch_crosshairDecor45;
extern vmCvar_t           ch_crosshairOpaque;
extern vmCvar_t           ch_crosshairDecorOpaque;

extern vmCvar_t           ch_crosshairAction;
extern vmCvar_t           ch_crosshairActionColor;
extern vmCvar_t           ch_crosshairActionScale;
extern vmCvar_t           ch_crosshairActionTime;
extern vmCvar_t           ch_crosshairDecorSize;
extern vmCvar_t           ch_crosshairDecorAction;
extern vmCvar_t           ch_crosshairDecorActionColor;
extern vmCvar_t           ch_crosshairDecorActionScale;
extern vmCvar_t           ch_crosshairDecorActionTime;

extern vmCvar_t           ch_crosshairAutoScale;

extern vmCvar_t           cg_dlightGauntlet;
extern vmCvar_t           cg_dlightMG;
extern vmCvar_t           cg_dlightSG;
extern vmCvar_t           cg_dlightGL;
extern vmCvar_t           cg_dlightRL;
extern vmCvar_t           cg_dlightLG;
extern vmCvar_t           cg_dlightRG;
extern vmCvar_t           cg_dlightPG;
extern vmCvar_t           cg_dlightBFG;
extern vmCvar_t           cg_gunColor;
extern vmCvar_t           cg_gunOpaque;
extern vmCvar_t           cg_conObituaries;

extern vmCvar_t           cg_lightningHitsoundRateFix;
extern vmCvar_t           cg_stackHitSounds;
extern vmCvar_t           cg_stackHitSoundsTimeout;
extern vmCvar_t           cg_drawCenterMessages;
extern vmCvar_t           cg_predictStepOffset;
extern vmCvar_t           cg_noVoteBeep;
extern vmCvar_t           cg_itemFx;

extern vmCvar_t           cg_shud_chatindex;
extern vmCvar_t           cg_shud_chatmsg0;
extern vmCvar_t           cg_shud_chatmsg1;
extern vmCvar_t           cg_shud_chatmsg2;
extern vmCvar_t           cg_shud_chatmsg3;
extern vmCvar_t           cg_shud_chatmsg4;
extern vmCvar_t           cg_shud_chatmsg5;
extern vmCvar_t           cg_shud_chatmsg6;
extern vmCvar_t           cg_shud_chatmsg7;
extern vmCvar_t           cg_shud_chatmsg8;
extern vmCvar_t           cg_shud_chatmsg9;
extern vmCvar_t           cg_shud_chatmsg10;
extern vmCvar_t           cg_shud_chatmsg11;
extern vmCvar_t           cg_shud_chatmsg12;
extern vmCvar_t           cg_shud_chatmsg13;
extern vmCvar_t           cg_shud_chatmsg14;
extern vmCvar_t           cg_shud_chatmsg15;

//
// cg_main.c
//
typedef struct cvarTable_s cvarTable_t;

struct cvarTable_s
{
	vmCvar_t*    vmCvar;
	char*        cvarName;
	char*        defaultString;
	int          cvarFlags;
	void (*onChanged)(cvarTable_t* cvart);
	int          forceModelModificationCount;
	cvarTable_t* next;
	cvarTable_t* prev;
};


const char* CG_ConfigString(int index);
const char* CG_Argv(int arg);

void QDECL CG_Printf(const char* msg, ...);

#ifdef __GNUC__
__attribute__((noreturn))
#endif
void QDECL CG_Error(const char* msg, ...);
void CG_PrintLog(char* msg);

void CG_StartMusic(void);

void CG_UpdateCvars(void);

int CG_CrosshairPlayer(void);
int CG_LastAttacker(void);
void CG_LoadMenus(const char* menuFile);
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int x, int y);
void CG_EventHandling(int type);
void CG_RankRunFrame(void);
void CG_SetScoreSelection(void* menu);
score_t* CG_GetSelectedScore(void);
void CG_BuildSpectatorString(void);

char* CG_OSPGetCvarName(vmCvar_t* cvar) ;
cvarTable_t* CG_GetCgCvarByName(const char* name);
void CG_CvarTouch(const char* name);
void CG_CvarResetToDefault(const char* name);

qhandle_t CG_GetFragSound(void);

//
// cg_view.c
//
void CG_TestModel_f(void);
void CG_TestGun_f(void);
void CG_TestModelNextFrame_f(void);
void CG_TestModelPrevFrame_f(void);
void CG_TestModelNextSkin_f(void);
void CG_TestModelPrevSkin_f(void);
void CG_ZoomDown_f(void);
void CG_ZoomUp_f(void);
void CG_ZoomToggle_f(void);
void CG_AddBufferedSound(sfxHandle_t sfx);

void CG_DrawActiveFrame(int serverTime, stereoFrame_t stereoView, qboolean demoPlayback);
float CG_DrawLagometer(float pos);
void CG_SetCvarDescription(const char* name, const char* description);


//
// cg_drawtools.c
//
typedef struct
{
	vec4_t color;
	int unknown;
	int x;
	int y;
	int w;
	int h;
	int s1;
	int t1;
	int s2;
	int t2;
	qhandle_t hShader;
	qboolean some_flag;
} OSP_PrintInfo_t;

void CG_AdjustFrom640(float* x, float* y, float* w, float* h);
void CG_AdjustFrom640_Old(float* x, float* y, float* w, float* h, qboolean correctWide);
void CG_FillRect(float x, float y, float width, float height, const float* color);
void CG_DrawPicOld(float x, float y, float width, float height, qhandle_t hShader);
void CG_DrawPic(float x, float y, float width, float height, qhandle_t hShader);

float CG_OSPDrawStringLength(const char* string, float ax, float aw, int proportional);
int CG_OSPDrawStringLenPix(const char* string, float charWidth, int flags, int toWidth);

void CG_OSPDrawStringPrepare(const char* from, char* to, int size);
void CG_OSPDrawString(float x, float y, const char* string, const vec4_t setColor, float charWidth, float charHeight, int maxWidth, int flags, vec4_t background);
void CG_FontSelect(int index);
int CG_FontIndexFromName(const char* name);

#define OSP_TEXT_CMD_MAX 2048

typedef enum
{
	OSP_TEXT_CMD_CHAR = 0,
	OSP_TEXT_CMD_STOP,
	OSP_TEXT_CMD_FADE,
	OSP_TEXT_CMD_TEXT_COLOR,
	OSP_TEXT_CMD_SHADOW_COLOR,
} text_command_type_t;

typedef struct
{
	text_command_type_t type;
	union
	{
		char character;
		float fade;
		vec4_t color;
	} value;
} text_command_t;

text_command_t* CG_CompileText(const char* text);
void CG_CompiledTextDestroy(text_command_t* root);

// flags for CG_DrawString
enum
{
	DS_VBOTTOM      = 0,   // vertical boootm is default
	DS_HLEFT        = 0,   // horizontal left is default
	DS_SHADOW       = 0x1, // draw shadow
	DS_FORCE_COLOR  = 0x2, // force color, ignore ^x
	DS_PROPORTIONAL = 0x4, // proportional font
	DS_HCENTER      = 0x8, // horizontal center
	DS_HRIGHT       = 0x10,// horizontal right
	DS_VTOP         = 0x20,// vertical top
	DS_VCENTER      = 0x40,// vertical center
};
void CG_DrawString(float x, float y, const char* string, const vec4_t setColor, float charWidth, float charHeight, int maxChars, int flags);

void CG_LoadFonts(void);

void CG_DrawStringExt(int x, int y, const char* string, const float* setColor,
                      qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars);
void CG_DrawBigString(int x, int y, const char* s, const float alpha, int flags, int font);
void CG_DrawSmallString(int x, int y, const char* s, float alpha, int flags, int font);
void CG_DrawSmallStringColor(int x, int y, const char* s, vec4_t color, int flags, int font);

int CG_DrawStrlen(const char* str);

float*   CG_FadeColor(int startMsec, int totalMsec);
float* CG_TeamColor(int team);
void CG_TileClear(void);
void CG_ColorForHealth(vec4_t hcolor, char* color);
void CG_GetColorForHealth(int health, int armor, vec4_t hcolor, char* ccolor);

void UI_DrawProportionalString(int x, int y, const char* str, int style, vec4_t color);
void CG_DrawRect(float x, float y, float width, float height, float size, const float* color);
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);

void CG_OSPDrawPoly(float x, float y, float w, float h, vec4_t color);
qboolean CG_OSPGetClientFontSize(const vmCvar_t* cvar, int* w, int* h);
int CG_OSPDrawStringWithShadow(int x, int y, const char* str, int charWidth, int charHeight, const vec4_t color, int maxChars);
int CG_OSPDrawStringOld(int x, int y, const char* str, int charWidth, int charHeight, vec4_t const colors, int maxChars, qboolean disableColorChange);
qboolean CG_Hex16GetColor(const char* str, float* color);

//
// cg_draw.c
//
typedef struct
{
	int charHeight;
	int charWidth;
	int maxStringLen;
	int hideBeforeRealtime;
	int numberOfStrings;
	int hideBeforeCGTime;
	int timeAppearance;
	int timeShow;
	int timeHiding;
	int showFromCGTime;
	int windowPosX;
	char string[24][128];
	vec4_t borderColor;
	vec4_t bodyColor;
} OSP_SlidingPrintContext_t;
extern  int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern  int numSortedTeamPlayers;
extern  int drawTeamOverlayModificationCount;
extern  char systemChat[256];
extern  char teamChat1[256];
extern  char teamChat2[256];
extern  int frameTime;
extern OSP_SlidingPrintContext_t ospPrintContext[16];
extern const char* monthName[12];
extern const char* dayOfWeekName[7];

void CG_AddLagometerFrameInfo(void);
void CG_AddLagometerSnapshotInfo(snapshot_t* snap);
void CG_CenterPrint(const char* str, int y, int charWidth);
void CG_DrawHead(float x, float y, float w, float h, int clientNum, vec3_t headAngles);
void CG_DrawActive(stereoFrame_t stereoView);
void CG_DrawFlagModel(float x, float y, float w, float h, int team, qboolean force2D);
void CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team);
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char* text, float adjust, int limit, int style);
int CG_Text_Width(const char* text, float scale, int limit);
int CG_Text_Height(const char* text, float scale, int limit);
void CG_SelectPrevPlayer(void);
void CG_SelectNextPlayer(void);
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char** args);
void CG_ShowResponseHead(void);
void CG_SetPrintString(int type, const char* p);
void CG_InitTeamChat(void);
void CG_GetTeamColor(vec4_t* color);
const char* CG_GetGameStatusText(void);
const char* CG_GetKillerText(void);
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles);
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
void CG_CheckOrderPending(void);
const char* CG_GameTypeString(void);
qboolean CG_YourTeamHasFlag(void);
qboolean CG_OtherTeamHasFlag(void);
qhandle_t CG_StatusHandle(int task);
void CG_DrawField(int x, int y, int width, int value, int w, int h);
void CG_OSPDrawField(int x, int y, int val);
void CG_DrawStatusBar(void);
float CG_DrawSnapshot(float y);
float CG_DrawFPS(float y);
float CG_DrawTimer(float y);
void CG_DrawTimer2(void);
float CG_DrawAttacker(float y);
void CG_DrawAmmoWarning(void);
void CG_DrawReward(void);
void CG_DrawWarmup(void);
void CG_ScanForCrosshairEntity(void);
void CG_OSPDrawLeftSlidingWindowsRoutine(OSP_SlidingPrintContext_t* context);
int CG_OSPDrawLeftSlidingWindow(float arg0, float arg1, float arg2, float time3Sec, int numberOfLines, int sizeOfLine, int w, int h, char* text, int windowPosX, float* borderColor, float* bodyColor);
void CG_OSPDrawIntermission(void);
void CG_OSPDrawCenterString(void);
void CG_OSPSetColor(vec4_t color);
void CG_OSPDrawPic(float x, float y, float w, float h, qhandle_t hShader);
void CG_OSPDraw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t pos, vec3_t angles, vec3_t angles2);

#define LAG_SAMPLES     1024
#define MAX_LAGOMETER_PING  900
#define MAX_LAGOMETER_RANGE 300

typedef struct
{
	int     frameSamples[LAG_SAMPLES];
	int     frameCount;
	int     snapshotFlags[LAG_SAMPLES];
	int     snapshotSamples[LAG_SAMPLES];
	int     snapshotCount;
} lagometer_t;

extern lagometer_t     lagometer;


//
// cg_player.c
//
void CG_Player(centity_t* cent);
void CG_ResetPlayerEntity(centity_t* cent);
void CG_AddRefEntityWithPowerups(refEntity_t* ent, entityState_t* state, int team);
void CG_NewClientInfo(int clientNum);
sfxHandle_t CG_CustomSound(int clientNum, const char* soundName);
void CG_UpdateOurClientInfo(void);
void CG_UpdateAllClientsInfo(void);

//
// cg_predict.c
//
void CG_BuildSolidList(void);
int CG_PointContents(const vec3_t point, int passEntityNum);
void CG_Trace(trace_t* result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
              int skipNumber, int mask);
void CG_PredictPlayerState(void);


//
// cg_events.c
//
void CG_CheckEvents(centity_t* cent);
const char*  CG_PlaceString(int rank);
void CG_EntityEvent(centity_t* cent, vec3_t position);
void CG_PainEvent(centity_t* cent, int health);


//
// cg_ents.c
//
void CG_SetEntitySoundPosition(centity_t* cent);
void CG_AddPacketEntities(void);
void CG_Beam(centity_t* cent);
void CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out);

void CG_PositionEntityOnTag(refEntity_t* entity, const refEntity_t* parent,
                            qhandle_t parentModel, char* tagName);
void CG_PositionRotatedEntityOnTag(refEntity_t* entity, const refEntity_t* parent,
                                   qhandle_t parentModel, char* tagName);



//
// cg_weapons.c
//
void CG_NextWeapon_f(void);
void CG_PrevWeapon_f(void);
void CG_Weapon_f(void);

void CG_RegisterWeapon(int weaponNum);
void CG_RegisterItemVisuals(int itemNum);

void CG_FireWeapon(centity_t* cent);
void CG_MissileHitWall(int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType);
void CG_MissileHitPlayer(int weapon, vec3_t origin, vec3_t dir, int entityNum);
void CG_ShotgunFire(entityState_t* es);
void CG_Bullet(vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum);

void CG_RailTrail(clientInfo_t* ci, vec3_t start, vec3_t end);
void CG_GrappleTrail(centity_t* ent, const weaponInfo_t* wi);
void CG_AddViewWeapon(playerState_t* ps);
void CG_AddPlayerWeapon(refEntity_t* parent, playerState_t* ps, centity_t* cent, int team);
void CG_DrawWeaponSelect(void);

void CG_OutOfAmmoChange(void);   // should this be in pmove?

void CG_UpdateWeaponDlightColor(weapon_t weapon);

//
// cg_marks.c
//
void    CG_InitMarkPolys(void);
void    CG_AddMarks(void);
void    CG_ImpactMark(qhandle_t markShader,
                      const vec3_t origin, const vec3_t dir,
                      float orientation,
                      float r, float g, float b, float a,
                      qboolean alphaFade,
                      float radius, qboolean temporary, qboolean force);

//
// cg_localents.c
//
void    CG_InitLocalEntities(void);
localEntity_t*   CG_AllocLocalEntity(void);
void    CG_AddLocalEntities(void);

//
// cg_effects.c
//
localEntity_t* CG_SmokePuff(const vec3_t p,
                            const vec3_t vel,
                            float radius,
                            float r, float g, float b, float a,
                            float duration,
                            int startTime,
                            int fadeInTime,
                            int leFlags,
                            qhandle_t hShader);
void CG_BubbleTrail(vec3_t start, vec3_t end, float spacing);
void CG_SpawnEffect(vec3_t org);
void CG_ScorePlum(int client, vec3_t org, int score);

void CG_GibPlayer(vec3_t playerOrigin);
void CG_BigExplode(vec3_t playerOrigin);

void CG_Bleed(vec3_t origin, int entityNum);

localEntity_t* CG_MakeExplosion(vec3_t origin, vec3_t dir,
                                qhandle_t hModel, qhandle_t shader, int msec,
                                qboolean isSprite);

//
// cg_snapshot.c
//
void CG_ProcessSnapshots(void);
void CG_TransitionEntity(centity_t* cent);

//
// cg_info.c
//
void CG_LoadingString(const char* s);
void CG_LoadingItem(int itemNum);
void CG_LoadingClient(int clientNum);
void CG_DrawInformation(void);

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard(void);
void CG_DrawOldTourneyScoreboard(void);
void CG_OSPShowStatsInfo(void);
qboolean CG_OSPDrawScoretable(void);

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand(void);
void CG_InitConsoleCommands(void);
void CG_OSPPrintTime_f(void);
void CG_OSPMoTD_f(void);
void CG_OSPWStatsDown_f(void);
void CG_OSPWStatsUp_f(void);

//
// cg_servercmds.c
//
#define CG_CHAT_COMMMON_ENABLED 1
#define CG_CHAT_TEAM_ENABLED 2
void CG_ExecuteNewServerCommands(int latestSequence);
void CG_ParseServerinfo(void);
void CG_SetConfigValues(void);
void CG_ShaderStateChanged(void);
void CG_RemoveChatEscapeChar(char* text);
void CG_RemoveChatEscapeCharAll(char* text);
void CG_StringMakeEscapeCharRAW(const char* in, char* out, int max);

//
// cg_playerstate.c
//
void CG_Respawn(void);
void CG_TransitionPlayerState(playerState_t* ps, playerState_t* ops);
void CG_CheckChangedPredictableEvents(playerState_t* ps);

//
// cg_customloc.c
//
extern qboolean customLocationsEnabled;
void CG_CustomLocationsLoad(void);
const char* CG_CustomLocationsGetName(const float* pos);
void CG_CustomLocationsSetLocation(const char* info, vec3_t loc);
qboolean CG_CustomLocationsTeamChatCode(const char* str, vec3_t cloc, char** cloc_begin, char** cloc_end);
void CG_CustomLocationsAddEntry(vec3_t pos, const char* str);
void CG_InitCTFLocations(void);
const char* CG_GetCTFLocation(int loc);

//
// cg_cvardescriptions.c
//
void CG_RegisterCvarDescriptions(void);
//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void        trap_Print(const char* fmt);

#ifndef Q3_VM

// Special functions to call from native VM. Fixes segmentation fault in Q3E.

// Use them as wrappers, pass acquired function address to `cmd` to make actual call.
int     trap_CG_GetValue_Q3E(int cmd, char* value, int valueSize, const char* key);
int     trap_CG_SetDescription_Q3E(int cmd, const char* name, const char* description);

#endif

// abort the game
void        trap_Error(const char* fmt);

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int         trap_Milliseconds(void);

// console variable interaction
void        trap_Cvar_Register(vmCvar_t* vmCvar, const char* varName, const char* defaultValue, int flags);
void        trap_Cvar_Update(vmCvar_t* vmCvar);
void        trap_Cvar_Set(const char* var_name, const char* value);
void        trap_Cvar_VariableStringBuffer(const char* var_name, char* buffer, int bufsize);

// ServerCommand and ConsoleCommand parameter access
int         trap_Argc(void);
void        trap_Argv(int n, char* buffer, int bufferLength);
void        trap_Args(char* buffer, int bufferLength);

// filesystem access
// returns length of file
int         trap_FS_FOpenFile(const char* qpath, fileHandle_t* f, fsMode_t mode);
void        trap_FS_Read(void* buffer, int len, fileHandle_t f);
void        trap_FS_Write(const void* buffer, int len, fileHandle_t f);
void        trap_FS_FCloseFile(fileHandle_t f);
int         trap_FS_Seek(fileHandle_t f, long offset, int origin);   // fsOrigin_t

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void        trap_SendConsoleCommand(const char* text);

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void        trap_AddCommand(const char* cmdName);

// send a string to the server over the network
void        trap_SendClientCommand(const char* s);

// force a screen update, only used during gamestate load
void        trap_UpdateScreen(void);

// model collision
void        trap_CM_LoadMap(const char* mapname);
int         trap_CM_NumInlineModels(void);
clipHandle_t trap_CM_InlineModel(int index);         // 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel(const vec3_t mins, const vec3_t maxs);
int         trap_CM_PointContents(const vec3_t p, clipHandle_t model);
int         trap_CM_TransformedPointContents(const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles);
void        trap_CM_BoxTrace(trace_t* results, const vec3_t start, const vec3_t end,
                             const vec3_t mins, const vec3_t maxs,
                             clipHandle_t model, int brushmask);
void        trap_CM_TransformedBoxTrace(trace_t* results, const vec3_t start, const vec3_t end,
                                        const vec3_t mins, const vec3_t maxs,
                                        clipHandle_t model, int brushmask,
                                        const vec3_t origin, const vec3_t angles);

// Returns the projection of a polygon onto the solid brushes in the world
int         trap_CM_MarkFragments(int numPoints, const vec3_t* points,
                                  const vec3_t projection,
                                  int maxPoints, vec3_t pointBuffer,
                                  int maxFragments, markFragment_t* fragmentBuffer);

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void        trap_S_StartSound(vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx);
void        trap_S_StopLoopingSound(int entnum);

// a local sound is always played full volume
void        trap_S_StartLocalSound(sfxHandle_t sfx, int channelNum);
void        trap_S_ClearLoopingSounds(qboolean killall);
void        trap_S_AddLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx);
void        trap_S_AddRealLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx);
void        trap_S_UpdateEntityPosition(int entityNum, const vec3_t origin);

// respatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void        trap_S_Respatialize(int entityNum, const vec3_t origin, vec3_t axis[3], int inwater);
sfxHandle_t trap_S_RegisterSound(const char* sample, qboolean compressed);       // returns buzz if not found
void        trap_S_StartBackgroundTrack(const char* intro, const char* loop);    // empty name stops music
void    trap_S_StopBackgroundTrack(void);


void        trap_R_LoadWorldMap(const char* mapname);

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t   trap_R_RegisterModel(const char* name);              // returns rgb axis if not found
qhandle_t   trap_R_RegisterSkin(const char* name);           // returns all white if not found
qhandle_t   trap_R_RegisterShader(const char* name);             // returns all white if not found
qhandle_t   trap_R_RegisterShaderNoMip(const char* name);            // returns all white if not found

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void        trap_R_ClearScene(void);
void        trap_R_AddRefEntityToScene(const refEntity_t* re);

// polys are intended for simple wall marks, not really for doing
// significant construction
void        trap_R_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t* verts);
void        trap_R_AddPolysToScene(qhandle_t hShader, int numVerts, const polyVert_t* verts, int numPolys);
void        trap_R_AddLightToScene(const vec3_t org, float intensity, float r, float g, float b);
int         trap_R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir);
void        trap_R_RenderScene(const refdef_t* fd);
void        trap_R_SetColor(const float* rgba);      // NULL = 1,1,1,1
void        trap_R_DrawStretchPic(float x, float y, float w, float h,
                                  float s1, float t1, float s2, float t2, qhandle_t hShader);
void        trap_R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs);
int         trap_R_LerpTag(orientation_t* tag, clipHandle_t mod, int startFrame, int endFrame,
                           float frac, const char* tagName);
void        trap_R_RemapShader(const char* oldShader, const char* newShader, const char* timeOffset);

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void        trap_GetGlconfig(glconfig_t* glconfig);

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void        trap_GetGameState(gameState_t* gamestate);

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void        trap_GetCurrentSnapshotNumber(int* snapshotNumber, int* serverTime);

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean    trap_GetSnapshot(int snapshotNumber, snapshot_t* snapshot);

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean    trap_GetServerCommand(int serverCommandNumber);

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int         trap_GetCurrentCmdNumber(void);

qboolean    trap_GetUserCmd(int cmdNumber, usercmd_t* ucmd);

// used for the weapon select and zoom
void        trap_SetUserCmdValue(int stateValue, float sensitivityScale);

// aids for VM testing
void        testPrintInt(char* string, int i);
void        testPrintFloat(char* string, float f);

int         trap_MemoryRemaining(void);
void        trap_R_RegisterFont(const char* fontName, int pointSize, fontInfo_t* font);
qboolean    trap_Key_IsDown(int keynum);
int         trap_Key_GetCatcher(void);
void        trap_Key_SetCatcher(int catcher);
int         trap_Key_GetKey(const char* binding);

int         trap_RealTime(qtime_t* qtime);

typedef enum
{
	SYSTEM_PRINT,
	CHAT_PRINT,
	TEAMCHAT_PRINT
} q3print_t; // bk001201 - warning: useless keyword or type name in empty declaration


int trap_CIN_PlayCinematic(const char* arg0, int xpos, int ypos, int width, int height, int bits);
e_status trap_CIN_StopCinematic(int handle);
e_status trap_CIN_RunCinematic(int handle);
void trap_CIN_DrawCinematic(int handle);
void trap_CIN_SetExtents(int handle, int x, int y, int w, int h);

void trap_SnapVector(float* v);

qboolean    trap_loadCamera(const char* name);
void        trap_startCamera(int time);
qboolean    trap_getCameraInfo(int time, vec3_t* origin, vec3_t* angles);

qboolean    trap_GetEntityToken(char* buffer, int bufferSize);

void    CG_ClearParticles(void);
void    CG_AddParticles(void);
void    CG_ParticleSnow(qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum);
void    CG_ParticleSmoke(qhandle_t pshader, centity_t* cent);
void    CG_AddParticleShrapnel(localEntity_t* le);
void    CG_ParticleSnowFlurry(qhandle_t pshader, centity_t* cent);
void    CG_ParticleBulletDebris(vec3_t org, vec3_t vel, int duration);
void    CG_ParticleSparks(vec3_t org, vec3_t vel, int duration, float x, float y, float speed);
void    CG_ParticleDust(centity_t* cent, vec3_t origin, vec3_t dir);
void    CG_ParticleMisc(qhandle_t pshader, vec3_t origin, int size, int duration, float alpha);
void    CG_ParticleExplosion(char* animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
extern qboolean     initparticles;
int CG_NewParticleArea(int num);

qboolean CG_DrawIntermission(void);
/*************************************************************************************************/
#define OSP_VERSION "0.06-test"


//
// cg_ospconfig.c
//
extern int modeShotgunPromode;
extern int modeShotgunNumberOfPellets;
extern float modeShotgunKoeff;
extern int wstatsWndId;
extern qboolean wstatsEnabled;
extern const char* weaponNames[10];
extern const char* weaponShortNames[10];
extern int global_viewlistFirstOption;
extern int statsInfo[24];

#define OSP_STATS_UNKNOWN0          0
#define OSP_STATS_SCORE             1
#define OSP_STATS_TEAM              2
#define OSP_STATS_KILLS             3
#define OSP_STATS_DEATHS            4
#define OSP_STATS_SUCIDES           5
#define OSP_STATS_TEAM_KILLS        6
#define OSP_STATS_DMG_TEAM          7
#define OSP_STATS_DMG_GIVEN         8
#define OSP_STATS_DMG_RCVD          9
#define OSP_STATS_WINS              10
#define OSP_STATS_LOSSES            11
#define OSP_STATS_CAPS              12
#define OSP_STATS_ASSIST            13
#define OSP_STATS_DEFENCES          14
#define OSP_STATS_RETURNS           15
#define OSP_STATS_TIME              16
#define OSP_STATS_MH                17
#define OSP_STATS_GA                18
#define OSP_STATS_RA                19
#define OSP_STATS_YA                20
#define OSP_STATS_WEAPON_MASK       21
#define OSP_STATS_UNKNOWN1          22
#define OSP_STATS_UNKNOWN2          23


// OSP Custom client
#define OSP_CUSTOM_CLIENT_ALT_WEAPON_FLAG      0x04
#define OSP_CUSTOM_CLIENT_OSP_HUD_FLAG         0x08
#define OSP_CUSTOM_CLIENT_TIMER_FLAG           0x10
#define OSP_CUSTOM_CLIENT_MAXFPS_FLAG          0x20

// OSP Custom client 2
#define OSP_CUSTOM_CLIENT_2_ENABLE_DMG_INFO    0x01
#define OSP_CUSTOM_CLIENT_2_IS_OPTION_ENABLED(BIT) (cgs.osp.custom_client_2 & BIT)
#define OSP_CUSTOM_CLIENT_2_IS_DMG_INFO_ALLOWED() OSP_CUSTOM_CLIENT_2_IS_OPTION_ENABLED(OSP_CUSTOM_CLIENT_2_ENABLE_DMG_INFO)

void CG_OSPCvarsRestrictValues(void);
qboolean CG_OSPIsGameTypeCA(int gametype);
qboolean CG_OSPIsGameTypeFreeze(void);
qboolean CG_OSPIsStatsHidden(qboolean check_gametype, qboolean check_warmup);
void CG_OSPUpdateUserInfo(qboolean arg);

void CG_OSPConfigSetMode(int value);
void CG_OSPConfigPmoveSet(int value);
void CG_OSPConfigMinMaxPacketsSet(int value);
void CG_OSPConfigMaxMaxPacketsSet(int value);
void CG_OSPConfigMinTimenudgeSet(int value);
void CG_OSPConfigMaxTimenudgeSet(int value);
void CG_OSPConfig0x368Set(qboolean value);
void CG_OSPConfigClanBaseTDMSet(qboolean value);
void CG_OSPConfigCustomClientSet(int value);
void CG_OSPConfigCustomClient2Set(int value);
void CG_OSPConfigModeSet(int value);
void CG_OSPConfigFreezeModeSet(int value);

qboolean CG_IsSpectator(void);
qboolean CG_IsFollowing(void);

//
//cg_osphud.c
//
extern int teamOverlayWidth;
void CG_OSPHUDRoutine(void);
float CG_OSPDrawPing(float y);

//
//cg_osphud.c
//
void CG_DrawCrosshair(void);

//
// cg_osputil.c
//
void CG_OSPNormalizeText(char* src, int size, char* dst);
int CG_OSPGetNormalizedStringSize(const char* str);
void CG_OSPColorsFromString(const char* s, float* a, float* b, float* c, float* d);
void CG_OSPColorFromChar(char c, float* vector);
void CG_OSPColorFromNumber(int number, float* vector);
void CG_OSPNormalizeNameCopy(char* from, char* to, unsigned int size);
void CG_DynamicMemReport(void);
const char* CG_LoadLine(const char* ptr, char* out, int outSize);
qboolean CG_ParseColorStr(const char* str, vec4_t out);


/*
 * Check memory after allocation
 * There is qvm shutdown in CG_Error, but infinitie loop needed to make static analyzers happy
 */
#define OSP_MEMORY_CHECK(TESTPTR) \
if(!TESTPTR){\
    CG_DynamicMemReport();\
    CG_Error( "%s:%d: Couldn't allocate memory\n", __FILE__, __LINE__);\
}

//
// cg_unlagged.c
//
void CG_PredictWeaponEffects(centity_t* cent);

//
// cg_playerscolors.c
//
void CG_RebuildPlayerColors(void);
void CG_PlayerColorsLoadDefault(playerColors_t* colors);
void CG_ClientInfoUpdateColors(clientInfo_t* ci, int clientNum);
void CG_PlayerColorsFromCS(playerColors_t* colors, playerColorsOverride_t* override, const char* color1, const char* color2);
//
// cg_chatfilter.c
//
#define CG_CHATFILTER_DEFAULT_FILE "chatfilter"
typedef enum
{
	MESSAGE_NOTALLOWED,
	MESSAGE_ALLOWED_PLAYER,
	MESSAGE_ALLOWED_OTHER,
} messageAllowed_t;
messageAllowed_t CG_ChatCheckMessageAllowed(const char* message);
void CG_ChatfilterLoadFile(const char* filename);
void CG_ChatfilterSaveFile(const char* filename);
void CG_ChatfilterAddName(const char* name);
void CG_ChatfilterDeleteName(const char* name);
qboolean CG_ChatfilterIsNameMuted(const char* name);
void CG_ChatfilterDump(void);

//
// cg_localevents.c
//
void CG_LocalEventCvarChanged_cg_drawTeamOverlay(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cl_maxpackets(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cl_timenudge(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_ch_recordMessage(cvarTable_t* cvart);

void CG_LocalEventCvarChanged_cg_followpowerup(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_followkiller(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_followviewcam(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_autoAction(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_snaps(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_useScreenShotJPEG(cvarTable_t* cvart);

void CG_LocalEventCvarChanged_r_lodCurveError(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_r_subdivisions(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_r_znear(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_trueLightning(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_r_shownormals(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_r_showtris(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_shadows(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_r_fullbright(cvarTable_t* cvart);

void CG_LocalEventCvarChanged_cg_swapSkins(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_enemyColors(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_enemyModel(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_teamColors(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_teamModel(cvarTable_t* cvart);

void CG_LocalEventCvarChanged_handicap(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_s_ambient(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_pmove_fixed(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_hitSounds(cvarTable_t* cvart);

void CG_LocalEventCvarChanged_cg_playerModelColors(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_playerRailColors(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_playerFrozenColor(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_teamModelColors(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_teamRailColors(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_teamFrozenColor(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_enemyModelColors(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_enemyRailColors(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_enemyFrozenColor(cvarTable_t* cvart);

void CG_LocalEventCvarChanged_cg_fragSound(cvarTable_t* cvart);

void CG_LocalEventCvarChanged_ch_file(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_shud(cvarTable_t* cvart);

void CG_LocalEventCvarChanged_ch_crosshairColor(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_ch_crosshairActionColor(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_ch_crosshairDecorColor(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_ch_crosshairDecorActionColor(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_ch_crosshairDecorOpaque(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_ch_crosshairOpaque(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_ch_crosshairActionScale(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_ch_crosshairDecorActionScale(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_ch_crosshairActionTime(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_ch_crosshairDecorActionTime(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_damageIndicatorOpaque(cvarTable_t* cvart);

void CG_LocalEventCvarChanged_cg_dlightGauntlet(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_dlightMG(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_dlightSG(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_dlightGL(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_dlightRL(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_dlightLG(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_dlightRG(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_dlightPG(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_dlightBFG(cvarTable_t* cvart);
void CG_LocalEventCvarChanged_cg_conObituaries(cvarTable_t* cvart);
#ifdef __cplusplus
}
#endif
#endif //ifndef CG_LOCAL_H
