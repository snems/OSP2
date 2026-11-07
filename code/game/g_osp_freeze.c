/*
 * g_osp_freeze.c: freeze tag (frozen bodies, thawing, round end).
 */

#include "g_local.h"

// OSP: this file's bss is FT_BodyKnockback's statics
// (mass, mapname).

// OSP: prototypes of other files' functions (not in g_local.h)
void    G_ExplodeMissile(gentity_t* ent);
gentity_t* SelectRandomDeathmatchSpawnPoint(gentity_t* avoid);   // OSP calls it without an `avoid` spot (FT_TeamWins)

/*
==================
FT_RespawnDelayMsec

freezetag_respawn (seconds, clamped to 1.7 .. 10) in msec
==================
*/
int FT_RespawnDelayMsec(void) {
	return 1000 * (freezetag_respawn.value > 10 ? 10 : (freezetag_respawn.value < 1.7f ? 1.7f : freezetag_respawn.value));
}

/*
==================
is_spectator

Nonzero for no client, spectators and followers
==================
*/
int is_spectator(gclient_t* client) {
	return !client || client->sess.sessionTeam == TEAM_SPECTATOR || client->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR
	       || client->sess.spectatorState == SPECTATOR_FOLLOW;
}

/*
==================
FT_RespawnFrozen

ClientRespawn in freeze tag: a frozen player stays where he is and follows his team until
thawed (bots just wait). Returns 0 when the normal respawn should run.
==================
*/
int FT_RespawnFrozen(gentity_t* ent) {
	vec3_t  angles;
	vec3_t  origin;

	if (level.intermissiontime || !ent->frozen) {
		return 0;
	}

	if (ent->r.svFlags & SVF_BOT) {
		ent->client->respawnTime = 0x7fffffff;
	} else if (!is_spectator(ent->client)) {
		VectorCopy(ent->r.currentOrigin, origin);
		angles[YAW] = ent->client->ps.stats[STAT_DEAD_YAW];
		angles[PITCH] = 0;
		angles[ROLL] = 0;
		ClientSpawn(ent);
		VectorCopy(origin, ent->client->ps.origin);
		SetClientViewAngle(ent, angles);
		ent->client->ps.persistant[PERS_TEAM] = TEAM_SPECTATOR;
		ent->client->sess.spectatorTime = osp_matchTime;
		ent->client->sess.spectatorState = SPECTATOR_FREE;
		ent->client->sess.spectatorClient = 0;
		Cmd_FollowCycle_f(ent, 1);
		trap_UnlinkEntity(ent);
	}
	return 1;
}

/*
==================
FT_FollowHelper
==================
*/
void FT_FollowHelper(gentity_t* ent) {
	if (!ent->frozen && ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
		gclient_t*   client;

		client = ent->client;
		if (osp_matchTime > client->respawnTime) {
			if (g_forcerespawn.integer > 0 && osp_matchTime - client->respawnTime > 1000 * g_forcerespawn.integer) {
				Cmd_FollowCycle_f(ent, 1);
			}
		}
	}
}

/*
==================
FT_SpectatorEndFrameHelper

Copies the followed client's playerState to a following spectator. The spectator keeps his
own score outside team games; in freeze tag he keeps every persistant value except the hits,
team and attacker (and the score).
==================
*/
void FT_SpectatorEndFrameHelper(gentity_t* ent, gclient_t* cl) {
	int     i;
	int     persistant[MAX_PERSISTANT];

	if (!IS_FREEZETAG) {
		int     score;

		score = ent->client->ps.persistant[PERS_SCORE];
		ent->client->ps = cl->ps;
		if (g_gametype.integer > GT_TOURNAMENT) {
			ent->client->ps.persistant[PERS_SCORE] = score;
		}
	} else {
		for (i = 0; i < MAX_PERSISTANT; i++) {
			persistant[i] = ent->client->ps.persistant[i];
		}
		ent->client->ps = cl->ps;
		for (i = 0; i < MAX_PERSISTANT; i++) {
			switch (i) {
				case PERS_SCORE:
					if (!IS_FREEZETAG) {
						break;
					}
				default:
					ent->client->ps.persistant[i] = persistant[i];
					break;
				case PERS_HITS:
				case PERS_TEAM:
				case PERS_ATTACKER:
					break;
			}
		}
	}
}

