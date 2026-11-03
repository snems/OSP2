/*
 * g_osp_cmds.c: OSP's client commands: OSP_ClientCommand (called last by
 * ClientCommand) dispatches lock / speclock / ready / invite / coach / timeout / drop / multiview /
 * viewcam ...; "<command> ?" prints the usage (OSP_CmdHelpCheck, osp_cmdHelpTexts); players,
 * stats, topshots, getstatsinfo; the #-tokens of chat messages (OSP_ChatTokens, G_Say). The code
 * is close to Wolfenstein: ET's g_cmds_ext.c, which comes from OSP.
 */
#include "../qcommon/q_shared.h"
#include "g_local.h"

// OSP: functions of other files used here
extern	void	OSP_Cmd_Settings_f( gentity_t *ent );	// g_osp_config.c

// OSP: an unused global or static of this file (OSP_ChatTokens' static buffer comes
// before it in the bss).
int		osp_unk_1773e4;

// weapon names of the stats lines, indexed by weapon (OSP_Cmd_TopShots_f, OSP_ChatTokens)
char	*osp_weaponShortNames[WP_BFG + 1] = {
	"",
	"GAUNTLET",
	"MG",
	"SG",
	"GL",
	"RL",
	"LG",
	"RG",
	"PG",
	"BFG"
};

// usage texts printed by "<command> ?" (OSP_CmdHelpCheck's cmdIndex)
char	*osp_cmdHelpTexts[] = {
	":^7 Locks/unlocks a player's team",
	":^7 Locks/unlocks a player's team from spectators",
	":^7 Toggles option to stay as a spectator in 1v1",
	":^7 Sets your status to ^5ready^7 to start a match",
	":^7 Lists all active players and their IDs/information",
	" <player_ID>:^7 Invites a player to join a team",
	":^7 Invites a player to spectate a spec-locked team",
	" <player_ID>:^7 Invites a player to coach team",
	":^7 Declines coach invitation or resigns coach status",
	" <player_ID>:^7 Kicks active coach from team",
	":^7 Accepts coach invitation/restarts coach view",
	" <player_ID>:^7 Removes a player from the team",
	" [player_ID]:^7 Resigns captainship.  Can optionally be given to another teammate",
	":^7 Sets an entire team's status to ^5ready^7 to start a match",
	":^7 Shows team captains",
	":^7 Pause the match or issue a team timeout/timein",
	" [player_ID]:^7 Shows weapon accuracy stats for a player",
	" [player_ID]:^7 Shows player stats + match info saved locally to a file",
	" [password]:^7 Shows/sets team joincode or used to join a locked team",
	" <item>:^7 Drops available weapon/ammo/flag",
	":^7 Shows BEST player for each weapon. Add ^3<weapon_ID>^7 to show all stats for a weapon",
	":^7 Creates a demo with a consistant naming scheme (OSP clients only)",
	":^7 Creates a screenshot with a consistant naming scheme (OSP clients only)",
	":^7 Shows WORST player for each weapon. Add ^3<weapon_ID>^7 to show all stats for a weapon",
	" <password>:^7 Become a referee (admin access)",
	":^7 Switches to cinematic camera mode",
	" [player_ID]:^7 Puts viewcam in follow mode.  Can optionally to follow a specific player",
	":^7 Toggle viewcam between manual/automatic change",
	":^7 Toggle ViewCam between static/dynamic views",
	" <player_ID>:^7 Adds a player to multi-screen view",
	":^7 Adds all active players to a multi-screen view",
	" [player_ID]:^7 Removes current selected or specific player from multi-screen view",
	":^7 Disables multiview mode and goes back to spectator mode",
	":^7 Moves through active screen in a multi-screen display",
	":^7 Cycles through players in current view",
	":^7 Follows current highlighted view",
	": ^7 Views entire red/blue team",
	" <r|b|s|none>:^7 Chooses a team or shows current team (s = spectator mode)",
	" <player_ID|red|blue>:^7 Spectates a particular player or team",
	":^7 Sets player's status to ^5not ready^7 to start a match",
	" <params>:^7 Calls a vote",
	":^7 Gives a list of most OSP commands"
};

/*
==================
OSP_ClientCommand

OSP's client commands, tried last by ClientCommand (and by OSP_Cmd_Ref_f). Returns nonzero when
cmd was an OSP command. fRefCommand is not used.
==================
*/
int OSP_ClientCommand( gentity_t *ent, char *cmd, qboolean fRefCommand ) {
	if ( g_gametype.integer >= GT_TEAM && ( !Q_stricmp( cmd, "lock" ) || !Q_stricmp( cmd, "unlock" ) ) ) {
		OSP_Cmd_Lock_f( ent, cmd );
		return 1;
	}
	if ( g_gametype.integer >= GT_TEAM && ( !Q_stricmp( cmd, "speclock" ) || !Q_stricmp( cmd, "specunlock" ) ) ) {
		OSP_Cmd_SpecLock_f( ent, cmd );
		return 1;
	}
	if ( g_gametype.integer != GT_SINGLE_PLAYER && !Q_stricmp( cmd, "ready" ) ) {
		OSP_Cmd_Ready_f( ent, qtrue );
		return 1;
	}
	if ( g_gametype.integer != GT_SINGLE_PLAYER && ( !Q_stricmp( cmd, "notready" ) || !Q_stricmp( cmd, "unready" ) ) ) {
		OSP_Cmd_Ready_f( ent, qfalse );
		return 1;
	}
	if ( g_gametype.integer >= GT_TEAM && ( !Q_stricmp( cmd, "teamready" ) || !Q_stricmp( cmd, "readyteam" ) ) ) {
		OSP_Cmd_TeamReady_f( ent );
		return 1;
	}
	if ( g_gametype.integer >= GT_TEAM && g_gametype.integer != 5 && ( !Q_stricmp( cmd, "invite" ) || !Q_stricmp( cmd, "pickplayer" ) ) ) {
		OSP_Cmd_Invite_f( ent );
		return 1;
	}
	if ( g_gametype.integer >= GT_TEAM && ( !Q_stricmp( cmd, "specinvite" ) || !Q_stricmp( cmd, "invitespec" ) ) ) {
		OSP_Cmd_SpecInvite_f( ent );
		return 1;
	}
	if ( g_gametype.integer >= GT_TEAM && ( !Q_stricmp( cmd, "remove" ) || !Q_stricmp( cmd, "kickplayer" ) ) ) {
		OSP_Cmd_Remove_f( ent );
		return 1;
	}
	if ( g_gametype.integer >= GT_TEAM && !Q_stricmp( cmd, "captains" ) ) {
		OSP_Cmd_Captains_f( ent );
		return 1;
	}
	if ( g_gametype.integer >= GT_TEAM && !Q_stricmp( cmd, "resign" ) ) {
		OSP_Cmd_Resign_f( ent );
		return 1;
	}
	if ( g_gametype.integer >= GT_TEAM && !Q_stricmp( cmd, "joincode" ) ) {
		OSP_Cmd_Joincode_f( ent );
		return 1;
	}
	if ( ( g_gametype.integer >= GT_TEAM || g_gametype.integer == GT_TOURNAMENT ) &&
		( !Q_stricmp( cmd, "time" ) || !Q_stricmp( cmd, "pause" ) || !Q_stricmp( cmd, "timeout" ) ) ) {
		OSP_Cmd_Timeout_f( ent, qtrue );
		return 1;
	}
	if ( ( g_gametype.integer >= GT_TEAM || g_gametype.integer == GT_TOURNAMENT ) &&
		( !Q_stricmp( cmd, "timein" ) || !Q_stricmp( cmd, "unpause" ) ) ) {
		OSP_Cmd_Timeout_f( ent, qfalse );
		return 1;
	}
	if ( !Q_stricmp( cmd, "help" ) || !Q_stricmp( cmd, "commands" ) || !Q_stricmp( cmd, "?" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 41 ) ) {
			OSP_Cmd_Help_f( ent );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "followpowerup" ) ) {
		ent->client->pers.followPowerup = 1 - ent->client->pers.followPowerup;
		trap_SendServerCommand( ent - g_entities, va( "print \"\nAuto-follow of powerup pickups is: ^3%s\n\n", ( ent->client->pers.followPowerup ) ? "ENABLED" : "DISABLED" ) );
		return 1;
	}
	if ( !Q_stricmp( cmd, "speconly" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 2 ) ) {
			OSP_Cmd_SpecOnly_f( ent );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "autorecord" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 21 ) ) {
			OSP_Cmd_AutoRecord( ent - g_entities, 1, 1 );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "maplist" ) ) {
		OSP_Cmd_MapList_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "mapload" ) ) {
		OSP_LoadMapSession( qtrue );
		return 1;
	}
	if ( !Q_stricmp( cmd, "range" ) ) {
		OSP_Cmd_Range_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "snapshot" ) ) {
		OSP_Cmd_Snapshot_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "snapshotdump" ) ) {
		OSP_Cmd_SnapshotDump_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewadd" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 29 ) ) {
			OSP_Cmd_ViewAdd_f( ent );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewall" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 30 ) && !ent->client->sess.coachTeam ) {
			OSP_Cmd_ViewAll_f( ent );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewremove" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 31 ) && !ent->client->sess.coachTeam ) {
			OSP_Cmd_ViewRemove_f( ent );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewfollow" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 35 ) ) {
			OSP_Cmd_ViewFollow_f( ent );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewnone" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 32 ) && !ent->client->sess.coachTeam ) {
			OSP_MV_RemoveAll( ent );
			ent->client->pers.followTeam = 0;
			ClientUserinfoChanged( ent - g_entities );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewframe" ) ) {
		OSP_Cmd_ViewFrame_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewnext" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 33 ) ) {
			OSP_Cmd_ViewNext_f( ent, 1 );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewprev" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 33 ) ) {
			OSP_Cmd_ViewNext_f( ent, -1 );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewcyclenext" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 34 ) ) {
			OSP_Cmd_ViewCycle_f( ent, 1 );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewcycleprev" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 34 ) ) {
			OSP_Cmd_ViewCycle_f( ent, -1 );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewred" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 36 ) ) {
			OSP_Cmd_ViewTeam_f( ent, TEAM_RED, qtrue );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewblue" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 36 ) ) {
			OSP_Cmd_ViewTeam_f( ent, TEAM_BLUE, qtrue );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "coach" ) ) {
		OSP_Cmd_Coach_f( ent, 1 );
		return 1;
	}
	if ( !Q_stricmp( cmd, "coachinvite" ) ) {
		OSP_Cmd_CoachInvite_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "coachdecline" ) ) {
		OSP_Cmd_CoachDecline_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "coachkick" ) ) {
		OSP_Cmd_CoachKick_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "vc_proximity" ) ) {
		OSP_Cmd_VCProximity_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "vc_info" ) ) {
		ent->client->pers.viewcamInfo = 1 - ent->client->pers.viewcamInfo;
		trap_SendServerCommand( ent - g_entities, va( "print \"ViewCam info is ^5%s\n", ( !ent->client->pers.viewcamInfo ) ? "DISABLED" : "ENABLED" ) );
		return 1;
	}
	if ( !Q_stricmp( cmd, "vc_follow" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 26 ) ) {
			OSP_Cmd_VCFollow_f( ent );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "vc_free" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 27 ) ) {
			ent->client->pers.viewcamFree = 1 - ent->client->pers.viewcamFree;
			trap_SendServerCommand( ent - g_entities, va( "print \"ViewCam is now ^5%s\n", ( ent->client->pers.viewcamFree ) ? "MANUAL" : "AUTOMATIC" ) );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "vc_view" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 28 ) ) {
			ent->client->pers.viewcamStatic = 1 - ent->client->pers.viewcamStatic;
			trap_SendServerCommand( ent - g_entities, va( "print \"ViewCam view is now ^5%s\n", ( ent->client->pers.viewcamStatic ) ? "STATIC" : "DYNAMIC" ) );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "drop" ) ) {
		OSP_Cmd_Drop_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "viewcam" ) ) {
		if ( !OSP_CmdHelpCheck( ent, 25 ) ) {
			OSP_Cmd_Viewcam_f( ent );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "settings" ) ) {
		OSP_Cmd_Settings_f( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "rhea" ) || !Q_stricmp( cmd, "osp" ) || !Q_stricmp( cmd, "dess" ) || !Q_stricmp( cmd, "shod" ) ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"OSP Tourney DM %s for Quake3!\n\"", "v(1.03a)" ) );
		trap_SendServerCommand( ent - g_entities, "print \"^5http://www.OrangeSmoothie.org/^7\n\n\"" );
		return 1;
	}
	if ( !Q_stricmp( cmd, "arqon" ) ) {
		trap_SendServerCommand( ent - g_entities, "print \"\n^3OSP > CPMA !!!\n\n\"" );
		return 1;
	}

	return 0;
}

