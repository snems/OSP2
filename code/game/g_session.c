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

// OSP: defined in g_main.c; declared here and not in g_local.h because an
// extern in the header would change g_main.c's bss order
extern int	osp_unk_1c054;


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char	*s;
	const char	*var;

	// OSP's session string carries its own fields and stats; a few
	// fields are packed into the high bits of others (<< 8)
	if ( g_gametype.integer == GT_TEAM && server_freezetag.integer && client->sess.sessionTeam != TEAM_SPECTATOR ) {
		client->sess.spectatorClient = 0;
		client->sess.spectatorState = SPECTATOR_NOT;
	}
	if ( level.osp_unk_3808 ) {
		OSP_ResetClientStats( client - level.clients );
	}

	if ( g_gametype.integer == GT_FFA || g_gametype.integer == GT_SINGLE_PLAYER ) {
		s = va( "%i %i %i %i %i %i %i %i %i:%s",
			client->sess.sessionTeam,
			client->sess.spectatorTime,
			client->sess.spectatorState,
			client->sess.spectatorClient,
			client->sess.wins,
			client->sess.losses,
			client->sess.ospClient,
			client->sess.welcomeShown,
			client->sess.referee,
			client->sess.ip );
	} else if ( g_gametype.integer != 5 ) {
		int		osp3cc = client->sess.captain;

		s = va( "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i:%s",
			client->sess.sessionTeam + (client->sess.megaHealthPickups << 8),
			client->sess.spectatorTime,
			client->sess.spectatorState + (client->sess.redArmorPickups << 8),
			client->sess.spectatorClient + (client->sess.yellowArmorPickups << 8),
			client->sess.wins,
			client->sess.losses,
			client->sess.ospClient,
			osp3cc,
			client->sess.welcomeShown,
			client->sess.referee,
			client->sess.specInvite,
			client->sess.coachTeam,
			client->sess.damageGiven,
			client->sess.damageReceived,
			client->sess.teamDamage,
			client->sess.kills,
			client->sess.deaths,
			client->sess.suicides,
			client->sess.teamKills,
			( client->sess.sessionTeam != TEAM_SPECTATOR && g_gametype.integer != GT_TOURNAMENT ) ? client->ps.persistant[PERS_SCORE] : 0,
			client->sess.nameTeam + (client->sess.greenArmorPickups << 8),
			client->sess.specOnly,
			client->sess.weaponStats[1].atts,
			client->sess.weaponStats[1].hits,
			client->sess.weaponStats[1].kills,
			client->sess.weaponStats[1].deaths,
			client->sess.weaponStats[2].atts,
			client->sess.weaponStats[2].hits,
			client->sess.weaponStats[2].kills,
			client->sess.weaponStats[2].deaths,
			client->sess.weaponStats[3].atts,
			client->sess.weaponStats[3].hits,
			client->sess.weaponStats[3].kills,
			client->sess.weaponStats[3].deaths,
			client->sess.weaponStats[5].atts,
			client->sess.weaponStats[5].hits,
			client->sess.weaponStats[5].kills,
			client->sess.weaponStats[5].deaths,
			client->sess.weaponStats[4].atts,
			client->sess.weaponStats[4].hits,
			client->sess.weaponStats[4].kills,
			client->sess.weaponStats[4].deaths,
			client->sess.weaponStats[7].atts,
			client->sess.weaponStats[7].hits,
			client->sess.weaponStats[7].kills,
			client->sess.weaponStats[7].deaths,
			client->sess.weaponStats[6].atts,
			client->sess.weaponStats[6].hits,
			client->sess.weaponStats[6].kills,
			client->sess.weaponStats[6].deaths,
			client->sess.weaponStats[8].atts,
			client->sess.weaponStats[8].hits,
			client->sess.weaponStats[8].kills,
			client->sess.weaponStats[8].deaths,
			client->sess.weaponStats[9].atts,
			client->sess.weaponStats[9].hits,
			client->sess.weaponStats[9].kills,
			client->sess.weaponStats[9].deaths,
			client->sess.ip );
	} else {
		s = va( "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i:%s",
			client->sess.sessionTeam + (client->sess.megaHealthPickups << 8),
			client->sess.spectatorTime,
			client->sess.spectatorState + (client->sess.redArmorPickups << 8),
			client->sess.spectatorClient + (client->sess.yellowArmorPickups << 8),
			client->sess.wins,
			client->sess.losses,
			client->sess.ospClient,
			client->sess.captain,
			client->sess.welcomeShown,
			client->sess.referee,
			client->sess.specInvite,
			client->sess.kills,
			client->sess.deaths,
			client->sess.damageGiven,
			client->sess.damageReceived,
			client->sess.nameTeam + (client->sess.greenArmorPickups << 8),
			client->sess.weaponStats[1].atts,
			client->sess.weaponStats[1].hits,
			client->sess.weaponStats[1].kills,
			client->sess.weaponStats[1].deaths,
			client->sess.weaponStats[2].atts,
			client->sess.weaponStats[2].hits,
			client->sess.weaponStats[2].kills,
			client->sess.weaponStats[2].deaths,
			client->sess.weaponStats[3].atts,
			client->sess.weaponStats[3].hits,
			client->sess.weaponStats[3].kills,
			client->sess.weaponStats[3].deaths,
			client->sess.weaponStats[5].atts,
			client->sess.weaponStats[5].hits,
			client->sess.weaponStats[5].kills,
			client->sess.weaponStats[5].deaths,
			client->sess.weaponStats[4].atts,
			client->sess.weaponStats[4].hits,
			client->sess.weaponStats[4].kills,
			client->sess.weaponStats[4].deaths,
			client->sess.weaponStats[7].atts,
			client->sess.weaponStats[7].hits,
			client->sess.weaponStats[7].kills,
			client->sess.weaponStats[7].deaths,
			client->sess.weaponStats[6].atts,
			client->sess.weaponStats[6].hits,
			client->sess.weaponStats[6].kills,
			client->sess.weaponStats[6].deaths,
			client->sess.weaponStats[8].atts,
			client->sess.weaponStats[8].hits,
			client->sess.weaponStats[8].kills,
			client->sess.weaponStats[8].deaths,
			client->sess.weaponStats[9].atts,
			client->sess.weaponStats[9].hits,
			client->sess.weaponStats[9].kills,
			client->sess.weaponStats[9].deaths,
			client->sess.ip );
	}

	var = va( "session%i", client - level.clients );

	trap_Cvar_Set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char	s[MAX_STRING_CHARS];
	char	*p;
	const char	*var;

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	// OSP's session format (see G_WriteClientSessionData)
	if ( g_gametype.integer == GT_FFA || g_gametype.integer == GT_SINGLE_PLAYER ) {
		sscanf( s, "%i %i %i %i %i %i %i %i %i",
			&client->sess.sessionTeam,
			&client->sess.spectatorTime,
			&client->sess.spectatorState,
			&client->sess.spectatorClient,
			&client->sess.wins,
			&client->sess.losses,
			&client->sess.ospClient,
			&client->sess.welcomeShown,
			&client->sess.referee );
	} else if ( g_gametype.integer != 5 ) {
		sscanf( s, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
			&client->sess.sessionTeam,
			&client->sess.spectatorTime,
			&client->sess.spectatorState,
			&client->sess.spectatorClient,
			&client->sess.wins,
			&client->sess.losses,
			&client->sess.ospClient,
			&client->sess.captain,
			&client->sess.welcomeShown,
			&client->sess.referee,
			&client->sess.specInvite,
			&client->sess.coachTeam,
			&client->sess.damageGiven,
			&client->sess.damageReceived,
			&client->sess.teamDamage,
			&client->sess.kills,
			&client->sess.deaths,
			&client->sess.suicides,
			&client->sess.teamKills,
			&client->ps.persistant[PERS_SCORE],
			&client->sess.nameTeam,
			&client->sess.specOnly,
			&client->sess.weaponStats[1].atts,
			&client->sess.weaponStats[1].hits,
			&client->sess.weaponStats[1].kills,
			&client->sess.weaponStats[1].deaths,
			&client->sess.weaponStats[2].atts,
			&client->sess.weaponStats[2].hits,
			&client->sess.weaponStats[2].kills,
			&client->sess.weaponStats[2].deaths,
			&client->sess.weaponStats[3].atts,
			&client->sess.weaponStats[3].hits,
			&client->sess.weaponStats[3].kills,
			&client->sess.weaponStats[3].deaths,
			&client->sess.weaponStats[5].atts,
			&client->sess.weaponStats[5].hits,
			&client->sess.weaponStats[5].kills,
			&client->sess.weaponStats[5].deaths,
			&client->sess.weaponStats[4].atts,
			&client->sess.weaponStats[4].hits,
			&client->sess.weaponStats[4].kills,
			&client->sess.weaponStats[4].deaths,
			&client->sess.weaponStats[7].atts,
			&client->sess.weaponStats[7].hits,
			&client->sess.weaponStats[7].kills,
			&client->sess.weaponStats[7].deaths,
			&client->sess.weaponStats[6].atts,
			&client->sess.weaponStats[6].hits,
			&client->sess.weaponStats[6].kills,
			&client->sess.weaponStats[6].deaths,
			&client->sess.weaponStats[8].atts,
			&client->sess.weaponStats[8].hits,
			&client->sess.weaponStats[8].kills,
			&client->sess.weaponStats[8].deaths,
			&client->sess.weaponStats[9].atts,
			&client->sess.weaponStats[9].hits,
			&client->sess.weaponStats[9].kills,
			&client->sess.weaponStats[9].deaths );
		client->sess.megaHealthPickups = client->sess.sessionTeam >> 8;
		client->sess.sessionTeam &= 0xff;
		client->sess.greenArmorPickups = client->sess.nameTeam >> 8;
		client->sess.nameTeam &= 0xff;
		client->sess.redArmorPickups = client->sess.spectatorState >> 8;
		client->sess.spectatorState &= 0xff;
		client->sess.yellowArmorPickups = client->sess.spectatorClient >> 8;
		client->sess.spectatorClient &= 0xff;
	} else {
		sscanf( s, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
			&client->sess.sessionTeam,
			&client->sess.spectatorTime,
			&client->sess.spectatorState,
			&client->sess.spectatorClient,
			&client->sess.wins,
			&client->sess.losses,
			&client->sess.ospClient,
			&client->sess.captain,
			&client->sess.welcomeShown,
			&client->sess.referee,
			&client->sess.specInvite,
			&client->sess.kills,
			&client->sess.deaths,
			&client->sess.damageGiven,
			&client->sess.damageReceived,
			&client->sess.nameTeam,
			&client->sess.weaponStats[1].atts,
			&client->sess.weaponStats[1].hits,
			&client->sess.weaponStats[1].kills,
			&client->sess.weaponStats[1].deaths,
			&client->sess.weaponStats[2].atts,
			&client->sess.weaponStats[2].hits,
			&client->sess.weaponStats[2].kills,
			&client->sess.weaponStats[2].deaths,
			&client->sess.weaponStats[3].atts,
			&client->sess.weaponStats[3].hits,
			&client->sess.weaponStats[3].kills,
			&client->sess.weaponStats[3].deaths,
			&client->sess.weaponStats[5].atts,
			&client->sess.weaponStats[5].hits,
			&client->sess.weaponStats[5].kills,
			&client->sess.weaponStats[5].deaths,
			&client->sess.weaponStats[4].atts,
			&client->sess.weaponStats[4].hits,
			&client->sess.weaponStats[4].kills,
			&client->sess.weaponStats[4].deaths,
			&client->sess.weaponStats[7].atts,
			&client->sess.weaponStats[7].hits,
			&client->sess.weaponStats[7].kills,
			&client->sess.weaponStats[7].deaths,
			&client->sess.weaponStats[6].atts,
			&client->sess.weaponStats[6].hits,
			&client->sess.weaponStats[6].kills,
			&client->sess.weaponStats[6].deaths,
			&client->sess.weaponStats[8].atts,
			&client->sess.weaponStats[8].hits,
			&client->sess.weaponStats[8].kills,
			&client->sess.weaponStats[8].deaths,
			&client->sess.weaponStats[9].atts,
			&client->sess.weaponStats[9].hits,
			&client->sess.weaponStats[9].kills,
			&client->sess.weaponStats[9].deaths );
		client->sess.megaHealthPickups = client->sess.sessionTeam >> 8;
		client->sess.sessionTeam &= 0xff;
		client->sess.greenArmorPickups = client->sess.nameTeam >> 8;
		client->sess.nameTeam &= 0xff;
		client->sess.redArmorPickups = client->sess.spectatorState >> 8;
		client->sess.spectatorState &= 0xff;
		client->sess.yellowArmorPickups = client->sess.spectatorClient >> 8;
		client->sess.spectatorClient &= 0xff;
	}

	p = strchr( s, ':' );
	if ( p ) {
		*p = 0;
		p++;
		strcpy( client->sess.ip, p );
	} else {
		strcpy( client->sess.ip, "UNKNOWN" );
	}

	if ( client->sess.coachTeam ) {
		client->sess.specInvite = client->sess.coachTeam;
		client->pers.followTeam = client->sess.coachTeam;
	}

	if ( !osp_numViewcams && client->sess.spectatorState >= 4 ) {
		client->sess.spectatorState = SPECTATOR_FREE;
	}
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo ) {
	clientSession_t	*sess;
	const char		*value;

	sess = &client->sess;

	// initial team determination
	// OSP resets its session fields in team games, can force a team
	// from the player name list and keeps filtered players / late joiners spectating
	if ( g_gametype.integer >= GT_TEAM ) {
		int		team;

		sess->captain = 0;
		sess->welcomeShown = 0;
		sess->referee = 0;
		sess->coachTeam = 0;
		team = OSP_PlayerNameTeam( client, userinfo );
		if ( team ) {
			sess->sessionTeam = team;
			BroadcastTeamChange( client, -1 );
		} else if ( g_teamAutoJoin.integer ) {
			sess->sessionTeam = PickTeam( -1 );
			BroadcastTeamChange( client, -1 );
		} else {
			// always spawn as spectator in team games
			sess->sessionTeam = TEAM_SPECTATOR;
			sess->roundTeam = 3;
		}
	} else {
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
			sess->roundTeam = 3;
		} else {
			int		clientNum;

			clientNum = client - level.clients;
			switch ( g_gametype.integer ) {
			default:
			case GT_FFA:
				if ( !( g_entities[clientNum].r.svFlags & SVF_BOT ) &&
					( OSP_CheckClientFilters( Info_ValueForKey( userinfo, "name" ), clientNum, 1 ) == 2 ||
					( level.warmupTime == 0 && g_doWarmup.integer != 0 && match_latejoin.integer == 0 &&
					( g_gametype.integer == GT_FFA || team_nocontrols.integer == 0 ) ) ) ) {
					sess->sessionTeam = TEAM_SPECTATOR;
					break;
				}
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 && 
					level.numNonSpectatorClients >= g_maxGameClients.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_TOURNAMENT:
				// if the game is full, go into a waiting mode
				if ( level.numNonSpectatorClients >= 2 ||
					OSP_CheckClientFilters( Info_ValueForKey( userinfo, "name" ), clientNum, 1 ) == 2 ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	G_WriteClientSessionData( client );
}



/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int			gt;

	// OSP keeps two team flags in "session" and, in round-based modes,
	// the scores in "ca_session"; a finished match is announced on the next map
	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	sscanf( s, "%i %i %i", &gt, &osp_teamInfo[TEAM_RED].specLocked, &osp_teamInfo[TEAM_BLUE].specLocked );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt ) {
		level.osp_unk_3808 = qtrue;
		level.newSession = qtrue;
		G_Printf( "\n***\n*** Gametype changed, clearing session data.\n***\n" );
	}

	if ( OSP_IsRoundBased( g_gametype.integer ) ) {
		int		winner;
		char	*teamNames[] = { "none", "Red", "Blue" };

		trap_Cvar_VariableStringBuffer( "ca_session", s, sizeof(s) );
		sscanf( s, "%i %i %i %i", &osp_roundsPlayed, &level.teamScores[TEAM_RED], &level.teamScores[TEAM_BLUE], &osp_unk_1c054 );
		winner = OSP_WinningTeam();
		if ( winner ) {
			char	serverinfo[MAX_INFO_STRING];

			Com_sprintf( osp_matchResultPrint, sizeof( osp_matchResultPrint ), "print \"%s wins the match: ^3%d - %d\n\"",
				osp_teamNames[winner], level.teamScores[winner], level.teamScores[TEAM_BLUE - winner + TEAM_RED] );
			Com_sprintf( osp_matchResultCP, sizeof( osp_matchResultCP ), "cp \"%s wins the match: ^3%d - %d\n\"",
				osp_teamNames[winner], level.teamScores[winner], level.teamScores[TEAM_BLUE - winner + TEAM_RED] );
			osp_roundsPlayed = 0;
			osp_unk_1c054 = 0;
			level.teamScores[TEAM_RED] = 0;
			level.teamScores[TEAM_BLUE] = 0;
			G_LogPrintf( va( "Game_End: Roundlimit - %s wins match\n", teamNames[winner] ) );
			trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );
			OSP_AdminLogPrintf( va( "Game_End\t%s\t%s", osp_modes[g_gametype.integer].name, Info_ValueForKey( serverinfo, "mapname" ) ) );
		} else {
			osp_matchResultPrint[0] = 0;
			osp_matchResultCP[0] = 0;
		}
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;
	gclient_t	*cl;

	// OSP session state (see G_InitWorldSession) and the multiview
	// session; players that left a team for spectating are saved with their old team
	if ( level.nextMode ) {
		trap_Cvar_Set( "session", "-1 0 0" );
		return;
	}

	if ( g_gametype.integer < GT_TEAM ) {
		trap_Cvar_Set( "session", va("%i", g_gametype.integer) );
	} else {
		if ( !level.intermissiontime ) {
			trap_Cvar_Set( "session", va("%i %i %i", g_gametype.integer, osp_teamInfo[TEAM_RED].specLocked, osp_teamInfo[TEAM_BLUE].specLocked) );
		} else {
			trap_Cvar_Set( "session", va("%i 0 0", g_gametype.integer) );
		}
		if ( OSP_IsRoundBased( g_gametype.integer ) ) {
			trap_Cvar_Set( "ca_session", va("%i %i %i %i", osp_roundsPlayed, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE], osp_unk_1c054) );
		}
	}

	OSP_MV_WriteSession();

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( level.clients[i].pers.connected != CON_DISCONNECTED ) {
			if ( cl->sess.sessionTeam != TEAM_RED && cl->sess.sessionTeam != TEAM_BLUE &&
				( cl->sess.roundTeam == TEAM_RED || cl->sess.roundTeam == TEAM_BLUE ) ) {
				cl->sess.sessionTeam = cl->sess.roundTeam;
			}
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
}
