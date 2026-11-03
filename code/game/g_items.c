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

  Items are any object that a player can touch to gain some effect.

  Pickup will return the number of seconds until they should respawn.

  all items should pop when dropped in lava or slime

  Respawnable items don't actually go away when picked up, they are
  just made invisible and untouchable.  This allows them to ride
  movers and respawn apropriately.
*/


#define	RESPAWN_ARMOR		25
#define	RESPAWN_HEALTH		35
#define	RESPAWN_AMMO		40
#define	RESPAWN_HOLDABLE	60
#define	RESPAWN_MEGAHEALTH	35//120
#define	RESPAWN_POWERUP		120


//======================================================================

int Pickup_Powerup( gentity_t *ent, gentity_t *other ) {
	int			quantity;
	int			i;
	gclient_t	*client;

	if ( !other->client->ps.powerups[ent->item->giTag] ) {
		// round timing to seconds to make multiple powerup timers
		// count in sync
		other->client->ps.powerups[ent->item->giTag] = 
			level.time - ( level.time % 1000 );
	}

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	other->client->ps.powerups[ent->item->giTag] += quantity * 1000;

	// give any nearby players a "denied" anti-reward
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		vec3_t		delta;
		float		len;
		vec3_t		forward;
		trace_t		tr;

		client = &level.clients[i];
		if ( client == other->client ) {
			continue;
		}
		if ( client->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
			continue;
		}
		// OSP: no anti-reward for spectators
		if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

    // if same team in team game, no sound
    // cannot use OnSameTeam as it expects to g_entities, not clients
  	if ( g_gametype.integer >= GT_TEAM && other->client->sess.sessionTeam == client->sess.sessionTeam  ) {
      continue;
    }

		// if too far away, no sound
		VectorSubtract( ent->s.pos.trBase, client->ps.origin, delta );
		len = VectorNormalize( delta );
		if ( len > 192 ) {
			continue;
		}

		// if not facing, no sound
		AngleVectors( client->ps.viewangles, forward, NULL, NULL );
		if ( DotProduct( delta, forward ) < 0.4 ) {
			continue;
		}

		// if not line of sight, no sound
		trap_Trace( &tr, client->ps.origin, NULL, NULL, ent->s.pos.trBase, ENTITYNUM_NONE, CONTENTS_SOLID );
		if ( tr.fraction != 1.0 ) {
			continue;
		}

		// anti-reward
		client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_DENIEDREWARD;
	}
	// OSP: respawn times from the CPM settings (battlesuit, CTF, others)
	return ( ent->item->giTag == PW_BATTLESUIT ) ? modeRespawnBattlesuit :
		( ( g_gametype.integer != GT_CTF ) ? modeRespawnPowerup : modeRespawnPowerupCTF );
}

//======================================================================

#ifdef MISSIONPACK
int Pickup_PersistantPowerup( gentity_t *ent, gentity_t *other ) {
	int		clientNum;
	char	userinfo[MAX_INFO_STRING];
	float	handicap;
	int		max;

	other->client->ps.stats[STAT_PERSISTANT_POWERUP] = ent->item - bg_itemlist;
	other->client->persistantPowerup = ent;

	switch( ent->item->giTag ) {
	case PW_GUARD:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		max = (int)(2 *  handicap);

		other->health = max;
		other->client->ps.stats[STAT_HEALTH] = max;
		other->client->ps.stats[STAT_MAX_HEALTH] = max;
		other->client->ps.stats[STAT_ARMOR] = max;
		other->client->pers.maxHealth = max;

		break;

	case PW_SCOUT:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		other->client->ps.stats[STAT_ARMOR] = 0;
		break;

	case PW_DOUBLER:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		break;
	case PW_AMMOREGEN:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		memset(other->client->ammoTimes, 0, sizeof(other->client->ammoTimes));
		break;
	default:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		break;
	}

	return -1;
}

//======================================================================
#endif

int Pickup_Holdable( gentity_t *ent, gentity_t *other ) {

	other->client->ps.stats[STAT_HOLDABLE_ITEM] = ent->item - bg_itemlist;

	// OSP: no kamikaze
	return RESPAWN_HOLDABLE;
}


//======================================================================

void Add_Ammo (gentity_t *ent, int weapon, int count)
{
	int		max;

	// OSP: CPM ammo limits
	switch ( weapon ) {
	case WP_SHOTGUN:
		max = modeMaxAmmoShotgun;
		break;
	case WP_GRENADE_LAUNCHER:
		max = modeMaxAmmoGrenade;
		break;
	case WP_ROCKET_LAUNCHER:
		max = modeMaxAmmoRocket;
		break;
	case WP_RAILGUN:
		max = modeMaxAmmoRail;
		break;
	default:
		max = 200;
		break;
	}

	ent->client->ps.ammo[weapon] += count;
	if ( ent->client->ps.ammo[weapon] > max ) {
		ent->client->ps.ammo[weapon] = max;
	}
}