/*
==================
OSP_Cmd_Help_f

"help" / "commands" / "?": the OSP commands in four columns, then the version and the
physics credits
==================
*/
void OSP_Cmd_Help_f( gentity_t *ent ) {
	int		i;
	int		rows;
	char	*aCommands[] = {
		"autorecord", "autoscreenshot", "bottomshots", "callvote", "captains", "coach",
		"coachdecline", "coachinvite", "coachkick", "credits", "currenttime", "drop",
		"help/commands", "invite", "joincode", "lock/unlock", "menu", "ready", "notready",
		"players", "readyteam", "ref/admin", "remove", "resign", "scores", "settings",
		"specinvite", "speclock", "speconly", "specunlock", "stats", "statsall", "statsblue",
		"statsdump", "statsred", "time/pause", "timein/unpause", "topshots", "vc_follow",
		"vc_free", "vc_view", "viewcam", "viewadd", "viewblue", "viewred", "viewfollow",
		"viewnext", "viewnone", "viewprev", "viewremove", "viewcyclenext", "viewcycleprev",
		NULL
	};
	int		num;
	char	**pCmd;

	i = 0;
	pCmd = aCommands;
	num = sizeof( aCommands ) / sizeof( aCommands[0] ) - 1;

	rows = num / 4;
	if ( num % 4 ) {
		rows++;
	}
	if ( rows < 0 ) {
		return;
	}

	trap_SendServerCommand( ent - g_entities, "print \"^5\nAvailable OSP Game-Commands:\n----------------------------\n\"" );
	for ( i = 0; i < rows && *pCmd; i++ ) {
		if ( i + rows * 3 + 1 <= num ) {
			trap_SendServerCommand( ent - g_entities, va( "print \"^3%-17s%-17s%-17s%-17s\n\"", aCommands[i], aCommands[i + rows],
				aCommands[i + rows * 2], aCommands[i + rows * 3] ) );
		} else if ( i + rows * 2 + 1 <= num ) {
			trap_SendServerCommand( ent - g_entities, va( "print \"^3%-17s%-17s%-17s\n\"", aCommands[i], aCommands[i + rows],
				aCommands[i + rows * 2] ) );
		} else if ( i + rows + 1 <= num ) {
			trap_SendServerCommand( ent - g_entities, va( "print \"^3%-17s%-17s\n\"", aCommands[i], aCommands[i + rows] ) );
		} else {
			trap_SendServerCommand( ent - g_entities, va( "print \"^3%-17s\n\"", aCommands[i] ) );
		}
	}

	trap_SendServerCommand( ent - g_entities, "print \"\nType: ^3\\command_name ?^7 for more information\n\"" );
	trap_SendServerCommand( ent - g_entities, va( "print \"\n^6OSP2 Tourney DM for Quake3 %s\n\"", "v(%s)" , XSTRINGIFY(OSP_VERSION)));
	trap_SendServerCommand( ent - g_entities, "print \"^5https://github.com/snems/OSP2^7\n\n\"" );

	if ( level.physicsMode & 1 ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3ProMode^7 settings provided by:\n\"" );
		trap_SendServerCommand( ent - g_entities, "print \"Challenge ProMode - http://www.promode.org\n\"" );
	} else if ( level.physicsMode & 2 ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3CQ3^7 settings provided by:\n\"" );
		trap_SendServerCommand( ent - g_entities, "print \"Challenge Q3 - http://www.promode.org\n\"" );
	}
}
/*
==================
OSP_CmdHelpCheck

"<command> ?": prints the usage text osp_cmdHelpTexts[cmdIndex] and returns 1
==================
*/
int OSP_CmdHelpCheck( gentity_t *ent, int cmdIndex ) {
	char	arg[MAX_TOKEN_CHARS];

	if ( !ent ) {
		return 0;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if ( !Q_stricmp( arg, "?" ) ) {
		trap_Argv( 0, arg, sizeof( arg ) );
		trap_SendServerCommand( ent - g_entities, va( "print \"\n^3%s%s\n\n\"", arg, osp_cmdHelpTexts[cmdIndex] ) );
		return 1;
	}

	return 0;
}

/*
==================
OSP_Cmd_UInfo_f

"uinfo <maxpackets> <snaps> <timenudge> <arg4> <follow flags> <autoaction> <jpeg>": client
settings sent by the OSP cgame (follow flags: 1 followpowerup, 2 followkiller, 4 followviewcam)
==================
*/
void OSP_Cmd_UInfo_f( gentity_t *ent ) {
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 8 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	ent->client->pers.maxPackets = atoi( arg );
	trap_Argv( 2, arg, sizeof( arg ) );
	ent->client->pers.snaps = atoi( arg );
	trap_Argv( 3, arg, sizeof( arg ) );
	ent->client->pers.timeNudge = atoi( arg );
	trap_Argv( 4, arg, sizeof( arg ) );
	ent->client->pers.osp_unk_3ac = atoi( arg );
	trap_Argv( 5, arg, sizeof( arg ) );
	ent->client->pers.followPowerup = ( atoi( arg ) & 1 ) ? 1 : 0;
	ent->client->pers.followKiller = ( atoi( arg ) & 2 ) ? 1 : 0;
	ent->client->pers.followViewcam = ( atoi( arg ) & 4 ) ? 1 : 0;
	trap_Argv( 6, arg, sizeof( arg ) );
	ent->client->pers.autoAction = atoi( arg );
	trap_Argv( 7, arg, sizeof( arg ) );
	ent->client->pers.screenshotJPEG = atoi( arg );
}

/*
==================
OSP_CmdFloodCheck

Team commands may be issued once every 5 seconds (shares pers.cmdDelayTime with
ClientUserinfoChanged). Returns 0 while the delay runs.
==================
*/
int OSP_CmdFloodCheck( gentity_t *ent ) {
	char	cmd[MAX_TOKEN_CHARS];

	if ( ent->client->pers.cmdDelayTime - level.time > 0 ) {
		trap_Argv( 0, cmd, sizeof( cmd ) );
		trap_SendServerCommand( ent - g_entities, va( "print \"Wait another %.1fs to issue ^3%s\n\"",
			1.0f * ( ent->client->pers.cmdDelayTime - level.time ) / 1000.0f, cmd ) );
		return 0;
	}

	ent->client->pers.cmdDelayTime = level.time + 5000;
	return 1;
}

/*
==================
OSP_Cmd_SpecOnly_f

"speconly" (1v1 only): toggles staying a spectator (sess.specOnly); turning it off restarts the
spectator queue time
==================
*/
void OSP_Cmd_SpecOnly_f( gentity_t *ent ) {
	if ( g_gametype.integer != GT_TOURNAMENT ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"\n^3speconly^7 for 1v1 mode only!\n\n" ) );
		return;
	}

	ent->client->sess.specOnly = 1 - ent->client->sess.specOnly;
	trap_SendServerCommand( ent - g_entities, va( "print \"\nStay as spectator is: ^3%s\n\n", ( ent->client->sess.specOnly ) ? "ENABLED" : "DISABLED" ) );
	if ( !ent->client->sess.specOnly ) {
		ent->client->sess.spectatorTime = level.time;
	}
}

/*
==================
OSP_Cmd_Lock_f

"lock" / "unlock": a player locks his own team; a referee (un)locks both teams
==================
*/
void OSP_Cmd_Lock_f( gentity_t *ent, char *cmd ) {
	int		team;

	if ( OSP_CmdHelpCheck( ent, 0 ) ) {
		return;
	}
	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3lock/unlock^7 is for team-games only!\n\"" );
		return;
	}
	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Team commands not enabled on this server.\n\"" );
		return;
	}

	team = OSP_ClientTeam( ent );
	if ( ent->client->sess.referee ) {
		if ( !Q_stricmp( cmd, "lock" ) ) {
			if ( !osp_teamInfo[TEAM_RED].locked ) {
				OSP_Ref_Lock( ent, TEAM_RED );
			}
			if ( !osp_teamInfo[TEAM_BLUE].locked ) {
				OSP_Ref_Lock( ent, TEAM_BLUE );
			}
		} else {
			if ( osp_teamInfo[TEAM_RED].locked ) {
				OSP_Ref_Lock( ent, TEAM_RED );
			}
			if ( osp_teamInfo[TEAM_BLUE].locked ) {
				OSP_Ref_Lock( ent, TEAM_BLUE );
			}
		}
	} else if ( team == TEAM_RED || team == TEAM_BLUE ) {
		if ( !Q_stricmp( cmd, "lock" ) ) {
			if ( osp_teamInfo[team].locked ) {
				trap_SendServerCommand( ent - g_entities, "print \"\n^3Your team is already locked!\n\n\"" );
				return;
			}
			osp_teamInfo[team].locked = 1;
			trap_SendServerCommand( ent - g_entities, va( "print \"The %s team is now locked!\n\"", osp_teamNames[team] ) );
		} else if ( !Q_stricmp( cmd, "unlock" ) ) {
			if ( !osp_teamInfo[team].locked ) {
				trap_SendServerCommand( ent - g_entities, "print \"\n^3Your team is already unlocked!\n\n\"" );
				return;
			}
			osp_teamInfo[team].locked = 0;
			trap_SendServerCommand( ent - g_entities, va( "print \"The %s team is now unlocked!\n\"", osp_teamNames[team] ) );
		}
	} else {
		trap_SendServerCommand( ent - g_entities, "print \"Spectators can't lock a team!\n\"" );
	}
}

/*
==================
OSP_SpecLockRemoveInvites

A team became spectator-locked: withdraws its spectator invitation (sess.specInvite bit team)
from every client that is neither a referee nor the team's coach
==================
*/
void OSP_SpecLockRemoveInvites( int team ) {
	int			i;
	gentity_t	*ent;

	for ( i = 0; i < level.maxclients; i++ ) {
		ent = g_entities + i;
		if ( !ent->inuse || ent->client->sess.referee || ent->client->sess.coachTeam == team ) {
			continue;
		}

		ent->client->sess.specInvite -= team;
		if ( ent->client->sess.specInvite < 0 ) {
			ent->client->sess.specInvite += team;
		}
	}
}

