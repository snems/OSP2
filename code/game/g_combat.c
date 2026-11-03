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
// g_combat.c

#include "g_local.h"


/*
============
ScorePlum
============
*/
void ScorePlum( gentity_t *ent, vec3_t origin, int score ) {
	gentity_t *plum;

	plum = G_TempEntity( origin, EV_SCOREPLUM );
	// only send this temp entity to a single client
	plum->r.svFlags |= SVF_SINGLECLIENT;
	plum->r.singleClient = ent->s.number;
	//
	plum->s.otherEntityNum = ent->s.number;
	plum->s.time = score;
}

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, vec3_t origin, int score ) {
	if ( !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup (OSP scores during a pause)
	if ( level.warmupTime && !osp_matchPaused ) {
		return;
	}
	// show score plum
	ScorePlum(ent, origin, score);
	//
	ent->client->ps.persistant[PERS_SCORE] += score;
	if ( g_gametype.integer == GT_TEAM && !IS_FREEZETAG )
		AddTeamScore( origin, ent->client->sess.sessionTeam, score );
	CalculateRanks( !IS_FREEZETAG );
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	// register: matches OSP's frame layout
	register int		i;
	register gitem_t	*item;
	register int		weapon;
	gentity_t	*drop;

	item = NULL;

	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.
	if ( weapon == WP_MACHINEGUN || weapon == WP_GRAPPLING_HOOK ) {
		if ( self->client->ps.weaponstate == WEAPON_DROPPING ) {
			weapon = self->client->pers.cmd.weapon;
		}
		// cmd.weapon comes straight from the client: reject anything outside the weapon
		// table before it is used as a shift count or an index
		if ( weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS ) {
			weapon = WP_NONE;
		}
		if ( !( self->client->ps.stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
			weapon = WP_NONE;
		}
	}

	// OSP drops (weapon_deaddrop, not in instagib or clan arena); with
	// modeBackpacks set the dropped item also carries the player's ammo (an ammo box when
	// the weapon is the machinegun or less) and the drop is counted in the weapon stats
	// (the outer if has no braces: the temporaries of the inner condition stay allocated)
	if ( modeBackpacks != 0 || ( weapon > WP_MACHINEGUN && weapon != WP_GRAPPLING_HOOK &&
		self->client->ps.ammo[ weapon ] != 0 ) )
		if ( weapon_deaddrop.integer != 0 && match_instagib.integer <= 0 && g_gametype.integer != 5 ) {
			if ( modeBackpacks && weapon <= WP_MACHINEGUN ) {
				char	*ammoNames[] = { "Bullets", "Shells", "Grenades", "Rockets", "Lightning", "Slugs", "Cells" };
				int		ammoWeapons[] = { WP_MACHINEGUN, WP_SHOTGUN, WP_GRENADE_LAUNCHER, WP_ROCKET_LAUNCHER, WP_LIGHTNING, WP_RAILGUN, WP_PLASMAGUN };
				int		j;

				weapon = -1;
				for ( j = 0; j < 7; j++ ) {
					if ( self->client->ps.ammo[ammoWeapons[j]] ) {
						item = BG_FindItem( ammoNames[j] );
						weapon = 10000;
					}
				}
			} else {
				// find the item type for this weapon
				item = BG_FindItemForWeapon( weapon );
			}

			if ( weapon > -1 && item ) {
				// spawn the item
				drop = Drop_Item( self, item, 0, 0 );
				if ( weapon > WP_MACHINEGUN && weapon < WP_GRAPPLING_HOOK ) {
					int		drops;

					drops = ( self->client->sess.weaponStats[weapon].hits >> 16 ) + 1;
					self->client->sess.weaponStats[weapon].hits =
						( self->client->sess.weaponStats[weapon].hits & 0xffff ) + ( drops << 16 );
				}
				if ( modeBackpacks ) {
					drop->s.eFlags |= EF_BACKPACK;
					drop->damage = ( ( self->client->ps.ammo[WP_MACHINEGUN] & 0xff ) << 8 ) + ( self->client->ps.ammo[WP_SHOTGUN] & 0xff );
					drop->health = ( ( self->client->ps.ammo[WP_GRENADE_LAUNCHER] & 0xff ) << 8 ) + ( self->client->ps.ammo[WP_ROCKET_LAUNCHER] & 0xff );
					drop->splashDamage = ( ( self->client->ps.ammo[WP_LIGHTNING] & 0xff ) << 8 ) + ( self->client->ps.ammo[WP_RAILGUN] & 0xff );
					drop->splashRadius = ( ( self->client->ps.ammo[WP_PLASMAGUN] & 0xff ) << 8 ) + ( self->client->ps.ammo[WP_BFG] & 0xff );
				}
			}
		}

	// drop all the powerups if not in teamplay (OSP: and holdables in
	// freeze tag mode 2)
	if ( g_gametype.integer != GT_TEAM || ( g_gametype.integer == GT_TEAM && server_freezetag.integer == 2 ) ) {
		float	dropAngle;

		dropAngle = 0;
		if ( g_gametype.integer == GT_TEAM && server_freezetag.integer == 2 ) {
			for ( i = 1 ; i < 6 ; i++ ) {
				if ( i == HI_KAMIKAZE ) {
					continue;
				}
				if ( bg_itemlist[self->client->ps.stats[STAT_HOLDABLE_ITEM]].giTag == i ) {
					item = BG_FindItemForHoldable( i );
					if ( item ) {
						dropAngle += 45;
						Drop_Item( self, item, dropAngle, 0 );
					}
					break;
				}
			}
		}
		for ( i = 1 ; i < PW_NUM_POWERUPS ; i++ ) {
			if ( self->client->ps.powerups[ i ] > level.time ) {
				if ( !( item = BG_FindItemForPowerup( i ) ) ) {
					continue;
				}
				dropAngle += 45;
				drop = Drop_Item( self, item, dropAngle, 0 );
				// decide how many seconds it has left
				drop->count = ( self->client->ps.powerups[ i ] - level.time ) / 1000;
				if ( drop->count < 1 ) {
					drop->count = 1;
				}
			}
		}
	}
}

#ifdef MISSIONPACK

/*
=================
TossClientCubes
=================
*/
extern gentity_t	*neutralObelisk;

void TossClientCubes( gentity_t *self ) {
	gitem_t		*item;
	gentity_t	*drop;
	vec3_t		velocity;
	vec3_t		angles;
	vec3_t		origin;

	self->client->ps.generic1 = 0;

	// this should never happen but we should never
	// get the server to crash due to skull being spawned in
	if (!G_EntitiesFree()) {
		return;
	}

	if( self->client->sess.sessionTeam == TEAM_RED ) {
		item = BG_FindItem( "Red Cube" );
	}
	else {
		item = BG_FindItem( "Blue Cube" );
	}

	angles[YAW] = (float)(level.time % 360);
	angles[PITCH] = 0;	// always forward
	angles[ROLL] = 0;

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;

	if( neutralObelisk ) {
		VectorCopy( neutralObelisk->s.pos.trBase, origin );
		origin[2] += 44;
	} else {
		VectorClear( origin ) ;
	}

	drop = LaunchItem( item, origin, velocity, NULL );

	drop->nextthink = level.time + g_cubeTimeout.integer * 1000;
	drop->think = G_FreeEntity;
	drop->spawnflags = self->client->sess.sessionTeam;
}


/*
=================
TossClientPersistantPowerups
=================
*/
void TossClientPersistantPowerups( gentity_t *ent ) {
	gentity_t	*powerup;

	if( !ent->client ) {
		return;
	}

	if( !ent->client->persistantPowerup ) {
		return;
	}

	powerup = ent->client->persistantPowerup;

	powerup->r.svFlags &= ~SVF_NOCLIENT;
	powerup->s.eFlags &= ~EF_NODRAW;
	powerup->r.contents = CONTENTS_TRIGGER;
	trap_LinkEntity( powerup );

	ent->client->ps.stats[STAT_PERSISTANT_POWERUP] = 0;
	ent->client->persistantPowerup = NULL;
}
#endif


/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;
	vec3_t		angles;

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw ( dir );

	angles[YAW] = vectoyaw ( dir );
	angles[PITCH] = 0; 
	angles[ROLL] = 0;
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer ) {
	gentity_t *ent;
	int i;

	//if this entity still has kamikaze
	if (self->s.eFlags & EF_KAMIKAZE) {
		// check if there is a kamikaze timer around for this owner
		for (i = 0; i < MAX_GENTITIES; i++) {
			ent = &g_entities[i];
			if (!ent->inuse)
				continue;
			if (ent->activator != self)
				continue;
			if (strcmp(ent->classname, "kamikaze timer"))
				continue;
			G_FreeEntity(ent);
			break;
		}
	}
	// OSP: the g_blood test moved here from body_die
	if ( !g_blood.integer ) {
		self->health = GIB_HEALTH+1;
	} else {
		G_AddEvent( self, EV_GIB_PLAYER, killer );
	}
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	if ( self->health > GIB_HEALTH ) {
		return;
	}

	GibEntity( self, 0 );
}


// these are just for logging, the client prints its own messages
char	*modNames[] = {
	"MOD_UNKNOWN",
	"MOD_SHOTGUN",
	"MOD_GAUNTLET",
	"MOD_MACHINEGUN",
	"MOD_GRENADE",
	"MOD_GRENADE_SPLASH",
	"MOD_ROCKET",
	"MOD_ROCKET_SPLASH",
	"MOD_PLASMA",
	"MOD_PLASMA_SPLASH",
	"MOD_RAILGUN",
	"MOD_LIGHTNING",
	"MOD_BFG",
	"MOD_BFG_SPLASH",
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
#ifdef MISSIONPACK
	"MOD_NAIL",
	"MOD_CHAINGUN",
	"MOD_PROXIMITY_MINE",
	"MOD_KAMIKAZE",
	"MOD_JUICED",
#endif
	"MOD_GRAPPLE",
	"MOD_SWITCHTEAM"
};

#ifdef MISSIONPACK
/*
==================
Kamikaze_DeathActivate
==================
*/
void Kamikaze_DeathActivate( gentity_t *ent ) {
	G_StartKamikaze(ent);
	G_FreeEntity(ent);
}

/*
==================
Kamikaze_DeathTimer
==================
*/
void Kamikaze_DeathTimer( gentity_t *self ) {
	gentity_t *ent;

	ent = G_Spawn();
	ent->classname = "kamikaze timer";
	VectorCopy(self->s.pos.trBase, ent->s.pos.trBase);
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->think = Kamikaze_DeathActivate;
	ent->nextthink = level.time + 5 * 1000;

	ent->activator = self;
}

#endif

/*
==================
CheckAlmostCapture
==================
*/
void CheckAlmostCapture( gentity_t *self, gentity_t *attacker ) {
	gentity_t	*ent;
	vec3_t		dir;
	char		*classname;

	// if this player was carrying a flag
	if ( self->client->ps.powerups[PW_REDFLAG] ||
		self->client->ps.powerups[PW_BLUEFLAG] ||
		self->client->ps.powerups[PW_NEUTRALFLAG] ) {
		// get the goal flag this player should have been going for
		if ( g_gametype.integer == GT_CTF ) {
			if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
				classname = "team_CTF_blueflag";
			}
			else {
				classname = "team_CTF_redflag";
			}
		}
		else {
			if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
				classname = "team_CTF_redflag";
			}
			else {
				classname = "team_CTF_blueflag";
			}
		}
		ent = NULL;
		do
		{
			ent = G_Find(ent, FOFS(classname), classname);
		} while (ent && (ent->flags & FL_DROPPED_ITEM));
		// if we found the destination flag and it's not picked up
		if (ent && !(ent->r.svFlags & SVF_NOCLIENT) ) {
			// if the player was *very* close
			VectorSubtract( self->client->ps.origin, ent->s.origin, dir );
			if ( VectorLength(dir) < 200 ) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				if ( attacker->client ) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}

/*
==================
CheckAlmostScored
==================
*/
void CheckAlmostScored( gentity_t *self, gentity_t *attacker ) {
	gentity_t	*ent;
	vec3_t		dir;
	char		*classname;

	// if the player was carrying cubes
	if ( self->client->ps.generic1 ) {
		if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
			classname = "team_redobelisk";
		}
		else {
			classname = "team_blueobelisk";
		}
		ent = G_Find(NULL, FOFS(classname), classname);
		// if we found the destination obelisk
		if ( ent ) {
			// if the player was *very* close
			VectorSubtract( self->client->ps.origin, ent->s.origin, dir );
			if ( VectorLength(dir) < 200 ) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				if ( attacker->client ) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}

/*
==================
player_die
==================
*/
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	gentity_t	*ent;
	int			anim;
	int			contents;
	int			killer;
	int			i;
	char		*killerName, *obit;

	if ( self->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if ( level.intermissiontime ) {
		return;
	}

	// check for an almost capture
	CheckAlmostCapture( self, attacker );
	// check for a player that almost brought in cubes
	CheckAlmostScored( self, attacker );

	if (self->client && self->client->hook) {
		Weapon_HookFree(self->client->hook);
	}
	self->client->ps.pm_type = PM_DEAD;

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	// OSP: a team switch (MOD_SWITCHTEAM) is logged as a suicide in the
	// default log format
	if ( meansOfDeath < 0 || meansOfDeath >= sizeof( modNames ) / sizeof( modNames[0] ) ) {
		obit = "<bad obituary>";
	} else if ( log_default.integer && meansOfDeath == MOD_SWITCHTEAM ) {
		obit = "MOD_SUICIDE";
	} else {
		obit = modNames[ meansOfDeath ];
	}

	self->client->pers.lastDeathLocation = self->client->pers.teamState.location;

	// OSP: no kill log during warmup (unless paused); OSP's own format
	// appends the weapon
	if ( !level.warmupTime && !osp_matchPaused ) {
		if ( log_default.integer ) {
			G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n",
				killer, self->s.number, ( meansOfDeath == MOD_SWITCHTEAM ) ? MOD_SUICIDE : meansOfDeath, killerName,
				self->client->pers.netname, obit );
		} else {
			G_LogPrintf("Kill: %i %i %i: %s killed %s by %s %i\n",
				killer, self->s.number, meansOfDeath, killerName,
				self->client->pers.netname, obit, self->s.weapon );
		}
	}

	// broadcast the death event to everyone
	if ( meansOfDeath != MOD_SWITCHTEAM ) {
		ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
		ent->s.eventParm = meansOfDeath;
		ent->s.otherEntityNum = self->s.number;
		ent->s.otherEntityNum2 = killer;
		ent->r.svFlags = SVF_BROADCAST;	// send to everyone
	}

	self->enemy = attacker;

	self->client->ps.persistant[PERS_KILLED]++;

	// OSP stats (deaths, kills, suicides, team kills) outside warmup
	if ( !level.warmupTime || osp_matchPaused ) {
		self->client->sess.deaths++;
	}

	if (attacker && attacker->client) {
		attacker->client->lastkilled_client = self->s.number;

		if ( meansOfDeath == MOD_SWITCHTEAM ) {
			OSP_ResetClientStats( self - g_entities );
		} else if ( attacker == self ) {
			AddScore( attacker, self->r.currentOrigin, -1 );
			if ( self->client->ps.powerups[PW_NEUTRALFLAG] || self->client->ps.powerups[PW_REDFLAG] ||
				self->client->ps.powerups[PW_BLUEFLAG] ) {
				self->client->pers.teamState.flagtime += level.time - self->client->pers.teamState.flagsince;
			}
			if ( !level.warmupTime || osp_matchPaused ) {
				self->client->sess.suicides++;
				self->client->sess.deaths--;
				self->client->sess.suicidePenalty += 0.5f;
				// too many suicides in a team game: ban (filters list) and kick
				if ( g_gametype.integer >= GT_TEAM && match_maxsuicides.integer &&
					self->client->sess.suicidePenalty > match_maxsuicides.integer &&
					!( self->r.svFlags & SVF_BOT ) ) {
					char	name[MAX_NETNAME];
					char	userinfo[MAX_INFO_STRING];

					if ( level.numFilters < MAX_OSP_FILTERS ) {
						trap_GetUserinfo( self - g_entities, userinfo, sizeof( userinfo ) );
						Q_strncpyz( name, Info_ValueForKey( userinfo, "name" ), sizeof( name ) );
						OSP_NormalizeName( self->client->pers.netname, MAX_NETNAME, name, 1 );
						osp_filters[level.numFilters].type = 4;
						strcpy( osp_filters[level.numFilters].name, name );
						strcpy( osp_filters[level.numFilters].ip, self->client->sess.ip );
						strcpy( osp_filters[level.numFilters].password, "none" );
						level.numFilters++;
					}
					trap_SendServerCommand( -1, va( "print \"%s has been banned due to excessive suicide!\n\"", self->client->pers.netname ) );
					trap_SendConsoleCommand( EXEC_APPEND, va( "clientkick %d\n", self - g_entities ) );
					return;
				}
			}
		} else if ( OnSameTeam( self, attacker ) ) {
			AddScore( attacker, self->r.currentOrigin, -1 );
			if ( self->client->ps.powerups[PW_NEUTRALFLAG] || self->client->ps.powerups[PW_REDFLAG] ||
				self->client->ps.powerups[PW_BLUEFLAG] ) {
				self->client->pers.teamState.flagtime += level.time - self->client->pers.teamState.flagsince;
			}
			if ( !level.warmupTime || osp_matchPaused ) {
				attacker->client->sess.teamKills++;
				self->client->sess.deaths--;
			}
		} else {
			AddScore( attacker, self->r.currentOrigin, 1 );
			if ( !level.warmupTime || osp_matchPaused ) {
				attacker->client->sess.kills++;
			}

			if( meansOfDeath == MOD_GAUNTLET ) {

				// play humiliation on player
				attacker->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;

				// add the sprite over the player's head
				attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
				attacker->client->ps.eFlags |= EF_AWARD_GAUNTLET;
				attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;

				// also play humiliation on target
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_GAUNTLETREWARD;
			}

			// check for two kills in a short amount of time
			// if this is close enough to the last kill, give a reward sound
			if ( level.time - attacker->client->lastKillTime < CARNAGE_REWARD_TIME ) {
				// play excellent on player
				attacker->client->ps.persistant[PERS_EXCELLENT_COUNT]++;

				// add the sprite over the player's head
				attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
				attacker->client->ps.eFlags |= EF_AWARD_EXCELLENT;
				attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;
			}
			attacker->client->lastKillTime = level.time;

		}
	} else {
		AddScore( self, self->r.currentOrigin, -1 );
		if ( !level.warmupTime || osp_matchPaused ) {
			self->client->sess.suicides++;
			self->client->sess.deaths--;
		}
	}

	// Add team bonuses
	if ( meansOfDeath != MOD_SWITCHTEAM ) {
		Team_FragBonuses(self, inflictor, attacker);
	}

	// if client is in a nodrop area, don't drop anything (but return CTF flags!)
	// OSP: level flag 2 allows drops in nodrop areas; returned flags count in
	// the flag time
	contents = trap_PointContents( self->r.currentOrigin, -1 );
	if ( !( contents & CONTENTS_NODROP ) || ( level.osp_unk_3834 & 2 ) ) {
		TossClientItems( self );
	}
	else {
		if ( self->client->ps.powerups[PW_NEUTRALFLAG] ) {		// only happens in One Flag CTF
			Team_ReturnFlag( TEAM_FREE );
			self->client->pers.teamState.flagtime += level.time - self->client->pers.teamState.flagsince;
		}
		else if ( self->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
			Team_ReturnFlag( TEAM_RED );
			self->client->pers.teamState.flagtime += level.time - self->client->pers.teamState.flagsince;
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
			Team_ReturnFlag( TEAM_BLUE );
			self->client->pers.teamState.flagtime += level.time - self->client->pers.teamState.flagsince;
		}
	}

	Cmd_Score_f( self );		// show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.numConnectedClients ; i++ ) {
		gclient_t	*client;
		int			clientNum;

		clientNum = level.sortedClients[i];
		client = &level.clients[clientNum];
		if ( !is_spectator( client ) ) {
			continue;
		}
		if ( client->sess.spectatorState == SPECTATOR_FOLLOW && client->sess.spectatorClient == self->s.number ) {
			Cmd_Score_f( g_entities + clientNum );
		}
	}

	self->takedamage = qtrue;	// can still be gibbed

	self->s.weapon = WP_NONE;
	self->s.powerups = 0;
	self->r.contents = CONTENTS_CORPSE;

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;
	LookAtKiller (self, inflictor, attacker);

	VectorCopy( self->s.angles, self->client->ps.viewangles );

	self->s.loopSound = 0;

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = osp_matchTime + ( g_gametype.integer == 5 ? 1700.0f : modeUnk_27dc );

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	// OSP: freeze tag
	if ( IS_FREEZETAG ) {
		player_freeze( self, attacker, meansOfDeath );
		if ( self->frozen ) {
			G_AddEvent( self, EV_DEATH1 + rand() % 3, killer );
			trap_LinkEntity( self );
			return;
		}
	}

	self->r.maxs[2] = -8;

	// never gib in a nodrop
	if ( (self->health <= GIB_HEALTH && ( !(contents & CONTENTS_NODROP) || ( level.osp_unk_3834 & 2 ) ) && g_blood.integer) || meansOfDeath == MOD_SUICIDE) {
		// gib death
		GibEntity( self, killer );
	} else {
		// normal death
		static int last_death;

		switch ( last_death ) {
		case 0:
			anim = BOTH_DEATH1;
			break;
		case 1:
			anim = BOTH_DEATH2;
			break;
		case 2:
		default:
			anim = BOTH_DEATH3;
			break;
		}

		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if ( self->health <= GIB_HEALTH ) {
			self->health = GIB_HEALTH+1;
		}

		self->client->ps.legsAnim =
			( ( self->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
		self->client->ps.torsoAnim =
			( ( self->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

		G_AddEvent( self, EV_DEATH1 + last_death, killer );

		// the body can still be gibbed
		self->die = body_die;

		// globally cycle through the different death animations
		last_death = ( last_death + 1 ) % 3;
	}

	trap_LinkEntity (self);

}


/*
================
CheckArmor
================
*/
int CheckArmor (gentity_t *ent, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			count;
	float		protection;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	// armor
	count = client->ps.stats[STAT_ARMOR];
	// OSP: CPM armor types
	protection = pm_armorPromode ? armorKoeff[ent->client->ps.stats[STAT_ARMOR_TYPE]] : ARMOR_PROTECTION;
	save = ceil( damage * protection );
	if (save >= count)
		save = count;

	if (!save)
		return 0;

	client->ps.stats[STAT_ARMOR] -= save;

	return save;
}

/*
================
RaySphereIntersections
================
*/
int RaySphereIntersections( vec3_t origin, float radius, vec3_t point, vec3_t dir, vec3_t intersections[2] ) {
	float b, c, d, t;

	//	| origin - (point + t * dir) | = radius
	//	a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	//	c = (point[0] - origin[0])^2 + (point[1] - origin[1])^2 + (point[2] - origin[2])^2 - radius^2;

	// normalize dir so a = 1
	VectorNormalize(dir);
	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	c = (point[0] - origin[0]) * (point[0] - origin[0]) +
		(point[1] - origin[1]) * (point[1] - origin[1]) +
		(point[2] - origin[2]) * (point[2] - origin[2]) -
		radius * radius;

	d = b * b - 4 * c;
	if (d > 0) {
		t = (- b + sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[0]);
		t = (- b - sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[1]);
		return 2;
	}
	else if (d == 0) {
		t = (- b ) / 2;
		VectorMA(point, t, dir, intersections[0]);
		return 1;
	}
	return 0;
}

#ifdef MISSIONPACK
/*
================
G_InvulnerabilityEffect
================
*/
int G_InvulnerabilityEffect( gentity_t *targ, vec3_t dir, vec3_t point, vec3_t impactpoint, vec3_t bouncedir ) {
	gentity_t	*impact;
	vec3_t		intersections[2], vec;
	int			n;

	if ( !targ->client ) {
		return qfalse;
	}
	VectorCopy(dir, vec);
	VectorInverse(vec);
	// sphere model radius = 42 units
	n = RaySphereIntersections( targ->client->ps.origin, 42, point, vec, intersections);
	if (n > 0) {
		impact = G_TempEntity( targ->client->ps.origin, EV_INVUL_IMPACT );
		VectorSubtract(intersections[0], targ->client->ps.origin, vec);
		vectoangles(vec, impact->s.angles);
		impact->s.angles[0] += 90;
		if (impact->s.angles[0] > 360)
			impact->s.angles[0] -= 360;
		if ( impactpoint ) {
			VectorCopy( intersections[0], impactpoint );
		}
		if ( bouncedir ) {
			VectorCopy( vec, bouncedir );
			VectorNormalize( bouncedir );
		}
		return qtrue;
	}
	else {
		return qfalse;
	}
}
#endif
/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
	gclient_t	*client;
	int			take;
	int			asave;
	int			knockback;
	int			max;

	if (!targ->takedamage) {
		return;
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued ) {
		return;
	}
	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER ) {
		if ( targ->use && targ->moverState == MOVER_POS1 ) {
			targ->use( targ, inflictor, attacker );
		}
		return;
	}
	// reduce damage by the attacker's handicap value
	// unless they are rocket jumping
	if ( attacker->client && attacker != targ ) {
		max = attacker->client->ps.stats[STAT_MAX_HEALTH];
		damage = damage * max / 100;
	}

	client = targ->client;

	if ( client ) {
		if ( client->noclip ) {
			return;
		}
	}

	if ( !dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize(dir);
	}

	knockback = damage;
	if ( knockback > 200 ) {
		knockback = 200;
	}
	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	}
	if ( dflags & DAMAGE_NO_KNOCKBACK ) {
		knockback = 0;
	}

	// figure momentum add, even if the damage won't be taken
	// OSP: no knockback from clan arena missiles fired before the round
	// started; CPM scales the knockback per means of death
	if ( knockback && targ->client && ( g_gametype.integer != 5 || !inflictor->timestamp ||
		targ == attacker || ( !level.warmupTime && inflictor->timestamp > level.startTime ) ) ) {
		vec3_t	kvel;
		float	scale;
		float	mass;

		scale = 1.0f;
		if ( targ == attacker && mod == MOD_ROCKET_SPLASH ) {
			// rocket jumps keep the full knockback
		} else {
			switch ( mod ) {
			case MOD_GAUNTLET:
				scale = modeUnk_2748;
				break;
			case MOD_SHOTGUN:
				scale = modeUnused8;
				break;
			case MOD_ROCKET:
				scale = modeUnk_2788;
				break;
			case MOD_ROCKET_SPLASH:
				scale = modeUnk_2784;
				break;
			case MOD_LIGHTNING:
				scale = modeUnk_278c;
				break;
			case MOD_PLASMA:
				scale = modeUnk_279c;
				break;
			case MOD_LAVA:
				scale = modeUnk_27f4;
				break;
			case MOD_SLIME:
				scale = modeUnk_27f4;
				break;
			}
		}

		mass = 200;

		VectorScale (dir, scale * g_knockback.value * (float)knockback / mass, kvel);
		targ->client->ps.velocity[0] += kvel[0];
		targ->client->ps.velocity[1] += kvel[1];
		targ->client->ps.velocity[2] += kvel[2];

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int		t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {
		// OSP: clan arena: before the round, damage only counts as hits;
		// self damage only hurts armor (team_armordamage)
		if ( g_gametype.integer == 5 ) {
			if ( level.warmupTime || ( inflictor->timestamp && inflictor->timestamp < level.startTime ) ) {
				if ( attacker->client && targ != attacker && !OnSameTeam( targ, attacker ) ) {
					attacker->client->ps.persistant[PERS_HITS] += damage;
				}
				return;
			}
			if ( targ == attacker ) {
				if ( team_armordamage.integer && client && !client->ps.powerups[PW_BATTLESUIT] ) {
					if ( ( damage = 0.3f * damage ) < 1 ) {
						damage = 1;
					}
					client->damage_armor += CheckArmor( targ, damage, dflags );
				}
				return;
			} else if ( OnSameTeam( targ, attacker ) && mod != MOD_TELEFRAG ) {
				return;
			}
		}

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
		if ( targ != attacker && OnSameTeam (targ, attacker) ) {
			if ( !g_friendlyFire.integer && mod != MOD_TELEFRAG ) {	// OSP: telefrags always kill
				return;
			}
		}

		// OSP: match_hurtself
		if ( targ == attacker && !match_hurtself.integer ) {
			return;
		}

		// check for godmode
		if ( targ->flags & FL_GODMODE ) {
			return;
		}

		// OSP: freeze tag: spawn protection, frozen bodies
		if ( IS_FREEZETAG ) {
			if ( client ) {
				if ( targ != attacker && level.time - client->respawnTime < 1000 ) {
					return;
				}
			} else if ( FT_BodyKnockback( targ, attacker, dir, mod, knockback ) ) {
				return;
			}
		}
	}

	// battlesuit protects from all radius damage (but takes knockback)
	// and protects 50% against all damage
	if ( client && client->ps.powerups[PW_BATTLESUIT] ) {
		G_AddEvent( targ, EV_POWERUP_BATTLESUIT, 0 );
		if ( ( dflags & DAMAGE_RADIUS ) || ( mod == MOD_FALLING ) ) {
			return;
		}
		damage *= modeUnk_27b8;
	}

	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	// OSP counts the damage, not the hits
	if ( attacker->client && targ != attacker && targ->health > 0
			&& targ->s.eType != ET_MISSILE
			&& targ->s.eType != ET_GENERAL) {
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->ps.persistant[PERS_HITS] -= damage;
		} else {
			attacker->client->ps.persistant[PERS_HITS] += damage;
		}
	}

	if ( attacker != targ && attacker->client && targ->client ) {
		attacker->client->pers.lastDamagedClient = targ->s.number + 1;
	}

	// always give half damage if hurting self
	// calculated after knockback, so rocket jumping works
	if ( targ == attacker) {
		damage *= 0.5;
	} else if ( damage < 1 ) {
		damage = 1;
	}

	if ( damage < 1 ) {
		damage = 1;
	}
	take = damage;

	// save some from armor
	asave = CheckArmor (targ, take, dflags);
	take -= asave;

	if ( g_debugDamage.integer ) {
		G_Printf( "%i (%i): client:%i health:%i damage:%i armor:%i\n", level.time, level.framenum, targ->s.number,
			targ->health, take, asave );
	}

	// OSP: stats log line
	if ( log_stat.integer && targ->client && !level.warmupTime && !osp_matchPaused ) {
		int		health;

		health = targ->health - take;
		if ( health < -999 ) {
			health = -999;
		}
		G_LogPrintf( "PS: %i %i %i %i %i\n", targ->s.number, health, targ->client->ps.stats[STAT_ARMOR],
			( attacker && attacker->s.number < MAX_CLIENTS && attacker->s.number >= 0 ) ? attacker->s.number : ENTITYNUM_WORLD, mod );
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) {
		if ( attacker ) {
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		if ( dir ) {
			VectorCopy ( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}
	}

	// See if it's the player hurting the emeny flag carrier
	if( g_gametype.integer == GT_CTF) {
		Team_CheckHurtCarrier(targ, attacker);
	}

	if (targ->client) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
	}

	// do the damage
	if (take) {
		targ->health -= take;
		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
			OSP_DamageStats( targ, attacker, damage, mod );
		}

		if ( targ->health <= 0 ) {
			if ( client )
				targ->flags |= FL_NO_KNOCKBACK;

			if (targ->health < -999)
				targ->health = -999;

			targ->enemy = attacker;
			targ->die (targ, inflictor, attacker, take, mod);
			return;
		} else if ( targ->pain ) {
			targ->pain (targ, attacker, take);
		}
	}

}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (gentity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale (midpoint, 0.5, midpoint);

	VectorCopy (midpoint, dest);
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;


	return qfalse;
}


/*
============
G_RadiusDamage
============
*/
// OSP passes the missile (inflictor) and takes the attacker from its parent
qboolean G_RadiusDamage ( vec3_t origin, gentity_t *inflictor, float damage, float radius,
					 gentity_t *ignore, int mod) {
	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;
	gentity_t	*attacker = inflictor->parent;

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		// OSP: clan arena: no splash damage to others during warmup or from
		// missiles fired before the round started
		if ( g_gametype.integer == 5 && ent != attacker &&
			( level.warmupTime || ( inflictor->timestamp && inflictor->timestamp < level.startTime ) ) )
			continue;
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

		if( CanDamage (ent, origin) ) {
			if( LogAccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			if ( ent == attacker ) {
				dir[2] += 24;
			} else {
				dir[2] += modeUnk_2744;	// OSP: CPM knockback height
			}
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
	}

	return hitClient;
}