/*
==================
OSP_PrintBackpack

OSP: picking up a backpack (EF_BACKPACK): gives the dropped weapon and the packed ammo
(two counts per int in damage/health/splashDamage/splashRadius, see TossClientItems) and
prints the contents
==================
*/
int OSP_PrintBackpack( gentity_t *ent, gentity_t *other ) {
	qboolean	printed = qfalse;
	char		*weaponName = NULL;
	char		*ammoNames[] = { "Bullets", "Shells", "Grenades", "Rockets", "Lightning", "Slugs", "Cells", "BFG" };
	int			i;
	gitem_t		*item;
	int			ammo[8];
	char		buf[MAX_STRING_CHARS];

	if ( !( ent->s.eFlags & EF_BACKPACK ) ) {
		return 0;
	}

	for ( item = bg_itemlist + 1 ; item->classname && !weaponName ; item++ ) {
		if ( item->giType == IT_WEAPON && ent->item->giType == IT_WEAPON && item->giTag == ent->item->giTag ) {
			weaponName = item->pickup_name;
			other->client->ps.stats[STAT_WEAPONS] |= ( 1 << ent->item->giTag );
		}
	}

	Com_sprintf( buf, sizeof( buf ), "print \"Backpack: ^3%s^7 (", weaponName ? weaponName : "No Weapon" );

	ammo[0] = ent->damage >> 8;
	ammo[1] = ent->damage & 0xff;
	ammo[2] = ent->health >> 8;
	ammo[3] = ent->health & 0xff;
	ammo[4] = ent->splashDamage >> 8;
	ammo[5] = ent->splashDamage & 0xff;
	ammo[6] = ent->splashRadius >> 8;
	ammo[7] = ent->splashRadius & 0xff;

	for ( i = 0 ; i < 8 ; i++ ) {
		if ( ammo[i] ) {
			strcat( buf, va( "%s%d %s", printed ? ", " : "", ammo[i], ammoNames[i] ) );
			Add_Ammo( other, i + WP_MACHINEGUN, ammo[i] );
			printed = qtrue;
		}
	}

	strcat( buf, ")\n\"" );
	trap_SendServerCommand( other - g_entities, buf );
	return 1;
}

int Pickup_Ammo (gentity_t *ent, gentity_t *other)
{
	int		quantity;

	// OSP: backpacks
	if ( OSP_PrintBackpack( ent, other ) ) {
		return 1;
	}

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		// OSP: CPM ammo box sizes
		switch ( ent->item->giTag ) {
		case WP_MACHINEGUN:
			quantity = modeAmmoBoxBullets;
			break;
		case WP_RAILGUN:
			quantity = modeAmmoBoxSlugs;
			break;
		default:
			quantity = ent->item->quantity;
			break;
		}
	}

	if ( other->client->ps.ammo[ ent->item->giTag ] < 200 ) {
		Add_Ammo (other, ent->item->giTag, quantity);
	}

	return modeRespawnAmmo;
}

//======================================================================