/*
==================
OSP_Cmd_SpecLock_f

"speclock" / "specunlock": a player toggles his team's spectator lock; a referee locks /
unlocks both teams
==================
*/
void OSP_Cmd_SpecLock_f( gentity_t *ent, char *cmd ) {
	int		team;

	if ( OSP_CmdHelpCheck( ent, 1 ) ) {
		return;
	}
	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3spectator lock/unlock^7 is for team-games only!\n\"" );
		return;
	}
	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Team commands not enabled on this server.\n\"" );
		return;
	}
	if ( !OSP_CmdFloodCheck( ent ) ) {
		return;
	}

	team = OSP_ClientTeam( ent );
	if ( ent->client->sess.referee ) {
		char	*msg = "print \"The %s team is already %slocked from spectators!\n\"";

		if ( !Q_stricmp( cmd, "speclock" ) ) {
			if ( !osp_teamInfo[TEAM_RED].specLocked ) {
				OSP_Ref_SpecLock( ent, TEAM_RED );
				OSP_SpecLockRemoveInvites( TEAM_RED );
			} else {
				trap_SendServerCommand( ent - g_entities, va( msg, osp_teamNames[TEAM_RED], "" ) );
			}
			if ( !osp_teamInfo[TEAM_BLUE].specLocked ) {
				OSP_Ref_SpecLock( ent, TEAM_BLUE );
				OSP_SpecLockRemoveInvites( TEAM_BLUE );
			} else {
				trap_SendServerCommand( ent - g_entities, va( msg, osp_teamNames[TEAM_BLUE], "" ) );
			}
		} else {
			if ( osp_teamInfo[TEAM_RED].specLocked ) {
				OSP_Ref_SpecLock( ent, TEAM_RED );
			} else {
				trap_SendServerCommand( ent - g_entities, va( msg, osp_teamNames[TEAM_RED], "un" ) );
			}
			if ( osp_teamInfo[TEAM_BLUE].specLocked ) {
				OSP_Ref_SpecLock( ent, TEAM_BLUE );
			} else {
				trap_SendServerCommand( ent - g_entities, va( msg, osp_teamNames[TEAM_BLUE], "un" ) );
			}
		}
	} else if ( team == TEAM_RED || team == TEAM_BLUE ) {
		osp_teamInfo[team].specLocked = 1 - osp_teamInfo[team].specLocked;
		if ( osp_teamInfo[team].specLocked ) {
			OSP_SpecLockRemoveInvites( team );
		}
		trap_SendServerCommand( ent - g_entities, va( "print \"Your team is now %s from spectators.\nUse ^3specinvite^7 to invite people to spectate.\n\"",
			( osp_teamInfo[team].specLocked ) ? "LOCKED" : "UNLOCKED" ) );
		trap_SendServerCommand( -1, va( "cp \"The %s team is now\n%s from Spectators!\n\"", osp_teamNames[team],
			( osp_teamInfo[team].specLocked ) ? "^1locked" : "^3unlocked" ) );
	} else {
		trap_SendServerCommand( ent - g_entities, "print \"Spectators can't speclock a team!\n\"" );
	}

	OSP_MV_Validate();
}

/*
==================
OSP_Cmd_Ready_f

"ready" (ready 1) / "notready", "unready" (ready 0) during warmup
==================
*/
void OSP_Cmd_Ready_f( gentity_t *ent, qboolean ready ) {
	if ( ( ready && OSP_CmdHelpCheck( ent, 3 ) ) || ( !ready && OSP_CmdHelpCheck( ent, 39 ) ) ) {
		return;
	}
	if ( OSP_IsRoundBased( g_gametype.integer ) && osp_roundsPlayed ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3ready/notready^7 not needed for this round.\n\"" );
		return;
	}

	OSP_NumReady();
	if ( !level.warmupTime || osp_matchPaused ) {
		trap_SendServerCommand( ent - g_entities, "print \"Match is already in progress!\n\"" );
		return;
	}
	if ( OSP_ClientTeam( ent ) == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"You must be in the game to be ^3ready^7!\n\"" );
		return;
	}
	if ( level.numPlayingClients < match_minplayers.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Not enough players to start match!\n\"" );
		return;
	}
	if ( !OSP_CmdFloodCheck( ent ) ) {
		return;
	}

	if ( !ent->client->pers.ready ) {
		if ( ready ) {
			ent->client->pers.ready = 1;
			ent->client->readyToExit = 1;
			OSP_ResetPlayerReady();
			trap_SendServerCommand( -1, va( "print \"%s^7 is ready!\n\"", ent->client->pers.netname ) );
			trap_SendServerCommand( -1, va( "cp \"\n%s\n^3is ready!\n\"", ent->client->pers.netname ) );
			OSP_StartCountdown();
		} else {
			trap_SendServerCommand( ent - g_entities, "print \"You are already NOT ready!\n\"" );
		}
	} else if ( ready ) {
		trap_SendServerCommand( ent - g_entities, "print \"You are already ready!\n\"" );
		OSP_StartCountdown();
	} else {
		ent->client->pers.ready = 0;
		ent->client->readyToExit = 0;
		OSP_ResetPlayerReady();
		trap_SendServerCommand( -1, va( "print \"%s^7 is NOT ready!\n\"", ent->client->pers.netname ) );
		trap_SendServerCommand( -1, va( "cp \"\n%s\n^3is NOT ready!\n\"", ent->client->pers.netname ) );
		OSP_StopCountdown();
	}
}

/*
==================
OSP_Cmd_Players_f

"players" / "playerlist" (and the server console's "players", ent NULL): every connected
client sorted by name with team, coach / referee flags, client settings and, in warmup,
the ready status
==================
*/
void OSP_Cmd_Players_f( gentity_t *ent ) {
	gclient_t	*cl;
	int			idx;
	int			i;
	char		n1[MAX_NETNAME];
	char		ref[16];
	char		ready[16];
	gentity_t	*cent;
	char		rate[256];
	char		*s2;
	char		*coach;
	int			tteam;
	int			count;
	int			user_rate;
	char		userinfo[MAX_INFO_STRING];
	int			max_rate;
	char		*s;

	count = 0;

	if ( ent && OSP_CmdHelpCheck( ent, 4 ) ) {
		return;
	}

	max_rate = trap_Cvar_VariableIntegerValue( "sv_maxrate" );

	if ( !level.warmupTime ) {
		if ( ent ) {
			trap_SendServerCommand( ent - g_entities, "print \"\n^3 ID^1 : ^3Player                    Nudge  Rate  MaxPkts  Snaps\n\"" );
			trap_SendServerCommand( ent - g_entities, "print \"^1-----------------------------------------------------------^7\n\"" );
		} else {
			G_Printf( " ID : Player                    Nudge  Rate  MaxPkts  Snaps\n" );
			G_Printf( "-----------------------------------------------------------\n" );
		}
	} else {
		if ( ent ) {
			trap_SendServerCommand( ent - g_entities, "print \"\n^3Status^1   : ^3ID^1 : ^3Player                    Nudge  Rate  MaxPkts  Snaps\n\"" );
			trap_SendServerCommand( ent - g_entities, "print \"^1---------------------------------------------------------------------^7\n\"" );
		} else {
			G_Printf( "Status   : ID : Player                    Nudge  Rate  MaxPkts  Snaps\n" );
			G_Printf( "---------------------------------------------------------------------\n" );
		}
	}

	for ( i = 0; i < level.numConnectedClients; i++ ) {
		idx = level.sortedByName[i];
		cl = &level.clients[idx];
		cent = g_entities + idx;

		SanitizeString( cl->pers.netname, n1, qfalse );
		n1[26] = 0;
		ref[0] = 0;
		ready[0] = 0;

		if ( cent->r.svFlags & SVF_BOT ) {
			strcpy( rate, va( "%s%s%s%s", "[BOT]", " -----", "       --", "     --" ) );
		} else if ( cl->pers.connected == CON_CONNECTING ) {
			strcpy( rate, va( "%s", "^3>>> CONNECTING <<<" ) );
		} else {
			trap_GetUserinfo( idx, userinfo, sizeof( userinfo ) );
			s = Info_ValueForKey( userinfo, "rate" );
			user_rate = atoi( s );
			if ( max_rate && user_rate > max_rate ) {
				user_rate = max_rate;
			}
			strcpy( rate, va( "%5d%6d%9d%7d", cl->pers.timeNudge, user_rate, cl->pers.maxPackets, cl->pers.snaps ) );
		}

		if ( !level.warmupTime ) {	// the ready status is shown in warmup only (an empty then-block in OSP)
		} else if ( cl->sess.sessionTeam == TEAM_SPECTATOR || cl->pers.connected == CON_CONNECTING ) {
			if ( ent ) {
				strcpy( ready, "^5--------^1 :" );
			} else {
				strcpy( ready, "-------- :" );
			}
		} else if ( cl->pers.ready || ( g_entities[idx].r.svFlags & SVF_BOT ) ) {
			if ( ent ) {
				strcpy( ready, "^3(READY)^1  :" );
			} else {
				strcpy( ready, "(READY)  :" );
			}
		} else {
			if ( ent ) {
				strcpy( ready, "NOTREADY^1 :" );
			} else {
				strcpy( ready, "NOTREADY :" );
			}
		}

		if ( cl->sess.referee ) {
			strcpy( ref, "REF" );
		}

		if ( cl->sess.coachTeam ) {
			tteam = cl->sess.coachTeam;
			if ( ent ) {
				coach = "^3C";
			} else {
				coach = "C";
			}
		} else {
			tteam = cl->sess.sessionTeam;
			coach = " ";
		}

		if ( g_gametype.integer < GT_TEAM ) {
			if ( ent ) {
				s2 = "^7 ";
			} else {
				s2 = " ";
			}
		} else if ( tteam == TEAM_RED ) {
			if ( ent ) {
				s2 = "^1R^7";
			} else {
				s2 = "R";
			}
		} else if ( tteam == TEAM_BLUE ) {
			if ( ent ) {
				s2 = "^4B^7";
			} else {
				s2 = "B";
			}
		} else {
			if ( ent ) {
				s2 = "^7 ";
			} else {
				s2 = " ";
			}
		}

		if ( ent ) {
			trap_SendServerCommand( ent - g_entities, va( "print \"%s%s%2d%s^1:%s %-26s^7%s  ^3%s\n\"",
				ready, s2, idx, coach, ( ref[0] ) ? "^3" : "^7", n1, rate, ref ) );
		} else {
			G_Printf( "%s%s%2d%s: %-26s%s  %s\n", ready, s2, idx, coach, n1, rate, ref );
		}

		count++;
	}

	if ( ent ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"\n^3%2d^7 total players\n\n\"", count ) );
	} else {
		G_Printf( "\n%2d total players\n\n", count );
	}
}

