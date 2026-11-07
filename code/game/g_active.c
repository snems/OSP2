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

#include "g_local.h"


/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback(gentity_t* player) {
	gclient_t*   client;
	float   count;
	vec3_t  angles;

	client = player->client;
	if (client->ps.pm_type == PM_DEAD) {
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if (count == 0) {
		return;     // didn't take any damage
	}

	if (count > 255) {
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if (client->damage_fromWorld) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} else {
		vectoangles(client->damage_from, angles);
		client->ps.damagePitch = angles[PITCH]/360.0 * 256;
		client->ps.damageYaw = angles[YAW]/360.0 * 256;
	}

	// play an apropriate pain sound
	if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE)) {
		player->pain_debounce_time = level.time + modePainDebounce;
		G_AddEvent(player, EV_PAIN, player->health);
		client->ps.damageEvent++;
	}


	client->ps.damageCount = count;

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects(gentity_t* ent) {
	qboolean    envirosuit;
	int         waterlevel;

	if (ent->client->noclip) {
		ent->client->airOutTime = level.time + 12000;   // don't need air
		return;
	}

	waterlevel = ent->waterlevel;

	envirosuit = ent->client->ps.powerups[PW_BATTLESUIT] > level.time;

	//
	// check for drowning
	//
	if (waterlevel == 3) {
		// envirosuit give air
		if (envirosuit) {
			ent->client->airOutTime = level.time + 10000;
		}

		// if out of air, start drowning
		if (ent->client->airOutTime < level.time) {
			// drown!
			ent->client->airOutTime += 1000;
			if (ent->health > 0) {
				// take more damage the longer underwater
				ent->damage += 2;
				if (ent->damage > 15)
					ent->damage = 15;

				// play a gurp sound instead of a normal pain sound
				if (ent->health <= ent->damage) {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("*drown.wav"));
				} else if (rand()&1) {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp1.wav"));
				} else {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp2.wav"));
				}

				// don't play a normal pain sound
				ent->pain_debounce_time = level.time + 200;

				G_Damage(ent, NULL, NULL, NULL, NULL,
				         ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	} else {
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if (waterlevel &&
	        (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME))) {
		if (ent->health > 0
		        && ent->pain_debounce_time <= level.time) {

			if (envirosuit) {
				G_AddEvent(ent, EV_POWERUP_BATTLESUIT, 0);
			} else {
				if (ent->watertype & CONTENTS_LAVA) {
					G_Damage(ent, NULL, NULL, NULL, NULL,
					         modeLavaDamage * waterlevel, 0, MOD_LAVA);
				}

				if (ent->watertype & CONTENTS_SLIME) {
					G_Damage(ent, NULL, NULL, NULL, NULL,
					         modeSlimeDamage * waterlevel, 0, MOD_SLIME);
				}
			}
		}
	}
}



/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound(gentity_t* ent) {
#ifdef MISSIONPACK
	if (ent->s.eFlags & EF_TICKING) {
		ent->client->ps.loopSound = G_SoundIndex("sound/weapons/proxmine/wstbtick.wav");
	}
	else
#endif
		if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME))) {
			ent->client->ps.loopSound = level.snd_fry;
		} else {
			ent->client->ps.loopSound = 0;
		}
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts(gentity_t* ent, pmove_t* pm) {
	int     i, j;
	trace_t trace;
	gentity_t*   other;

	memset(&trace, 0, sizeof(trace));
	for (i=0 ; i<pm->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pm->touchents[j] == pm->touchents[i]) {
				break;
			}
		}
		if (j != i) {
			continue;   // duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		if ((ent->r.svFlags & SVF_BOT) && (ent->touch)) {
			ent->touch(ent, other, &trace);
		}

		if (!other->touch) {
			continue;
		}

		other->touch(other, ent, &trace);
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void    G_TouchTriggers(gentity_t* ent) {
	int         i, num;
	int         touch[MAX_GENTITIES];
	gentity_t*   hit;
	trace_t     trace;
	vec3_t      mins, maxs;
	static vec3_t   range = { 40, 40, 52 };

	if (!ent->client) {
		return;
	}

	// dead clients don't activate triggers!
	if (ent->client->ps.stats[STAT_HEALTH] <= 0) {
		return;
	}

	VectorSubtract(ent->client->ps.origin, range, mins);
	VectorAdd(ent->client->ps.origin, range, maxs);

	num = trap_EntitiesInBox(mins, maxs, touch, MAX_GENTITIES);

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd(ent->client->ps.origin, ent->r.mins, mins);
	VectorAdd(ent->client->ps.origin, ent->r.maxs, maxs);

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];

		if (!hit->touch && !ent->touch) {
			continue;
		}
		if (!(hit->r.contents & CONTENTS_TRIGGER)) {
			continue;
		}

		// ignore most entities if a spectator
		if (is_spectator(ent->client)) {
			if (hit->s.eType != ET_TELEPORT_TRIGGER &&
			        // this is ugly but adding a new ET_? type will
			        // most likely cause network incompatibilities
			        hit->touch != Touch_DoorTrigger) {
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if (hit->s.eType == ET_ITEM) {
			if (!BG_PlayerTouchesItem(&ent->client->ps, &hit->s, level.time)) {
				continue;
			}
		} else {
			if (!trap_EntityContact(mins, maxs, hit)) {
				continue;
			}
		}

		memset(&trace, 0, sizeof(trace));

		if (hit->touch) {
			hit->touch(hit, ent, &trace);
		}

		if ((ent->r.svFlags & SVF_BOT) && (ent->touch)) {
			ent->touch(ent, hit, &trace);
		}
	}

	// if we didn't touch a jump pad this pmove frame
	if (ent->client->ps.jumppad_frame != ent->client->ps.pmove_framecount) {
		ent->client->ps.jumppad_frame = 0;
		ent->client->ps.jumppad_ent = 0;
	}
}