int Pickup_Weapon (gentity_t *ent, gentity_t *other) {
	int		weapon;
	int		quantity;

	// OSP: backpacks
	if ( OSP_PrintBackpack( ent, other ) ) {
		return 1;
	}

	weapon = ent->item->giTag;
	if ( ent->count < 0 ) {
		quantity = 0; // None for you, sir!
	} else {
		if ( ent->count ) {
			quantity = ent->count;
		} else {
			switch ( weapon ) {
			case WP_MACHINEGUN:
				quantity = modeWeaponAmmoMG;
				break;
			default:
				quantity = ent->item->quantity;
				break;
			}
		}

		// dropped items and teamplay weapons always have full ammo
		// (OSP: in freeze tag mode 1 weapons give half their ammo)
		if ( ! (ent->flags & FL_DROPPED_ITEM) && ( g_gametype.integer != GT_TEAM ||
			( g_gametype.integer == GT_TEAM && server_freezetag.integer == 1 ) ) ) {
			if ( g_gametype.integer == GT_TEAM && server_freezetag.integer == 1 ) {
				if ( other->client->ps.ammo[ weapon ] < quantity / 2 ) {
					quantity = quantity - other->client->ps.ammo[ weapon ];
				} else {
					quantity = quantity / 2;
				}
			} else {
				// respawning rules
				// drop the quantity if the already have over the minimum
				if ( other->client->ps.ammo[ weapon ] < quantity ) {
					quantity = quantity - other->client->ps.ammo[ weapon ];
				} else {
					quantity = 1;		// only add a single shot
				}
			}
		}
	}

	// add the weapon
	other->client->ps.stats[STAT_WEAPONS] |= ( 1 << weapon );
	other->client->weaponCount[weapon]++;

	if ( other->client->ps.ammo[ weapon ] < 200 ) {
		Add_Ammo( other, weapon, quantity );
	}

	if (ent->item->giTag == WP_GRAPPLING_HOOK)
		other->client->ps.ammo[weapon] = -1; // unlimited ammo

	// OSP: weapon stats, pickups in the high 16 bits
	if ( weapon > WP_MACHINEGUN && weapon < WP_GRAPPLING_HOOK ) {
		quantity = ( other->client->sess.weaponStats[weapon].atts >> 16 ) + 1;
		other->client->sess.weaponStats[weapon].atts =
			( other->client->sess.weaponStats[weapon].atts & 0xffff ) + ( quantity << 16 );
	}

	// team deathmatch has slow weapon respawns
	return ( g_gametype.integer == GT_TEAM ) ? g_weaponTeamRespawn.integer : g_weaponRespawn.integer;
}

/*
==================
OSP_ItemThink

OSP: think function of a health item held back while its activator stays above 100 health
(checked every second); afterwards the item respawns in 20 seconds
==================
*/
void OSP_ItemThink( gentity_t *ent ) {
	if ( !ent->activator || strcmp( ent->activator->classname, "player" ) || !ent->activator->client ||
		ent->activator->client->pers.connected != CON_CONNECTED ||
		ent->activator->client->ps.stats[STAT_HEALTH] <= 100 ) {
		ent->think = RespawnItem;
		ent->nextthink = level.time + 20000;
	} else {
		ent->nextthink = level.time + 1000;
	}
}


//======================================================================

int Pickup_Health (gentity_t *ent, gentity_t *other) {
	int			max;
	int			quantity;

	// small and mega healths will go over the max
	if ( ent->item->quantity != 5 && ent->item->quantity != 100 ) {
		max = other->client->ps.stats[STAT_MAX_HEALTH];
	} else {
		max = other->client->ps.stats[STAT_MAX_HEALTH] * 2;
	}

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	// OSP: health taken, in the high 16 bits of sess.losses
	other->client->sess.losses = ( other->client->sess.losses & 0xffff ) +
		( ( ( other->client->sess.losses >> 16 ) + quantity ) << 16 );

	other->health += quantity;

	if (other->health > max ) {
		other->health = max;
	}
	other->client->ps.stats[STAT_HEALTH] = other->health;

	if ( log_stat.integer != 0 && level.warmupTime == 0 && osp_matchPaused == 0 ) {
		G_LogPrintf( "PS: %i %i %i -1 HEALTH_PICKUP\n", other->s.number, other->health, other->client->ps.stats[STAT_ARMOR] );
	}

	if ( ent->item->quantity == 100 ) {		// mega health respawns slow
		other->client->sess.megaHealthPickups++;
		// OSP: the mega health respawn timer starts when the player's health drops to 100
		if ( modeMegaHealthWait ) {
			if ( g_gametype.integer >= GT_TEAM ) {
				return RESPAWN_MEGAHEALTH;
			}
			ent->think = OSP_ItemThink;
			ent->activator = other;
			return 1;
		}
		return RESPAWN_MEGAHEALTH;
	}

	return modeRespawnHealth;
}

//======================================================================