/*
==================
OSP_Cmd_Invite_f

"invite" / "pickplayer <player>": a captain invites a player to join his team (may join it
while locked, pers.invitedTeam)
==================
*/
void OSP_Cmd_Invite_f( gentity_t *ent ) {
	char		arg[MAX_TOKEN_CHARS];
	int			team;
	gentity_t	*other;
	int			pid;

	if ( OSP_CmdHelpCheck( ent, 5 ) ) {
		return;
	}
	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3invite^7 is for team-games only!\n\"" );
		return;
	}
	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Team commands not enabled on this server.\n\"" );
		return;
	}

	team = OSP_ClientTeam( ent );
	if ( team == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"Spectators can't invite players!\n\"" );
		return;
	}
	if ( !team_allcaptain.integer && !ent->client->sess.captain ) {
		trap_SendServerCommand( ent - g_entities, "print \"Only team captains invite players!\n\"" );
		return;
	}
	if ( !level.warmupTime && OSP_IsRoundBased( g_gametype.integer ) ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can only invite players between rounds!\n\"" );
		return;
	}
	if ( !OSP_CmdFloodCheck( ent ) ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		OSP_PrintInvalidPlayerID( ent );
		return;
	}
	if ( level.clients + pid == ent->client ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't invite yourself!\n\"" );
		return;
	}

	other = g_entities + pid;
	if ( OSP_ClientTeam( other ) == team ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't invite a teammate!\n\"" );
		return;
	}

	other->client->pers.invitedTeam = team;
	if ( team == TEAM_RED ) {
		Com_sprintf( arg, sizeof( arg ), "print \"*** You've been invited to join team ^1RED^7!\nTo accept, in the console type: ^7team r^0\n\"" );
	} else {
		Com_sprintf( arg, sizeof( arg ), "print \"*** You've been invited to join team ^4BLUE^7!\nTo accept, in the console type: ^7team b^0\n\"" );
	}
	trap_SendServerCommand( other - g_entities, arg );
	Com_sprintf( arg, sizeof( arg ), "print \"%s^7 has been sent an invitation.\n\"", other->client->pers.netname );
	trap_SendServerCommand( ent - g_entities, arg );
}

/*
==================
OSP_Cmd_SpecInvite_f

"specinvite" / "invitespec <player>": a captain lets a spectator follow his (spec-locked) team
(sess.specInvite: 1 red, 2 blue, 3 both)
==================
*/
void OSP_Cmd_SpecInvite_f( gentity_t *ent ) {
	gentity_t	*other;
	int			team;
	int			pid;
	char		arg[MAX_TOKEN_CHARS];

	if ( OSP_CmdHelpCheck( ent, 6 ) ) {
		return;
	}
	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3specinvite^7 is for team-games only!\n\"" );
		return;
	}
	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Team commands not enabled on this server.\n\"" );
		return;
	}

	team = OSP_ClientTeam( ent );
	if ( team == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"Spectators can't specinvite players!\n\"" );
		return;
	}
	if ( !team_allcaptain.integer && !ent->client->sess.captain ) {
		trap_SendServerCommand( ent - g_entities, "print \"Only team captains specinvite players!\n\"" );
		return;
	}
	if ( !OSP_CmdFloodCheck( ent ) ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		OSP_PrintInvalidPlayerID( ent );
		return;
	}
	if ( level.clients + pid == ent->client ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't specinvite yourself!\n\"" );
		return;
	}

	other = g_entities + pid;
	if ( OSP_ClientTeam( other ) != TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't specinvite a non-spectator!\n\"" );
		return;
	}

	if ( team == TEAM_RED ) {
		if ( other->client->sess.specInvite == 0 || other->client->sess.specInvite == 2 ) {
			other->client->sess.specInvite++;
		}
	} else if ( other->client->sess.specInvite < 2 ) {
		other->client->sess.specInvite += 2;
	}

	trap_SendServerCommand( other - g_entities, va( "print \"*** You've been invited to spectate the %s team!\n\"", osp_teamNames[team] ) );
	trap_SendServerCommand( ent - g_entities, va( "print \"%s^7 has been sent a spectator invitation.\n\"", other->client->pers.netname ) );
}

/*
==================
OSP_Cmd_CoachInvite_f

"coachinvite <player>" (TDM / CTF): a captain invites a spectator to coach his team
(pers.coachInvite, accepted with "coach")
==================
*/
void OSP_Cmd_CoachInvite_f( gentity_t *ent ) {
	gentity_t	*other;
	int			pid;
	char		arg[MAX_TOKEN_CHARS];

	if ( OSP_CmdHelpCheck( ent, 7 ) ) {
		return;
	}
	if ( g_gametype.integer != GT_TEAM && g_gametype.integer != GT_CTF ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3specinvite^7 is for Team DM/CTF only!\n\"" );
		return;
	}
	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Team commands not enabled on this server.\n\"" );
		return;
	}
	if ( OSP_ClientTeam( ent ) == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"Spectators can't coachinvite players!\n\"" );
		return;
	}
	if ( !team_allcaptain.integer && !ent->client->sess.captain ) {
		trap_SendServerCommand( ent - g_entities, "print \"Only team captains coachinvite players!\n\"" );
		return;
	}
	if ( !OSP_CmdFloodCheck( ent ) ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		OSP_PrintInvalidPlayerID( ent );
		return;
	}
	if ( level.clients + pid == ent->client ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't coachinvite yourself!\n\"" );
		return;
	}

	other = g_entities + pid;
	if ( OSP_ClientTeam( other ) != TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"You may only coachinvite a spectator.\n\"" );
		return;
	}
	if ( other->client->pers.coachInvite ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"%s^7 has already been invited to coach the %s team.\n\"",
			other->client->pers.netname, osp_teamNames[other->client->pers.coachInvite] ) );
		return;
	}

	other->client->pers.coachInvite = OSP_ClientTeam( ent );
	trap_SendServerCommand( other - g_entities, va( "print \"\n^3*** You've been invited to coach the %s^3 team!\n^3Type ^7\\coach^3 to accept the invitation.\n\"",
		osp_teamNames[other->client->pers.coachInvite] ) );
	trap_SendServerCommand( ent - g_entities, va( "print \"%s^7 has been sent a coaching invitation.\n\"", other->client->pers.netname ) );
}

/*
==================
OSP_Cmd_CoachDecline_f

"coachdecline": declines a coach invitation, or resigns as a coach
==================
*/
void OSP_Cmd_CoachDecline_f( gentity_t *ent ) {
	if ( OSP_CmdHelpCheck( ent, 8 ) ) {
		return;
	}
	if ( !ent->client->pers.coachInvite && !ent->client->sess.coachTeam ) {
		trap_SendServerCommand( ent - g_entities, "print \"You haven't been invited to coach a team!\n\"" );
		return;
	}
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		ent->client->pers.coachInvite = 0;
		return;
	}

	if ( !ent->client->sess.coachTeam ) {
		trap_SendServerCommand( -1, va( "cp \"%s^7 has declined\nthe %s team's coach invitation\n\"",
			ent->client->pers.netname, osp_teamNames[ent->client->pers.coachInvite] ) );
	} else {
		trap_SendServerCommand( -1, va( "cp \"%s^7 has resigned\nfrom coaching the %s team\n\"",
			ent->client->pers.netname, osp_teamNames[ent->client->sess.coachTeam] ) );
		trap_SendServerCommand( -1, va( "print \"%s^7 has resigned from coaching the %s team\n\"",
			ent->client->pers.netname, osp_teamNames[ent->client->sess.coachTeam] ) );
	}

	ent->client->pers.coachInvite = 0;
	ent->client->sess.coachTeam = 0;
	ent->client->pers.followTeam = 0;
	ent->client->pers.mvSavedCount = 0;
	if ( !ent->client->sess.referee ) {
		ent->client->sess.specInvite = 0;
	}
	SetTeam( ent, "s" );
}

/*
==================
OSP_Cmd_CoachKick_f

"coachkick <player>": a captain removes his team's coach
==================
*/
void OSP_Cmd_CoachKick_f( gentity_t *ent ) {
	gentity_t	*other;
	char		arg[MAX_TOKEN_CHARS];
	int			pid;

	if ( OSP_CmdHelpCheck( ent, 9 ) ) {
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Team commands not enabled on this server.\n\"" );
		return;
	}
	if ( !team_allcaptain.integer && !ent->client->sess.captain ) {
		trap_SendServerCommand( ent - g_entities, "print \"Only team captains coachkick players!\n\"" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		OSP_PrintInvalidPlayerID( ent );
		return;
	}

	other = g_entities + pid;
	if ( other->client->sess.sessionTeam != TEAM_SPECTATOR || other->client->sess.coachTeam != ent->client->sess.sessionTeam ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"%s^7 is not a coach for your team!\n\"", other->client->pers.netname ) );
		return;
	}

	trap_SendServerCommand( -1, va( "cp \"%s^7 has been removed\nas a coach for the %s team\n\"",
		other->client->pers.netname, osp_teamNames[ent->client->sess.sessionTeam] ) );
	other->client->sess.coachTeam = 0;
	other->client->pers.followTeam = 0;
	other->client->pers.mvSavedCount = 0;
	if ( !other->client->sess.referee ) {
		other->client->sess.specInvite = 0;
	}
	OSP_MV_RemoveAll( other );
	SetTeam( other, "s" );
}

/*
==================
OSP_Cmd_Coach_f

"coach" (fHelp 1; SpectatorThink / ClientEndFrame call it with 0 to refresh the coach view):
accepts a coach invitation, then views the coached team
==================
*/
void OSP_Cmd_Coach_f( gentity_t *ent, qboolean fHelp ) {
	if ( fHelp && OSP_CmdHelpCheck( ent, 10 ) ) {
		return;
	}
	if ( !ent->client->pers.coachInvite && !ent->client->sess.coachTeam ) {
		trap_SendServerCommand( ent - g_entities, "print \"You have not been invited to coach a team!\n\"" );
		return;
	}
	if ( ent->client->sess.nameTeam ) {
		trap_SendServerCommand( ent - g_entities, "print \"Not allowed to switch from current team.\n\"" );
		return;
	}

	if ( ent->client->pers.coachInvite ) {
		if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
			trap_SendServerCommand( ent - g_entities, "print \"You must be a spectator to become a coach!\n\"" );
			return;
		}

		ent->client->sess.coachTeam = ent->client->pers.coachInvite;
		ent->client->pers.followTeam = ent->client->pers.coachInvite;
		SetTeam( ent, "s" );
		if ( !ent->client->sess.referee ) {
			ent->client->sess.specInvite = ent->client->pers.coachInvite;
		}
		ent->client->pers.coachInvite = 0;
		ClientUserinfoChanged( ent - g_entities );

		trap_SendServerCommand( -1, va( "cp \"%s^7 has become\na coach for the %s team\n\"",
			ent->client->pers.netname, osp_teamNames[ent->client->sess.coachTeam] ) );
		trap_SendServerCommand( -1, va( "print \"%s^7 has become a coach for the %s team\n\"",
			ent->client->pers.netname, osp_teamNames[ent->client->sess.coachTeam] ) );
	}

	OSP_Cmd_ViewTeam_f( ent, ent->client->sess.coachTeam, qtrue );
}

