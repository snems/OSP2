/*
===========================================================================
Copyright (C) 2006 Neil Toronto.

This file is part of the Unlagged source code.

Unlagged source code is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Unlagged source code is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Unlagged source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "cg_local.h"


/*
=======================
CG_PredictWeaponEffects

Draws predicted effects for the railgun, shotgun, and machinegun.  The
lightning gun is done in CG_LightningBolt, since it was just a matter
of setting the right origin and angles.
=======================
*/
void CG_PredictWeaponEffects(centity_t* cent)
{
	entityState_t* ent = &cent->currentState;

	// if the client isn't us, forget it
	if (cent->currentState.number != cg.predictedPlayerState.clientNum)
	{
		return;
	}


	// was it a rail attack?
	if (ent->weapon == WP_RAILGUN && (cg_delag.integer & 1 || cg_delag.integer & 4) && !cg.demoPlayback)
	{
		vec3_t      muzzlePoint, forward, right, up;
		trace_t trace;
		vec3_t endPoint;
		// get the muzzle point
		VectorCopy(cg.predictedPlayerState.origin, muzzlePoint);
		muzzlePoint[2] += cg.predictedPlayerState.viewheight;

		// get forward, right, and up
		AngleVectors(cg.predictedPlayerState.viewangles, forward, right, up);
		VectorMA(muzzlePoint, 14, forward, muzzlePoint);
		// do we have it on for the rail gun?

		// trace forward
		VectorMA(muzzlePoint, 8192, forward, endPoint);


		// find the rail's end point
		CG_Trace(&trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, cg.predictedPlayerState.clientNum, CONTENTS_SOLID);

		// do the magic-number adjustment
		VectorMA(muzzlePoint, 4, right, muzzlePoint);
		VectorMA(muzzlePoint, -1, up, muzzlePoint);

		// draw a rail trail
		CG_RailTrail(&cgs.clientinfo[cent->currentState.clientNum], muzzlePoint, trace.endpos);
		//Com_Printf( "Predicted rail trail\n" );

		// explosion at end if not SURF_NOIMPACT
		if (!(trace.surfaceFlags & SURF_NOIMPACT))
		{
			// predict an explosion
			CG_MissileHitWall(ent->weapon, cg.predictedPlayerState.clientNum, trace.endpos, trace.plane.normal, IMPACTSOUND_DEFAULT);
		}
	}
	// do not delag shotgun and machinegun for comaptibility to old OSP servers
}

/*
================
CG_Cvar_ClampInt

Clamps a cvar between two integer values, returns qtrue if it had to.
================
*/
qboolean CG_Cvar_ClampInt(const char* name, vmCvar_t* vmCvar, int min, int max)
{
	if (vmCvar->integer > max)
	{
		CG_Printf("Allowed values are %d to %d.\n", min, max);

		Com_sprintf(vmCvar->string, MAX_CVAR_VALUE_STRING, "%d", max);
		vmCvar->value = max;
		vmCvar->integer = max;

		trap_Cvar_Set(name, vmCvar->string);
		return qtrue;
	}

	if (vmCvar->integer < min)
	{
		CG_Printf("Allowed values are %d to %d.\n", min, max);

		Com_sprintf(vmCvar->string, MAX_CVAR_VALUE_STRING, "%d", min);
		vmCvar->value = min;
		vmCvar->integer = min;

		trap_Cvar_Set(name, vmCvar->string);
		return qtrue;
	}

	return qfalse;
}