int Pickup_Armor( gentity_t *ent, gentity_t *other ) {
	// OSP: armor taken, in the high 16 bits of sess.wins, and armor pickups by type
	other->client->sess.wins = ( other->client->sess.wins & 0xffff ) +
		( ( ( other->client->sess.wins >> 16 ) + ent->item->quantity ) << 16 );
	if ( ent->item->quantity == 25 )
		other->client->sess.greenArmorPickups++;
	if ( ent->item->quantity == 50 )
		other->client->sess.yellowArmorPickups++;
	if ( ent->item->quantity == 100 )
		other->client->sess.redArmorPickups++;

	if ( armor_q2style.integer ) {
		// OSP: Quake 2 style armor limits
		if ( ent->item->quantity == 5 ) {
			other->client->ps.stats[STAT_ARMOR] += 5;
		} else if ( ent->item->quantity == 50 ) {
			if ( other->client->ps.stats[STAT_ARMOR] >= 100 ) {
				return 0;
			}
			other->client->ps.stats[STAT_ARMOR] += 50;
			if ( other->client->ps.stats[STAT_ARMOR] > 100 ) {
				other->client->ps.stats[STAT_ARMOR] = 100;
			}
		} else if ( other->client->ps.stats[STAT_ARMOR] < 200 ) {
			other->client->ps.stats[STAT_ARMOR] += 100;
			if ( other->client->ps.stats[STAT_ARMOR] > 200 ) {
				other->client->ps.stats[STAT_ARMOR] = 200;
			}
		} else {
			return 0;
		}
	} else if ( pm_armorPromode ) {
		// OSP: CPM armor classes (register: matches OSP's frame layout)
		register int	armorType;
		register int	armorMax;
		register float	scale;
		register int	armorAdd;
		int				playerType;

		armorType = ARMOR_GREEN;
		armorMax = 200;
		armorAdd = 0;
		scale = 1.0f;
		playerType = other->client->ps.stats[STAT_ARMOR_TYPE];

		switch ( ent->item->quantity ) {
		case 100:
			armorType = ARMOR_RED;
			armorAdd = 150;
			armorMax = 200;
			break;
		case 50:
			armorType = ARMOR_YELLOW;
			armorAdd = 100;
			armorMax = 150;
			break;
		case 25:
			armorType = ARMOR_GREEN;
			armorAdd = 50;
			armorMax = 100;
			break;
		case 5:
			if ( other->client->ps.stats[STAT_ARMOR] <= 0 ) {
				playerType = ARMOR_GREEN;
			}
			armorType = playerType;
			armorAdd = 5;
			armorMax = 200;
			break;
		}

		if ( playerType != armorType ) {
			scale = armorKoeff[playerType] / armorKoeff[armorType];
		}
		other->client->ps.stats[STAT_ARMOR] *= scale;
		other->client->ps.stats[STAT_ARMOR] += armorAdd;
		if ( other->client->ps.stats[STAT_ARMOR] > armorMax ) {
			other->client->ps.stats[STAT_ARMOR] = armorMax;
		}
		other->client->ps.stats[STAT_ARMOR_TYPE] = armorType;
	} else {
		other->client->ps.stats[STAT_ARMOR] += ent->item->quantity;
		if ( other->client->ps.stats[STAT_ARMOR] > other->client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
			other->client->ps.stats[STAT_ARMOR] = other->client->ps.stats[STAT_MAX_HEALTH] * 2;
		}
		other->client->ps.stats[STAT_ARMOR_TYPE] = ARMOR_YELLOW;
	}

	if ( log_stat.integer != 0 && level.warmupTime == 0 && osp_matchPaused == 0 ) {
		G_LogPrintf( "PS: %i %i %i -1 ARMOR_PICKUP\n", other->s.number, other->health, other->client->ps.stats[STAT_ARMOR] );
	}

	return RESPAWN_ARMOR;
}

//======================================================================

/*
===============
RespawnItem
===============
*/
void RespawnItem( gentity_t *ent ) {
	// randomly select from teamed entities
	if (ent->team) {
		gentity_t	*master;
		gentity_t	*self;
		int	count;
		int choice;

		if ( !ent->teammaster ) {
			G_Error( "RespawnItem: bad teammaster");
		}
		master = ent->teammaster;
		self = ent;

		// OSP: only the items that are not disabled take part
		for (count = 0, ent = master; ent; ent = ent->teamchain) {
			if ( OSP_ItemAllowed( ent ) ) {
				count++;
			}
		}

		if ( !count ) {
			self->nextthink = 0;
			self->think = 0;
			return;
		}

		choice = rand() % count;

		for (count = 0, ent = master; count < choice || !OSP_ItemAllowed( ent ); ent = ent->teamchain) {
			if ( OSP_ItemAllowed( ent ) ) {
				count++;
			}
		}
	}

	ent->r.contents = CONTENTS_TRIGGER;
	ent->s.eFlags &= ~EF_NODRAW;
	ent->r.svFlags &= ~SVF_NOCLIENT;
	ent->s.eFlags &= ~EF_BACKPACK;
	trap_LinkEntity (ent);

	if ( ent->item->giType == IT_POWERUP ) {
		// play powerup spawn sound to all clients
		gentity_t	*te;

		// if the powerup respawn sound should Not be global
		if (ent->speed) {
			te = G_TempEntity( ent->s.pos.trBase, EV_GENERAL_SOUND );
		}
		else {
			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
		}
		te->s.eventParm = G_SoundIndex( "sound/items/poweruprespawn.wav" );
		te->r.svFlags |= SVF_BROADCAST;
	}

	// play the normal respawn sound only to nearby clients
	G_AddEvent( ent, EV_ITEM_RESPAWN, 0 );

	ent->nextthink = 0;
}