/*
==================
OSP_Cmd_Remove_f

"remove <player>" / "kickplayer": a captain (or referee) moves a teammate to the spectators
==================
*/
void OSP_Cmd_Remove_f( gentity_t *ent ) {
	gentity_t	*targ;
	int			team;
	int			pid;
	char		arg[MAX_TOKEN_CHARS];

	if ( OSP_CmdHelpCheck( ent, 11 ) ) {
		return;
	}

	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3remove^7 is for team-games only!\n\"" );
		return;
	}

	team = OSP_ClientTeam( ent );
	if ( team == TEAM_SPECTATOR && !ent->client->sess.referee ) {
		trap_SendServerCommand( ent - g_entities, "print \"Spectators can't remove players!\n\"" );
		return;
	}

	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Team commands not enabled on this server.\n\"" );
		return;
	}

	if ( !ent->client->sess.captain && !ent->client->sess.referee ) {
		trap_SendServerCommand( ent - g_entities, "print \"Only team captains or referees can remove players!\n\"" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		OSP_PrintInvalidPlayerID( ent );
		return;
	}

	if ( &level.clients[pid] == ent->client ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't remove yourself!\n\"" );
		return;
	}

	targ = g_entities + pid;
	if ( targ->client->sess.sessionTeam != team && !ent->client->sess.referee ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can only remove people on your team!\n\"" );
		return;
	}

	targ->client->pers.ready = 0;
	targ->client->readyToExit = 0;
	targ->client->pers.invitedTeam = 0;
	targ->client->sess.roundTeam = 0;

	// kill him (makes sure he loses flags, etc)
	targ->flags &= ~FL_GODMODE;
	targ->client->ps.stats[STAT_HEALTH] = targ->health = 0;
	player_die( targ, targ, targ, 100000, MOD_SWITCHTEAM );

	targ->client->sess.sessionTeam = TEAM_SPECTATOR;
	targ->client->sess.spectatorTime = level.time;
	targ->client->pers.osp_unk_2bc = level.time;
	targ->client->sess.spectatorState = SPECTATOR_FREE;
	targ->client->sess.spectatorClient = 0;
	ClientUserinfoChanged( targ - g_entities );

	trap_SendServerCommand( -1, va( "cp \"^5%s removed from team %s\n\"", targ->client->pers.netname, osp_teamNames[team] ) );
	trap_SendServerCommand( targ - g_entities, va( "print \"^5You've been removed from the %s team\n\"", osp_teamNames[team] ) );
	OSP_ResetPlayerReady();
	OSP_StartCountdown();
}

/*
==================
OSP_Cmd_Resign_f

"resign [player]": a captain passes the captaincy to the given teammate, or to the next
connected teammate
==================
*/
void OSP_Cmd_Resign_f( gentity_t *ent ) {
	int			i;
	gentity_t	*targ;
	int			pid;
	char		arg[MAX_TOKEN_CHARS];

	if ( OSP_CmdHelpCheck( ent, 12 ) ) {
		return;
	}

	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3resign^7 is for team-games only!\n\"" );
		return;
	}

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"Spectators can't resign!\n\"" );
		return;
	}

	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Captains not enabled on this server.\n\"" );
		return;
	}

	if ( !OSP_CmdFloodCheck( ent ) ) {
		return;
	}

	if ( !ent->client->sess.captain ) {
		trap_SendServerCommand( ent - g_entities, "print \"Only team captains can resign!\n\"" );
		return;
	}

	if ( TeamCount( -1, ent->client->sess.sessionTeam ) == 1 ) {
		trap_SendServerCommand( ent - g_entities, "print \"You are the only player on the team!\n\"" );
		return;
	}

	if ( trap_Argc() == 1 ) {
		for ( i = ( ent - g_entities + 1 ) % level.maxclients; i != ent - g_entities; i = ( i + 1 ) % level.maxclients ) {
			if ( level.clients[i].pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( level.clients[i].sess.sessionTeam == ent->client->sess.sessionTeam ) {
				level.clients[i].sess.captain = 1;
				ent->client->sess.captain = 0;
				trap_SendServerCommand( ent - g_entities, va( "print \"Captainship resigned.  Its been assigned to: %s\n\"", level.clients[i].pers.netname ) );
				trap_SendServerCommand( i, "print \"** ^5You are now team captain!!^7\n\"" );
				OSP_TeamPrint( ent->client->sess.sessionTeam, va( "cp \"%s\n^7is now team captain\n\"", level.clients[i].pers.netname ) );
				return;
			}
		}

		trap_SendServerCommand( ent - g_entities, "print \"Couldn't find another valid player to assign captainship!\n\"" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		OSP_PrintInvalidPlayerID( ent );
		return;
	}

	if ( &level.clients[pid] == ent->client ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't resign to yourself!\n\"" );
		return;
	}

	targ = g_entities + pid;
	if ( targ->client->sess.sessionTeam != ent->client->sess.sessionTeam ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can only resign to people on your team!\n\"" );
		return;
	}

	targ->client->sess.captain = 1;
	ent->client->sess.captain = 0;
	trap_SendServerCommand( ent - g_entities, va( "print \"Captainship resigned to: %s\n\"", targ->client->pers.netname ) );
	trap_SendServerCommand( pid, "print \"** ^5You are now team captain!!^7\n\"" );
	OSP_TeamPrint( ent->client->sess.sessionTeam, va( "cp \"%s\n^7is now team captain\n\"", targ->client->pers.netname ) );
}

/*
==================
OSP_Cmd_TeamReady_f

"teamready" / "readyteam": readies (and locks) the whole team during warmup
==================
*/
void OSP_Cmd_TeamReady_f( gentity_t *ent ) {
	int		i;
	int		team;

	if ( OSP_CmdHelpCheck( ent, 13 ) ) {
		return;
	}

	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3teamready^7 is for team-games only!\n\"" );
		return;
	}

	if ( OSP_IsRoundBased( g_gametype.integer ) && osp_roundsPlayed ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3teamready^7 is only needed for first round!\n\"" );
		return;
	}

	if ( team_noteamready.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"\n^3teamready^7 command had been disabled.\n\n\"" );
		return;
	}

	team = OSP_ClientTeam( ent );
	if ( team == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"Spectators can't ready-up teams!\n\"" );
		return;
	}

	if ( !OSP_CmdFloodCheck( ent ) ) {
		return;
	}

	if ( !level.warmupTime || osp_matchPaused ) {
		trap_SendServerCommand( ent - g_entities, "print \"Match is already in progress!\n\"" );
		return;
	}

	if ( level.numPlayingClients < match_minplayers.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Not enough players to start match!\n\"" );
		return;
	}

	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			level.clients[i].pers.ready = 1;
			level.clients[i].readyToExit = 1;
		}
	}

	osp_teamInfo[team].locked = 1;
	trap_SendServerCommand( -1, va( "cp \"The %s team is now ready!\n\"", osp_teamNames[team] ) );
	OSP_ResetPlayerReady();
	OSP_StartCountdown();
}

/*
==================
OSP_Cmd_Captains_f

"captains": prints both teams' captains
==================
*/
void OSP_Cmd_Captains_f( gentity_t *ent ) {
	int		i;
	char	red[256];
	char	blue[256];

	if ( OSP_CmdHelpCheck( ent, 14 ) ) {
		return;
	}

	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3captains^7 is for team-games only!\n\"" );
		return;
	}

	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Captains not enabled on this server.\n\"" );
		return;
	}

	Com_sprintf( red, sizeof( red ), "print \"^3No captains found for team ^1RED^7\n\"" );
	Com_sprintf( blue, sizeof( blue ), "print \"^3No captains found for team ^4BLUE^7\n\"" );

	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.captain ) {
			if ( level.clients[i].sess.sessionTeam == TEAM_RED ) {
				Com_sprintf( red, sizeof( red ), "print \"Captain for team ^1RED^7 is: ^3%s^7\n\"", level.clients[i].pers.netname );
			} else if ( level.clients[i].sess.sessionTeam == TEAM_BLUE ) {
				Com_sprintf( blue, sizeof( blue ), "print \"Captain for team ^4BLUE^7 is: ^3%s^7\n\"", level.clients[i].pers.netname );
			}
		}
	}

	trap_SendServerCommand( ent - g_entities, red );
	trap_SendServerCommand( ent - g_entities, blue );
}

/*
==================
OSP_Cmd_Timeout_f

"time" / "pause" / "timeout" (fPause 1), "timein" / "unpause" (fPause 0): team (or 1v1 player)
timeouts; a referee pauses / unpauses with OSP_Ref_Pause
==================
*/
void OSP_Cmd_Timeout_f( gentity_t *ent, qboolean fPause ) {
	int		team;
	char	cmd[MAX_TOKEN_CHARS];

	if ( OSP_CmdHelpCheck( ent, 15 ) ) {
		return;
	}

	if ( ent->client->sess.referee ) {
		OSP_Ref_Pause( ent );
		return;
	} else {
		if ( !OSP_CmdFloodCheck( ent ) ) {
			return;
		}
		ent->client->pers.cmdDelayTime = level.time + 7500;
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( g_gametype.integer < GT_TEAM && g_gametype.integer != GT_TOURNAMENT ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"^3%s^7 is for 1v1/team-games only!\n\"", cmd ) );
		return;
	}

	team = OSP_ClientTeam( ent );
	if ( team == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"Spectators can't call ^3%s^7!\n\"", cmd ) );
		return;
	}

	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Team commands not enabled on this server.\n\"" );
		return;
	}

	if ( !ent->client->sess.captain && !team_allcaptain.integer && g_gametype.integer != GT_TOURNAMENT ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"Only team captains can call ^3%s^7!\n\"", cmd ) );
		return;
	}

	if ( level.warmupTime ) {
		if ( !osp_matchPaused ) {
			trap_SendServerCommand( ent - g_entities, va( "print \"%s not allowed during warmup!\n\"", cmd ) );
			return;
		}
		trap_SendServerCommand( ent - g_entities, va( "print \"A pause is already in progress!  ^3%s^7 command ignored.\n\"", cmd ) );
		return;
	}

	if ( ( ( g_gametype.integer == GT_TOURNAMENT && !ent->client->pers.timeouts ) ||
		( g_gametype.integer >= GT_TEAM && !osp_teamInfo[team].timeouts ) ) && !osp_matchPaused ) {
		trap_SendServerCommand( ent - g_entities, "print \"Sorry you have no more timeouts to call.\n\"" );
		return;
	}

	if ( !osp_matchPaused ) {
		if ( osp_unpausing ) {
			return;
		}

		if ( !fPause ) {
			trap_SendServerCommand( ent - g_entities, "print \"The match is not paused!  ^3timein^7 command ignored.\n\"" );
			return;
		}

		OSP_GlobalSound( "sound/world/klaxon1.wav" );
		if ( g_gametype.integer >= GT_TEAM ) {
			osp_matchPaused = team;
			osp_teamInfo[team].timeouts--;
			trap_SendServerCommand( -1, va( "cp \"The %s team has called time-out!\n\"", osp_teamNames[team] ) );
			trap_SendServerCommand( -1, va( "print \"The %s team has called time-out! (%s^7)\n\"", osp_teamNames[team], ent->client->pers.netname ) );
			if ( team == TEAM_RED ) {
				OSP_UpdateScoreCvars( "Timeout: RED" );
				G_LogPrintf( "Timeout: RED team\n" );
			} else {
				OSP_UpdateScoreCvars( "Timeout: BLUE" );
				G_LogPrintf( "Timeout: BLUE team\n" );
			}
		} else {
			osp_matchPaused = ent - g_entities + 1;
			ent->client->pers.timeouts--;
			trap_SendServerCommand( -1, va( "cp \"%s\n^7has called time-out!\n\"", ent->client->pers.netname ) );
			level.osp_unk_1e00 = 1;
			OSP_UpdateScoreCvars( "Timeout" );
			G_LogPrintf( "Timeout: %s\n", ent->client->pers.netname );
		}

		osp_timeoutEndTime = level.time + 1000 * match_timeoutlength.integer;
		trap_SetConfigstring( CS_LEVEL_START_TIME, va( "%i", osp_timeoutEndTime ) );
		return;
	}

	if ( fPause == qtrue ) {
		trap_SendServerCommand( ent - g_entities, "print \"A pause is already in progress!  ^3timeout^7 command ignored.\n\"" );
		return;
	}

	if ( ( g_gametype.integer >= GT_TEAM && team != osp_matchPaused ) ||
		( g_gametype.integer == GT_TOURNAMENT && ent - g_entities + 1 != osp_matchPaused ) ) {
		trap_SendServerCommand( ent - g_entities, "print \"Sorry, you didn't call the current time-out!\n\"" );
		return;
	}

	if ( osp_unpausing ) {
		return;
	}

	osp_unpausing = 1;
	level.warmupTime = level.time + 5000;
	osp_timeoutEndTime = 0;
	trap_SetConfigstring( CS_WARMUP, va( "%i", level.warmupTime ) );

	if ( g_gametype.integer == GT_TOURNAMENT ) {
		trap_SendServerCommand( -1, va( "cp \"%s\n^7has called time-in!\n\"", ent->client->pers.netname ) );
		G_LogPrintf( "Timein: %s\n", ent->client->pers.netname );
	} else {
		trap_SendServerCommand( -1, va( "cp \"The %s team has called time-in!\n\"", osp_teamNames[team] ) );
		G_LogPrintf( "Timein: %s team\n", osp_teamNames[team] );
	}
}

