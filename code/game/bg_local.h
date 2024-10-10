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
// bg_local.h -- local definitions for the bg (both games) files
#ifdef __cplusplus
extern "C" {
#endif

#define MIN_WALK_NORMAL 0.7f        // can't walk on very steep slopes

#define STEPSIZE        18

#define JUMP_VELOCITY   270

#define TIMER_LAND      130
#define TIMER_GESTURE   (34*66+50)

#define OVERCLIP        1.001f

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct
{
	vec3_t      forward, right, up;
	float       frametime;

	int         msec;

	qboolean    walking;
	qboolean    groundPlane;
	trace_t     groundTrace;

	float       impactSpeed;

	vec3_t      previous_origin;
	vec3_t      previous_velocity;
	int         previous_waterlevel;
} pml_t;

extern  pmove_t*     pm;//-V707
extern  pml_t       pml;

// movement parameters
extern  float   pm_stopspeed;
extern  float   pm_duckScale;
extern  float   pm_swimScale;
extern  float   pm_wadeScale;

extern  float   pm_accelerate;
extern  float   pm_airaccelerate;
extern  float   pm_wateraccelerate;
extern  float   pm_flyaccelerate;

extern  float   pm_friction;
extern  float   pm_waterfriction;
extern  float   pm_flightfriction;

extern  int     c_pmove;

extern int modePromodePhysKoeff; //1870ec
extern float modePromode_pm_airaccelerate_1; //3450
extern int modePredictionKoeff2;   //3454
extern float modePromode_pm_airaccelerate_2; //3458
extern float modeWishspeedLimit; //345c
extern int modePredictionKoeff1;   //3460
extern float modeSwimScale1;         //3468
extern float modeSwimScale2;         //346c
extern float modeShotgunKoeff;    //3494
extern int modeShotgunNumberOfPellets;    //3498
extern float modeUnused8;    //349c
extern int modeMaxAmmoShotgun;    //34a0
extern int modeGrenadeTime;    //34a4
extern int modeMaxAmmoGrenade;    //34a8
extern int modeMaxAmmoRocket;     //34b0
extern int modeMaxAmmoRail;       //34c0
extern int modeBeginWeaponChangeTime;    //34d0
extern int modeFinishWeaponChangeTime;    //34d4
extern int modePMNoAmmoTime;    //34d8
extern int pm_armorPromode;    //34e4
extern int modeHitLevelSounds;    //3514
extern int modePickupDistance;    //20e8
extern int modeUnknown2;    //3528
extern int modeUnknown3;    //352c
extern int modeUnknown4;    //3530
extern int modeShotgunPromode;    //3514

void PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce);
void PM_ClipVelocityOSP(vec3_t in, vec3_t normal, vec3_t out, float overbounce);
void PM_AddTouchEnt(int entityNum);
void PM_AddEvent(int newEvent);

qboolean    PM_SlideMove(qboolean gravity);
void        PM_StepSlideMove(qboolean gravity);


#ifdef __cplusplus
}
#endif