/*
===============
Touch_Item
===============
*/
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace) {
	int			respawn;

	if ( !other->client || other->health < 1 )
		return;		// dead people can't pickup

	// OSP
	if ( other->frozen ) {
		return;
	}
	// OSP: no flag pickups during warmup
	if ( bg_itemlist[ent->s.modelindex].giType == IT_TEAM && level.warmupTime ) {
		return;
	}

	// the same pickup rules are used for client side and server side
	// (OSP: disabled items; the player who dropped an item can't pick it up again at once)
	if ( !OSP_ItemAllowed( ent ) || ( ent->parent == other && level.time < ent->timestamp ) ||
		!BG_CanItemBeGrabbed( g_gametype.integer, &ent->s, &other->client->ps, armor_q2style.integer ) ) {
		return;
	}

	if ( level.warmupTime == 0 && osp_matchPaused == 0 ) {
		G_LogPrintf( "Item: %i %s\n", other->s.number, ent->item->classname );
	}

	// call the item-specific pickup function
	switch( ent->item->giType ) {
	case IT_WEAPON:
		respawn = Pickup_Weapon(ent, other);
//		predict = qfalse;
		break;
	case IT_AMMO:
		respawn = Pickup_Ammo(ent, other);
//		predict = qfalse;
		break;
	case IT_ARMOR:
		respawn = Pickup_Armor(ent, other);
		break;
	case IT_HEALTH:
		respawn = Pickup_Health(ent, other);
		break;
	case IT_POWERUP:
		respawn = Pickup_Powerup(ent, other);
		break;
	case IT_TEAM:
		respawn = Pickup_Team(ent, other);
		break;
	case IT_HOLDABLE:
		respawn = Pickup_Holdable(ent, other);
		break;
	default:
		return;
	}

	if ( !respawn ) {
		return;
	}

	// OSP: remembered for the chat tokens
	other->client->lastPickupName = ent->item->pickup_name;

	// powerup pickups are global broadcasts
	if ( ent->item->giType == IT_POWERUP || ent->item->giType == IT_TEAM) {
		// if we want the global sound to play
		if (!ent->speed) {
			gentity_t	*te;

			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
			te->s.eventParm = ent->s.modelindex;
			te->r.svFlags |= SVF_BROADCAST;
		} else {
			gentity_t	*te;

			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
			te->s.eventParm = ent->s.modelindex;
			// only send this temp entity to a single client
			te->r.svFlags |= SVF_SINGLECLIENT;
			te->r.singleClient = other->s.number;
		}
		OSP_FollowPowerupPickup( other );
	}

	// fire item targets
	G_UseTargets (ent, other);

	// play the normal pickup sound (OSP: after the targets, not for powerups and flags)
	if ( ent->item->giType != IT_POWERUP && ent->item->giType != IT_TEAM ) {
		if ( other->client->pers.predictItemPickup ) {
			if ( other->client->ps.externalEvent ) {
				// OSP: an external event is pending, use a temp entity
				gentity_t	*te;

				te = G_TempEntity( other->r.currentOrigin, EV_ITEM_PICKUP );
				G_AddEvent( te, EV_ITEM_PICKUP, ent->s.modelindex );
			} else {
				G_AddPredictableEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );
			}
		} else {
			int		event;

			event = other->client->ps.externalEvent & ~EV_EVENT_BITS;
			if ( event == EV_ITEM_PICKUP ) {
				// OSP: the player already has a pickup event this frame: let the item
				// send it and finish the pickup 100 msec later (OSP_DelayedPickupThink)
				G_AddEvent( ent, EV_ITEM_PICKUP, ent->s.modelindex );
				ent->pickupRespawn = respawn;
				ent->nextthink = level.time + 100;
				ent->think = OSP_DelayedPickupThink;
				ent->s.eFlags |= EF_NODRAW;
				ent->r.contents = 0;
				ent->s.modelindex2 = ( other - g_entities ) + 16;
				return;
			}
			G_AddEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );
		}
	}

	// wait of -1 will not respawn
	if ( ent->wait == -1 ) {
		ent->r.svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->unlinkAfterEvent = qtrue;
		return;
	}

	// non zero wait overrides respawn time (OSP: not for a held back mega health)
	if ( ent->wait && ent->think != OSP_ItemThink ) {
		respawn = ent->wait;
	}

	// random can be used to vary the respawn time
	if ( ent->random ) {
		respawn += crandom() * ent->random;
		if ( respawn < 1 ) {
			respawn = 1;
		}
	}

	// dropped items will not respawn
	if ( ent->flags & FL_DROPPED_ITEM ) {
		ent->freeAfterEvent = qtrue;
	}

	// picked up items still stay around, they just don't
	// draw anything.  This allows respawnable items
	// to be placed on movers.
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->r.contents = 0;

	// ZOID
	// A negative respawn times means to never respawn this item (but don't
	// delete it).  This is used by items that are respawned by third party
	// events such as ctf flags
	if ( respawn <= 0 ) {
		// OSP: flags placed in the map return after 5 seconds
		if ( !( ent->flags & FL_DROPPED_ITEM ) && ( !Q_stricmp( ent->classname, "team_CTF_redflag" ) ||
			!Q_stricmp( ent->classname, "team_CTF_blueflag" ) ) ) {
			ent->nextthink = level.time + 5000;
			ent->think = OSP_FlagReturnThink;
			return;
		}
		ent->nextthink = 0;
		ent->think = 0;
	} else {
		ent->nextthink = level.time + respawn * 1000;
		if ( ent->think != OSP_ItemThink ) {
			ent->think = RespawnItem;
		}
	}
}