/*
==================
OSP_Cmd_Stats_f

"stats [player]" (fDump 0) / "statsdump [player]" (fDump 1): weapon stats of a player (yourself
or the followed player by default); statsdump also sends the scores table as a dump
==================
*/
void OSP_Cmd_Stats_f( gentity_t *ent, qboolean fDump ) {
	char	arg[MAX_TOKEN_CHARS];
	int		pid;

	if ( !fDump ) {
		if ( OSP_CmdHelpCheck( ent, 16 ) ) {
			return;
		}
	} else {
		if ( OSP_CmdHelpCheck( ent, 17 ) ) {
			return;
		}
	}

	if ( trap_Argc() < 2 ) {
		if ( OSP_ClientTeam( ent ) != TEAM_SPECTATOR ) {
			OSP_PrintWeaponStats( ent, ent );
		} else if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			OSP_PrintWeaponStats( ent, g_entities + ent->client->sess.spectatorClient );
		} else {
			trap_SendServerCommand( ent - g_entities, "print \"Type ^3\\stats <player_id>^7 to see stats on an active player.\n\"" );
			return;
		}
	} else {
		trap_Argv( 1, arg, sizeof( arg ) );
		if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
			OSP_PrintInvalidPlayerID( ent );
			return;
		}
		OSP_PrintWeaponStats( ent, g_entities + pid );
	}

	if ( fDump ) {
		int		autoAction;

		autoAction = ent->client->pers.autoAction;
		ent->client->pers.autoAction = 1;
		if ( g_gametype.integer == GT_TEAM ) {
			if ( !IS_FREEZETAG ) {
				OSP_Scores_TDM( ent, 1 );
			} else {
				OSP_Scores_FT( ent, 1 );
			}
		} else if ( g_gametype.integer == GT_CTF ) {
			OSP_Scores_CTF( ent, 1 );
		} else if ( g_gametype.integer == 5 ) {
			OSP_Scores_CA( ent, 1 );
		} else {
			OSP_Scores_FFA( ent, 1 );
		}
		ent->client->pers.autoAction = autoAction;
	}
}

/*
==================
OSP_Cmd_StatsAll_f

"statsall" (team 0: every player), "statsred" / "statsblue": weapon stats of each player
==================
*/
void OSP_Cmd_StatsAll_f( gentity_t *ent, int team ) {
	int		i;

	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected != CON_CONNECTED ||
			( !team && level.clients[i].sess.sessionTeam == TEAM_SPECTATOR ) ||
			( team && level.clients[i].sess.sessionTeam != team ) ) {
			continue;
		}
		OSP_PrintWeaponStats( ent, g_entities + i );
	}
}

// OSP: attacks a player needs before the "topshots" /
// "bottomshots" tables list his accuracy with a weapon (indexed by weapon)
int		osp_minShots[WP_NUM_WEAPONS] = {
	999,	// WP_NONE
	999,	// WP_GAUNTLET
	50,		// WP_MACHINEGUN
	100,	// WP_SHOTGUN
	10,		// WP_GRENADE_LAUNCHER
	20,		// WP_ROCKET_LAUNCHER
	150,	// WP_LIGHTNING
	20,		// WP_RAILGUN
	60,		// WP_PLASMAGUN
	30,		// WP_BFG
	999		// WP_GRAPPLING_HOOK
};
// weapon of the "topshots <weapon>" table (OSP_SortShotsCompare's sort key)
int		osp_shotsWeapon = 0;

/*
==================
OSP_BuildBestShotsLine

"topshots" / "bottomshots" without a weapon: for each weapon the players with the best (fBest)
or worst accuracy, sent as one "bstats" / "bbstats" command
==================
*/
void OSP_BuildBestShotsLine( gentity_t *ent, qboolean fBest ) {
	gclient_t	*cl;
	int			w;
	int			i;
	float		acc;
	int			best;
	int			shots;
	int			list[MAX_CLIENTS];
	int			n;
	char		buf[MAX_STRING_CHARS];
	int			cnt;

	buf[0] = 0;

	for ( w = WP_MACHINEGUN; w <= WP_PLASMAGUN; w++ ) {
		best = fBest ? 0 : 99999;
		n = 0;
		cnt = 0;

		for ( i = 0; i < level.maxclients; i++ ) {
			cl = &level.clients[i];
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}

			shots = cl->sess.weaponStats[w].atts & 0xffff;
			if ( shots >= osp_minShots[w] ) {
				acc = 100.0f * ( cl->sess.weaponStats[w].hits & 0xffff ) / shots;
				list[n++] = i;
				if ( ( fBest ? acc : best ) > ( fBest ? best : acc ) ) {
					best = acc;
					cnt++;
				}
			}
		}

		if ( !best && fBest ) {
			continue;
		}
		if ( !fBest && cnt < 2 ) {
			continue;
		}

		for ( i = 0; i < n; i++ ) {
			cl = &level.clients[list[i]];
			acc = 100.0f * ( cl->sess.weaponStats[w].hits & 0xffff ) / ( cl->sess.weaponStats[w].atts & 0xffff );
			if ( ( fBest ? acc : best + 0.999f ) >= ( fBest ? best : acc ) ) {
				Q_strcat( buf, sizeof( buf ), va( " %d %d %d %d %d %d", w, list[i],
					cl->sess.weaponStats[w].hits & 0xffff, cl->sess.weaponStats[w].atts & 0xffff,
					cl->sess.weaponStats[w].kills, cl->sess.weaponStats[w].deaths ) );
			}
		}
	}

	trap_SendServerCommand( ent - g_entities, va( "bstats%s %s 0", fBest ? "" : "b", buf ) );
}

/*
==================
OSP_SortShotsCompare

qsort callback of "topshots <weapon>": best accuracy with osp_shotsWeapon first; spectators,
connecting players and players below osp_minShots last
==================
*/
int QDECL OSP_SortShotsCompare( const void *a, const void *b ) {
	gclient_t	*ca, *cb;
	float		accA, accB;

	ca = &level.clients[ *(int *)a ];
	cb = &level.clients[ *(int *)b ];

	if ( ca->sess.spectatorState == SPECTATOR_SCOREBOARD || ca->sess.spectatorClient < 0 ) {
		return 1;
	}
	if ( cb->sess.spectatorState == SPECTATOR_SCOREBOARD || cb->sess.spectatorClient < 0 ) {
		return -1;
	}

	if ( ca->pers.connected == CON_CONNECTING ) {
		return 1;
	}
	if ( cb->pers.connected == CON_CONNECTING ) {
		return -1;
	}

	if ( ca->sess.sessionTeam == TEAM_SPECTATOR ) {
		return 1;
	}
	if ( cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		return -1;
	}

	if ( ( ca->sess.weaponStats[osp_shotsWeapon].atts & 0xffff ) < osp_minShots[osp_shotsWeapon] ) {
		return 1;
	}
	if ( ( cb->sess.weaponStats[osp_shotsWeapon].atts & 0xffff ) < osp_minShots[osp_shotsWeapon] ) {
		return -1;
	}

	accA = 100.0f * ( ca->sess.weaponStats[osp_shotsWeapon].hits & 0xffff ) / ( ca->sess.weaponStats[osp_shotsWeapon].atts & 0xffff );
	accB = 100.0f * ( cb->sess.weaponStats[osp_shotsWeapon].hits & 0xffff ) / ( cb->sess.weaponStats[osp_shotsWeapon].atts & 0xffff );
	if ( accA > accB ) {
		return -1;
	}
	return 1;
}

/*
==================
OSP_Cmd_TopShots_f

"topshots [weapon]" (fBest 1) / "bottomshots [weapon]" (fBest 0): accuracy table of one weapon
("astats"), or the best / worst player of each weapon without an argument
==================
*/
void OSP_Cmd_TopShots_f( gentity_t *ent, qboolean fBest ) {
	int		i;
	int		shots;
	int		best;
	char	buf[MAX_STRING_CHARS];
	int		cnt;

	cnt = 0;

	if ( !ent || OSP_CmdHelpCheck( ent, fBest ? 20 : 23 ) ) {
		return;
	}

	if ( trap_Argc() < 2 ) {
		OSP_BuildBestShotsLine( ent, fBest );
		return;
	}

	best = fBest ? 0 : 99999;
	trap_Argv( 1, buf, sizeof( buf ) );
	if ( !( osp_shotsWeapon = atoi( buf ) ) || osp_shotsWeapon < WP_MACHINEGUN || osp_shotsWeapon > WP_PLASMAGUN ) {
		for ( osp_shotsWeapon = WP_PLASMAGUN; osp_shotsWeapon >= WP_MACHINEGUN; osp_shotsWeapon-- ) {
			if ( !Q_stricmp( buf, osp_weaponShortNames[osp_shotsWeapon] ) ) {
				break;
			}
		}
	}

	if ( osp_shotsWeapon < WP_MACHINEGUN ) {
		OSP_CmdHelpCheck( ent, 20 );
		return;
	}

	memcpy( level.sortedByAccuracy, level.sortedClients, sizeof( level.sortedClients ) );
	qsort( level.sortedByAccuracy, level.numConnectedClients, sizeof( int ), OSP_SortShotsCompare );

	buf[0] = 0;
	for ( i = 0; i < level.numConnectedClients; i++ ) {
		gclient_t	*cl;
		float		acc;

		cl = &level.clients[ level.sortedByAccuracy[i] ];
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}

		shots = cl->sess.weaponStats[osp_shotsWeapon].atts & 0xffff;
		if ( shots >= osp_minShots[osp_shotsWeapon] ) {
			acc = 100.0f * ( cl->sess.weaponStats[osp_shotsWeapon].hits & 0xffff ) / shots;
			cnt++;
			best = ( ( fBest ? acc : best ) > ( fBest ? best : acc ) ) ? (int)acc : best;
			Q_strcat( buf, sizeof( buf ), va( " %d %d %d %d %d", level.sortedByAccuracy[i],
				cl->sess.weaponStats[osp_shotsWeapon].hits & 0xffff, cl->sess.weaponStats[osp_shotsWeapon].atts & 0xffff,
				cl->sess.weaponStats[osp_shotsWeapon].kills, cl->sess.weaponStats[osp_shotsWeapon].deaths ) );
		}
	}

	trap_SendServerCommand( ent - g_entities, va( "astats%s %d %d%s", fBest ? "" : "b", cnt, best, buf ) );
}

