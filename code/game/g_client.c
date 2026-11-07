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

// g_client.c -- client functions that don't happen every frame

static vec3_t   playerMins = {-15, -15, -24};
static vec3_t   playerMaxs = {15, 15, 32};

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch(gentity_t* ent) {
	int     i;

	G_SpawnInt("nobots", "0", &i);
	if (i) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt("nohumans", "0", &i);
	if (i) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t* ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch(ent);
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission(gentity_t* ent) {

}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag(gentity_t* spot) {
	// OSP checks the distance to every playing client (or, in freeze tag,
	// to a frozen client's body) instead of trap_EntitiesInBox
	int         i;
	gentity_t*   ent;
	vec3_t      dir;

	for (i = 0 ; i < level.numConnectedClients ; i++) {
		ent = &g_entities[level.sortedClients[i]];
		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
			continue;
		}
		if (IS_FREEZETAG && is_spectator(ent->client)) {
			if (ent->target_ent && ent->target_ent->inuse) {
				VectorSubtract(spot->s.pos.trBase, ent->target_ent->r.currentOrigin, dir);
				if (VectorNormalize(dir) <= 60) {
					return qtrue;
				}
			}
			continue;
		}
		VectorSubtract(spot->s.pos.trBase, ent->client->ps.origin, dir);
		if (VectorNormalize(dir) <= 60) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define MAX_SPAWN_POINTS    128
gentity_t* SelectNearestDeathmatchSpawnPoint(vec3_t from) {
	gentity_t*   spot;
	vec3_t      delta;
	float       dist, nearestDist;
	gentity_t*   nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

		VectorSubtract(spot->s.origin, from, delta);
		dist = VectorLength(delta);
		if (dist < nearestDist) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define MAX_SPAWN_POINTS    128
gentity_t* SelectRandomDeathmatchSpawnPoint(gentity_t* avoid) {
	gentity_t*   spot;
	int         count;
	int         selection;
	gentity_t*   spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if (SpotWouldTelefrag(spot) || spot == avoid) {
			continue;
		}
		spots[ count ] = spot;
		count++;
	}

	if (!count) {    // no spots that won't telefrag
		return NULL;
	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t* SelectRandomFurthestSpawnPoint(vec3_t avoidPoint, vec3_t origin, vec3_t angles) {
	gentity_t*   spot;
	vec3_t      delta;
	float       dist;
	float       list_dist[64];
	gentity_t*   list_spot[64];
	int         numSpots, rnd, i, j;

	numSpots = 0;
	spot = NULL;

	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if (SpotWouldTelefrag(spot)) {
			continue;
		}
		VectorSubtract(spot->s.origin, avoidPoint, delta);
		dist = VectorLength(delta);
		for (i = 0; i < numSpots; i++) {
			if (dist > list_dist[i]) {
				if (numSpots >= 64)
					numSpots = 64-1;
				for (j = numSpots; j > i; j--) {
					list_dist[j] = list_dist[j-1];
					list_spot[j] = list_spot[j-1];
				}
				list_dist[i] = dist;
				list_spot[i] = spot;
				numSpots++;
				if (numSpots > 64)
					numSpots = 64;
				break;
			}
		}
		if (i >= numSpots && numSpots < 64) {
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
		}
	}
	if (!numSpots) {
		spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
		if (!spot)
			return NULL;
		VectorCopy(spot->s.origin, origin);
		origin[2] += 9;
		VectorCopy(spot->s.angles, angles);
		return spot;
	}

	// select a random spot from the spawn points furthest away
	rnd = random() * (numSpots / 2);

	VectorCopy(list_spot[rnd]->s.origin, origin);
	origin[2] += 9;
	VectorCopy(list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t* SelectSpawnPoint(vec3_t avoidPoint, vec3_t origin, vec3_t angles) {
	return SelectRandomFurthestSpawnPoint(avoidPoint, origin, angles);

	/*
	gentity_t   *spot;
	gentity_t   *nearestSpot;

	nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

	spot = SelectRandomDeathmatchSpawnPoint ( );
	if ( spot == nearestSpot ) {
	    // roll again if it would be real close to point of death
	    spot = SelectRandomDeathmatchSpawnPoint ( );
	    if ( spot == nearestSpot ) {
	        // last try
	        spot = SelectRandomDeathmatchSpawnPoint ( );
	    }
	}

	// find a single player start spot
	if (!spot) {
	    G_Error( "Couldn't find a spawn point" );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
	*/
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t* SelectInitialSpawnPoint(vec3_t origin, vec3_t angles) {
	gentity_t*   spot;

	spot = NULL;
	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if (spot->spawnflags & 1) {
			break;
		}
	}

	if (!spot || SpotWouldTelefrag(spot)) {
		return SelectSpawnPoint(vec3_origin, origin, angles);
	}

	VectorCopy(spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy(spot->s.angles, angles);

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t* SelectSpectatorSpawnPoint(vec3_t origin, vec3_t angles) {
	FindIntermissionPoint();

	VectorCopy(level.intermission_origin, origin);
	VectorCopy(level.intermission_angle, angles);

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue(void) {
	int     i;
	gentity_t*   ent;

	level.bodyQueIndex = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink(gentity_t* ent) {
	if (level.time - ent->timestamp > 6500) {
		// the body ques are never actually freed, they are just unlinked
		trap_UnlinkEntity(ent);
		ent->physicsObject = qfalse;
		return;
	}
	ent->nextthink = level.time + 100;
	ent->s.pos.trBase[2] -= 1;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue(gentity_t* ent) {
#ifdef MISSIONPACK
	gentity_t*   e;
	int i;
#endif
	gentity_t*       body;
	int         contents;

	trap_UnlinkEntity(ent);

	// if client is in a nodrop area, don't leave the body
	contents = trap_PointContents(ent->s.origin, -1);
	if ((contents & CONTENTS_NODROP) && !(level.osp_unk_3834 & 2)) {
		return;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;

	trap_UnlinkEntity(body);

	body->s = ent->s;
	body->s.eFlags = EF_DEAD;       // clear EF_TALK, etc
#ifdef MISSIONPACK
	if (ent->s.eFlags & EF_KAMIKAZE) {
		body->s.eFlags |= EF_KAMIKAZE;

		// check if there is a kamikaze timer around for this owner
		for (i = 0; i < MAX_GENTITIES; i++) {
			e = &g_entities[i];
			if (!e->inuse)
				continue;
			if (e->activator != ent)
				continue;
			if (strcmp(e->classname, "kamikaze timer"))
				continue;
			e->activator = body;
			break;
		}
	}
#endif
	body->s.powerups = 0;   // clear powerups
	body->s.loopSound = 0;  // clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;        // don't bounce
	if (body->s.groundEntityNum == ENTITYNUM_NONE) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy(ent->client->ps.velocity, body->s.pos.trDelta);
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

	// change the animation to the last-frame only, so the sequence
	// doesn't repeat anew for the body
	switch (body->s.legsAnim & ~ANIM_TOGGLEBIT) {
		case BOTH_DEATH1:
		case BOTH_DEAD1:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
			break;
		case BOTH_DEATH2:
		case BOTH_DEAD2:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
			break;
		case BOTH_DEATH3:
		case BOTH_DEAD3:
		default:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
			break;
	}

	body->r.svFlags = ent->r.svFlags;
	VectorCopy(ent->r.mins, body->r.mins);
	VectorCopy(ent->r.maxs, body->r.maxs);
	VectorCopy(ent->r.absmin, body->r.absmin);
	VectorCopy(ent->r.absmax, body->r.absmax);

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.contents = CONTENTS_CORPSE;
	body->r.ownerNum = ent->s.number;

	body->nextthink = level.time + 5000;
	body->think = BodySink;

	body->die = body_die;

	// don't take more damage if already gibbed
	if (ent->health <= GIB_HEALTH) {
		body->takedamage = qfalse;
	} else {
		body->takedamage = qtrue;
	}


	VectorCopy(body->s.pos.trBase, body->r.currentOrigin);
	trap_LinkEntity(body);
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle(gentity_t* ent, vec3_t angle) {
	int         i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int     cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy(angle, ent->s.angles);
	VectorCopy(ent->s.angles, ent->client->ps.viewangles);
}

/*
================
respawn
================
*/
void respawn(gentity_t* ent) {
	// OSP: clan arena players respawn through OSP_Respawn outside warmup,
	// frozen freeze tag players may stay in their body
	if (g_gametype.integer == 5 && !level.warmupTime
	        && (ent->client->sess.sessionTeam == TEAM_RED || ent->client->sess.sessionTeam == TEAM_BLUE)) {
		OSP_Respawn(ent);
		return;
	}
	if (IS_FREEZETAG && FT_RespawnFrozen(ent)) {
		return;
	}

	CopyToBodyQue(ent);
	ClientSpawn(ent);

	// add a teleportation effect
	if (!ent->client->pers.waitingForSpawn) {
		gentity_t*   tent;

		tent = G_TempEntity(ent->client->ps.origin, EV_PLAYER_TELEPORT_IN);
		tent->s.clientNum = ent->s.clientNum;
	}
}

/*
================
TeamCount

Returns number of players on a team
================
*/
team_t TeamCount(int ignoreClientNum, int team) {
	int     i;
	int     count = 0;

	for (i = 0 ; i < level.maxclients ; i++) {
		if (i == ignoreClientNum) {
			continue;
		}
		if (level.clients[i].pers.connected == CON_DISCONNECTED) {
			continue;
		}
		// OSP: round based modes don't count dead players
		if (OSP_IsRoundBased(g_gametype.integer) && level.clients[i].ps.pm_type == PM_DEAD) {
			continue;
		}
		if (level.clients[i].sess.sessionTeam == team) {
			count++;
		}
	}

	return count;
}

/*
================
TeamLeader

Returns the client number of the team leader
================
*/
int TeamLeader(int team) {
	int     i;

	for (i = 0 ; i < level.maxclients ; i++) {
		if (level.clients[i].pers.connected == CON_DISCONNECTED) {
			continue;
		}
		if (level.clients[i].sess.sessionTeam == team) {
			if (level.clients[i].sess.teamLeader)
				return i;
		}
	}

	return -1;
}


/*
================
PickTeam

================
*/
team_t PickTeam(int ignoreClientNum) {
	int     counts[TEAM_NUM_TEAMS];

	counts[TEAM_BLUE] = TeamCount(ignoreClientNum, TEAM_BLUE);
	counts[TEAM_RED] = TeamCount(ignoreClientNum, TEAM_RED);

	// OSP: OSP_CheckTeamJoin refuses locked / full teams
	if (counts[TEAM_BLUE] > counts[TEAM_RED] && OSP_CheckTeamJoin(TEAM_RED, NULL, 0)) {
		return TEAM_RED;
	}
	if (counts[TEAM_RED] > counts[TEAM_BLUE] && OSP_CheckTeamJoin(TEAM_BLUE, NULL, 0)) {
		return TEAM_BLUE;
	}
	// equal team count, so join the team with the lowest score
	if (level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] && OSP_CheckTeamJoin(TEAM_RED, NULL, 0)) {
		return TEAM_RED;
	}
	if (OSP_CheckTeamJoin(TEAM_BLUE, NULL, 0)) {
		return TEAM_BLUE;
	}
	return TEAM_SPECTATOR;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
static void ForceClientSkin(gclient_t* client, char* model, const char* skin) {
	char* p;

	if ((p = Q_strrchr(model, '/')) != 0) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}

/*
===========
ClientCheckName
============
*/
static void ClientCleanName(const char* in, char* out, int outSize) {
	int     len, colorlessLen;
	char    ch;
	char*    p;
	int     spaces;

	//save room for trailing null byte
	outSize--;

	len = 0;
	colorlessLen = 0;
	p = out;
	*p = 0;
	spaces = 0;

	while (1) {
		ch = *in++;
		if (!ch) {
			break;
		}

		// don't allow leading spaces
		if (!*p && ch == ' ') {
			continue;
		}

		// check colors
		if (ch == Q_COLOR_ESCAPE) {
			// solo trailing carat is not a color prefix
			if (!*in) {
				break;
			}

			// OSP allows black in a name

			// make sure room in dest for both chars
			if (len > outSize - 2) {
				break;
			}

			*out++ = ch;
			*out++ = *in++;
			len += 2;
			continue;
		}

		// don't allow too many consecutive spaces
		if (ch == ' ') {
			spaces++;
			if (spaces > 3) {
				continue;
			}
		}
		else {
			spaces = 0;
		}

		if (len > outSize - 1) {
			break;
		}

		*out++ = ch;
		colorlessLen++;
		len++;
	}
	*out = 0;

	// don't allow empty names
	if (*p == 0 || colorlessLen == 0) {
		Q_strncpyz(p, "UnnamedPlayer", outSize);
	}
}


/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged(int clientNum) {
	gentity_t* ent;
	int     i, team, health;
	char*    s;
	char    model[MAX_QPATH];
	char    headModel[MAX_QPATH];
	char    oldname[MAX_STRING_CHARS];
	gclient_t*   client;
	char    c1[16];     // the client reads at most 4 characters; bounded so a long value cannot bloat CS_PLAYERS
	char    c2[16];
	char    userinfo[MAX_INFO_STRING];

	ent = g_entities + clientNum;
	client = ent->client;

	trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo)) {
		strcpy(userinfo, "\\name\\badinfo");
	}

	// the local client is decided in ClientConnect, not from the "ip" key here: some engines
	// keep a client-supplied "ip" in later userinfo updates

	// OSP: version of the OSP client cgame
	s = Info_ValueForKey(userinfo, "osp_client");
	if (*s)
		client->sess.ospClient = atoi(s);

	// check the item prediction
	s = Info_ValueForKey(userinfo, "cg_predictItems");
	if (!atoi(s)) {
		client->pers.predictItemPickup = qfalse;
	} else {
		client->pers.predictItemPickup = qtrue;
	}

	// set name
	Q_strncpyz(oldname, client->pers.netname, sizeof(oldname));
	s = Info_ValueForKey(userinfo, "name");

	// OSP: name filters, name change flood protection, relay (QTV/GTV) names
	if (Q_stricmp(s, oldname) && !(ent->r.svFlags & SVF_BOT)) {
		i = OSP_CheckClientFilters(s, ent - g_entities, 1);
	} else {
		i = 1;
	}

	if (i == 0 || i == 5 || i == 6
	        || (strstr(oldname, "QTV") == oldname && strstr(s, "QTV") != s)
	        || (strstr(oldname, "GTV") == oldname && strstr(s, "GTV") != s)) {
		s = Info_ValueForKey(userinfo, "name");
		if (i != 6) {
			OSP_AdminLogPrintf(va("Player_Invalid_NameChange\tOrig=%s\tNew=%s", oldname, s));
		}
		if (!client->sess.ospClient) {
			// the old name can't be forced on a non-OSP client
			if (client->pers.connected == CON_CONNECTED) {
				if (i != 6) {
					trap_SendServerCommand(ent - g_entities, va("print \"\n*** Name '%s^7' is not valid!\n*** Reconnect with '%s'\n\n", s, oldname));
					trap_DropClient(clientNum, va(": kicked for ^3invalid name selection!", oldname));
				} else {
					trap_SendServerCommand(ent - g_entities, va("print \"\n*** Can only update names every 5 seconds!\n\n"));
					trap_DropClient(clientNum, va(": kicked for too rapid of a name change!"));
				}
			}
			return;
		} else {
			if (i == 6) {
				trap_SendServerCommand(ent - g_entities, va("print \"\n*** Wait another %.1f seconds to change your name\n\"",
				                       (ent->client->pers.cmdDelayTime - level.time) * 1.0f / 1000.0f));
			} else {
				trap_SendServerCommand(ent - g_entities, va("print \"New name (%s^7) not valid!  Keeping '%s'\n\"", s, oldname));
			}
			trap_SendServerCommand(ent - g_entities, va("stuff seta name ' %s' ", oldname));
		}
	} else {
		s = Info_ValueForKey(userinfo, "name");
		ClientCleanName(s, client->pers.netname, sizeof(client->pers.netname));
		OSP_SortClients();
	}

	if (client->sess.sessionTeam == TEAM_SPECTATOR) {
		if (client->sess.spectatorState == SPECTATOR_SCOREBOARD) {
			Q_strncpyz(client->pers.netname, "scoreboard", sizeof(client->pers.netname));
		}
	}

	if (client->pers.connected == CON_CONNECTED) {
		if (strcmp(oldname, client->pers.netname)) {
			trap_SendServerCommand(-1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname,
			                              client->pers.netname));
			OSP_AdminLogPrintf(va("Player_NameChange\t%s\t%s", oldname, client->pers.netname));
		}
	}

	// set max health
	health = atoi(Info_ValueForKey(userinfo, "handicap"));
	client->pers.maxHealth = health;
	if (client->pers.maxHealth < 1 || client->pers.maxHealth > 100) {
		client->pers.maxHealth = 100;
	}
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;

	// set model
	Q_strncpyz(model, Info_ValueForKey(userinfo, "model"), sizeof(model));
	Q_strncpyz(headModel, Info_ValueForKey(userinfo, "headmodel"), sizeof(headModel));

	// bots set their team a few frames later
	if (g_gametype.integer >= GT_TEAM && g_entities[clientNum].r.svFlags & SVF_BOT) {
		s = Info_ValueForKey(userinfo, "team");
		if (!Q_stricmp(s, "red") || !Q_stricmp(s, "r")) {
			team = TEAM_RED;
		} else if (!Q_stricmp(s, "blue") || !Q_stricmp(s, "b")) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			team = PickTeam(clientNum);
		}
	}
	else {
		team = client->sess.sessionTeam;
		// OSP: dead players of round based modes keep their team's skin
		if (OSP_IsRoundBased(g_gametype.integer) && client->sess.roundTeam) {
			team = client->sess.roundTeam;
		}
	}

	// OSP forces the team skins again
	switch (team) {
		case TEAM_RED:
			ForceClientSkin(client, model, "red");
			ForceClientSkin(client, headModel, "red");
			break;
		case TEAM_BLUE:
			ForceClientSkin(client, model, "blue");
			ForceClientSkin(client, headModel, "blue");
			break;
	}
	// don't ever use a default skin in teamplay, it would just waste memory
	// however bots will always join a team but they spawn in as spectator
	if (g_gametype.integer >= GT_TEAM && client->sess.sessionTeam == TEAM_SPECTATOR) {
		if (client->sess.roundTeam == TEAM_BLUE) {
			ForceClientSkin(client, model, "blue");
			ForceClientSkin(client, headModel, "blue");
		} else {
			ForceClientSkin(client, model, "red");
			ForceClientSkin(client, headModel, "red");
		}
	}

	// teamInfo
	s = Info_ValueForKey(userinfo, "teamoverlay");
	client->pers.teamInfo = (!*s || atoi(s) != 0);

	// colors
	Q_strncpyz(c1, Info_ValueForKey(userinfo, Info_ValueForKey(userinfo, "color1") ? "color1" : "color"), sizeof(c1));
	Q_strncpyz(c2, Info_ValueForKey(userinfo, "color2"), sizeof(c2));

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	if (ent->r.svFlags & SVF_BOT) {
		s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\rt\\%i",
		       client->pers.netname, client->sess.sessionTeam, model, headModel, c1, c2,
		       client->pers.maxHealth, client->sess.wins & 0xffff, client->sess.losses & 0xffff,
		       Info_ValueForKey(userinfo, "skill"),
		       (client->sess.sessionTeam == TEAM_SPECTATOR) ?
		       (OSP_IsRoundBased(g_gametype.integer) ? client->sess.roundTeam : TEAM_SPECTATOR) :
		       client->sess.sessionTeam);
	} else {
		s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\rt\\%i\\st\\%i",
		       client->pers.netname, client->sess.sessionTeam, model, headModel, c1, c2,
		       client->pers.maxHealth, client->sess.wins & 0xffff, client->sess.losses & 0xffff,
		       (client->sess.sessionTeam == TEAM_SPECTATOR) ?
		       (client->pers.followTeam ? client->pers.followTeam :
		        (OSP_IsRoundBased(g_gametype.integer) ? client->sess.roundTeam : TEAM_SPECTATOR)) :
		       client->sess.sessionTeam,
		       client->sess.coachTeam ? client->sess.coachTeam + 8 :
		       ((client->sess.sessionTeam == TEAM_SPECTATOR) ? client->pers.followTeam : 0));
	}

	trap_SetConfigstring(CS_PLAYERS+clientNum, s);

	// this is not the userinfo, more like the configstring actually
	G_LogPrintf("ClientUserinfoChanged: %i %s\n", clientNum, s);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char* ClientConnect(int clientNum, qboolean firstTime, qboolean isBot) {
	gclient_t*   client;
	char*        value;
	gentity_t*   ent;
	char        userinfo[MAX_INFO_STRING];
	int         i;
	char        buf[MAX_INFO_STRING];
	int         reason;
	int         j, k;

	// OSP lets rejected clients through during the first two seconds of a
	// map (map change, reconnect) and stores the reason in client->connectReason instead
	reason = 0;
	ent = &g_entities[ clientNum ];

	trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));

	if (!isBot) {
		// the engine inserts the real address as "ip" on every connect; a userinfo without it
		// (an over-long one on 1.32b engines) would otherwise pass every address filter
		if (firstTime && !*Info_ValueForKey(userinfo, "ip")) {
			return "Bad userinfo";
		}
		// check to see if they are on the banned IP list
		value = Info_ValueForKey(userinfo, "ip");
		if (G_FilterPacket(value)) {
			if (!firstTime || level.time - level.startTime < 2000) {
				reason = 4;
			} else {
				return "Address is Banned";
			}
		}

		// the first slot may be reserved for a GTV / QTV relay
		value = Info_ValueForKey(userinfo, "name");
		if (level.gtvClient == -1 && strstr(value, "QTV-") != value && strstr(value, "GTV-") != value
		        && level.gtvWaitTime > level.time) {
			if (!firstTime || level.time - level.startTime < 2000) {
				reason = 1;
			} else {
				return "Waiting for GTV to Connect";
			}
		}

		// name / address / tag filters
		i = OSP_CheckClientFilters(value, clientNum, 0);
		if (i != 1 && i != 2) {
			value = Info_ValueForKey(userinfo, "name");
			if (!firstTime || level.time - level.startTime < 2000) {
				reason = i + 1;
			} else {
				switch (i) {
					case 0:
						G_Printf("** Invalid connect name: \"%s\"\n", value);
						OSP_AdminLogPrintf(va("ConnectReject:\tNameBan\tName=%s", value));
						return "Invalid playername";
					case 3:
						value = Info_ValueForKey(userinfo, "ip");
						G_Printf("** Invalid address connect: \"%s\"\n", value);
						OSP_AdminLogPrintf(va("ConnectReject:\tAddrBan\tAddr=%s", value));
						return "Address is Banned";
					case 4:
						G_Printf("** Bad password from \"%s\"\n", value);
						OSP_AdminLogPrintf(va("ConnectReject:\tPasswordBan\tName=%s", value));
						return "Invalid password";
					case 5:
						G_Printf("** Invalid tag in: \"%s\"\n", value);
						OSP_AdminLogPrintf(va("ConnectReject:\tTagBan\tName=%s", value));
						return "Invalid playername/tag";
				}
			}
		}

		// check for a password
		value = Info_ValueForKey(userinfo, "password");
		if (g_password.string[0] && Q_stricmp(g_password.string, "none") &&
		        strcmp(g_password.string, value) != 0) {
			if (!firstTime || level.time - level.startTime < 900) {
				reason = 5;
			} else {
				return "Invalid password";
			}
		}
	}

	// they can connect
	ent->client = level.clients + clientNum;
	client = ent->client;

	memset(client, 0, sizeof(*client));

	client->pers.connected = CON_CONNECTING;
	ent->r.svFlags &= ~SVF_NOSERVERINFO;

	if (isBot) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		if (!G_BotConnect(clientNum, !firstTime)) {
			return "BotConnectfailed";
		}
	}

	// read or initialize the session data
	trap_Cvar_VariableStringBuffer("session", buf, sizeof(buf));
	sscanf(buf, "%i %i %i", &i, &j, &k);
	if (firstTime || level.newSession || i < 0) {
		G_InitSessionData(client, userinfo);
	}
	G_ReadSessionData(client);
	client->inactivityWarning = qfalse;
	client->inactivityTime = level.time + 60000;
	client->pers.followTeam = client->sess.coachTeam;

	// check for local client. On the first connect the engine itself wrote the "ip" key; after a
	// map change the userinfo may carry a client-supplied one, so use the address kept in the session
	value = firstTime ? Info_ValueForKey(userinfo, "ip") : client->sess.ip;
	if (!strcmp(value, "localhost")) {
		client->pers.localClient = qtrue;
		// OSP: a listen server can't do OSP authentication
		if (server_ospauth.integer) {
			trap_Cvar_Set("server_ospauth", "2");
		}
	}

	// get and distribute relevent paramters
	G_LogPrintf("ClientConnect: %i\n", clientNum);
	ClientUserinfoChanged(clientNum);

	// don't do the "xxx connected" messages if they were caried over from previous level
	if (firstTime) {
		trap_SendServerCommand(-1, va("print \"%s ^7connected\n\"", client->pers.netname));
		if (isBot) {
			OSP_AdminLogPrintf(va("Connect\t%s\tQ3A_BOT\tid=%d", client->pers.netname, clientNum));
			strcpy(client->sess.ip, "SERVER_BOT");
		} else {
			value = Info_ValueForKey(userinfo, "ip");
			if (value && *value) {
				char*    p;

				p = strchr(value, ':');
				if (p) {
					*p = 0;
				}
				Q_strncpyz(client->sess.ip, value, sizeof(client->sess.ip));
			} else {
				strcpy(client->sess.ip, "UNKNOWN");
			}
			OSP_AdminLogPrintf(va("Connect\t%s\t%s\tid=%d", client->pers.netname, client->sess.ip, clientNum));
		}
	}

	if (g_gametype.integer >= GT_TEAM &&
	        client->sess.sessionTeam != TEAM_SPECTATOR && !OSP_IsRoundBased(g_gametype.integer)) {
		BroadcastTeamChange(client, -1);
	}

	if (level.gtvClient == -1) {
		level.gtvClient = clientNum + 100;
	}

	// count current clients and rank for scoreboard
	CalculateRanks(qtrue);
	OSP_SortClients();

	client->connectReason = reason;

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin(int clientNum) {
	gentity_t*   ent;
	gclient_t*   client;
	gentity_t*   tent;
	int         flags;
	int         score;

	ent = g_entities + clientNum;

	client = level.clients + clientNum;

	if (ent->r.linked) {
		trap_UnlinkEntity(ent);
	}
	G_InitGentity(ent);
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.teamState.state = TEAM_BEGIN;
	client->pers.followTeam = client->pers.followTeam ? client->pers.followTeam : client->sess.coachTeam;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	flags = client->ps.eFlags;
	score = client->ps.persistant[PERS_SCORE];
	memset(&client->ps, 0, sizeof(client->ps));
	client->ps.eFlags = flags;

	// OSP keeps the score from the session stats
	if (g_gametype.integer == 5) {
		client->ps.persistant[PERS_SCORE] = client->sess.kills + (client->sess.wins & 0xffff) + client->sess.damageGiven / 100;
		if (osp_roundsPlayed) {
			client->pers.caDamage = client->sess.damageGiven % 100;
		}
	} else if (IS_FREEZETAG) {
		client->ps.persistant[PERS_SCORE] = client->sess.kills + (client->sess.losses & 0xffff) - (client->sess.suicides + client->sess.teamKills);
		if (client->sess.sessionTeam == TEAM_SPECTATOR && !level.warmupTime && !freezetag_latejoin.integer) {
			client->pers.osp_unk_290 = level.teamScores[TEAM_RED] + level.teamScores[TEAM_BLUE] + 1;
		}
	} else if (level.warmupTime) {
		client->ps.persistant[PERS_SCORE] = score;
	}

	// locate ent at a spawn point
	ClientSpawn(ent);

	if (client->sess.sessionTeam != TEAM_SPECTATOR) {
		// send event
		tent = G_TempEntity(ent->client->ps.origin, EV_PLAYER_TELEPORT_IN);
		tent->s.clientNum = ent->s.clientNum;

		if (g_gametype.integer != GT_TOURNAMENT) {
			trap_SendServerCommand(-1, va("print \"%s" S_COLOR_WHITE " entered the game%s\n\"", client->pers.netname,
			                              (g_gametype.integer >= GT_TEAM) ? va(" (%s)", osp_teamNames[client->sess.sessionTeam]) : ""));
		} else {
			client->pers.timeouts = match_timeoutcount.integer;
		}

		// the first player of a team becomes its captain
		if (g_gametype.integer >= GT_TEAM) {
			if (!team_nocontrols.integer && !TeamCount(ent - g_entities, client->sess.sessionTeam)) {
				client->sess.captain = 1;
			}
		}

		if (IS_FREEZETAG && client->pers.osp_unk_290 > 0
		        && client->pers.osp_unk_290 == level.teamScores[TEAM_RED] + level.teamScores[TEAM_BLUE] + 1) {
			player_freeze(ent, NULL, MOD_SUICIDE);
		}
	}

	if (ent->r.svFlags & SVF_BOT) {
		ent->client->sess.welcomeShown = 1;
		ent->client->pers.resultShown = 1;
	} else {
		if (!ent->client->sess.welcomeShown) {
			ent->client->sess.welcomeShown = 1;
			OSP_DelayPrintf(0, level.time + 2000, ent);
		}
		if (OSP_IsRoundBased(g_gametype.integer) && osp_matchResultPrint[0] && !ent->client->pers.resultShown) {
			ent->client->pers.resultShown = 1;
			OSP_DelayPrintf(1, level.time + 1500, ent);
		}
	}

	G_LogPrintf("ClientBegin: %i\n", clientNum);

	// count current clients and rank for scoreboard
	CalculateRanks(qtrue);
	OSP_SortClients();
	level.clientChangeTime = level.time;
	client->inactivityWarning = qfalse;
	client->inactivityTime = level.time + 60000;
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(gentity_t* ent) {
	int     index;
	vec3_t  spawn_origin, spawn_angles;
	gclient_t*   client;
	int     i;
	clientPersistant_t  saved;
	clientSession_t     savedSess;
	int     persistant[MAX_PERSISTANT];
	int     events[MAX_PS_EVENTS];
	gentity_t*   spawnPoint;
	int     flags;
	int     savedPing;
//	char *savedAreaBits;
	int     accuracy_hits, accuracy_shots;
	int     eventSequence;
	char    userinfo[MAX_INFO_STRING];
	int     connectReason;
	int     votedFlag;
	int     inactivityTime;

	spawnPoint = NULL;
	index = ent - g_entities;
	client = ent->client;
	client->ps.generic1 = 0;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if (client->sess.sessionTeam == TEAM_SPECTATOR || level.intermissionQueued || level.intermissiontime) {
		// OSP: viewcam spectators start at their camera
		if (client->sess.spectatorState < 4 || level.intermissionQueued || level.intermissiontime) {
			spawnPoint = SelectSpectatorSpawnPoint(
			                 spawn_origin, spawn_angles);
		} else {
			spawnPoint = NULL;
			VectorCopy(osp_viewcams[client->pers.viewcam].origin, spawn_origin);
			VectorCopy(osp_viewcams[client->pers.viewcam].angles, spawn_angles);
		}
	} else if (g_gametype.integer == GT_CTF) {
		// all base oriented team games use the CTF spawn points
		spawnPoint = SelectCTFSpawnPoint(
		                 client->sess.sessionTeam,
		                 client->pers.teamState.state,
		                 spawn_origin, spawn_angles);
	} else {
		do {
			// the first spawn should be at a good looking spot
			if (!client->pers.initialSpawn && client->pers.localClient) {
				client->pers.initialSpawn = qtrue;
				spawnPoint = SelectInitialSpawnPoint(spawn_origin, spawn_angles);
				if (!spawnPoint) {
					client->pers.initialSpawn = qfalse;
					break;
				}
			} else {
				// OSP: in freeze tag, spawn away from the first living enemy
				if (IS_FREEZETAG) {
					for (i = 0 ; i < level.numConnectedClients ; i++) {
						gentity_t*   other;

						other = &g_entities[level.sortedClients[i]];
						if (!is_spectator(other->client) && !other->frozen
						        && other->client->sess.sessionTeam != ent->client->sess.sessionTeam) {
							spawnPoint = SelectSpawnPoint(
							                 other->client->ps.origin,
							                 spawn_origin, spawn_angles);
							break;
						}
					}
				}
				if (!spawnPoint) {
					// don't spawn near existing origin if possible
					spawnPoint = SelectSpawnPoint(
					                 client->ps.origin,
					                 spawn_origin, spawn_angles);
				}
				if (!spawnPoint) {
					break;
				}
			}

			// Tim needs to prevent bots from spawning at the initial point
			// on q3dm0...
			if ((spawnPoint->flags & FL_NO_BOTS) && (ent->r.svFlags & SVF_BOT)) {
				continue;   // try again
			}
			// just to be symetric, we have a nohumans option...
			if ((spawnPoint->flags & FL_NO_HUMANS) && !(ent->r.svFlags & SVF_BOT)) {
				continue;   // try again
			}

			break;

		} while (1);
	}

	// OSP: no free spawn point
	if (!spawnPoint && !is_spectator(client)) {
		client->pers.waitingForSpawn = qtrue;
		return;
	}
	client->pers.waitingForSpawn = qfalse;
	client->pers.teamState.state = TEAM_ACTIVE;

	// always clear the kamikaze flag
	ent->s.eFlags &= ~EF_KAMIKAZE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

	// clear everything but the persistant data

	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
//	savedAreaBits = client->areabits;
	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;
	votedFlag = client->ps.eFlags & EF_VOTED;
	connectReason = client->connectReason;
	inactivityTime = client->inactivityTime;
	for (i = 0 ; i < MAX_PERSISTANT ; i++) {
		persistant[i] = client->ps.persistant[i];
	}
	for (i = 0 ; i < MAX_PS_EVENTS ; i++) {
		events[i] = client->ps.events[i];
	}
	eventSequence = client->ps.eventSequence;

	memset(client, 0, sizeof(*client));  // bk FIXME: Com_Memset?

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
//	client->areabits = savedAreaBits;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;
	client->ps.eFlags = votedFlag | flags;
	client->connectReason = connectReason;
	client->inactivityWarning = qfalse;
	client->inactivityTime = inactivityTime;

	for (i = 0 ; i < MAX_PERSISTANT ; i++) {
		client->ps.persistant[i] = persistant[i];
	}
	for (i = 0 ; i < MAX_PS_EVENTS ; i++) {
		client->ps.events[i] = events[i];
	}
	client->ps.eventSequence = eventSequence;
	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + 12000;

	trap_GetUserinfo(index, userinfo, sizeof(userinfo));
	// set max health
	client->pers.maxHealth = atoi(Info_ValueForKey(userinfo, "handicap"));
	if (client->pers.maxHealth < 1 || client->pers.maxHealth > 100) {
		client->pers.maxHealth = 100;
	}
	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;

	if (level.warmupTime) {
		OSP_ResetPlayerReady();
	}

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;

	VectorCopy(playerMins, ent->r.mins);
	VectorCopy(playerMaxs, ent->r.maxs);

	client->ps.clientNum = index;

	// OSP: weapons, ammo, health and armor come from the mode settings
	OSP_SetSpawnWeapons(ent);

	G_SetOrigin(ent, spawn_origin);
	VectorCopy(spawn_origin, client->ps.origin);

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	trap_GetUsercmd(client - level.clients, &ent->client->pers.cmd);
	SetClientViewAngle(ent, spawn_angles);

	if (is_spectator(client)) {

	} else {
		G_KillBox(ent);
		trap_LinkEntity(ent);
	}

	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = osp_matchTime;
	client->latched_buttons = 0;

	// set default animations
	client->ps.torsoAnim = TORSO_STAND;
	client->ps.legsAnim = LEGS_IDLE;

	if (level.intermissiontime) {
		MoveClientToIntermission(ent);
	} else {
		// fire the targets of the spawn point
		G_UseTargets(spawnPoint, ent);
		OSP_SelectSpawnWeapon(ent);
	}

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ent->client->ps.weaponTime = 325;
	ClientThink(ent-g_entities);

	// positively link the client, even if the command times are weird
	if (!is_spectator(client)) {
		BG_PlayerStateToEntityState(&client->ps, &ent->s, qtrue, 0);
		VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);
		trap_LinkEntity(ent);
		if (log_stat.integer && !level.warmupTime && !osp_matchPaused) {
			G_LogPrintf("PS: %i %i %i -1 RESPAWN\n", ent->s.number, ent->health, ent->client->ps.stats[STAT_ARMOR]);
		}
	}

	// run the presend to set anything else
	ClientEndFrame(ent);

	if (IS_FREEZETAG) {
		CalculateRanks(qtrue);
	}

	// clear entity state values
	BG_PlayerStateToEntityState(&client->ps, &ent->s, qtrue, 0);
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect(int clientNum) {
	gentity_t*   ent;
	int         team;

	// cleanup if we are kicking a bot that
	// hasn't spawned yet
	G_RemoveQueuedBotBegin(clientNum);

	ent = g_entities + clientNum;
	if (!ent->client) {
		return;
	}

	// OSP removed the "stop any following clients" loop (OSP_MV_Validate
	// below fixes the followers / multiview windows)
	level.clientChangeTime = level.time;
	if (ent->client->pers.mvCount) {
		OSP_MV_RemoveAll(ent);
	}
	ent->client->pers.mvCount = 0;

	// send effect if they were completely connected
	if (ent->client->pers.connected == CON_CONNECTED
	        && !is_spectator(ent->client)) {
		gentity_t*   tent;

		tent = G_TempEntity(ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT);
		tent->s.clientNum = ent->s.clientNum;

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		if (!level.intermissiontime) {
			TossClientItems(ent);
		}
	}

	G_LogPrintf("ClientDisconnect: %i\n", clientNum);
	OSP_AdminLogPrintf(va("Disconnect\t%s", ent->client->pers.netname));
	if (!level.intermissiontime) {
		OSP_LogWeaponStats(ent);
	}

	// if we are playing in tourney mode and losing, give a win to the other player
	if ((g_gametype.integer == GT_TOURNAMENT)
	        && !level.intermissiontime
	        && (!level.warmupTime || (osp_matchPaused && level.warmupTime != -1))
	        && level.sortedClients[1] == clientNum) {
		level.clients[ level.sortedClients[0] ].sess.wins++;
		ClientUserinfoChanged(level.sortedClients[0]);
		osp_matchPaused = 0;
		level.warmupTime = 0;
	}

	// OSP: the referee who paused the match left
	if (osp_matchPaused && ent->client->sess.referee && osp_matchPaused == (ent - g_entities) + 10001) {
		level.warmupTime = level.time + 5000;
		osp_timeoutEndTime = 0;
		trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
		trap_SendServerCommand(-1, "cp \"^1Referee disconnected!^7 Match resuming...\n\"");
		return;
	}

	ent->r.svFlags &= ~SVF_NOSERVERINFO;
	trap_UnlinkEntity(ent);
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->frozen = 0;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
	team = ent->client->sess.sessionTeam;
	ent->client->sess.sessionTeam = TEAM_FREE;
	ent->client->sess.ip[0] = 0;
	ent->client->pers.netname[0] = 0;

	trap_SetConfigstring(CS_PLAYERS + clientNum, "");

	CalculateRanks(qtrue);
	OSP_SortClients();

	if (team == TEAM_RED || team == TEAM_BLUE) {
		if (!team_nocontrols.integer && ent->client->sess.captain) {
			OSP_SetCaptain(ent);
		}
		OSP_StartCountdown();
	}

	if (ent->r.svFlags & SVF_BOT) {
		BotAIShutdownClient(clientNum, qfalse);
	}

	OSP_MV_Validate();
	if (IS_FREEZETAG && !level.warmupTime) {
		level.osp_unk_3804 = 0;
		FT_CheckRoundEnd();
	}
}