/*
==================
FT_BodyFree

Removes a frozen body (thaws its player first while the body is still frozen)
==================
*/
void FT_BodyFree(gentity_t* ent) {
	if (ent->frozen) {
		FT_Thaw(ent->target_ent);
	}
	ent->frozen = 0;
	ent->s.powerups = 0;
	G_FreeEntity(ent);
}

/*
==================
FT_CheckThaw

Called by the body's think. A living teammate of the frozen player within 100 units of the
body starts the thaw (2 s with dmflags 1024 or in CTF, else 3 s, body->count holds the end
time, body->activator the thawer); when the time is up and the thawer (or another teammate)
is still close, the player is thawed: obituary, "Kill:" log line with MOD_UNKNOWN, a thaw
(sess.losses) and a point for the thawer, and the body is destroyed. No teammate close: the
thaw is reset.
==================
*/
void FT_CheckThaw(gentity_t* self) {
	gentity_t*   ent;
	vec3_t      dir;
	int         i;
	gentity_t*   tent;

	for (i = 0; i < level.numConnectedClients; i++) {
		ent = &g_entities[ level.sortedClients[i] ];
		if (ent->health < 1) {
			continue;
		}
		if (ent->client->sess.sessionTeam != self->spawnflags) {
			continue;
		}
		if (is_spectator(ent->client)) {
			continue;
		}
		VectorSubtract(self->r.currentOrigin, ent->client->ps.origin, dir);
		if (VectorLength(dir) > 100) {
			continue;
		}

		if (!self->count) {
			self->count = osp_matchTime + (((g_dmflags.integer & 1024) || g_gametype.integer == GT_CTF) ? 2000 : 3000);
			G_Sound(self, CHAN_AUTO, self->noise_index);
			self->activator = ent;
			return;
		}

		if (self->count < osp_matchTime) {
			if (self->activator == ent) {
			} else if (!self->activator->inuse) {
			} else if (self->activator->health < 1) {
			} else {
				VectorSubtract(self->s.pos.trBase, self->activator->s.pos.trBase, dir);
				if (VectorLength(dir) > 100) {
				} else if (is_spectator(self->activator->client)) {
				} else {
					ent = self->activator;
				}
			}

			tent = G_TempEntity(self->target_ent->r.currentOrigin, EV_OBITUARY);
			tent->s.eventParm = MOD_UNKNOWN;
			tent->s.otherEntityNum = self->target_ent->s.number;
			tent->s.otherEntityNum2 = ent->s.number;
			tent->r.svFlags = SVF_BROADCAST;
			G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", ent->s.number, self->target_ent->s.number, 0,
			            ent->client->pers.netname, self->target_ent->client->pers.netname, "MOD_UNKNOWN");
			ent->client->sess.losses++;
			AddScore(ent, self->s.pos.trBase, 1);
			G_Damage(self, NULL, NULL, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
			return;
		}
		return;
	}
	self->count = 0;
}