/*
==================
OSP_DelayedPickupThink

OSP: finishes a pickup delayed by Touch_Item (the rest of Touch_Item)
==================
*/
void OSP_DelayedPickupThink( gentity_t *ent ) {
	int		respawn;

	respawn = ent->pickupRespawn;

	// wait of -1 will not respawn
	if ( ent->wait == -1 ) {
		ent->r.svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->unlinkAfterEvent = qtrue;
		return;
	}

	// non zero wait overrides respawn time
	if ( ent->wait ) {
		respawn = ent->wait;
	}

	// random can be used to vary the respawn time
	if ( ent->random ) {
		respawn += crandom() * ent->random;
		if ( respawn < 1 ) {
			respawn = 1;
		}
	}

	// dropped items will not respawn
	if ( ent->flags & FL_DROPPED_ITEM ) {
		ent->freeAfterEvent = qtrue;
	}

	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->r.contents = 0;

	if ( respawn <= 0 ) {
		if ( !( ent->flags & FL_DROPPED_ITEM ) && ( !Q_stricmp( ent->classname, "team_CTF_redflag" ) ||
			!Q_stricmp( ent->classname, "team_CTF_blueflag" ) ) ) {
			ent->nextthink = level.time + 5000;
			ent->think = OSP_FlagReturnThink;
			return;
		}
		ent->nextthink = 0;
		ent->think = 0;
		return;
	}
	ent->nextthink = level.time + respawn * 1000 - 100;
	ent->think = RespawnItem;
}


//======================================================================

/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity, gentity_t *dropper ) {
	gentity_t	*dropped;

	dropped = G_Spawn();

	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	dropped->s.modelindex2 = 1; // This is non-zero is it's a dropped item

	dropped->classname = item->classname;
	dropped->item = item;
	VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
	VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
	dropped->r.contents = CONTENTS_TRIGGER;

	dropped->touch = Touch_Item;

	// OSP: the player who dropped the item can't pick it up for 2 seconds
	if ( dropper ) {
		dropped->parent = dropper;
		dropped->timestamp = level.time + 2000;
	}

	G_SetOrigin( dropped, origin );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;
	VectorCopy( velocity, dropped->s.pos.trDelta );

	dropped->s.eFlags |= EF_BOUNCE_HALF;
#ifdef MISSIONPACK
	if ((g_gametype.integer == GT_CTF || g_gametype.integer == GT_1FCTF)			&& item->giType == IT_TEAM) { // Special case for CTF flags
#else
	if (g_gametype.integer == GT_CTF && item->giType == IT_TEAM) { // Special case for CTF flags
#endif
		dropped->think = Team_DroppedFlagThink;
		dropped->nextthink = level.time + 30000;
		Team_CheckDroppedItem( dropped );
	} else { // auto-remove after 30 seconds
		dropped->think = G_FreeEntity;
		dropped->nextthink = level.time + 30000;
	}

	dropped->flags = FL_DROPPED_ITEM;

	trap_LinkEntity (dropped);

	return dropped;
}

/*
================
Drop_Item

Spawns an item and tosses it forward
================
*/
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle, gentity_t *dropper ) {
	vec3_t	velocity;
	vec3_t	angles;
	vec3_t	origin;

	// OSP: players drop from their exact (unsnapped) origin with server_ospauth bit 1
	if ( ( server_ospauth.integer & 1 ) && ent->s.eType == ET_PLAYER ) {
		VectorCopy( ent->client->ps.origin, origin );
	} else {
		VectorCopy( ent->s.pos.trBase, origin );
	}

	VectorCopy( ent->s.apos.trBase, angles );
	angles[YAW] += angle;
	angles[PITCH] = 0;	// always forward

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;

	return LaunchItem( item, origin, velocity, dropper );
}