/*
=================
SpectatorThink
=================
*/
// OSP: spectators who are not referees and have no invitation can't watch a
// spec-locked team (sess.specInvite: bit 1 red, bit 2 blue invitation)
#define OSP_SPECLOCKED( client ) \
    ( !client->sess.referee \
    && ( ( ( client->sess.specInvite == 0 || client->sess.specInvite == 2 ) && osp_teamInfo[TEAM_RED].specLocked ) \
    || ( client->sess.specInvite < 2 && osp_teamInfo[TEAM_BLUE].specLocked ) ) )

void SpectatorThink(gentity_t* ent, usercmd_t* ucmd) {
	pmove_t pm;
	gclient_t*   client;

	client = ent->client;

	// OSP's spectator states: 4 viewcam following a player, 5 viewcam,
	// 6 locked-out view, 7 multiview
	if (client->sess.spectatorState == 5) {
		client->ps.pm_type = PM_FREEZE;
		client->ps.pm_flags |= PMF_FOLLOW;
		client->ps.eFlags &= ~EF_DEAD;
		client->ps.stats[STAT_HEALTH] = 100;

		// set up for pmove
		memset(&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;   // spectators can fly through bodies
		if (g_dmflags.integer & 512) {
			pm.tracemask &= ~CONTENTS_PLAYERCLIP;
		}
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		// perform a pmove
		Pmove(&pm);
		OSP_ViewcamThink(ent, 0);

		if (OSP_SPECLOCKED(client)) {
			SetTeam(ent, "s");
			return;
		}
		if (ent->client->pers.viewcamInfo && ent->client->pers.viewcamInfoLast != ent->client->pers.viewcam) {
			trap_SendServerCommand(ent - g_entities, va("print \"^5vc_info: ^3(%d) %.3f %.3f %.3f %.3f %.3f [%d]\n\"",
			                       client->pers.viewcam, client->ps.origin[0], client->ps.origin[1], client->ps.origin[2],
			                       client->ps.viewangles[0], client->ps.viewangles[1], osp_viewcams[client->pers.viewcam].range));
		}
		if (client->sess.spectatorClient >= 0 && client->sess.spectatorClient < level.maxclients) {
			client->ps.generic1 = client->sess.spectatorClient + 32;
		}
		if (ucmd->upmove && client->pers.osp_unk_2bc < level.time) {
			client->oldbuttons = client->buttons;
			client->buttons = ucmd->buttons;
			client->pers.osp_unk_2bc = level.time + 750;
			if (ucmd->upmove > 0) {
				OSP_ViewcamCyclePlayer(ent, 1);
			} else if (ucmd->upmove < 0) {
				OSP_ViewcamCyclePlayer(ent, -1);
			}
			return;
		}
		if ((client->buttons & BUTTON_USE_HOLDABLE) && !(client->oldbuttons & BUTTON_USE_HOLDABLE)
		        && client->sess.spectatorTime + 750 < level.time && client->pers.osp_unk_2bc < level.time) {
			client->oldbuttons = client->buttons;
			client->buttons = ucmd->buttons;
			client->pers.osp_unk_2bc = level.time + 750;
			ent->client->pers.viewcamStatic = 1 - ent->client->pers.viewcamStatic;
			trap_SendServerCommand(ent - g_entities, va("print \"^3ViewCam view is now ^5%s\n",
			                       ent->client->pers.viewcamStatic ? "STATIC" : "DYNAMIC"));
			return;
		}
	} else if (client->sess.spectatorState == 4) {
		int     cam;
		vec3_t  dir;

		client->ps.pm_type = PM_FREEZE;
		client->ps.pm_flags |= PMF_FOLLOW;
		client->ps.eFlags &= ~EF_DEAD;
		client->ps.stats[STAT_HEALTH] = 100;

		// set up for pmove
		memset(&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;   // spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		// perform a pmove
		Pmove(&pm);

		if (level.numPlayingClients && !ent->client->pers.viewcamFree) {
			cam = osp_viewcamCurrent;
		} else {
			cam = ent->client->pers.viewcam;
		}
		if (cam >= 0 && osp_viewcams[cam].target >= 0) {
			if (cam != ent->client->pers.viewcam) {
				VectorCopy(osp_viewcams[cam].origin, ent->s.origin);
				VectorCopy(osp_viewcams[cam].origin, ent->client->ps.origin);
				ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
				ent->client->pers.viewcam = cam;
			}
			VectorSubtract(level.clients[osp_viewcams[cam].target].ps.origin, osp_viewcams[cam].origin, dir);
			if (ent->client->pers.viewcamStatic) {
				VectorCopy(osp_viewcams[ent->client->pers.viewcam].angles, ent->client->ps.viewangles);
			} else {
				vectoangles(dir, ent->client->ps.viewangles);
			}
		} else {
			if (ent->client->pers.viewcamStatic) {
				VectorCopy(osp_viewcams[ent->client->pers.viewcam].angles, ent->client->ps.viewangles);
			} else {
				VectorCopy(osp_viewcams[ent->client->pers.viewcam].viewAngles, ent->client->ps.viewangles);
			}
		}

		if (OSP_SPECLOCKED(client)) {
			SetTeam(ent, "s");
			return;
		}
		if (ent->client->pers.viewcamInfo && ent->client->pers.viewcamInfoLast != cam) {
			ent->client->pers.viewcamInfoLast = cam;
			trap_SendServerCommand(ent - g_entities, va("print \"^5vc_info: ^3(%d) %.3f %.3f %.3f %.3f %.3f [%d]\n\"",
			                       osp_viewcams[cam].id, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2],
			                       ent->client->ps.viewangles[0], ent->client->ps.viewangles[1], osp_viewcams[cam].range));
		}
		if (client->sess.spectatorClient >= 0 && client->sess.spectatorClient < level.maxclients) {
			client->ps.generic1 = client->sess.spectatorClient + 32;
		}
		if (ucmd->upmove && client->pers.osp_unk_2bc < level.time) {
			client->oldbuttons = client->buttons;
			client->buttons = ucmd->buttons;
			client->pers.osp_unk_2bc = level.time + 750;
			if (ucmd->upmove > 0 && (client->pers.viewcamFree || !level.numPlayingClients)) {
				OSP_ViewcamCyclePosition(ent, 1);
				return;
			}
			if (client->pers.viewcamFree || !level.numPlayingClients) {
				OSP_ViewcamCyclePosition(ent, -1);
			}
			return;
		}
		if ((client->buttons & BUTTON_USE_HOLDABLE) && !(client->oldbuttons & BUTTON_USE_HOLDABLE)
		        && client->sess.spectatorTime + 750 < level.time && client->pers.osp_unk_2bc < level.time) {
			client->oldbuttons = client->buttons;
			client->buttons = ucmd->buttons;
			client->pers.osp_unk_2bc = level.time + 750;
			ent->client->pers.viewcamStatic = 1 - ent->client->pers.viewcamStatic;
			trap_SendServerCommand(ent - g_entities, va("print \"^3ViewCam view is now ^5%s\n",
			                       ent->client->pers.viewcamStatic ? "STATIC" : "DYNAMIC"));
			return;
		}
	} else if (client->sess.spectatorState == SPECTATOR_FREE) {
		client->ps.pm_type = PM_SPECTATOR;
		client->ps.speed = 400; // faster than normal

		// set up for pmove
		memset(&pm, 0, sizeof(pm));
		client->ps.generic1 = 0;
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;   // spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		// perform a pmove
		Pmove(&pm);
		// save results of pmove
		VectorCopy(client->ps.origin, ent->s.origin);

		G_TouchTriggers(ent);
		if (client->sess.coachTeam) {
			OSP_Cmd_Coach_f(ent, 0);
		}
		trap_UnlinkEntity(ent);

		if (OSP_SPECLOCKED(client)) {
			SetTeam(ent, "s");
			return;
		}
	} else if (client->sess.spectatorState == SPECTATOR_FOLLOW) {
		int     buttons, oldbuttons;

		oldbuttons = client->buttons;
		buttons = ucmd->buttons;
		client->ps.generic1 = 0;
		if (!client->sess.referee && client->sess.sessionTeam == TEAM_SPECTATOR) {
			if (osp_teamInfo[TEAM_RED].specLocked && level.clients[client->sess.spectatorClient].sess.sessionTeam == TEAM_RED
			        && (client->sess.specInvite == 0 || client->sess.specInvite == 2)) {
				SetTeam(ent, "s");
				return;
			}
			if (osp_teamInfo[TEAM_BLUE].specLocked && level.clients[client->sess.spectatorClient].sess.sessionTeam == TEAM_BLUE
			        && client->sess.specInvite < 2) {
				SetTeam(ent, "s");
				return;
			}
		}
		if (!client->sess.coachTeam && !client->pers.mvSavedCount && (buttons & BUTTON_ATTACK) && !(oldbuttons & BUTTON_ATTACK)
		        && ((osp_teamInfo[TEAM_RED].specLocked && (client->sess.specInvite == 0 || client->sess.specInvite == 2))
		            || (osp_teamInfo[TEAM_BLUE].specLocked && client->sess.specInvite < 2))) {
			return;
		}
		if (ucmd->upmove && client->pers.osp_unk_2bc < level.time) {
			client->oldbuttons = client->buttons;
			client->buttons = ucmd->buttons;
			client->pers.osp_unk_2bc = level.time + 750;
			Cmd_FollowCycle_f(ent, (ucmd->upmove > 0) ? 1 : -1);
			return;
		}
		if (client->sess.sessionTeam != TEAM_SPECTATOR) {
			return;
		}
	} else if (client->sess.spectatorState == 6) {
		client->ps.speed = 0;
		client->ps.pm_type = PM_FREEZE;
		client->ps.pm_flags |= PMF_FOLLOW;
		client->ps.generic1 = 0;

		// set up for pmove
		memset(&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;   // spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		// perform a pmove
		Pmove(&pm);

		if (client->sess.referee || client->sess.specInvite > 2
		        || (!osp_teamInfo[TEAM_RED].specLocked && !osp_teamInfo[TEAM_BLUE].specLocked)) {
			SetTeam(ent, "s");
			return;
		}
		if (osp_numViewcams) {
			VectorCopy(osp_viewcams[0].origin, ent->s.origin);
			VectorCopy(osp_viewcams[0].origin, client->ps.origin);
			VectorCopy(osp_viewcams[0].angles, client->ps.viewangles);
		} else {
			VectorCopy(level.intermission_origin, ent->s.origin);
			VectorCopy(level.intermission_origin, client->ps.origin);
			VectorCopy(level.intermission_angle, client->ps.viewangles);
		}
		client->ps.viewangles[0] += 180;
		client->ps.viewangles[2] += 180;
		client->ps.eFlags ^= EF_TELEPORT_BIT;
		if (!ent->client->sess.specInvite && osp_teamInfo[TEAM_RED].specLocked && osp_teamInfo[TEAM_BLUE].specLocked) {
			return;
		}
	} else if (client->sess.spectatorState == 7) {
		client->ps = level.clients[ent - g_entities].ps;
		client->ps.pm_type = PM_FREEZE;
		client->ps.eFlags &= ~EF_DEAD;
		client->ps.stats[STAT_HEALTH] = 100;
		client->ps.generic1 = 0;

		// set up for pmove
		memset(&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;   // spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		// perform a pmove
		Pmove(&pm);
		client->ps.eFlags ^= EF_TELEPORT_BIT;

		if ((level.time / 1000) % 7 == 0 && client->pers.osp_unk_2bc != level.time / 1000) {
			int     i;
			char    buf[256];

			client->pers.osp_unk_2bc = level.time / 1000;
			trap_SendServerCommand(ent - g_entities, "cp \" \n \nMulti-view demo created with\nOSP client version >= 0.99v\n \n^3http://www.OrangeSmoothie.org/\n\"");
			strcpy(buf, va("viewlist %d", client->pers.mvCount));
			for (i = 0 ; i < client->pers.mvCount ; i++) {
				strcat(buf, va(" %d", client->pers.mvClients[i]));
			}
			trap_SendServerCommand(ent - g_entities, va("%s\n", buf));
			trap_SendServerCommand(ent - g_entities, va("viewactive %d\n", client->pers.mvActive));
		}
		if (!client->pers.mvCount) {
			return;
		}

		client->oldbuttons = client->buttons;
		client->buttons = ucmd->buttons;
		if ((client->buttons & BUTTON_ATTACK) && !(client->oldbuttons & BUTTON_ATTACK)
		        && client->sess.spectatorTime + 750 < level.time && client->pers.osp_unk_2bc < level.time) {
			client->pers.osp_unk_2bc = level.time + 750;
			client->sess.spectatorTime = level.time;
			OSP_Cmd_ViewFollow_f(ent);
			trap_SendServerCommand(ent - g_entities, "print \"^5CHASECAM:^3  Hit ^7FIRE^3 to return to multi-view.\n");
			return;
		}
		OSP_MV_UpdatePortal(ent);
		return;
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	// attack button cycles through chasecam, viewcam and freefloat
	if ((client->buttons & BUTTON_ATTACK) && !(client->oldbuttons & BUTTON_ATTACK)
	        && client->sess.spectatorTime + 750 < level.time && client->pers.osp_unk_2bc < level.time) {
		int     state;

		state = client->sess.spectatorState;
		if (client->sess.coachTeam || client->pers.mvSavedCount) {
			OSP_Cmd_ViewFollow_f(ent);
			return;
		}
		client->pers.osp_unk_2bc = level.time + 750;

		if (level.numPlayingClients && (state == SPECTATOR_FREE || state == 6)) {
			if (state == SPECTATOR_FREE && osp_numViewcams > 0) {
				trap_SendServerCommand(ent - g_entities, "print \"^5CHASECAM:^3  Hit ^7JUMP^3 to cycle, ^7FIRE^3 to go to ViewCam.\n");
			} else {
				trap_SendServerCommand(ent - g_entities, "print \"^5CHASECAM:^3  Hit ^7JUMP^3 to cycle.\n");
			}
			Cmd_FollowCycle_f(ent, 1);
			return;
		}

		if (osp_numViewcams > 0 && (state == SPECTATOR_FOLLOW || (state == SPECTATOR_FREE && !level.numPlayingClients))) {
			if (ent->client->pers.followViewcam && level.numPlayingClients) {
				if (--ent->client->sess.spectatorClient < 0) {
					ent->client->sess.spectatorClient = level.maxclients - 1;
				}
				trap_SendServerCommand(ent - g_entities, "print \"^5VIEWCAM:^3  Hit ^7FIRE^3 to go to FreeFloat, ^7JUMP^3 to cycle through players.\n");
				OSP_ViewcamCyclePlayer(ent, 1);
				return;
			}
			osp_numViewcamClients++;
			trap_SendServerCommand(ent - g_entities, "print \"^5VIEWCAM:^3  Hit ^7FIRE^3 to go to FreeFloat, ^7JUMP^3 to cycle positions.\n");
			OSP_ViewcamCyclePosition(ent, 0);
			return;
		}

		if (state == SPECTATOR_SCOREBOARD || state == SPECTATOR_FREE) {
			return;
		}
		if (state == 4) {
			osp_numViewcamClients--;
		}
		if (level.numPlayingClients) {
			trap_SendServerCommand(ent - g_entities, "print \"^5FREEFLOAT:^3  Hit ^7FIRE^3 to go to ChaseCam.\n");
		} else if (state != 6) {
			if (osp_numViewcams > 0) {
				trap_SendServerCommand(ent - g_entities, "print \"^5FREEFLOAT:^3  Hit ^7FIRE^3 to go to ViewCam.\n");
			} else {
				trap_SendServerCommand(ent - g_entities, "print \"^5FREEFLOAT:^3  Viewing the level.\n");
			}
		}
		StopFollowing(ent);
	}
}



/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer(gclient_t* client) {
	// OSP has a separate spectator timeout and never drops players while
	// fewer than two can vote, during a pause, or referees / OSP-flagged clients
	if ((!g_inactivity.integer && client->sess.sessionTeam != TEAM_SPECTATOR)
	        || (!g_spectatorInactivity.integer && client->sess.sessionTeam == TEAM_SPECTATOR)
	        || level.numVotingClients < 2
	        || client->sess.coachTeam || client->sess.referee
	        || g_entities[client->ps.clientNum].frozen || osp_matchPaused) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityWarning = qfalse;
		client->inactivityTime = level.time + 60 * 1000;
	} else if (client->pers.cmd.forwardmove ||
	           client->pers.cmd.rightmove ||
	           client->pers.cmd.upmove ||
	           (client->pers.cmd.buttons & BUTTON_ATTACK)) {
		client->inactivityWarning = qfalse;
		client->inactivityTime = level.time + 1000 * ((client->sess.sessionTeam != TEAM_SPECTATOR) ?
		                         g_inactivity.integer : g_spectatorInactivity.integer);
	} else if (level.time > client->inactivityTime && client->inactivityWarning) {
		client->inactivityWarning = qfalse;
		client->inactivityTime = level.time + 60 * 1000;
		trap_DropClient(client - level.clients, "Dropped due to inactivity");
		return qfalse;
	} else if (!client->inactivityWarning && level.time > client->inactivityTime - 10000) {
		client->inactivityWarning = qtrue;
		trap_SendServerCommand(client - level.clients, "cp \"^310 seconds until inactivity drop!\n\"");
		trap_SendServerCommand(client - level.clients, "print \"^310 seconds until inactivity drop!\n\"");
		G_Printf("10s inactivity warning issued to: %s\n", client->pers.netname);
		client->inactivityTime = level.time + 10000;
	}
	return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions(gentity_t* ent, int msec) {
	gclient_t*   client;

	client = ent->client;
	client->timeResidual += msec;

	while (client->timeResidual >= 1000) {
		client->timeResidual -= 1000;

		// regenerate
		// OSP logs the health and armor changes (log_stat)
		if (client->ps.powerups[PW_REGEN]) {
			if (ent->health < client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health += 15;
				if (ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1) {
					ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
				}
				G_AddEvent(ent, EV_POWERUP_REGEN, 0);
				if (log_stat.integer && !level.warmupTime) {
					G_LogPrintf("PS: %i %i %i -1 REGENERATION\n", ent->s.number, ent->health, ent->client->ps.stats[STAT_ARMOR]);
				}
			} else if (ent->health < client->ps.stats[STAT_MAX_HEALTH] * 2) {
				ent->health += 5;
				if (ent->health > client->ps.stats[STAT_MAX_HEALTH] * 2) {
					ent->health = client->ps.stats[STAT_MAX_HEALTH] * 2;
				}
				G_AddEvent(ent, EV_POWERUP_REGEN, 0);
				if (log_stat.integer && !level.warmupTime) {
					G_LogPrintf("PS: %i %i %i -1 REGENERATION\n", ent->s.number, ent->health, ent->client->ps.stats[STAT_ARMOR]);
				}
			}
		} else {
			// count down health when over max
			if (ent->health > client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health--;
				if (log_stat.integer && !level.warmupTime) {
					G_LogPrintf("PS: %i %i %i -1 HEALTH_MAX\n", ent->s.number, ent->health, ent->client->ps.stats[STAT_ARMOR]);
				}
			}
		}

		// count down armor when over max (not in clan arena, Q2 style or promode armor)
		if (client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH]
		        && g_gametype.integer != 5 && !armor_q2style.integer && !pm_armorPromode) {
			client->ps.stats[STAT_ARMOR]--;
			if (log_stat.integer && !level.warmupTime) {
				G_LogPrintf("PS: %i %i %i -1 ARMOR_MAX\n", ent->s.number, ent->health, ent->client->ps.stats[STAT_ARMOR]);
			}
		}
	}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink(gclient_t* client) {
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;
	// OSP: the ready flag toggles, at most every 300 msec
	if (client->pers.osp_unk_2bc + 300 < level.time || client->pers.osp_unk_2bc > level.time)
		if (client->buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE) & (client->oldbuttons ^ client->buttons)) {
			client->readyToExit ^= 1;
			client->pers.osp_unk_2bc = level.time;
		}
	client->ps.pm_flags |= PMF_FOLLOW;
}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents(gentity_t* ent, int oldEventSequence) {
	int     i, j;
	int     event;
	gclient_t* client;
	int     damage;
	vec3_t  dir;
	vec3_t  origin, angles;
//	qboolean fired;
	gitem_t* item;
	gentity_t* drop;

	client = ent->client;

	if (oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS) {
		oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
	}
	for (i = oldEventSequence ; i < client->ps.eventSequence ; i++) {
		event = client->ps.events[ i & (MAX_PS_EVENTS-1) ];

		switch (event) {
			case EV_FALL_MEDIUM:
			case EV_FALL_FAR:
				if (ent->s.eType != ET_PLAYER) {
					break;      // not in the player model
				}
				// OSP: no falling damage in instagib
				if ((g_dmflags.integer & DF_NO_FALLING) || match_instagib.integer > 0) {
					break;
				}
				if (event == EV_FALL_FAR) {
					damage = 10;
				} else {
					damage = 5;
				}
				VectorSet(dir, 0, 0, 1);
				ent->pain_debounce_time = level.time + 200; // no normal pain sound
				G_Damage(ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
				break;

			case EV_FIRE_WEAPON:
				// OSP: no shots in clan arena warmup unless match_cawarmupfire
				if (OSP_PM_CanFire()) {
					FireWeapon(ent);
				}
				break;

			case EV_USE_ITEM1:      // teleporter
				// drop flags in CTF
				item = NULL;
				j = 0;

				if (ent->client->ps.powerups[ PW_REDFLAG ]) {
					item = BG_FindItemForPowerup(PW_REDFLAG);
					j = PW_REDFLAG;
				} else if (ent->client->ps.powerups[ PW_BLUEFLAG ]) {
					item = BG_FindItemForPowerup(PW_BLUEFLAG);
					j = PW_BLUEFLAG;
				} else if (ent->client->ps.powerups[ PW_NEUTRALFLAG ]) {
					item = BG_FindItemForPowerup(PW_NEUTRALFLAG);
					j = PW_NEUTRALFLAG;
				}

				if (item) {
					drop = Drop_Item(ent, item, 0, 0);
					// decide how many seconds it has left
					drop->count = (ent->client->ps.powerups[ j ] - level.time) / 1000;
					if (drop->count < 1) {
						drop->count = 1;
					}

					ent->client->ps.powerups[ j ] = 0;
				}

#ifdef MISSIONPACK
				if (g_gametype.integer == GT_HARVESTER) {
					if (ent->client->ps.generic1 > 0) {
						if (ent->client->sess.sessionTeam == TEAM_RED) {
							item = BG_FindItem("Blue Cube");
						} else {
							item = BG_FindItem("Red Cube");
						}
						if (item) {
							for (j = 0; j < ent->client->ps.generic1; j++) {
								drop = Drop_Item(ent, item, 0, 0);
								if (ent->client->sess.sessionTeam == TEAM_RED) {
									drop->spawnflags = TEAM_BLUE;
								} else {
									drop->spawnflags = TEAM_RED;
								}
							}
						}
						ent->client->ps.generic1 = 0;
					}
				}
#endif
				SelectSpawnPoint(ent->client->ps.origin, origin, angles);
				TeleportPlayer(ent, origin, angles);
				break;

			case EV_USE_ITEM2:      // medkit
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH] + 25;

				break;

#ifdef MISSIONPACK
			case EV_USE_ITEM3:      // kamikaze
				// make sure the invulnerability is off
				ent->client->invulnerabilityTime = 0;
				// start the kamikze
				G_StartKamikaze(ent);
				break;

			case EV_USE_ITEM4:      // portal
				if (ent->client->portalID) {
					DropPortalSource(ent);
				}
				else {
					DropPortalDestination(ent);
				}
				break;
			case EV_USE_ITEM5:      // invulnerability
				ent->client->invulnerabilityTime = level.time + 10000;
				break;
#endif

			default:
				break;
		}
	}

}

#ifdef MISSIONPACK
/*
==============
StuckInOtherClient
==============
*/
static int StuckInOtherClient(gentity_t* ent) {
	int i;
	gentity_t*   ent2;

	ent2 = &g_entities[0];
	for (i = 0; i < MAX_CLIENTS; i++, ent2++) {
		if (ent2 == ent) {
			continue;
		}
		if (!ent2->inuse) {
			continue;
		}
		if (!ent2->client) {
			continue;
		}
		if (ent2->health <= 0) {
			continue;
		}
		//
		if (ent2->r.absmin[0] > ent->r.absmax[0])
			continue;
		if (ent2->r.absmin[1] > ent->r.absmax[1])
			continue;
		if (ent2->r.absmin[2] > ent->r.absmax[2])
			continue;
		if (ent2->r.absmax[0] < ent->r.absmin[0])
			continue;
		if (ent2->r.absmax[1] < ent->r.absmin[1])
			continue;
		if (ent2->r.absmax[2] < ent->r.absmin[2])
			continue;
		return qtrue;
	}
	return qfalse;
}
#endif

void BotTestSolid(vec3_t origin);

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real(gentity_t* ent) {
	gclient_t*   client;
	pmove_t     pm;
	int         oldEventSequence;
	int         msec;
	usercmd_t*   ucmd;
	int         pmoveMsec = 33;

	client = ent->client;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED) {
		return;
	}
	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if (ucmd->serverTime > level.time + 200) {
		ucmd->serverTime = level.time + 200;
//		G_Printf("serverTime <<<<<\n" );
	}
	if (ucmd->serverTime < level.time - 1000) {
		ucmd->serverTime = level.time - 1000;
//		G_Printf("serverTime >>>>>\n" );
	}

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if (msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW) {
		return;
	}
	if (msec > 200) {
		msec = 200;
	}

	// OSP: pmove_msec is only clamped (and the msec rounded) with pmove_fixed
	if (pmove_fixed.integer) {
		if (pmove_msec.integer < 8) {
			trap_Cvar_Set("pmove_msec", "8");
			trap_Cvar_Update(&pmove_msec);
			pmoveMsec = 8;
		}
		else if (pmove_msec.integer > 33) {
			trap_Cvar_Set("pmove_msec", "33");
			trap_Cvar_Update(&pmove_msec);
			pmoveMsec = 33;
		} else {
			pmoveMsec = pmove_msec.integer;
		}

		ucmd->serverTime = ((ucmd->serverTime + pmoveMsec-1) / pmoveMsec) * pmoveMsec;
		msec = ((msec + pmoveMsec-1) / pmoveMsec) * pmoveMsec;
	}

	//
	// check for exiting intermission
	//
	if (level.intermissiontime || level.intermissionQueued) {
		ClientIntermissionThink(client);
		VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);
		VectorCopy(ent->client->ps.origin, ent->s.origin);
		return;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if (!ClientInactivityTimer(client)) {
		return;
	}

	// spectators don't do much
	if (is_spectator(client)) {
		if (client->sess.spectatorState == SPECTATOR_SCOREBOARD) {
			return;
		}
		SpectatorThink(ent, ucmd);
		return;
	}

	// OSP: retry a spawn that found no free spawn point
	if (client->pers.waitingForSpawn) {
		ClientSpawn(ent);
		if (!client->pers.waitingForSpawn) {
			gentity_t*   tent;

			// add a teleportation effect
			tent = G_TempEntity(ent->client->ps.origin, EV_PLAYER_TELEPORT_IN);
			tent->s.clientNum = ent->s.clientNum;
		}
		return;
	}

	// clear the rewards if time
	if (level.time > client->rewardTime) {
		client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP);
	}

	if (client->noclip) {
		client->ps.pm_type = PM_NOCLIP;
	} else if (client->ps.stats[STAT_HEALTH] <= 0) {
		client->ps.pm_type = PM_DEAD;
	} else if (osp_matchPaused) {
		// OSP: players are frozen during a pause
		client->ps.pm_type = PM_FREEZE;
		client->ps.pm_flags |= PMF_FOLLOW;
	} else {
		client->ps.pm_type = PM_NORMAL;
		client->ps.pm_flags &= ~PMF_FOLLOW;
	}

	client->ps.gravity = g_gravity.value;

	// set speed
	client->ps.speed = g_speed.value;

	if (client->ps.powerups[PW_HASTE]) {
		client->ps.speed *= 1.3;
	}

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset(&pm, 0, sizeof(pm));

	// check for the hit-scan gauntlet, don't let the action
	// go through as an attack unless it actually hits something
	if (client->ps.weapon == WP_GAUNTLET && !(ucmd->buttons & BUTTON_TALK) &&
	        (ucmd->buttons & BUTTON_ATTACK) && client->ps.weaponTime <= 0 && OSP_PM_CanFire()) {
		pm.gauntletHit = CheckGauntletAttack(ent);
	}

	if (ent->flags & FL_FORCE_GESTURE) {
		ent->flags &= ~FL_FORCE_GESTURE;
		client->pers.cmd.buttons |= BUTTON_GESTURE;
	}

	pm.ps = &client->ps;
	pm.cmd = *ucmd;
	if (pm.ps->pm_type == PM_DEAD) {
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else if (ent->r.svFlags & SVF_BOT) {
		pm.tracemask = MASK_PLAYERSOLID | CONTENTS_BOTCLIP;
	}
	else {
		pm.tracemask = MASK_PLAYERSOLID;
		// OSP dmflags 512 ignores player clips
		if (g_dmflags.integer & 512) {
			pm.tracemask &= ~CONTENTS_PLAYERCLIP;
		}
	}
	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = (level.physicsMode & 1) || (g_dmflags.integer & DF_NO_FOOTSTEPS) > 0;

	pm.pmove_fixed = pmove_fixed.integer;
	pm.pmove_msec = pmoveMsec;

	VectorCopy(client->ps.origin, client->oldOrigin);

	Pmove(&pm);

	// save results of pmove
	if (ent->client->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
		ent->osp_unk_228 = 0;
	}
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate(&ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue, qfalse);
	}
	else {
		BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue, qfalse);
	}

	// OSP: offhand grapple (BUTTON_HOOK) with hook_delaytime between shots
	if (hook_enable.integer) {
		if ((pm.cmd.buttons & BUTTON_HOOK) && ent->client->ps.pm_type != PM_DEAD && !ent->client->fireHeld
		        && ent->client->pers.hookTime <= level.time) {
			Weapon_GrapplingHook_Fire(ent);
		}
		if (ent->client->ps.pm_type == PM_DEAD || (!(pm.cmd.buttons & BUTTON_HOOK) && ent->client->fireHeld)) {
			ent->client->fireHeld = qfalse;
			ent->client->pers.hookTime = level.time + ((hook_delaytime.integer < 100) ? 100 : hook_delaytime.integer);
		}
		// Let go of the hook if we aren't firing
		if (client->hook && !client->fireHeld) {
			Weapon_HookFree(client->hook);
		}
	}

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy(ent->s.pos.trBase, ent->r.currentOrigin);

	VectorCopy(pm.mins, ent->r.mins);
	VectorCopy(pm.maxs, ent->r.maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	// execute client events
	if (!osp_matchPaused) {
		ClientEvents(ent, oldEventSequence);
	}

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity(ent);
	if (!ent->client->noclip) {
		G_TouchTriggers(ent);
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);

	//test for solid areas in the AAS file
	BotTestAAS(ent->r.currentOrigin);

	// touch other objects
	ClientImpacts(ent, &pm);

	// save results of triggers and client events
	if (ent->client->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
		ent->osp_unk_228 = 0;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// check for respawning
	if (client->ps.stats[STAT_HEALTH] <= 0) {
		// wait for the attack button to be pressed
		if (osp_matchTime > client->respawnTime) {
			int     forceRespawn;

			forceRespawn = level.physicsMode ? modeForceRespawn : g_forcerespawn.integer;
			// forcerespawn is to prevent users from waiting out powerups
			// (OSP: round based modes respawn after 750 msec)
			if ((OSP_IsRoundBased(g_gametype.integer) && osp_matchTime - client->respawnTime > 750)
			        || (forceRespawn > 0 &&
			            (osp_matchTime - client->respawnTime) + (OSP_IsRoundBased(g_gametype.integer) ? 1700.0f : modeUnk_27dc) > 1000 * forceRespawn)) {
				respawn(ent);
				return;
			}

			// pressing attack or use is the normal respawn method
			// (OSP: PROMODE needs a new press)
			if ((ucmd->buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE))
			        && (!(level.physicsMode & 1) || !(client->oldbuttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE)))) {
				respawn(ent);
			}
		}
		return;
	}

	// perform once-a-second actions
	if (!osp_matchPaused) {
		ClientTimerActions(ent, msec);
	}
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink(int clientNum) {
	gentity_t* ent;

	ent = g_entities + clientNum;
	trap_GetUsercmd(clientNum, &ent->client->pers.cmd);

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.time;

	if (!(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer) {
		ClientThink_real(ent);
	}
}


void G_RunClient(gentity_t* ent) {
	if (!(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer) {
		return;
	}
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real(ent);
}


/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame(gentity_t* ent) {
	gclient_t*   cl;
	int         newClient;
	int         clientNum;
	int         killer;

	cl = NULL;
	clientNum = ent->client->sess.spectatorClient;

	// OSP: when the followed player is dead, follow a flag carrier / powerup
	// holder (followpowerup) or his killer (followkiller)
	if (clientNum >= 0) {
		int     team;

		team = ent->client->sess.sessionTeam;
		cl = &level.clients[ clientNum ];
		if (cl->ps.pm_type == PM_DEAD) {
			newClient = -1;
			if (ent->client->pers.followPowerup) {
				int         i, j, n;
				gclient_t*   other;

				newClient = ent->client->sess.spectatorClient;
				if (newClient < 0 || (!level.clients[newClient].ps.powerups[PW_REDFLAG] && !level.clients[newClient].ps.powerups[PW_BLUEFLAG])) {
					for (j = 0 ; j < level.numConnectedClients ; j++) {
						n = level.sortedClients[j];
						other = &level.clients[n];
						if (other->pers.connected != CON_CONNECTED
						        || other->sess.sessionTeam == TEAM_SPECTATOR
						        || other->ps.pm_type == PM_DEAD
						        || (IS_FREEZETAG && team != TEAM_SPECTATOR && team != other->sess.sessionTeam)
						        || OSP_FollowHelper(ent, n, ent->client->pers.followTeam)) {
							continue;
						}
						if (other->ps.powerups[PW_REDFLAG] || other->ps.powerups[PW_BLUEFLAG]) {
							newClient = n;
							break;
						}
						for (i = 0 ; i < MAX_POWERUPS ; i++) {
							if (other->ps.powerups[i]) {
								newClient = n;
							}
						}
					}
				}
				if (newClient > -1) {
					ent->client->sess.spectatorClient = newClient;
					clientNum = newClient;
					cl = &level.clients[ newClient ];
				}
			}
			if (newClient == -1) {
				if (ent->client->pers.followKiller && team == TEAM_SPECTATOR && ent->client->pers.followKillerTime < level.time) {
					killer = g_entities[clientNum].enemy - g_entities;
					if (killer >= 0 && killer < level.maxclients && osp_matchTime + 1000 > cl->respawnTime
					        && !OSP_FollowHelper(ent, killer, ent->client->pers.followTeam)) {
						ent->client->sess.spectatorClient = killer;
						clientNum = killer;
						cl = &level.clients[ killer ];
						ent->client->pers.followKillerTime = level.time + 7500;
					}
				}
			}
		}
	}

	// if we are doing a chase cam or a remote view, grab the latest info
	if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW) {
		// team follow1 and team follow2 go to whatever clients are playing
		if (clientNum == -1) {
			clientNum = level.follow1;
		} else if (clientNum == -2) {
			clientNum = level.follow2;
		}
		if (clientNum >= 0) {
			cl = &level.clients[ clientNum ];
			if (cl->pers.connected == CON_CONNECTED && !is_spectator(cl)) {
				int     flags, ping;

				flags = (cl->ps.eFlags & ~(EF_VOTED | EF_TEAMVOTED)) | (ent->client->ps.eFlags & (EF_VOTED | EF_TEAMVOTED));
				ping = ent->client->ps.ping;
				FT_SpectatorEndFrameHelper(ent, cl);
				if (server_realping.integer) {
					ent->client->ps.ping = ping;
				}
				ent->client->ps.pm_flags |= PMF_FOLLOW;
				ent->client->ps.eFlags = flags;
				return;
			} else {
				// drop them to free spectators unless they are dedicated camera followers
				if (ent->client->sess.spectatorClient >= 0) {
					if (ent->client->sess.coachTeam) {
						OSP_Cmd_Coach_f(ent, 0);
					} else if (!IS_FREEZETAG) {
						ent->client->sess.spectatorState = SPECTATOR_FREE;
						ClientBegin(ent->client - level.clients);
					} else {
						Cmd_FollowCycle_f(ent, 1);
					}
				}
			}
		}
	}

	if (ent->client->sess.spectatorState == 5) {
		// OSP spectator state 5 shows the followed player's stats
		if (cl && cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR) {
			if (!ent->client->pers.viewcamStatic) {
				memcpy(ent->client->ps.stats, cl->ps.stats, sizeof(cl->ps.stats));
				memcpy(ent->client->ps.powerups, cl->ps.powerups, sizeof(cl->ps.powerups));
				memcpy(ent->client->ps.ammo, cl->ps.ammo, sizeof(cl->ps.ammo));
			}
			ent->client->ps.pm_flags |= PMF_FOLLOW;
		} else if (ent->client->sess.coachTeam) {
			OSP_Cmd_Coach_f(ent, 0);
		} else {
			ent->client->sess.spectatorState = SPECTATOR_FREE;
			ClientBegin(ent->client - level.clients);
		}
	} else if (ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD) {
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame(gentity_t* ent) {
	int         i;

	// OSP: first frames after connecting: drop clients that were only let in
	// to see the reason (ClientConnect's connectReason), restore a spectator's coach /
	// multiview state, tell team players their team's joincode
	if (!ent->client->pers.connectChecked) {
		if (ent->client->connectReason > 0 && (level.warmupTime || !g_doWarmup.integer)) {
			int     reason;

			if (osp_matchTime - ent->client->pers.enterTime > 2000) {
				char*    msg;

				reason = ent->client->connectReason - 1;
				switch (reason) {
					case 0:
						G_Printf("** Invalid connect name: \"%s\"\n", ent->client->pers.netname);
						OSP_AdminLogPrintf(va("ConnectReject:\tNameBan\tName=%s", ent->client->pers.netname));
						msg = "Invalid playername";
						break;
					case 3:
						G_Printf("** Invalid address connect: \"%s\"\n", ent->client->sess.ip);
						OSP_AdminLogPrintf(va("ConnectReject:\tAddrBan\tAddr=%s", ent->client->sess.ip));
						msg = "Address is banned";
						break;
					case 4:
						G_Printf("** Bad password from \"%s\"\n", ent->client->pers.netname);
						OSP_AdminLogPrintf(va("ConnectReject:\tPasswordBan\tName=%s", ent->client->pers.netname));
						msg = "Invalid server password";
						break;
					case 5:
						G_Printf("** Invalid tag in: \"%s\"\n", ent->client->pers.netname);
						OSP_AdminLogPrintf(va("ConnectReject:\tTagBan\tName=%s", ent->client->pers.netname));
						msg = "Invalid playername";
						break;
					default:
						G_Printf("** General filter error: \"%s\" (%d)\n", ent->client->pers.netname, reason);
						OSP_AdminLogPrintf(va("ConnectReject:\tUnknownBan\tName=%s", ent->client->pers.netname));
						msg = "Filter Violation";
						break;
				}
				trap_SendServerCommand(ent - g_entities, va("print \"^3\n\n*** %s ***\n\n\"", msg));
				trap_DropClient(ent->client - level.clients, va(": ^3*** %s ***", msg));
				return;
			}
		} else if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
			if (osp_matchTime - ent->client->pers.enterTime > 750) {
				if (ent->client->sess.coachTeam) {
					OSP_Cmd_Coach_f(ent, 0);
				} else {
					OSP_MV_ReadSession(ent->client - level.clients);
				}
				ent->client->pers.connectChecked = 1;
			}
		} else if (g_gametype.integer >= GT_TEAM && !OSP_IsRoundBased(g_gametype.integer)
		           && !match_latejoin.integer && !team_nocontrols.integer) {
			if (!level.warmupTime && level.time - ent->client->pers.enterTime > 2000) {
				trap_SendServerCommand(ent - g_entities, va("print \"^5Team joincode is: ^3%s^7\n\"",
				                       osp_teamInfo[ent->client->sess.sessionTeam].joincode));
				ent->client->pers.connectChecked = 1;
			}
		} else {
			ent->client->pers.connectChecked = 1;
		}
	}

	if (is_spectator(ent->client)) {
		SpectatorClientEndFrame(ent);
		return;
	}

	// turn off any expired powerups
	// (OSP: powerup timers stand still during a pause)
	for (i = 0 ; i < MAX_POWERUPS ; i++) {
		if (osp_matchPaused && ent->client->ps.powerups[ i ] && ent->client->ps.powerups[ i ] != INT_MAX) {
			ent->client->ps.powerups[ i ] += level.time - level.previousTime;
		}
		if (ent->client->ps.powerups[ i ] < level.time) {
			ent->client->ps.powerups[ i ] = 0;
		}
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if (level.intermissiontime) {
		return;
	}

	// burn from lava, etc
	P_WorldEffects(ent);

	// apply all the damage taken this frame
	P_DamageFeedback(ent);

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if (level.time - ent->client->lastCmdTime > 1000) {
		ent->s.eFlags |= EF_CONNECTION;
	} else {
		ent->s.eFlags &= ~EF_CONNECTION;
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;   // FIXME: get rid of ent->health...

	G_SetClientSound(ent);

	// set the latest infor
	// (OSP: server_ospauth bit 1 sends the offset origin, see BG_PlayerStateToEntityState)
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate(&ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue, (server_ospauth.integer & 1) != 0);
	}
	else {
		BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue, (server_ospauth.integer & 1) != 0);
	}

	VectorCopy(ent->client->ps.origin, ent->client->oldOrigin);
}