/*
==================
OSP_Cmd_Joincode_f

"joincode [password]": players see their team's join code; a spectator joins the team whose
code he gives (locked teams, match running without match_latejoin)
==================
*/
void OSP_Cmd_Joincode_f( gentity_t *ent ) {
	register int	i;		// register: matches OSP's frame layout
	register int	team;
	char	buf[MAX_TOKEN_CHARS];

	if ( OSP_CmdHelpCheck( ent, 18 ) ) {
		return;
	}

	if ( OSP_IsRoundBased( g_gametype.integer ) ) {
		trap_SendServerCommand( ent - g_entities, "print \"Joincodes not used in round-based games!\n\"" );
		return;
	}

	team = OSP_ClientTeam( ent );
	if ( team != TEAM_SPECTATOR ) {
		if ( !level.warmupTime ) {
			if ( !match_latejoin.integer && !team_nocontrols.integer ) {
				trap_SendServerCommand( ent - g_entities, va( "print \"^5Team joincode is: ^3%s^7\n\"", osp_teamInfo[ent->client->sess.sessionTeam].joincode ) );
				return;
			}
			trap_SendServerCommand( ent - g_entities, "print \"Open teamplay.  No joincode needed.\n\"" );
			return;
		}
		trap_SendServerCommand( ent - g_entities, "print \"Match is in warmup.  Joincodes are not yet set.\n\"" );
		return;
	}

	if ( level.warmupTime ) {
		trap_SendServerCommand( ent - g_entities, "print \"Match is in warmup.  Joincodes not accepted.\n\"" );
		return;
	}

	if ( match_latejoin.integer || team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Open teamplay.  No joincode needed.\n\"" );
		return;
	}

	if ( trap_Argc() != 2 ) {
		trap_SendServerCommand( ent - g_entities, "print \"Usage: ^3joincode <password>^7\n\"" );
		return;
	}

	trap_Argv( 1, buf, sizeof( buf ) );
	for ( i = TEAM_RED; i <= TEAM_BLUE; i++ ) {
		if ( osp_teamInfo[i].joincode[0] && !Q_stricmp( buf, osp_teamInfo[i].joincode ) ) {
			break;
		}
	}

	if ( i > TEAM_BLUE ) {
		trap_SendServerCommand( ent - g_entities, "print \"Invalid joincode!\n\"" );
		return;
	}

	if ( TeamCount( -1, i ) >= team_maxplayers.integer ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"Sorry, team %s is full!\n", osp_teamNames[i] ) );
		return;
	}

	ent->client->pers.teamState.state = TEAM_BEGIN;
	ent->client->sess.spectatorTime = level.time;
	ent->client->pers.ready = 0;
	ent->client->readyToExit = 0;
	ent->client->sess.sessionTeam = i;
	ent->client->sess.spectatorState = SPECTATOR_NOT;
	ent->client->sess.spectatorClient = 0;
	BroadcastTeamChange( ent->client, TEAM_SPECTATOR );
	ClientUserinfoChanged( ent - g_entities );
	ClientBegin( ent - g_entities );
	OSP_ResetPlayerReady();
}

/*
==================
OSP_Cmd_Drop_f

"drop <item>" (match_dropitems, team DM / CTF): "flag" in CTF, a weapon ("sg" .. "pg", with a
pack of its ammo) or an ammo pack; without a valid item it lists them
==================
*/
void OSP_Cmd_Drop_f( gentity_t *ent ) {
	int			weap = -1;
	int			ammo = -1;
	gitem_t		*item = NULL;
	int			qty;
	char		*ammoNames[7] = { "Bullets", "Shells", "Grenades", "Rockets", "Lightning", "Slugs", "Cells" };	char		*weapShortNames[6] = { "sg", "gl", "rl", "lg", "rg", "pg" };
	char		*weapNames[6] = { "Shotgun", "Grenade Launcher", "Rocket Launcher", "Lightning Gun", "Railgun", "Plasma Gun" };
	int			ammoQuantity[7] = { 50, 10, 5, 5, 60, 10, 30 };
	int			ammoWeapons[7] = { WP_MACHINEGUN, WP_SHOTGUN, WP_GRENADE_LAUNCHER, WP_ROCKET_LAUNCHER, WP_LIGHTNING, WP_RAILGUN, WP_PLASMAGUN };
	int			i;
	gentity_t	*drop;
	char		arg[MAX_TOKEN_CHARS];

	if ( OSP_CmdHelpCheck( ent, 19 ) ) {
		return;
	}

	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"Spectators can't drop anything\n\"" );
		return;
	}

	if ( !match_dropitems.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3Item dropping is currently disabled\n" );
		return;
	}

	if ( g_gametype.integer != GT_TEAM && g_gametype.integer != GT_CTF ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3Item dropping only allowed in Team DM/CTF gamemodes!\n\"" );
		return;
	}

	if ( g_gametype.integer == GT_CTF ) {
		trap_Argv( 1, arg, sizeof( arg ) );
		if ( !Q_stricmp( arg, "flag" ) ) {
			qty = ( ent->client->sess.sessionTeam == TEAM_RED ) ? PW_BLUEFLAG : PW_REDFLAG;
			if ( !ent->client->ps.powerups[qty] ) {
				trap_SendServerCommand( ent - g_entities, "print \"^5You aren't carrying a flag!\n\"" );
				return;
			}

			item = BG_FindItemForPowerup( qty );
			drop = Drop_Item( ent, item, 45, ent );
			drop->count = ( ent->client->ps.powerups[qty] - level.time ) / 1000;
			ent->client->ps.powerups[qty] = 0;
			if ( drop->count < 1 ) {
				drop->count = 1;
			}
			return;
		}
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	weap = -1;
	ammo = -1;
	for ( i = 0; i < 7; i++ ) {
		if ( !Q_stricmp( arg, ammoNames[i] ) ) {
			ammo = i;
			break;
		}
		if ( i && !Q_stricmp( arg, weapShortNames[i - 1] ) ) {
			weap = i - 1;
			ammo = i;
			break;
		}
	}

	if ( ammo == -1 ) {
		trap_SendServerCommand( ent - g_entities, "print \"Valid items to drop:\n" );
		for ( i = 0; i < 6; i++ ) {
			trap_SendServerCommand( ent - g_entities, va( "print \"   ^3%s (%s)\n", weapShortNames[i], weapNames[i] ) );
		}
		for ( i = 0; i < 7; i++ ) {
			trap_SendServerCommand( ent - g_entities, va( "print \"   ^3%s\n", ammoNames[i] ) );
		}
		if ( trap_Argc() != 1 ) {
			trap_SendServerCommand( ent - g_entities, va( "print \"\nUnknown item to drop: ^3%s\n\"", arg ) );
		}
		return;
	}

	if ( weap != -1 ) {
		if ( !ent->client->weaponCount[ammoWeapons[ammo]] ) {
			trap_SendServerCommand( ent - g_entities, va( "print \"You don't have a ^3%s^7 to drop!\n", weapNames[weap] ) );
			return;
		} else if ( ent->client->weaponCount[ammoWeapons[ammo]] == 1 && ent->s.weapon == ammoWeapons[ammo] ) {
			trap_SendServerCommand( ent - g_entities, "print \"Sorry, you cant drop the weapon you are holding!\n" );
			return;
		} else {
			item = BG_FindItem( weapNames[weap] );
			if ( --ent->client->weaponCount[ammoWeapons[ammo]] == 0 ) {
				ent->client->ps.stats[STAT_WEAPONS] &= ~( 1 << ammoWeapons[ammo] );
			}
		}
	}

	qty = ammoQuantity[ammo];
	if ( ent->client->ps.ammo[ammoWeapons[ammo]] >= qty ) {
		ent->client->ps.ammo[ammoWeapons[ammo]] -= qty;
		if ( weap == -1 ) {
			item = BG_FindItem( ammoNames[ammo] );
		}
	} else if ( !ent->client->ps.ammo[ammoWeapons[ammo]] ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"You have no ^3%s^7 to drop!\n\"", ammoNames[ammo] ) );
		return;
	} else {
		trap_SendServerCommand( ent - g_entities, va( "print \"Not enough ^3%s^7 to drop!\n\"", ammoNames[ammo] ) );
		return;
	}

	if ( item ) {
		drop = Drop_Item( ent, item, 45, ent );
		drop->count = qty;
	}
}

/*
==================
OSP_Cmd_GetStatsInfo_f

"getstatsinfo" (OSP cgame +wstats window): "statsinfo <match stats> <weapon mask> <weapon stats>"
of the player (or the followed / multiview player), "statsinfo 0" when there is none
==================
*/
void OSP_Cmd_GetStatsInfo_f( gentity_t *ent ) {
	gclient_t	*cl;
	int			w;
	char		buf[MAX_STRING_CHARS];
	int			mask;
	int			pid;
	int			inIntermission;
	char		info[MAX_STRING_CHARS];
	char		*s;

	mask = 0;
	inIntermission = ( level.intermissiontime || level.intermissionQueued );

	if ( ( ent->client->sess.sessionTeam != TEAM_SPECTATOR && ( !IS_FREEZETAG || !is_spectator( ent->client ) || inIntermission ) ) ||
		( inIntermission && OSP_IsRoundBased( g_gametype.integer ) && OSP_ClientTeam( ent ) != TEAM_SPECTATOR ) ) {
		pid = ent - g_entities;
	} else if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_VIEWCAM_FOLLOW ) {
		pid = ent->client->sess.spectatorClient;
	} else if ( ent->client->sess.spectatorState == SPECTATOR_MULTIVIEW && ent->client->pers.mvCount ) {
		pid = ent->client->pers.mvClients[ent->client->pers.mvActive];
	} else if ( ent->client->sess.roundTeam == TEAM_RED || ent->client->sess.roundTeam == TEAM_BLUE ||
		( ent->client->sess.sessionTeam != TEAM_SPECTATOR && IS_FREEZETAG ) ) {
		pid = ent - g_entities;
	} else {
		trap_SendServerCommand( ent - g_entities, "statsinfo 0" );
		return;
	}

	cl = &level.clients[pid];
	// current flag carry time (pid is reused)
	pid = ( cl->ps.powerups[PW_REDFLAG] || cl->ps.powerups[PW_BLUEFLAG] ) ? level.time - cl->pers.teamState.flagsince : 0;

	s = va( "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i ",
		1,
		cl->ps.persistant[PERS_SCORE],
		( g_gametype.integer != 5 ) ? cl->sess.sessionTeam : ( ( cl->sess.sessionTeam == TEAM_SPECTATOR ) ? cl->sess.roundTeam : cl->sess.sessionTeam ),
		cl->sess.kills,
		cl->sess.deaths,
		cl->sess.suicides,
		cl->sess.teamKills,
		cl->sess.teamDamage,
		cl->sess.damageGiven,
		cl->sess.damageReceived,
		cl->sess.wins,
		cl->sess.losses,
		cl->pers.teamState.captures,
		cl->pers.teamState.assists,
		cl->pers.teamState.basedefense + cl->pers.teamState.carrierdefense,
		cl->pers.teamState.flagrecovery,
		cl->pers.teamState.flagtime + pid,
		cl->sess.megaHealthPickups,
		cl->sess.greenArmorPickups,
		cl->sess.redArmorPickups,
		cl->sess.yellowArmorPickups );
	strcpy( info, s );

	buf[0] = 0;
	for ( w = WP_GAUNTLET; w <= WP_BFG; w++ ) {
		if ( cl->sess.weaponStats[w].atts || cl->sess.weaponStats[w].hits || cl->sess.weaponStats[w].deaths ) {
			mask |= 1 << w;
			Q_strcat( buf, sizeof( buf ), va( " %d %d %d %d", cl->sess.weaponStats[w].hits, cl->sess.weaponStats[w].atts,
				cl->sess.weaponStats[w].kills, cl->sess.weaponStats[w].deaths ) );
		}
	}

	trap_SendServerCommand( ent - g_entities, va( "statsinfo %s %d%s", info, mask, buf ) );
}