/*
================
Use_Item

Respawn the item
================
*/
void Use_Item( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	if ( OSP_ItemAllowed( ent ) ) {
		RespawnItem( ent );
	}
}

//======================================================================

/*
================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
void FinishSpawningItem( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		dest;

	VectorSet( ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS );
	VectorSet( ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS );

	ent->s.eType = ET_ITEM;
	ent->s.modelindex = ent->item - bg_itemlist;		// store item number in modelindex
	ent->s.modelindex2 = 0; // zero indicates this isn't a dropped item

	ent->r.contents = CONTENTS_TRIGGER;
	ent->touch = Touch_Item;
	// useing an item causes it to respawn
	ent->use = Use_Item;

	if ( ent->spawnflags & 1 ) {
		// suspended
		G_SetOrigin( ent, ent->s.origin );
	} else {
		// drop to floor
		VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
		trap_Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
		if ( tr.startsolid ) {
			G_Printf ("FinishSpawningItem: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEntity( ent );
			return;
		}

		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;

		G_SetOrigin( ent, tr.endpos );
	}

	// team slaves and targeted items aren't present at start
	// (OSP: neither are disabled items, unless they are the master of a team)
	if ( ( ent->flags & FL_TEAMSLAVE ) || ent->targetname ||
		( !OSP_ItemAllowed( ent ) && ( !ent->team || ( ent->flags & FL_TEAMSLAVE ) ) ) ) {
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		return;
	}

	// powerups don't spawn in for a while
	if ( ent->item->giType == IT_POWERUP ) {
		float	respawn;

		// OSP: 15..30 seconds in PROMODE / CQ3, 30..60 in VQ3
		respawn = ( level.physicsMode ? 15 * random() : 15 * crandom() + 30 ) + 15;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
		return;
	}

	// OSP: a disabled team master lets one of its team spawn in 2..8 seconds
	if ( !OSP_ItemAllowed( ent ) && ent->team && !( ent->flags & FL_TEAMSLAVE ) ) {
		float	respawn;

		respawn = 3 * crandom() + 5;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
		return;
	}

	trap_LinkEntity (ent);
}

qboolean	itemRegistered[MAX_ITEMS];

/*
==================
OSP_ItemsHelper

OSP: true if the item is registered for this map (no callers in OSP 1.03a)
==================
*/
qboolean OSP_ItemsHelper( gitem_t *item ) {
	return ( item && itemRegistered[ item - bg_itemlist ] );
}

/*
==================
G_CheckTeamItems
==================
*/
void G_CheckTeamItems( void ) {

	// Set up team stuff
	Team_InitGame();

	if( g_gametype.integer == GT_CTF ) {
		gitem_t	*item;

		// check for the two flags
		item = BG_FindItem( "Red Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_redflag in map" );
		}
		item = BG_FindItem( "Blue Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_blueflag in map" );
		}
	}
#ifdef MISSIONPACK
	if( g_gametype.integer == GT_1FCTF ) {
		gitem_t	*item;

		// check for all three flags
		item = BG_FindItem( "Red Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_redflag in map" );
		}
		item = BG_FindItem( "Blue Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_blueflag in map" );
		}
		item = BG_FindItem( "Neutral Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_neutralflag in map" );
		}
	}

	if( g_gametype.integer == GT_OBELISK ) {
		gentity_t	*ent;

		// check for the two obelisks
		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_redobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_redobelisk in map" );
		}

		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_blueobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_blueobelisk in map" );
		}
	}

	if( g_gametype.integer == GT_HARVESTER ) {
		gentity_t	*ent;

		// check for all three obelisks
		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_redobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_redobelisk in map" );
		}

		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_blueobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_blueobelisk in map" );
		}

		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_neutralobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_neutralobelisk in map" );
		}
	}
#endif
}