/*
==================
FT_BodyWorldEffects

Called by the body's think every 250 msec: lava / slime destroy the body (and kill the
player) after the respawn delay, a body that lies still in a nodrop area is removed, water
events, and the jump pads / teleporters the body touches.
==================
*/
void FT_BodyWorldEffects(gentity_t* self) {
	gentity_t*   hit;
	int         i, num;
	int         touch[MAX_GENTITIES];
	int         contents;
	vec3_t      mins, maxs;
	vec3_t      origin;
	int         oldWaterlevel;

	VectorCopy(self->r.currentOrigin, origin);
	origin[2] -= 23;
	contents = trap_PointContents(origin, -1);
	if (contents & (CONTENTS_LAVA | CONTENTS_SLIME)) {
		if (osp_matchTime - self->pain_debounce_time > FT_RespawnDelayMsec()) {
			G_Damage(self, NULL, NULL, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
		}
		return;
	}
	if (self->s.pos.trType == TR_STATIONARY && (contents & CONTENTS_NODROP)) {
		if (osp_matchTime - self->pain_debounce_time > FT_RespawnDelayMsec()) {
			FT_BodyFree(self);
		}
		return;
	}
	self->pain_debounce_time = osp_matchTime;

	oldWaterlevel = self->waterlevel;
	self->waterlevel = 0;
	if (contents & (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME))
		self->waterlevel = 3;
	self->watertype = contents;
	if (!oldWaterlevel && self->waterlevel)
		G_AddEvent(self, EV_WATER_TOUCH, 0);
	if (oldWaterlevel && !self->waterlevel)
		G_AddEvent(self, EV_WATER_LEAVE, 0);

	VectorAdd(self->r.currentOrigin, self->r.mins, mins);
	VectorAdd(self->r.currentOrigin, self->r.maxs, maxs);
	num = trap_EntitiesInBox(mins, maxs, touch, MAX_GENTITIES);
	for (i = 0; i < num; i++) {
		hit = &g_entities[touch[i]];
		if (!hit->touch) {
			continue;
		}
		switch (hit->s.eType) {
			case ET_PUSH_TRIGGER:
				if (self->s.pos.trDelta[2] < 100) {
					G_Sound(self, CHAN_AUTO, G_SoundIndex("sound/world/jumppad.wav"));
				}
				VectorCopy(hit->s.origin2, self->s.pos.trDelta);
				self->s.pos.trType = TR_GRAVITY;
				self->s.pos.trTime = level.time;
				break;
			case ET_TELEPORT_TRIGGER:
			{
				gentity_t*   dest;
				vec3_t      fwd;
				gentity_t*   tent;

				dest = G_PickTarget(hit->target);
				tent = G_TempEntity(self->r.currentOrigin, EV_PLAYER_TELEPORT_OUT);
				tent->s.clientNum = self->s.number;
				tent = G_TempEntity(dest->s.origin, EV_PLAYER_TELEPORT_IN);
				tent->s.clientNum = self->s.number;
				trap_UnlinkEntity(self);
				G_SetOrigin(self, dest->s.origin);
				self->s.pos.trBase[2] += 60;
				self->s.pos.trType = TR_GRAVITY;
				AngleVectors(dest->s.angles, fwd, NULL, NULL);
				VectorScale(fwd, 800, self->s.pos.trDelta);
				self->s.pos.trDelta[2] += 42;
				self->s.pos.trTime = level.time;
				self->s.eFlags ^= EF_TELEPORT_BIT;
				VectorCopy(dest->s.angles, self->s.angles);
				G_KillBox(self);
				trap_LinkEntity(self);
			}
			break;
		}
	}
}

/*
==================
FT_BodyThink

The frozen body's think (every 100 msec). The body goes away when its player left, changed
team or respawned elsewhere; after 150 s (60 s with dmflags 1024 or in CTF) the player is
thawed and the body destroyed.
==================
*/
void FT_BodyThink(gentity_t* self) {
	self->nextthink = level.time + 100;

	if (!self->target_ent || !self->target_ent->client || !self->target_ent->inuse
	        || self->spawnflags != self->target_ent->client->sess.sessionTeam) {
		FT_BodyFree(self);
		return;
	}
	if (self->s.otherEntityNum != self->target_ent->s.number) {
		FT_BodyFree(self);
		return;
	}
	if (level.intermissiontime || level.intermissionQueued) {
		return;
	}

	if (osp_matchTime - self->timestamp > 150000 || (((g_dmflags.integer & 1024) || g_gametype.integer == GT_CTF)
	        && osp_matchTime - self->timestamp > 60000)) {
		FT_Thaw(self->target_ent);
		G_Damage(self, NULL, NULL, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
		return;
	}

	if (self->frozen) {
		if (!self->target_ent->frozen) {
			G_Damage(self, NULL, NULL, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
			return;
		}
		self->s.eFlags &= ~EF_TELEPORT_BIT;
		FT_CheckThaw(self);
		if (self->last_move_time < level.time) {
			FT_BodyWorldEffects(self);
			self->last_move_time = level.time + 250;
		}
	} else {
		if (osp_matchTime - self->timestamp > 6500) {
			FT_BodyFree(self);
			return;
		}
		self->s.pos.trBase[2] -= 1;
	}
}

/*
==================
FT_BodyDie

The body's die callback: gibbed below -40 health (a frozen body thaws its player, without
blood the body is destroyed with a telefrag instead).
==================
*/
void FT_BodyDie(gentity_t* self, gentity_t* inflictor, gentity_t* attacker, int damage, int mod) {
	gentity_t*   tent;

	if (self->health > -40) {
		return;
	}

	if (self->frozen && !g_blood.integer) {
		FT_Thaw(self->target_ent);
		G_Damage(self, NULL, NULL, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
		return;
	}

	tent = G_TempEntity(self->r.currentOrigin, EV_GIB_PLAYER);
	if (self->frozen) {
		tent->s.eventParm = 255;
	}
	FT_BodyFree(self);
}

/*
==================
FT_BodyKnockback

G_Damage: a client's hit on a frozen player moves his body (the knockback of a 175 unit mass,
300 on the maps where bodies get stuck or with dmflags 1024) instead of damaging him.
Nonzero when targ is a frozen player hit by a client.
==================
*/
int FT_BodyKnockback(gentity_t* targ, gentity_t* attacker, vec3_t dir, int mod, int knockback) {
	static float    mass;
	static char     mapname[128];
	vec3_t          kvel;

	if (mass == 0) {
		char    serverinfo[MAX_INFO_STRING];

		trap_GetServerinfo(serverinfo, sizeof(serverinfo));
		strncpy(mapname, Info_ValueForKey(serverinfo, "mapname"), sizeof(mapname) - 1);
		mapname[sizeof(mapname) - 1] = 0;
		if (!Q_stricmp(mapname, "q3tourney3") || !Q_stricmp(mapname, "q3dm16") || !Q_stricmp(mapname, "q3dm17") ||
		        !Q_stricmp(mapname, "q3dm18") || !Q_stricmp(mapname, "q3dm19") || !Q_stricmp(mapname, "q3tourney6") ||
		        !Q_stricmp(mapname, "q3ctf4") || !Q_stricmp(mapname, "mpq3ctf4") || !Q_stricmp(mapname, "mpq3tourney6") ||
		        !Q_stricmp(mapname, "mpteam6")) {
			mass = 300;
		} else {
			mass = 175;
		}
		if (g_dmflags.integer & 1024) {
			mass = 300;
		}
	}

	if (attacker->client && targ->frozen) {
		if (knockback) {
			VectorScale(dir, g_knockback.value * (float)knockback / mass, kvel);
			VectorCopy(targ->r.currentOrigin, targ->s.pos.trBase);
			VectorAdd(targ->s.pos.trDelta, kvel, targ->s.pos.trDelta);
			targ->s.pos.trType = TR_GRAVITY;
			if (targ->s.pos.trTime == level.time) {
				targ->s.pos.trDelta[2] += 8;
			} else {
				if (targ->s.pos.trDelta[2] <= 0)
					targ->s.pos.trDelta[2] += 24;
				targ->s.pos.trTime = level.time;
			}
			G_RunItem(targ);
		}
		return 1;
	}
	return 0;
}

/*
==================
is_body

A frozen player's body ("freezebody")
==================
*/
int is_body(gentity_t* ent) {
	return ent && ent->inuse && ent->classname && !Q_stricmp(ent->classname, "freezebody");
}

/*
==================
is_body_freeze

A body that still holds a frozen player
==================
*/
int is_body_freeze(gentity_t* ent) {
	return is_body(ent) ? ent->frozen : 0;
}

/*
==================
FT_CopyToBody

Spawns the frozen body of ent: a copy of the player that can be pushed around and shot
(think FT_BodyThink, die FT_BodyDie); ent->target_ent and the body's target_ent link them.
==================
*/
void FT_CopyToBody(gentity_t* ent) {
	gentity_t*   body;

	body = G_Spawn();
	body->classname = "freezebody";
	body->s = ent->s;
	body->s.eFlags = 0;
	body->s.powerups = 1 << PW_BATTLESUIT;
	body->s.number = body - g_entities;
	body->timestamp = osp_matchTime;
	body->pain_debounce_time = osp_matchTime;
	body->physicsObject = qtrue;
	G_SetOrigin(body, ent->r.currentOrigin);
	body->s.pos.trType = TR_GRAVITY;
	body->s.pos.trTime = level.time;
	VectorCopy(ent->client->ps.velocity, body->s.pos.trDelta);
	body->s.event = 0;

	// a running / standing player freezes in a random jump or land pose
	switch (body->s.legsAnim & ~ANIM_TOGGLEBIT) {
		case LEGS_WALKCR:
		case LEGS_WALK:
		case LEGS_RUN:
		case LEGS_BACK:
		case LEGS_SWIM:
		case LEGS_IDLE:
		case LEGS_IDLECR:
		case LEGS_TURN:
		case LEGS_BACKCR:
		case LEGS_BACKWALK:
			switch (rand() % 4) {
				case 0:
					body->s.legsAnim = LEGS_JUMP;
					break;
				case 1:
					body->s.legsAnim = LEGS_LAND;
					break;
				case 2:
					body->s.legsAnim = LEGS_JUMPB;
					break;
				case 3:
					body->s.legsAnim = LEGS_LANDB;
					break;
			}
			break;
	}

	body->r.svFlags = ent->r.svFlags;
	VectorCopy(ent->r.mins, body->r.mins);
	VectorCopy(ent->r.maxs, body->r.maxs);
	VectorCopy(ent->r.absmin, body->r.absmin);
	VectorCopy(ent->r.absmax, body->r.absmax);
	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY;
	body->r.contents = CONTENTS_BODY;
	body->think = FT_BodyThink;
	body->nextthink = level.time + 100;
	body->die = FT_BodyDie;
	body->takedamage = qtrue;
	ent->target_ent = body;
	body->target_ent = ent;
	body->s.otherEntityNum = ent->s.number;
	body->noise_index = G_SoundIndex("sound/player/tankjr/jump1.wav");
	body->frozen = 1;
	body->spawnflags = ent->client->sess.sessionTeam;
	body->waterlevel = ent->waterlevel;
	body->count = 0;
	body->physicsBounce = 0.33333f;
	body->s.weapon = WP_NONE;
	trap_LinkEntity(body);
}

/*
==================
player_freeze

player_die in freeze tag: the player is frozen into a body, or simply respawns after a
FT_RespawnDelayMsec delay for the deaths that don't freeze (world kills, suicides by
command, team switches).
==================
*/
void player_freeze(gentity_t* self, gentity_t* attacker, int mod) {
	if (level.warmupTime || !IS_FREEZETAG) {
		return;
	}

	level.osp_unk_3804 = osp_matchTime + 300;

	switch (mod) {
		case MOD_UNKNOWN:
		case MOD_CRUSH:
		case MOD_TELEFRAG:
		case MOD_TARGET_LASER:
		case MOD_TRIGGER_HURT:
		case MOD_GRAPPLE:
		case MOD_SWITCHTEAM:
			self->health = 0;
			self->client->respawnTime = osp_matchTime + FT_RespawnDelayMsec();
			return;
	}

	FT_CopyToBody(self);
	self->frozen = 1;
	self->health = GIB_HEALTH;
	self->r.contents = 0;
	self->r.maxs[2] = -8;
	self->s.eType = ET_INVISIBLE;
	self->takedamage = qfalse;
	self->client->pers.osp_unk_290 = level.teamScores[TEAM_RED] + level.teamScores[TEAM_BLUE] + 1;
	trap_UnlinkEntity(self);
	CalculateRanks(1);
}

/*
==================
FT_Thaw

Thaws a frozen player: he respawns where his body is not (FT_RespawnPlayer)
==================
*/
void FT_Thaw(gentity_t* ent) {
	if (!ent || !ent->inuse || !ent->frozen) {
		return;
	}

	if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW) {
		StopFollowing(ent);
		ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		ent->client->ps.pm_time = 100;
	}
	ent->frozen = 0;
	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
		ent->client->pers.osp_unk_290 = 0;
	}
	ent->client->respawnTime = osp_matchTime + 500;
	FT_RespawnPlayer(ent);
}

/*
==================
FT_RespawnPlayer

Respawns a dead (not frozen) player who is following a teammate, with a teleport effect.
Nonzero when it did.
==================
*/
int FT_RespawnPlayer(gentity_t* ent) {
	gclient_t*   client;
	gentity_t*   tent;

	client = ent->client;
	if (!client) {
		return 0;
	}
	if (client->sess.sessionTeam == TEAM_SPECTATOR) {
		return 0;
	}
	if (ent->frozen) {
		return 0;
	}

	client->sess.spectatorClient = 0;
	client->sess.spectatorState = SPECTATOR_NOT;
	ClientSpawn(ent);

	tent = G_TempEntity(client->ps.origin, EV_PLAYER_TELEPORT_IN);
	tent->s.clientNum = ent->s.clientNum;
	return 1;
}

/*
==================
FT_CheckRoundEnd

Every 500 msec: sends the frozen players as a bit mask (STAT_CLIENTS_READY) and gives the
round to the other team when every player of a team is frozen.
==================
*/
void FT_CheckRoundEnd(void) {
	int         i;
	gentity_t*   ent;
	int         clientNum;
	int         alive[TEAM_NUM_TEAMS];
	int         frozenMask;

	frozenMask = 0;
	if (level.osp_unk_3804 > osp_matchTime || level.intermissiontime || level.intermissionQueued) {
		return;
	}
	level.osp_unk_3804 = osp_matchTime + 500;

	for (i = 0; i < level.numConnectedClients; i++) {
		clientNum = level.sortedClients[i];
		if (!g_entities[clientNum].frozen) {
			continue;
		}
		if (clientNum < 32) {
			frozenMask |= 1 << clientNum;
		}
	}

	memset(alive, 0, sizeof(alive));
	// OSP bug: one client too many (sortedClients[numConnectedClients])
	for (i = 0; i <= level.numConnectedClients; i++) {
		ent = g_entities + level.sortedClients[i];
		ent->client->ps.stats[STAT_CLIENTS_READY] = frozenMask;
		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
			continue;
		}
		if (!is_spectator(ent->client) && !ent->frozen && ent->health > 0) {
			alive[ent->client->sess.sessionTeam]++;
		}
	}

	for (i = TEAM_RED; i <= TEAM_BLUE; i++) {
		if (!alive[i] && TeamCount(-1, i) > 0) {
			FT_TeamWins(~i & 3);
		}
	}
}

/*
==================
FT_TeamWins

team scores the round: everybody is thawed, the living players get their spawn health,
weapons and a round win (sess.wins), late joiners wait for the next round, missiles and
dropped powerups are removed.
==================
*/
void FT_TeamWins(int team) {
	gentity_t*   ent;
	gclient_t*   client;
	int         i;
	int         health;
	gentity_t*   spot;
	gentity_t*   tent;

	AddTeamScore(vec3_origin, team, 1);
	CalculateRanks(0);
	spot = SelectRandomDeathmatchSpawnPoint(NULL);

	for (i = 0; i < level.maxclients; i++) {
		ent = g_entities + i;
		client = ent->client;
		if (!client->pers.connected) {
			continue;
		}
		if (client->sess.sessionTeam == TEAM_SPECTATOR) {
			client->pers.osp_unk_290 = level.teamScores[TEAM_RED] + level.teamScores[TEAM_BLUE] + 1;
			continue;
		}
		if (ent->frozen) {
			if (!(g_dmflags.integer & 128) || client->sess.sessionTeam != team) {
				ent->client->respawnTime = osp_matchTime;
				FT_Thaw(ent);
			}
			continue;
		}
		if (ent->health <= 0) {
			ent->client->respawnTime = osp_matchTime;
			continue;
		}
		if (g_dmflags.integer & 64) {
			continue;
		}

		memset(client->ps.powerups, 0, sizeof(client->ps.powerups));
		health = client->ps.stats[STAT_HEALTH];
		OSP_SetSpawnWeapons(ent);
		if (g_gametype.integer == GT_TEAM && server_freezetag.integer == 2 && health > client->ps.stats[STAT_HEALTH]) {
			client->ps.stats[STAT_HEALTH] = health;
			ent->health = health;
		}
		if (!(g_dmflags.integer & 1024)) {
			G_UseTargets(spot, ent);
		}
		OSP_SelectSpawnWeapon(ent);
		ent->client->sess.wins++;
	}

	tent = G_TempEntity(vec3_origin, EV_GLOBAL_TEAM_SOUND);
	tent->s.eventParm = (team == TEAM_RED);
	tent->r.svFlags |= SVF_BROADCAST;
	trap_SendServerCommand(-1, va("cp \"%s team scores!\n\"", osp_teamNames[team]));
	trap_SendServerCommand(-1, va("print \"^3*** %s team scores!\n\"", osp_teamNames[team]));

	for (i = level.maxclients, ent = g_entities + level.maxclients; i < level.num_entities; i++, ent++) {
		if (!ent->inuse) {
			continue;
		}
		if (ent->think == G_ExplodeMissile) {
			G_FreeEntity(ent);
		}
		if (ent->s.eType == ET_ITEM && ent->item->giType == IT_POWERUP && ent->think == G_FreeEntity &&
		        g_gametype.integer == GT_TEAM && server_freezetag.integer == 1) {
			G_FreeEntity(ent);
		}
	}
}