/*
==================
OSP_ChatTokenTrace

The closest item entity of a kind (type 0: ammo, 1: armor / health, 2: weapons, powerups,
armor (no shards) and the mega health), for the chat tokens; fVisible: in the player's view
(65% cone, clear trace), else anywhere in his PVS, also dropped items
==================
*/
gentity_t *OSP_ChatTokenTrace( gentity_t *ent, int type, qboolean fVisible ) {
	gentity_t	*other;
	vec3_t		origin;
	vec3_t		dir;
	int			i;
	float		dist;
	float		bestDist;
	vec3_t		forward;
	gentity_t	*best;
	trace_t		tr;

	best = NULL;
	bestDist = 8192 * 8192 * 3;
	VectorCopy( ent->r.currentOrigin, origin );
	origin[2] += ent->client->ps.viewheight;

	for ( i = MAX_CLIENTS; i < level.num_entities; i++ ) {
		other = g_entities + i;
		if ( other->s.eType != ET_ITEM ) {
			continue;
		}
		if ( type == 0 && other->item->giType != IT_AMMO ) {
			continue;
		}
		if ( type == 1 && other->item->giType != IT_ARMOR && other->item->giType != IT_HEALTH ) {
			continue;
		}
		if ( type == 2 && other->item->giType != IT_WEAPON && other->item->giType != IT_POWERUP &&
			( other->item->giType != IT_ARMOR || other->item->quantity <= 5 ) &&
			( other->item->giType != IT_HEALTH || other->item->quantity != 100 ) ) {
			continue;
		}
		if ( fVisible && ( other->s.eFlags & EF_NODRAW ) ) {
			continue;
		}
		if ( !fVisible && ( other->flags & FL_DROPPED_ITEM ) ) {
			continue;
		}
		if ( !fVisible && ( other->s.eFlags & EF_NODRAW ) && !other->nextthink ) {
			continue;
		}

		dist = ( origin[0] - other->r.currentOrigin[0] ) * ( origin[0] - other->r.currentOrigin[0] ) +
			( origin[1] - other->r.currentOrigin[1] ) * ( origin[1] - other->r.currentOrigin[1] ) +
			( origin[2] - other->r.currentOrigin[2] ) * ( origin[2] - other->r.currentOrigin[2] );
		if ( dist > bestDist ) {
			continue;
		}

		if ( fVisible ) {
			VectorSubtract( other->r.currentOrigin, origin, dir );
			VectorNormalize( dir );
			AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
			if ( DotProduct( dir, forward ) < 0.65f ) {
				continue;
			}
			trap_Trace( &tr, origin, NULL, NULL, other->r.currentOrigin, ent->s.number, CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME );
			if ( tr.fraction != 1.0f ) {
				continue;
			}
		} else if ( !trap_InPVS( origin, other->r.currentOrigin ) ) {
			continue;
		}

		bestDist = dist;
		best = other;
	}

	return best;
}

/*
==================
OSP_ChatTokenNearestTeammate

The closest connected teammate ("#f")
==================
*/
gentity_t *OSP_ChatTokenNearestTeammate( gentity_t *ent ) {
	gentity_t	*other;
	int			i;
	vec3_t		origin;
	float		dist;
	float		bestDist;
	gentity_t	*nearest;

	nearest = NULL;
	bestDist = 8192 * 8192 * 3;
	VectorCopy( ent->r.currentOrigin, origin );

	for ( i = 0; i < level.maxclients; i++ ) {
		other = g_entities + i;
		if ( other->client->pers.connected != CON_CONNECTED || other == ent || !OnSameTeam( ent, other ) ) {
			continue;
		}

		dist = fabs( origin[0] - other->r.currentOrigin[0] ) + fabs( origin[1] - other->r.currentOrigin[1] ) +
			fabs( origin[2] - other->r.currentOrigin[2] );
		if ( dist < bestDist ) {
			bestDist = dist;
			nearest = other;
		}
	}

	return nearest;
}

/*
==================
OSP_ChatTokenHelper

Copies src to dst (at most size - 1 characters), returns the number of characters copied
==================
*/
int OSP_ChatTokenHelper( const char *src, char *dst, int size ) {
	const char	*s;
	char		*start;

	s = src;
	start = dst;
	while ( *s && dst - start < size - 1 ) {
		*dst++ = *s++;
	}
	*dst = 0;

	return dst - start;
}

// OSP: powerup names of "#u", indexed by powerup (OSP's list
// has no neutral flag, so the names from PW_NEUTRALFLAG on are one off: OSP bug, kept for compatibility)
static char	*osp_chatPowerupNames[PW_NUM_POWERUPS] = {
	"None",
	"Quad",
	"BS",
	"Haste",
	"Invis",
	"Regen",
	"Flight",
	"Red Flag",
	"Blue Flag",
	"Scout",
	"Guard",
	"Doubler",
	"AmmoRegen",
	"Invun"
};
// health / armor colors of "#H" / "#A" (below 50, below 100, 100 and more)
static char	*osp_chatValueColors[3] = { "^1", "^3", "^7" };

/*
==================
OSP_ChatTokens

Expands the chat tokens (at most 6, see osp-q3-Commands.txt): #a / #A armor, #c death location,
#d last attacker, #f nearest teammate, #h / #H health, #i item in view, #k ammo in view,
#l nearest item, #m weapons low on ammo, #p last pickup, #r armor / health in view, #t last
player damaged, #u powerups, #w weapon, ## a '#'. color is the chat color, restored after a token.
==================
*/
char *OSP_ChatTokens( gentity_t *ent, int color, const char *text ) {
	static char	buf[MAX_SAY_TEXT];
	int			len;
	int			tokens;

	len = 0;
	tokens = 0;

	while ( *text && len < sizeof( buf ) - 1 ) {
		if ( *text != '#' || !text[1] || tokens >= 6 ) {
			buf[len++] = *text++;
			continue;
		} else {
			char	*s = NULL;
			int		c;

			tokens++;
			c = tolower( *++text );
			switch ( c ) {
			case 0:
				tokens--;
				break;
			case '#':
				buf[len++] = '#';
				tokens--;
				break;
			case 'a': {
				int		armor;

				armor = ent->client->ps.stats[STAT_ARMOR];
				s = va( "%s%i", ( *text == 'a' ) ? "" : osp_chatValueColors[(int)Com_Clamp( 0, 100, armor ) / 50], armor );
				break;
			}
			case 'c': {
				gentity_t	*loc;

				for ( loc = level.locationHead; loc && !s; loc = loc->nextTrain ) {
					if ( loc->health == ent->client->pers.lastDeathLocation ) {
						s = loc->message;
					}
				}
				break;
			}
			case 'd': {
				int		pid;

				pid = ent->client->ps.persistant[PERS_ATTACKER];
				if ( pid >= 0 && pid < MAX_CLIENTS ) {
					len += OSP_NormalizeName( level.clients[pid].pers.netname, sizeof( buf ) - len, buf + len, qfalse );
				}
				break;
			}
			case 'f': {
				gentity_t	*other;

				other = OSP_ChatTokenNearestTeammate( ent );
				len += ( !other ) ? 0 : OSP_NormalizeName( other->client->pers.netname, sizeof( buf ) - len, buf + len, qfalse );
				break;
			}
			case 'h': {
				int		health;

				health = ent->client->ps.stats[STAT_HEALTH];
				s = va( "%s%i", ( *text == 'h' ) ? "" : osp_chatValueColors[(int)Com_Clamp( 0, 100, health ) / 50], health );
				break;
			}
			case 'i': {
				gentity_t	*item;

				item = OSP_ChatTokenTrace( ent, 2, qtrue );
				if ( item ) {
					s = item->item->pickup_name;
				}
				break;
			}
			case 'k': {
				gentity_t	*item;

				item = OSP_ChatTokenTrace( ent, 0, qtrue );
				if ( item ) {
					s = item->item->pickup_name;
				}
				break;
			}
			case 'l': {
				gentity_t	*item;

				item = OSP_ChatTokenTrace( ent, 2, qfalse );
				if ( item ) {
					s = item->item->pickup_name;
				}
				break;
			}
			case 'm': {
				int		w = 0;
				int		found = 0;

				for ( w = WP_BFG; w > 0; w-- ) {
					int		ammo;

					ammo = ent->client->ps.ammo[w];
					if ( ( ent->client->ps.stats[STAT_WEAPONS] & ( 1 << w ) ) && ammo >= 0 && ammo <= 5 ) {
						len += OSP_ChatTokenHelper( va( "%s^1%s^7-ammo", found ? " " : "[", osp_weaponShortNames[w] ), buf + len, sizeof( buf ) - len );
						found = 1;
					}
				}
				len += OSP_ChatTokenHelper( va( "%c%c%s", Q_COLOR_ESCAPE, color, found ? "]" : " " ), buf + len, sizeof( buf ) - len );
				break;
			}
			case 'p': {
				s = ent->client->lastPickupName;
				break;
			}
			case 'r': {
				gentity_t	*item;

				item = OSP_ChatTokenTrace( ent, 1, 1 );
				if ( item ) {
					s = item->item->pickup_name;
				}
				break;
			}
			case 't': {
				int		pid;

				pid = ent->client->pers.lastDamagedClient;
				if ( pid > 0 && pid <= MAX_CLIENTS ) {
					len += OSP_NormalizeName( level.clients[pid - 1].pers.netname, sizeof( buf ) - len, buf + len, qfalse );
				}
				break;
			}
			case 'u': {
				int		i = 0;
				int		found = 0;

				for ( i = 1; i < PW_NUM_POWERUPS; i++ ) {
					if ( ent->client->ps.powerups[i] > level.time ) {
						len += OSP_ChatTokenHelper( va( "%s^3%s", found ? " " : "[", osp_chatPowerupNames[i] ), buf + len, sizeof( buf ) - len );
						found = 1;
					}
				}
				len += OSP_ChatTokenHelper( va( "%c%c%s", Q_COLOR_ESCAPE, color, found ? "]" : " " ), buf + len, sizeof( buf ) - len );
				break;
			}
			case 'w': {
				len += OSP_ChatTokenHelper( va( "%s", osp_weaponShortNames[ent->s.weapon] ), buf + len, sizeof( buf ) - len );
				break;
			}
			default:
				buf[len++] = '#';
				buf[len++] = *text;
				tokens--;
				break;
			}

			if ( s ) {
				len += OSP_ChatTokenHelper( s, buf + len, sizeof( buf ) - len );
				len += OSP_ChatTokenHelper( va( "%c%c", Q_COLOR_ESCAPE, color ), buf + len, sizeof( buf ) - len );
			}
		}
		text++;
	}

	buf[len] = 0;
	return buf;
}