/*
==============
ClearRegisteredItems
==============
*/
void ClearRegisteredItems( void ) {
	memset( itemRegistered, 0, sizeof( itemRegistered ) );

	// players always start with the base weapon (OSP: the gauntlet, the others depend
	// on the start weapons settings)
	RegisterItem( BG_FindItemForWeapon( WP_GAUNTLET ) );
	OSP_RegisterStartWeapons();
#ifdef MISSIONPACK
	if( g_gametype.integer == GT_HARVESTER ) {
		RegisterItem( BG_FindItem( "Red Cube" ) );
		RegisterItem( BG_FindItem( "Blue Cube" ) );
	}
#endif
}

/*
===============
RegisterItem

The item will be added to the precache list
===============
*/
void RegisterItem( gitem_t *item ) {
	if ( !item ) {
		G_Error( "RegisterItem: NULL" );
	}
	itemRegistered[ item - bg_itemlist ] = qtrue;
}


/*
===============
SaveRegisteredItems

Write the needed items to a config string
so the client will know which ones to precache
===============
*/
void SaveRegisteredItems( void ) {
	char	string[MAX_ITEMS+1];
	int		i;
	int		count;

	count = 0;
	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( itemRegistered[i] ) {
			count++;
			string[i] = '1';
		} else {
			string[i] = '0';
		}
	}
	string[ bg_numItems ] = 0;

	G_Printf( "%i items registered\n", count );
	trap_SetConfigstring(CS_ITEMS, string);
}

/*
============
G_SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void G_SpawnItem (gentity_t *ent, gitem_t *item) {
	G_SpawnFloat( "random", "0", &ent->random );
	G_SpawnFloat( "wait", "0", &ent->wait );

	RegisterItem( item );

	ent->item = item;
	// some movers spawn on the second frame, so delay item
	// spawns until the third frame so they can ride trains
	ent->nextthink = level.time + FRAMETIME * 2;
	ent->think = FinishSpawningItem;

	ent->physicsBounce = 0.50;		// items are bouncy

	if ( item->giType == IT_POWERUP ) {
		G_SoundIndex( "sound/items/poweruprespawn.wav" );
		G_SpawnFloat( "noglobalsound", "0", &ent->speed);
	}

#ifdef MISSIONPACK
	if ( item->giType == IT_PERSISTANT_POWERUP ) {
		ent->s.generic1 = ent->spawnflags;
	}
#endif
}


/*
================
G_BounceItem

================
*/
void G_BounceItem( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	// cut the velocity to keep from bouncing forever
	VectorScale( ent->s.pos.trDelta, ent->physicsBounce, ent->s.pos.trDelta );

	// check for stop
	if ( trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40 ) {
		trace->endpos[2] += 1.0;	// make sure it is off ground
		SnapVector( trace->endpos );
		G_SetOrigin( ent, trace->endpos );
		ent->s.groundEntityNum = trace->entityNum;
		// OSP: landing sound in freeze tag, at most every 700 msec
		if ( IS_FREEZETAG && ent->pain_debounce_time < level.time - 700 ) {
			ent->pain_debounce_time = level.time;
			G_AddEvent( ent, EV_FALL_SHORT, 0 );
		}
		return;
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_RunItem

================
*/
void G_RunItem( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			contents;
	int			mask;

	// if groundentity has been set to -1, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == -1 ) {
		if ( ent->s.pos.trType != TR_GRAVITY ) {
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY ) {
		// check think function
		G_RunThink( ent );
		return;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;//MASK_SOLID;
	}
	// OSP: frozen bodies don't pass through their own player
	if ( IS_FREEZETAG && is_body_freeze( ent ) )
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin,
			ent->s.number, mask );
	else
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin,
			ent->r.ownerNum, mask );

	VectorCopy( tr.endpos, ent->r.currentOrigin );

	if ( tr.startsolid ) {
		tr.fraction = 0;
	}

	trap_LinkEntity( ent );	// FIXME: avoid this for stationary?

	// check think function
	G_RunThink( ent );

	if ( tr.fraction == 1 ) {
		return;
	}

	// if it is in a nodrop volume, remove it
	contents = trap_PointContents( ent->r.currentOrigin, -1 );
	// (OSP: unless the map flags in level.osp_unk_3834 disable nodrop)
	if ( ( contents & CONTENTS_NODROP ) && !( level.osp_unk_3834 & 2 ) ) {
		if (ent->item && ent->item->giType == IT_TEAM) {
			Team_FreeEntity(ent);
		} else if ( IS_FREEZETAG && is_body( ent ) ) {
			// OSP: a frozen body is removed only after the respawn time
			if ( level.time - ent->timestamp > (int)( 1000 * freezetag_respawn.value ) ) {
				FT_BodyFree( ent );
			}
		} else {
			G_FreeEntity( ent );
		}
		return;
	}

	G_BounceItem( ent, &tr );
}

