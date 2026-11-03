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
// g_local.h -- local definitions for game module

#include "q_shared.h"
#include "bg_public.h"
#include "g_public.h"

//==================================================================

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"baseq3"

#define BODY_QUEUE_SIZE		8

#define INFINITE			1000000

#define	FRAMETIME			100					// msec
#define	CARNAGE_REWARD_TIME	3000
#define REWARD_SPRITE_TIME	2000

#define	INTERMISSION_DELAY_TIME	1000
#define	SP_INTERMISSION_DELAY_TIME	5000

// gentity->flags
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_DROPPED_ITEM			0x00001000
#define FL_NO_BOTS				0x00002000	// spawn point not for bot use
#define FL_NO_HUMANS			0x00004000	// spawn point just for bots
#define FL_FORCE_GESTURE		0x00008000	// force gesture on client

// movers are things like doors, plats, buttons, etc
typedef enum {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1
} moverState_t;

#define SP_PODIUM_MODEL		"models/mapobjects/podium/podium4.md3"

//============================================================================

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

struct gentity_s {
	entityState_t	s;				// communicated by server to clients
	entityShared_t	r;				// shared by both the server system and game

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	struct gclient_s	*client;			// NULL if not a client

	qboolean	inuse;

	char		*classname;			// set in QuakeEd
	int			spawnflags;			// set in QuakeEd

	qboolean	neverFree;			// if true, FreeEntity will only unlink
									// bodyque uses this

	int			flags;				// FL_* variables

	char		*model;
	char		*model2;
	int			freetime;			// level.time when the object was freed
	int			osp_unk_228;		// unknown, OSP field
	
	int			eventTime;			// events will be cleared EVENT_VALID_MSEC after set
	qboolean	freeAfterEvent;
	qboolean	unlinkAfterEvent;

	qboolean	physicsObject;		// if true, it can be pushed by movers and fall off edges
									// all game items are physicsObjects, 
	float		physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce
	int			clipmask;			// brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance

	// movers
	moverState_t moverState;
	int			soundPos1;
	int			sound1to2;
	int			sound2to1;
	int			soundPos2;
	int			soundLoop;
	gentity_t	*parent;
	gentity_t	*nextTrain;
	gentity_t	*prevTrain;
	vec3_t		pos1, pos2;

	char		*message;

	int			timestamp;		// body queue sinking, etc

	float		angle;			// set in editor, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*team;
	char		*targetShaderName;
	char		*targetShaderNewName;
	gentity_t	*target_ent;

	float		speed;
	vec3_t		movedir;

	int			nextthink;
	void		(*think)(gentity_t *self);
	void		(*reached)(gentity_t *self);	// movers call this when hitting endpoint
	void		(*blocked)(gentity_t *self, gentity_t *other);
	void		(*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
	void		(*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
	void		(*pain)(gentity_t *self, gentity_t *attacker, int damage);
	void		(*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

	int			pain_debounce_time;
	int			fly_sound_debounce_time;	// wind tunnel
	int			last_move_time;

	int			health;

	qboolean	takedamage;

	int			damage;
	int			splashDamage;	// quad will increase this without increasing radius
	int			splashRadius;
	int			methodOfDeath;
	int			splashMethodOfDeath;

	int			count;

	gentity_t	*chain;
	gentity_t	*enemy;
	gentity_t	*activator;
	gentity_t	*teamchain;		// next entity in team
	gentity_t	*teammaster;	// master of the team

#ifdef MISSIONPACK
	int			kamikazeTime;
	int			kamikazeShockTime;
#endif

	int			watertype;
	int			waterlevel;

	int			noise_index;

	// timing variables
	float		wait;
	float		random;

	gitem_t		*item;			// for bonus items

	// OSP 1.03a additions
	int			osp_unk_32c;
	int			pickupRespawn;		// respawn time of a pickup finished by OSP_DelayedPickupThink (Touch_Item)
	int			frozen;		// OSP: frozen in freeze tag (player_die after player_freeze; target_ent is the body, Team_GetLocation)
};


typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum {
	SPECTATOR_NOT,
	SPECTATOR_FREE,
	SPECTATOR_FOLLOW,
	SPECTATOR_SCOREBOARD,
	// OSP spectator states (SetTeam, SpectatorThink)
	SPECTATOR_VIEWCAM,			// 4: viewcam spot looking at a player ("team viewcam")
	SPECTATOR_VIEWCAM_FOLLOW,	// 5: viewcam following a player
	SPECTATOR_LOCKED,			// 6: spec-locked teams, no view of the players
	SPECTATOR_MULTIVIEW			// 7: multiview
} spectatorState_t;

typedef enum {
	TEAM_BEGIN,		// Beginning a team game, spawn at base
	TEAM_ACTIVE		// Now actively playing
} playerTeamStateState_t;

typedef struct {
	playerTeamStateState_t	state;

	int			location;

	int			captures;
	int			basedefense;
	int			carrierdefense;
	int			flagrecovery;
	int			fragcarrier;
	int			flagtime;		// OSP: total flag carry time, += level.time - flagsince (Use_target_remove_powerups, OSP_Scores_CTF, OSP_Cmd_GetStatsInfo_f)
	int			assists;

	float		lasthurtcarrier;
	float		lastreturnedflag;
	float		flagsince;
	float		lastfraggedcarrier;
} playerTeamState_t;

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define	FOLLOW_ACTIVE1	-1
#define	FOLLOW_ACTIVE2	-2

// OSP per-weapon session stats (clientSession_t.weaponStats, indexed by
// weapon_t; the session cvar stores weapons 1..9)
typedef struct {
	int			atts;		// attacks (FireWeapon: pellets for the shotgun) in the low 16 bits, pickups (Pickup_Weapon) in the high 16 bits
	int			hits;		// hits (OSP_DamageStats) in the low 16 bits, drops (TossClientItems) in the high 16 bits
	int			kills;		// OSP_DamageStats, attacker side
	int			deaths;		// OSP_DamageStats, victim side
} ospWeaponStats_t;

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct {
	team_t		sessionTeam;
	int			spectatorTime;		// for determining next-in-line to play
	spectatorState_t	spectatorState;
	int			spectatorClient;	// for chasecam and follow mode
	int			wins, losses;		// tournament stats (OSP: the high 16 bits hold armor taken / health taken, Pickup_Armor / Pickup_Health)
	qboolean	teamLeader;			// true when this client is a team leader

	// OSP 1.03a additions
	// G_WriteClientSessionData /
	// G_ReadSessionData store most of them in the "session%i" cvar.
	int			captain;		// OSP: team captain (first player of a team, ClientBegin; OSP_SetCaptain picks a new one when he leaves, ClientDisconnect)
	int			ospClient;		// OSP: "osp_client" userinfo value, 0 for a non-OSP cgame (ClientUserinfoChanged)
	char		ip[32];				// OSP: client address without the port, "SERVER_BOT" / "UNKNOWN" (ClientConnect); written as ":%s" in the session cvar
	int			roundTeam;		// OSP: team of a dead / waiting player in round based modes (skin and "rt" in ClientUserinfoChanged, OSP_CheckTeamJoin)
	int			referee;		// OSP: referee (ClientDisconnect resumes the match he paused)
	int			welcomeShown;		// OSP: welcome message sent (ClientBegin)
	int			specInvite;		// OSP: spec-lock invitations (bit 1 red, bit 2 blue; SpectatorThink lets invited spectators watch a spec-locked team)
	ospWeaponStats_t	weaponStats[WP_NUM_WEAPONS];	// per-weapon stats, 16 bytes each (OSP_Cmd_TopShots_f, OSP_PrintWeaponStats, TossClientItems)
	int			coachTeam;		// team a coach coaches (OSP_Cmd_Coach_f; coaches follow only that team, Cmd_Follow_f; "st" = coachTeam + 8 in ClientUserinfoChanged)
	int			damageGiven;		// (OSP_DamageStats)
	int			damageReceived;	// OSP_DamageStats
	int			deaths;			// (player_die)
	int			nameTeam;		// team given to the player's name by player_file (OSP_PlayerNameTeam); team / follow commands are refused while set (Cmd_Team_f)
	int			kills;			// player_die
	float		suicidePenalty;	// +0.5 per suicide, compared with match_maxsuicides (player_die, Cmd_Kill_f)
	int			specOnly;		// "speconly" (OSP_Cmd_SpecOnly_f); OSP_CheckClientFilters keeps the player a spectator
	int			suicides;		// player_die
	int			megaHealthPickups;	// (Pickup_Health)
	int			greenArmorPickups;	// Pickup_Armor, quantity 25
	int			redArmorPickups;	// Pickup_Armor, quantity 100
	int			yellowArmorPickups;	// Pickup_Armor, quantity 50
	int			teamDamage;		// damage given to teammates (OSP_DamageStats)
	int			teamKills;		// player_die
} clientSession_t;

//
#define MAX_NETNAME			36
#define	MAX_VOTE_COUNT		3

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct {
	clientConnected_t	connected;	
	usercmd_t	cmd;				// we would lose angles if not persistant
	qboolean	localClient;		// true if "ip" info key is "localhost"
	qboolean	initialSpawn;		// the first spawn should be at a cool location
	qboolean	predictItemPickup;	// based on cg_predictItems userinfo
	qboolean	pmoveFixed;			//
	char		netname[MAX_NETNAME];
	int			maxHealth;			// for handicapping
	int			enterTime;			// level.time the client entered the game
	playerTeamState_t teamState;	// status in teamplay games
	int			voteCount;			// to prevent people from constantly calling votes
	int			teamVoteCount;		// to prevent people from constantly calling votes
	qboolean	teamInfo;			// send team overlay updates?

	// OSP 1.03a additions
	// osp_unk_<n>: unknown OSP fields at gclient_t offset n
	int			lastDamagedClient;	// client number + 1 of the last player this one damaged (G_Damage, OSP_ChatTokens)
	int			authRetries;		// failed / timed-out client auth replies (OSP_Cmd_ClientAuth_f drops after 3, OSP_CheckAuthTimeout after 2)
	int			authTime;			// client auth: 0 not asked, > 0 reply deadline (request time + 5 s), -1 passed or disabled
	int			authChallenge;		// level.time sent with "psohtua" (part of the auth secret)
	int			autoAction;			// the client's cg_autoAction bits (1 stats dump, 2 screenshot, 4 record, 8 team stats, 16 all stats)
	int			osp_unk_280;
	int			specDoorTime;		// next level.time a spectator may pass a door (Touch_DoorTrigger)
	int			coachInvite;		// team that invited the player to coach it, 0 = none (OSP_Cmd_CoachInvite_f sets it, OSP_Cmd_Coach_f accepts, OSP_Cmd_CoachDecline_f clears; kept by the multiview commands)
	int			caDamage;			// clan arena damage not yet turned into score (one point per 100, OSP_DamageStats)
	int			osp_unk_290;		// unknown, OSP field (freeze tag)
	int			followKiller;		// OSP: follow the killer of the followed player (SpectatorClientEndFrame)
	int			followPowerup;		// OSP: follow the flag carrier / a powerup holder when the followed player dies (SpectatorClientEndFrame)
	int			followTeam;		// team a spectator's follow is locked on ("follow red/blue", Cmd_Follow_f; OSP_FollowNext)
	int			followViewcam;		// the viewcam follows one player (cg_followviewcam; OSP_Cmd_Viewcam_f, SpectatorThink)
	int			hookTime;		// OSP: earliest time of the next offhand hook (hook_delaytime, ClientThink_real)
	int			invitedTeam;		// team that invited the player (OSP_Cmd_Invite_f, OSP_Cmd_Remove_f); may join it while locked (OSP_CheckTeamJoin)
	int			connectChecked;		// OSP: ClientEndFrame's first-frames checks are done (refused-client drop, coach / multiview restore, team joincode)
	int			followKillerTime;		// OSP: earliest time of the next followKiller switch (+7500 msec)
	int			lastDeathLocation;	// teamState.location at death (player_die)
	int			waitingForSpawn;		// OSP: ClientSpawn found no free spawn point; ClientThink_real retries (respawn: no teleport effect)
	int			osp_unk_2bc;		// unknown, OSP field (referee)
	int			mvCount;		// OSP: number of multiview windows (mvClients entries; ClientDisconnect, TeamplayInfoMessage, CheckTeamStatus); widely used
	int			mvActive;		// OSP: active multiview window ("viewactive %d", SpectatorThink)
	int			mvSavedCount;		// windows saved by "viewfollow" (follow the active window; viewfollow again restores mvSavedClients)
	int			mvSavedClients[12];	// mvClients saved by "viewfollow" (OSP_Cmd_ViewFollow_f)
	int			mvClients[12];		// OSP: clients shown in the multiview windows (TeamplayInfoMessage, OSP_MV_RemoveAll)
	gentity_t	*mvPortals[12];	// "misc_portal_surface" entity of each multiview window (OSP_MV_AddView, OSP_MV_UpdatePortal, OSP_MV_RemoveView)
	int			ready;				// player is ready (OSP_Cmd_Ready_f, OSP_CheckReady, OSP_NumReady)
	int			cmdDelayTime;		// earliest time of the next name change (ClientUserinfoChanged, OSP_CheckClientFilters) or delayed command: OSP_CmdFloodCheck +5 s, "timeout" +7.5 s, a referee's unpause +5 s
	int			timeouts;		// OSP: timeouts left (match_timeoutcount, ClientBegin in 1v1)
	int			snaps;				// client snaps, "uinfo" argument 2 (OSP_Cmd_UInfo_f; "Snaps" of OSP_Cmd_Players_f)
	int			maxPackets;			// client cl_maxpackets, "uinfo" argument 1 (OSP_Cmd_UInfo_f; "MaxPkts" of OSP_Cmd_Players_f)
	int			timeNudge;			// client cl_timenudge, "uinfo" argument 3 (OSP_Cmd_UInfo_f; "Nudge" of OSP_Cmd_Players_f)
	int			screenshotJPEG;		// client cg_useScreenShotJPEG (OSP_Cmd_AutoRecord)
	int			viewcamInfo;		// OSP: print vc_info lines (camera spot, origin, angles) when the viewcam changes (SpectatorThink)
	int			viewcamFree;		// manual camera cycling while players play ("vc_free"; SpectatorThink, OSP_Cmd_Viewcam_f)
	int			viewcamHoldTime;	// level.time + 1600 after OSP_ViewcamThink changed the camera (not read)
	vec3_t		viewcamAngles;		// view angles of the player-following viewcam (OSP_ViewcamCyclePlayer, OSP_ViewcamThink)
	int			viewcamInfoLast;		// OSP: viewcam of the last vc_info line
	int			osp_unk_394;
	int			viewcam;		// OSP: index into osp_viewcams (ClientSpawn: viewcam spectators spawn at its origin)
	int			snapshotCount;		// number printed by the next "snapshot" (OSP_Cmd_Snapshot_f)
	int			viewcamStatic;		// OSP: viewcam view STATIC (camera angles) instead of DYNAMIC (looking at the player), toggled with USE (SpectatorThink)
	int			votesCalled;	// votes called on this map, limited by vote_limit (Cmd_CallVote_f)
	int			resultShown;		// OSP: last round-based match result printed (ClientBegin)
	int			osp_unk_3ac;		// unknown, OSP field
} clientPersistant_t;


// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
	// ps MUST be the first element, because the server expects it
	playerState_t	ps;				// communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t	pers;
	clientSession_t		sess;

	qboolean	readyToExit;		// wishes to leave the intermission

	qboolean	noclip;

	int			lastCmdTime;		// level.time of last usercmd_t, for EF_CONNECTION
									// we can't just use pers.lastCommand.time, because
									// of the g_sycronousclients case
	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	vec3_t		oldOrigin;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation
	qboolean	damage_fromWorld;	// if true, don't use the damage_from vector

	int			accurateCount;		// for "impressive" reward sound

	int			accuracy_shots;		// total number of shots
	int			accuracy_hits;		// total number of hits

	//
	int			lastkilled_client;	// last client that this client killed
	int			lasthurt_client;	// last client that damaged this client
	int			lasthurt_mod;		// type of damage the client did

	// timers
	int			respawnTime;		// can respawn when time > this, force after g_forcerespwan
	int			inactivityTime;		// kick players when time > this
	qboolean	inactivityWarning;	// qtrue if the five seoond warning has been given
	int			rewardTime;			// clear the EF_AWARD_IMPRESSIVE, etc when time > this

	int			airOutTime;

	int			lastKillTime;		// for multiple kill rewards

	qboolean	fireHeld;			// used for hook
	gentity_t	*hook;				// grapple hook if out

	int			switchTeamTime;		// time the player switched teams
	char		*lastPickupName;	// pickup_name of the last item picked up (Touch_Item; read by OSP_ChatTokens)

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int			timeResidual;

#ifdef MISSIONPACK
	gentity_t	*persistantPowerup;
	int			portalID;
	int			ammoTimes[WP_NUM_WEAPONS];
	int			invulnerabilityTime;
#endif

	char		*areabits;

	// OSP 1.03a additions
	int			connectReason;		// OSP: why ClientConnect would have refused the client (filter result + 1, 1 GTV wait, 4 banned address, 5 password); ClientEndFrame drops him with the reason
	int			weaponCount[WP_NUM_WEAPONS];	// copies of each weapon held: 1 for spawn weapons (OSP_SetSpawnWeapons), +1 per pickup (Pickup_Weapon); "drop" decrements it and removes the weapon at 0 (OSP_Cmd_Drop_f)
};


//
// this structure is cleared as each map is entered
//
#define	MAX_SPAWN_VARS			64
#define	MAX_SPAWN_VARS_CHARS	2048	// OSP: 2048 (id: 4096), G_AddSpawnVarToken
#define	MAX_OSP_CUSTOMMODES		20		// OSP: custom modes of mode_customfile (OSP_LoadCustomModes)
#define	MAX_OSP_FILE_CHARS		16384	// OSP: buffer a whole text/config file is read into (maps, motd, restrict, ...)

typedef struct {
	struct gclient_s	*clients;		// [maxclients]

	struct gentity_s	*gentities;
	int			gentitySize;
	int			num_entities;		// current number, <= MAX_GENTITIES

	int			warmupTime;			// restart match at this time

	fileHandle_t	logFile;

	// store latched cvars here that we want to get at often
	int			maxclients;

	int			framenum;
	int			time;					// in msec
	int			previousTime;			// so movers can back up when blocked

	int			startTime;				// level.time the map was started

	int			teamScores[TEAM_NUM_TEAMS];
	int			lastTeamLocationTime;		// last time of client team location update

	qboolean	newSession;				// don't use any old session data, because
										// we changed gametype

	qboolean	restarted;				// waiting for a map_restart to fire

	int			numConnectedClients;
	int			numNonSpectatorClients;	// includes connecting clients
	int			numPlayingClients;		// connected, non-spectators
	int			sortedClients[MAX_CLIENTS];		// sorted by score
	int			sortedByName[MAX_CLIENTS];		// OSP: connected clients sorted by sanitized name (OSP_SortClients; team overlay, OSP_Cmd_Players_f)
	int			sortedByAccuracy[MAX_CLIENTS];	// sortedClients re-sorted by the accuracy with osp_shotsWeapon (OSP_Cmd_TopShots_f, qsort with OSP_SortShotsCompare)
	int			follow1, follow2;		// clientNums for auto-follow spectators

	int			snd_fry;				// sound index for standing in lava

	int			warmupModificationCount;	// for detecting if g_warmup is changed

	// voting state
	char		voteString[MAX_STRING_CHARS];
	char		voteDisplayString[MAX_STRING_CHARS];
	int			voteTime;				// level.time vote was called
	int			voteExecuteTime;		// time the vote is executed
	int			voteYes;
	int			voteNo;
	int			numVotingClients;		// set by CalculateRanks

	// team voting state
	char		teamVoteString[2][MAX_STRING_CHARS];
	int			teamVoteTime[2];		// level.time vote was called
	int			teamVoteYes[2];
	int			teamVoteNo[2];
	int			numteamVotingClients[2];// set by CalculateRanks

	// spawn variables
	qboolean	spawning;				// the G_Spawn*() functions are valid
	int			numSpawnVars;
	char		*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
	int			numSpawnVarChars;
	char		spawnVarChars[MAX_SPAWN_VARS_CHARS];

	// intermission state
	int			intermissionQueued;		// intermission was qualified, but
										// wait INTERMISSION_DELAY_TIME before
										// actually going there so the last
										// frag can be watched.  Disable future
										// kills during this delay
	int			intermissiontime;		// time the intermission was started
	int			osp_unk_1dac;			// unknown, OSP field
	char		*changemap;
	qboolean	readyToExit;			// at least one client wants to exit
	int			exitTime;
	vec3_t		intermission_origin;	// also used for spectator spawns
	vec3_t		intermission_angle;

	qboolean	locationLinked;			// target_locations get linked
	gentity_t	*locationHead;			// head of the location list
	int			bodyQueIndex;			// dead bodies
	gentity_t	*bodyQue[BODY_QUEUE_SIZE];
#ifdef MISSIONPACK
	int			portalSequence;
#endif

	// OSP 1.03a additions
	int			osp_unk_1e00;		// unknown, OSP field
	int			numFilters;		// entries in osp_filters (OSP_LoadFilters, OSP_CheckFilters, OSP_Ref_Ban, player_die)
	int			botMinPlayersUpdateTime;	// G_CheckMinimumPlayers: bot_minplayers is updated every 8 s
	int			botCvarUpdateTime;	// BotAIStartFrame: bot cvars are updated every 5 s
	int			numStdModes;		// standard modes enabled by server_availmodes (OSP_LoadCustomModes, OSP_ModeAvailable)
	int			numVoteModes;		// standard + custom modes available for voting (OSP_LoadCustomModes, OSP_ModeAvailable, OSP_Vote_Parse)
	char		customModeConfig[MAX_OSP_CUSTOMMODES][MAX_OSPATH];	// config file of custom mode osp_numModes + i ("exec" when it is played)
	char		modeList[MAX_STRING_CHARS];		// "   %d - %s" lines of the vote modes (OSP_PrintAvailableModes)
	char		customModeName[MAX_OSP_CUSTOMMODES][24];	// name of custom mode osp_numModes + i (OSP_ModeName)
	int			nextMode;		// mode + 1 to switch to, 0 none (set by OSP_Vote_Execute's mode vote and the start / default mode; applied by OSP_Vote_Mode, OSP_MapRestart, ExitLevel; BeginIntermission, CheckExitRules, CheckVote)
	int			engineCheck;		// 0 not checked yet, 1 engine older than 1.27l, 2 1.27l or newer (players hidden from the serverinfo, OSP_HidePlayersInServerinfo; team scores in Score_Red / Score_Blue)
	int			cvarUpdateTime;		// next periodic G_UpdateCvars, every 3 s (G_RunFrame)
	int			osp_unk_3804;		// unknown, OSP field (freeze tag round state)
	int			osp_unk_3808;		// unknown, OSP field
	int			mapReloadNeeded;	// OSP_MapRestart reloads the map ("vstr nextmap") instead of map_restart; set by G_UpdateCvars (g_password / sv_maxclients changes) and OSP_LoadModeMaplist
	int			clientChangeTime;		// OSP: level.time of the last ClientBegin / ClientDisconnect (and G_InitGame)
	char		map_config[32];	// "configmap" config of the next map (OSP_NextMap), executed by ExitLevel
	int			osp_unk_3834;		// unknown, OSP field
	int			overtimeCount;		// overtimes played (CheckExitRules, CheckTournament)
	int			overtimeMinutes;	// minutes added to the timelimit by overtimes (CheckExitRules)
	int			suddenDeath;		// sudden death announced (CheckExitRules)
	int			physicsMode;		// 1 PROMODE, 2 CQ3, 0 VQ3 (set by OSP_SetPhysicsMode from server_promode / server_cq3; the CPM_UpdateSettings mode)
	int			gtvClient;		// OSP: -1 while waiting for a GTV / QTV relay, then its client number + 100 (ClientConnect)
	int			gtvWaitTime;		// OSP: other clients are refused ("Waiting for GTV to Connect") until this time (ClientConnect)
	int			startMode;		// mode_start + 1 while the start mode is still to be loaded (OSP_PrintAvailableModes sets it, OSP_DefaultModeLoad loads it), 0 = none (OSP_InitGame, OSP_MapRestart)
	int			forceReady;		// every player counts as ready (set by OSP_Ref_AllReady, read by OSP_NumReady, cleared by OSP_StartCountdown)
	int			timeLeftInterval;	// period of the time-left score cvar update, 7900 (G_InitGame, CheckTournament)
	int			timeLeftTime;		// next time-left score cvar update (CheckTournament)
	int			cpmUsed;			// set once PROMODE / CQ3 physics ran on this map; from then on OSP_SetPhysicsMode sets g_weapon(team)respawn
} level_locals_t;


//
// g_spawn.c
//
qboolean	G_SpawnString( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out );
void		G_SpawnEntitiesFromString( void );
char *G_NewString( const char *string );

//
// g_cmds.c
//
void Cmd_Score_f (gentity_t *ent);
void StopFollowing( gentity_t *ent );
void BroadcastTeamChange( gclient_t *client, int oldTeam );
void SetTeam( gentity_t *ent, char *s );
void Cmd_FollowCycle_f( gentity_t *ent, int dir );
void SanitizeString( char *in, char *out, qboolean toLower );	// OSP: third argument; skips ^Xrrggbb colors
int ClientNumberFromString( gentity_t *to, char *s );
qboolean Cmd_CallVote_f( gentity_t *ent, qboolean fRefCommand );	// OSP: also called by OSP_Cmd_Ref_f (fRefCommand)

//
// g_items.c
//
void G_CheckTeamItems( void );
void G_RunItem( gentity_t *ent );
void RespawnItem( gentity_t *ent );
void OSP_ItemThink( gentity_t *ent );
void OSP_DelayedPickupThink( gentity_t *ent );

void UseHoldableItem( gentity_t *ent );
void PrecacheItem (gitem_t *it);
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle, gentity_t *dropper );	// OSP: dropper (OSP_Cmd_Drop_f), passed to LaunchItem
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity, gentity_t *dropper );	// OSP: dropper
void SetRespawn (gentity_t *ent, float delay);
void G_SpawnItem (gentity_t *ent, gitem_t *item);
void FinishSpawningItem( gentity_t *ent );
void Think_Weapon (gentity_t *ent);
int ArmorIndex (gentity_t *ent);
void	Add_Ammo (gentity_t *ent, int weapon, int count);
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace);

void ClearRegisteredItems( void );
void RegisterItem( gitem_t *item );
void SaveRegisteredItems( void );

//
// g_utils.c
//
int G_ModelIndex( char *name );
int		G_SoundIndex( char *name );
void	G_TeamCommand( team_t team, char *cmd );
void	G_KillBox (gentity_t *ent);
gentity_t *G_Find (gentity_t *from, size_t fieldofs, const char *match);
gentity_t *G_PickTarget (char *targetname);
void	G_UseTargets (gentity_t *ent, gentity_t *activator);
void	G_SetMovedir ( vec3_t angles, vec3_t movedir);

void	G_InitGentity( gentity_t *e );
gentity_t	*G_Spawn (void);
gentity_t *G_TempEntity( vec3_t origin, int event );
void	G_Sound( gentity_t *ent, int channel, int soundIndex );
void	G_FreeEntity( gentity_t *e );
qboolean	G_EntitiesFree( void );

void	G_TouchTriggers (gentity_t *ent);
void	G_TouchSolids (gentity_t *ent);

float	*tv (float x, float y, float z);
char	*vtos( const vec3_t v );

float vectoyaw( const vec3_t vec );

void G_AddPredictableEvent( gentity_t *ent, int event, int eventParm );
void G_AddEvent( gentity_t *ent, int event, int eventParm );
void G_SetOrigin( gentity_t *ent, vec3_t origin );
void AddRemap(const char *oldShader, const char *newShader, float timeOffset);
const char *BuildShaderStateConfig(void);

//
// g_combat.c
//
qboolean CanDamage (gentity_t *targ, vec3_t origin);
void G_Damage (gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod);
qboolean G_RadiusDamage (vec3_t origin, gentity_t *inflictor, float damage, float radius, gentity_t *ignore, int mod);	// OSP passes the missile, attacker = inflictor->parent
int G_InvulnerabilityEffect( gentity_t *targ, vec3_t dir, vec3_t point, vec3_t impactpoint, vec3_t bouncedir );
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void TossClientItems( gentity_t *self );
#ifdef MISSIONPACK
void TossClientPersistantPowerups( gentity_t *self );
#endif
void TossClientCubes( gentity_t *self );

// damage flags
#define DAMAGE_RADIUS				0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR				0x00000002	// armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK			0x00000004	// do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION		0x00000008  // armor, shields, invulnerability, and godmode have no effect
#ifdef MISSIONPACK
#define DAMAGE_NO_TEAM_PROTECTION	0x00000010  // armor, shields, invulnerability, and godmode have no effect
#endif

//
// g_missile.c
//
void G_RunMissile( gentity_t *ent );

gentity_t *fire_blaster (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_plasma (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir);
gentity_t *fire_bfg (gentity_t *self, vec3_t start, vec3_t dir);
gentity_t *fire_grapple (gentity_t *self, vec3_t start, vec3_t dir);
#ifdef MISSIONPACK
gentity_t *fire_nail( gentity_t *self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up );
gentity_t *fire_prox( gentity_t *self, vec3_t start, vec3_t aimdir );
#endif


//
// g_mover.c
//
void G_RunMover( gentity_t *ent );
void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace );

//
// g_trigger.c
//
void trigger_teleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace );


//
// g_misc.c
//
void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles );
#ifdef MISSIONPACK
void DropPortalSource( gentity_t *ent );
void DropPortalDestination( gentity_t *ent );
#endif


//
// g_weapon.c
//
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker );
void CalcMuzzlePoint ( gentity_t *ent, vec3_t forward_, vec3_t right_, vec3_t up_, vec3_t muzzlePoint );
void SnapVectorTowards( vec3_t v, vec3_t to );
qboolean CheckGauntletAttack( gentity_t *ent );
void Weapon_HookFree (gentity_t *ent);
void Weapon_GrapplingHook_Fire (gentity_t *ent);
void Weapon_HookThink (gentity_t *ent);


//
// g_client.c
//
team_t TeamCount( int ignoreClientNum, int team );
int TeamLeader( int team );
team_t PickTeam( int ignoreClientNum );
void SetClientViewAngle( gentity_t *ent, vec3_t angle );
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles );
void CopyToBodyQue( gentity_t *ent );
void respawn (gentity_t *ent);
void BeginIntermission (void);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
void InitBodyQue (void);
void ClientSpawn( gentity_t *ent );
void player_die (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void AddScore( gentity_t *ent, vec3_t origin, int score );
void CalculateRanks( int checkExit );	// OSP: nonzero = also run CheckExitRules (and the intermission scoreboard)
void OSP_ResetCvarDefaults( void );
qboolean SpotWouldTelefrag( gentity_t *spot );

//
// g_svcmds.c
//
qboolean	ConsoleCommand( void );
void G_ProcessIPBans(void);
qboolean G_FilterPacket (char *from);

//
// g_weapon.c
//
void FireWeapon( gentity_t *ent );
#ifdef MISSIONPACK
void G_StartKamikaze( gentity_t *ent );
#endif

//
// p_hud.c
//
void MoveClientToIntermission (gentity_t *client);
void G_SetStats (gentity_t *ent);
void DeathmatchScoreboardMessage (gentity_t *client);

//
// g_cmds.c
//

//
// g_pweapon.c
//


//
// g_main.c
//
void FindIntermissionPoint( void );
void SetLeader(int team, int client);
void G_RunThink (gentity_t *ent);
void QDECL G_LogPrintf( const char *fmt, ... );
void SendScoreboardMessageToAllClients( void );
void QDECL G_Printf( const char *fmt, ... );
void QDECL G_Error( const char *fmt, ... );

//
// g_client.c
//
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
void ClientBegin( int clientNum );
void ClientCommand( int clientNum );

//
// g_active.c
//
void ClientThink( int clientNum );
void ClientEndFrame( gentity_t *ent );
void G_RunClient( gentity_t *ent );

//
// g_team.c
//
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 );
void Team_CheckDroppedItem( gentity_t *dropped );
qboolean CheckObeliskAttack( gentity_t *obelisk, gentity_t *attacker );

//
// g_mem.c
//
void *G_Alloc( int size );
void G_InitMemory( void );
void Svcmd_GameMem_f( void );

//
// g_session.c
//
void G_ReadSessionData( gclient_t *client );
void G_InitSessionData( gclient_t *client, char *userinfo );

void G_InitWorldSession( void );
void G_WriteSessionData( void );

//
// g_arenas.c
//
void UpdateTournamentInfo( void );
void SpawnModelsOnVictoryPads( void );
void Svcmd_AbortPodium_f( void );

//
// g_bot.c
//
void G_InitBots( qboolean restart );
char *G_GetBotInfoByNumber( int num );
char *G_GetBotInfoByName( const char *name );
void G_CheckBotSpawn( void );
void G_RemoveQueuedBotBegin( int clientNum );
qboolean G_BotConnect( int clientNum, qboolean restart );
void Svcmd_AddBot_f( void );
void Svcmd_BotList_f( void );
void BotInterbreedEndMatch( void );

// ai_main.c
#define MAX_FILEPATH			144

//bot settings
typedef struct bot_settings_s
{
	char characterfile[MAX_FILEPATH];
	int skill;		// OSP: int (id: float)
	char team[MAX_FILEPATH];
} bot_settings_t;

int BotAISetup( int restart );
char *BotMapTitle( void );	// ai_chat.c
int BotAIShutdown( int restart );
int BotAILoadMap( int restart );
int BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart);
int BotAIShutdownClient( int client, qboolean restart );
int BotAIStartFrame( int time );
void BotTestAAS(vec3_t origin);

#include "g_team.h" // teamplay specific stuff


extern	level_locals_t	level;
extern	gentity_t		g_entities[MAX_GENTITIES];

#define	FOFS(x) ((size_t)&(((gentity_t *)0)->x))

// OSP order. lcc lays out a file's uninitialised globals in reverse
// order of first declaration, so this list puts the vmCvar_ts
// of each file at the top of its bss in ascending address order.
// g_main.c
extern	vmCvar_t	g_gametype;
extern	vmCvar_t	g_dedicated;
extern	vmCvar_t	g_cheats;
extern	vmCvar_t	g_maxclients;			// allow this many total, including spectators
extern	vmCvar_t	g_maxGameClients;		// allow this many active
extern	vmCvar_t	g_restarted;
extern	vmCvar_t	g_allowVote;
extern	vmCvar_t	g_banIPs;
extern	vmCvar_t	g_blood;
extern	vmCvar_t	g_capturelimit;
extern	vmCvar_t	g_debugAlloc;
extern	vmCvar_t	g_debugDamage;
extern	vmCvar_t	g_debugMove;
extern	vmCvar_t	g_dmflags;
extern	vmCvar_t	g_doWarmup;
extern	vmCvar_t	g_filterBan;
extern	vmCvar_t	g_forcerespawn;
extern	vmCvar_t	g_fraglimit;
extern	vmCvar_t	g_friendlyFire;
extern	vmCvar_t	g_gravity;
extern	vmCvar_t	g_knockback;
extern	vmCvar_t	g_inactivity;
extern	vmCvar_t	g_motd;
extern	vmCvar_t	g_needpass;
extern	vmCvar_t	g_password;
extern	vmCvar_t	g_quadfactor;
extern	vmCvar_t	g_spectatorInactivity;
extern	vmCvar_t	g_speed;
extern	vmCvar_t	g_synchronousClients;
extern	vmCvar_t	g_teamAutoJoin;
extern	vmCvar_t	g_teamForceBalance;
extern	vmCvar_t	g_timelimit;
extern	vmCvar_t	g_warmup;
extern	vmCvar_t	g_weaponRespawn;
extern	vmCvar_t	g_weaponTeamRespawn;
extern	vmCvar_t	g_smoothClients;
extern	vmCvar_t	pmove_fixed;
extern	vmCvar_t	pmove_msec;
extern	vmCvar_t	g_rankings;
// g_osp_admin.c
extern	vmCvar_t	admin_log;
// g_osp_bot.c
extern	vmCvar_t	bot_autoload;
// g_osp_config.c
extern	vmCvar_t	z_m_current;
extern	vmCvar_t	mode_customfile;
extern	vmCvar_t	z_m_default;
extern	vmCvar_t	mode_start;
extern	vmCvar_t	server_availmodes;
// g_osp_customgfx.c
extern	vmCvar_t	server_gfxspecfile;
// g_osp_decals.c
extern	vmCvar_t	decal_directory;
// g_osp_main.c
extern	vmCvar_t	armor_q2style;
extern	vmCvar_t	bot_checktime;
extern	vmCvar_t	default_change;
extern	vmCvar_t	default_settings;
extern	vmCvar_t	freezetag_latejoin;
extern	vmCvar_t	freezetag_respawn;
extern	vmCvar_t	gauntlet_damage;
extern	vmCvar_t	gauntlet_disable;
extern	vmCvar_t	hook_delaytime;
extern	vmCvar_t	hook_enable;
extern	vmCvar_t	hook_holdtime;
extern	vmCvar_t	hook_sky;
extern	vmCvar_t	hook_speed;
extern	vmCvar_t	hook_speedpull;
extern	vmCvar_t	instagib_reload;
extern	vmCvar_t	item_disable;
extern	vmCvar_t	log_default;
extern	vmCvar_t	log_stat;
extern	vmCvar_t	match_carespawn;
extern	vmCvar_t	match_cawarmupfire;
extern	vmCvar_t	match_dropitems;
extern	vmCvar_t	match_hurtself;
extern	vmCvar_t	match_instagib;
extern	vmCvar_t	match_latejoin;
extern	vmCvar_t	match_maxsuicides;
extern	vmCvar_t	match_overtime;
extern	vmCvar_t	match_overtimecount;
extern	vmCvar_t	match_overtimelength;
extern	vmCvar_t	match_poweruprespawn;
extern	vmCvar_t	match_mutespecs;
extern	vmCvar_t	match_roundtimer;
extern	vmCvar_t	match_timeoutlength;
extern	vmCvar_t	match_timeoutcount;
extern	vmCvar_t	match_winbyscore;
extern	vmCvar_t	mg_damage;
extern	vmCvar_t	mg_damageTeamplay;
extern	vmCvar_t	osp_restart;
extern	vmCvar_t	powerup_disable;
extern	vmCvar_t	railgun_damage;
extern	vmCvar_t	roundlimit;
extern	vmCvar_t	replace_machinegun;
extern	vmCvar_t	replace_shotgun;
extern	vmCvar_t	replace_glauncher;
extern	vmCvar_t	replace_rlauncher;
extern	vmCvar_t	replace_lightning;
extern	vmCvar_t	replace_railgun;
extern	vmCvar_t	replace_plasmagun;
extern	vmCvar_t	replace_bfg;
extern	vmCvar_t	replace_invis;
extern	vmCvar_t	replace_quad;
extern	vmCvar_t	replace_battlesuit;
extern	vmCvar_t	replace_haste;
extern	vmCvar_t	replace_regen;
extern	vmCvar_t	replace_flight;
extern	vmCvar_t	Score_Time;
extern	vmCvar_t	server_customclient;
extern	vmCvar_t	server_batcher;
extern	vmCvar_t	server_cq3;
extern	vmCvar_t	server_fastrail;
extern	vmCvar_t	server_freezetag;
extern	vmCvar_t	server_lgcooldown;
extern	vmCvar_t	server_maxpacketsmin;
extern	vmCvar_t	server_maxpacketsmax;
extern	vmCvar_t	server_ospauth;
extern	vmCvar_t	server_promode;
extern	vmCvar_t	server_realping;
extern	vmCvar_t	server_record;
extern	vmCvar_t	server_thrufloors;
extern	vmCvar_t	server_timenudgemin;
extern	vmCvar_t	server_timenudgemax;
extern	vmCvar_t	start_armor;
extern	vmCvar_t	start_health;
extern	vmCvar_t	start_bullets;
extern	vmCvar_t	start_shells;
extern	vmCvar_t	start_rockets;
extern	vmCvar_t	start_grenades;
extern	vmCvar_t	start_lightning;
extern	vmCvar_t	start_slugs;
extern	vmCvar_t	start_cells;
extern	vmCvar_t	start_bfg;
extern	vmCvar_t	warmup_armor;
extern	vmCvar_t	weapon_deaddrop;
extern	vmCvar_t	weapon_have;
extern	vmCvar_t	weapon_start;
// OSP: non-cvar globals of this region (descending; see g_osp_maps.c below)
extern	int			osp_voteType;		// index of the vote being called (Cmd_CallVote_f / OSP vote handlers; CheckVote: 21 no "Referee changed" print, 27 resets level.nextMode)
extern	int			osp_mapWeapons[9];	// [weapon - 1] the weapon's items spawn on this map (OSP_ItemAllowed); warmup spawns get them (OSP_SetSpawnWeapons)
// g_osp_maps.c
// OSP: the map list (maps.txt / map_autoload / mapsession<n> cvars), above this
// file's cvars
#define	MAX_OSP_MAPS	128
typedef struct {
	int		minPlayers;		// the map is picked only with minPlayers..maxPlayers connected clients
	int		maxPlayers;		// -32145 marks a "deletemap" entry (OSP_ParseMapEntry)
	int		fraglimit;		// < 0 keeps the server setting
	int		capturelimit;
	int		timelimit;
	int		roundlimit;
	int		visited;		// already played in this cycle (map_once)
	char	config[32];		// "configmap" config executed instead of the limits
	char	name[MAX_QPATH];
} ospMap_t;
extern	ospMap_t	osp_maps[MAX_OSP_MAPS];
extern	vmCvar_t	map_autoload;
extern	vmCvar_t	map_debug;
extern	vmCvar_t	map_file;
extern	vmCvar_t	map_halt;
extern	vmCvar_t	map_nocount;
extern	vmCvar_t	map_once;
extern	vmCvar_t	map_queue;
extern	vmCvar_t	map_random;
extern	vmCvar_t	map_rotate;
extern	vmCvar_t	mapsessionauto;
extern	vmCvar_t	mapsessioncount;
// OSP: non-cvar OSP globals of this region, in descending address order (lcc: reverse
// order of first declaration); g_osp_maps.c declares the rest.
extern	int			osp_nextMapIndex;	// map list index chosen by OSP_SetMapIndex (vote / referee), used by OSP_NextMap when osp_mapIndexSet
extern	char		osp_mapFileList[0x10000];	// maps/*.bsp file list of map_autoload (OSP_LoadMaplist, OSP_LoadMapSession)
extern	char		osp_voteMapFileList[0x10000];	// maps/*.bsp file list of OSP_FindMapFile (callvote map)
extern	int			osp_roundsPlayed;		// rounds played ("Round %d/%d" prints it + 1); round-based modes, saved in "ca_session" with the team scores (G_InitWorldSession, G_WriteSessionData, OSP_CheckReady)
extern	int			osp_matchPaused;	// nonzero while a timeout/pause runs (G_RunMover, OSP_Cmd_Timeout_f, OSP_Ref_Pause, ...)
extern	int			osp_unpausing;		// the "Match resuming in N seconds" countdown runs (set by the unpause of OSP_Cmd_Timeout_f / OSP_Ref_Pause, cleared by OSP_UnpauseComplete)
extern	int			osp_timeoutEndTime;		// end of the running timeout, < 0 = paused by a referee (CheckTournament); cleared with the pause when the pausing referee disconnects (ClientDisconnect)
extern	int			osp_matchTime;		// level time without the paused time (G_RunFrame: levelTime - osp_pausedTime); time base for respawns (player_die: respawnTime = it + 1700), G_RunFrame, freeze tag
extern	char		osp_matchResultPrint[120];	// "print" line of the last round-based match result (G_InitWorldSession, ClientBegin, OSP_ClientWelcome)
extern	char		osp_matchResultCP[120];		// "cp" line of the same (G_InitWorldSession, OSP_ClientWelcome, OSP_RoundStart)
// g_osp_motd.c
extern	vmCvar_t	server_motdfile;
extern	vmCvar_t	server_motd1;
extern	vmCvar_t	server_motd2;
extern	vmCvar_t	server_motd3;
extern	vmCvar_t	server_motd4;
extern	vmCvar_t	server_motd5;
extern	vmCvar_t	server_motd6;
extern	vmCvar_t	server_motd7;
extern	vmCvar_t	server_motd8;
// g_osp_referee.c
extern	vmCvar_t	ref_password;
// g_osp_restrict.c
extern	vmCvar_t	filter_file;
extern	vmCvar_t	player_file;
extern	vmCvar_t	player_always;
// OSP: filters.txt entries (bans), right below the cvars
typedef struct {
	int		osp_unk_0;
	int		type;			// 1 NAME, 2 TAG, 3 ADDR, 4 ADDRBAN (automatic ban for suicides, player_die), 5 PASS (OSP_Svcmd_BanList_f)
	char	name[MAX_NETNAME];
	char	ip[16];
	char	password[16];	// "none" for automatic bans (player_die); "pass=" of the ban list
} ospFilter_t;
#define	MAX_OSP_FILTERS	450
extern	ospFilter_t	osp_filters[MAX_OSP_FILTERS];
// g_osp_team.c
extern	vmCvar_t	match_readypercent;
extern	vmCvar_t	match_minplayers;
extern	vmCvar_t	team_allcaptain;
extern	vmCvar_t	team_armordamage;
extern	vmCvar_t	team_maxplayers;
extern	vmCvar_t	team_nocontrols;
extern	vmCvar_t	team_noteamready;
// OSP: the rest of this file's bss is one per-team table (TEAM_FREE, TEAM_RED,
// TEAM_BLUE; 0x30 bytes each).
typedef struct {
	char	joincode[8];	// "Team joincode is ..." (ClientEndFrame)
	int		specLocked;		// spectators can't follow the team (G_InitWorldSession, G_WriteSessionData, SetTeam, SpectatorThink, OSP_Cmd_SpecLock_f)
	int		locked;			// "The %s team is locked" (OSP_CheckTeamJoin)
	char	teamName[24];	// team name ("^1%s^7 team", BroadcastTeamChange)
	int		osp_unk_28;
	int		timeouts;		// set to match_timeoutcount
} ospTeamInfo_t;
extern	ospTeamInfo_t	osp_teamInfo[3];
// g_osp_viewcam.c
extern	vmCvar_t	viewcam_directory;
// OSP: the viewcam table (memset with 0x4c00 bytes by
// OSP_LoadViewcam), declared after the cvar so that it lands below it
// OSP: fields named from g_osp_viewcam.c (OSP_LoadViewcam, OSP_ViewcamUpdate)
typedef struct {
	int		distSum;		// sum of the distances of the players the camera sees (OSP_ViewcamUpdate; OSP_ViewcamRunFrame prefers the lowest average)
	int		id;				// first number of the viewcam file line (vc_info, vc_proximity)
	int		activity;		// players seen + 700 per quad carrier + 1000 per flag carrier (OSP_ViewcamUpdate's result)
	int		closestClient;	// closest player seen
	int		target;			// client the camera looks at, -1 none
	int		range;			// view range, 450 unless the file line gives more than 50
	float	closestDist;	// distance of closestClient
	vec3_t	viewAngles;		// dynamic view angles (looking at target)
	vec3_t	closestAngles;	// angles to closestClient
	vec3_t	origin;			// spot (spawn point of viewcam spectators, ClientSpawn)
	vec3_t	angles;			// static view angles (pitch, yaw from the file)
} ospViewcam_t;
#define	MAX_OSP_VIEWCAMS	256
#define	MAX_OSP_SNAPSHOTS	256
extern	ospViewcam_t	osp_viewcams[MAX_OSP_VIEWCAMS];	// indexed by pers.viewcam
// g_osp_vote.c
extern	char		osp_voteValue[256];	// argument of the vote being called
extern	vmCvar_t	vote_allow_allcaptain;
extern	vmCvar_t	vote_allow_altgraphics;
extern	vmCvar_t	vote_allow_armordamage;
extern	vmCvar_t	vote_allow_capfps;
extern	vmCvar_t	vote_allow_capturelimit;
extern	vmCvar_t	vote_allow_ca_startrespawn;
extern	vmCvar_t	vote_allow_ca_warmupfire;
extern	vmCvar_t	vote_allow_cq3;
extern	vmCvar_t	vote_allow_dropitems;
extern	vmCvar_t	vote_allow_falling;
extern	vmCvar_t	vote_allow_fastrail;
extern	vmCvar_t	vote_allow_fraglimit;
extern	vmCvar_t	vote_allow_hook;
extern	vmCvar_t	vote_allow_instagib;
extern	vmCvar_t	vote_allow_items;
extern	vmCvar_t	vote_allow_kick;
extern	vmCvar_t	vote_allow_lgcooldown;
extern	vmCvar_t	vote_allow_map;
extern	vmCvar_t	vote_allow_maxpackets;
extern	vmCvar_t	vote_allow_machinegun;
extern	vmCvar_t	vote_allow_mutespecs;
extern	vmCvar_t	vote_allow_notimer;
extern	vmCvar_t	vote_allow_osphud;
extern	vmCvar_t	vote_allow_overtime;
extern	vmCvar_t	vote_allow_pmove;
extern	vmCvar_t	vote_allow_poweruprespawn;
extern	vmCvar_t	vote_allow_powerups;
extern	vmCvar_t	vote_allow_promode;
extern	vmCvar_t	vote_allow_referee;
extern	vmCvar_t	vote_allow_reload;
extern	vmCvar_t	vote_allow_respawn;
extern	vmCvar_t	vote_allow_roundlimit;
extern	vmCvar_t	vote_allow_teamdamage;
extern	vmCvar_t	vote_allow_thrufloors;
extern	vmCvar_t	vote_allow_timelimit;
extern	vmCvar_t	vote_allow_timenudge;
extern	vmCvar_t	vote_allow_weaponrespawn;
extern	vmCvar_t	vote_limit;
extern	vmCvar_t	vote_percent;
// OSP: non-cvar globals of this region (descending; see g_osp_maps.c above)
extern	char		osp_voteMap[];	// map of a "mode" vote
extern	int			osp_numViewcams;	// viewcam spots loaded (OSP_LoadViewcam; G_ReadSessionData, SpectatorThink, ...)
extern	int			osp_viewcamCurrent;	// the action camera shown to viewcam spectators (state 4) while players play, -1 none (OSP_ViewcamRunFrame; SpectatorThink)
extern	int			osp_viewcamLast;	// previous action camera (OSP_ViewcamRunFrame, never read)
extern	int			osp_viewcamRandomSpot;	// camera spot of the intermission view (OSP_ViewcamRandomSpot)
extern	int			osp_viewcamRandomTime;	// OSP: next intermission viewcam change (OSP_ViewcamRandomSpot sets level.time + 8000; CheckIntermissionExit)
extern	int			osp_numViewcamClients;	// spectators on the action camera (state 4): SpectatorThink counts them, G_RunFrame runs OSP_ViewcamRunFrame while nonzero
// id's, not in OSP 1.03a (MISSIONPACK)
extern	vmCvar_t	g_obeliskHealth;
extern	vmCvar_t	g_obeliskRegenPeriod;
extern	vmCvar_t	g_obeliskRegenAmount;
extern	vmCvar_t	g_obeliskRespawnDelay;
extern	vmCvar_t	g_cubeTimeout;
extern	vmCvar_t	g_redteam;
extern	vmCvar_t	g_blueteam;
extern	vmCvar_t	g_enableDust;
extern	vmCvar_t	g_enableBreath;
extern	vmCvar_t	g_singlePlayer;
extern	vmCvar_t	g_proxMineTimeout;

void	trap_Printf( const char *fmt );
void	trap_Error( const char *fmt );
int		trap_Milliseconds( void );
int		trap_Argc( void );
void	trap_Argv( int n, char *buffer, int bufferLength );
void	trap_Args( char *buffer, int bufferLength );
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );
int		trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
int		trap_FS_Seek( fileHandle_t f, long offset, int origin ); // fsOrigin_t
void	trap_SendConsoleCommand( int exec_when, const char *text );
void	trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void	trap_Cvar_Update( vmCvar_t *cvar );
void	trap_Cvar_Set( const char *var_name, const char *value );
int		trap_Cvar_VariableIntegerValue( const char *var_name );
float	trap_Cvar_VariableValue( const char *var_name );
void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void	trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *gameClients, int sizeofGameClient );
void	trap_DropClient( int clientNum, const char *reason );
void	trap_SendServerCommand( int clientNum, const char *text );
void	trap_SetConfigstring( int num, const char *string );
void	trap_GetConfigstring( int num, char *buffer, int bufferSize );
void	trap_GetUserinfo( int num, char *buffer, int bufferSize );
void	trap_SetUserinfo( int num, const char *buffer );
void	trap_GetServerinfo( char *buffer, int bufferSize );
int		trap_RealTime( qtime_t *qtime );
void	trap_SetBrushModel( gentity_t *ent, const char *name );
void	trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
int		trap_PointContents( const vec3_t point, int passEntityNum );
qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 );
qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
void	trap_AdjustAreaPortalState( gentity_t *ent, qboolean open );
qboolean trap_AreasConnected( int area1, int area2 );
void	trap_LinkEntity( gentity_t *ent );
void	trap_UnlinkEntity( gentity_t *ent );
int		trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
int		trap_BotAllocateClient( void );
void	trap_BotFreeClient( int clientNum );
void	trap_GetUsercmd( int clientNum, usercmd_t *cmd );
qboolean	trap_GetEntityToken( char *buffer, int bufferSize );

int		trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points);
void	trap_DebugPolygonDelete(int id);

int		trap_BotLibSetup( void );
int		trap_BotLibShutdown( void );
int		trap_BotLibVarSet(char *var_name, char *value);
int		trap_BotLibVarGet(char *var_name, char *value, int size);
int		trap_BotLibDefine(char *string);
int		trap_BotLibStartFrame(float time);
int		trap_BotLibLoadMap(const char *mapname);
int		trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue);
int		trap_BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);

int		trap_BotGetSnapshotEntity( int clientNum, int sequence );
int		trap_BotGetServerCommand(int clientNum, char *message, int size);
void	trap_BotUserCommand(int client, usercmd_t *ucmd);

int		trap_AAS_BBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas);
int		trap_AAS_AreaInfo( int areanum, void /* struct aas_areainfo_s */ *info );
void	trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info);

int		trap_AAS_Initialized(void);
void	trap_AAS_PresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs);
float	trap_AAS_Time(void);

int		trap_AAS_PointAreaNum(vec3_t point);
int		trap_AAS_PointReachabilityAreaIndex(vec3_t point);
int		trap_AAS_TraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas);

int		trap_AAS_PointContents(vec3_t point);
int		trap_AAS_NextBSPEntity(int ent);
int		trap_AAS_ValueForBSPEpairKey(int ent, char *key, char *value, int size);
int		trap_AAS_VectorForBSPEpairKey(int ent, char *key, vec3_t v);
int		trap_AAS_FloatForBSPEpairKey(int ent, char *key, float *value);
int		trap_AAS_IntForBSPEpairKey(int ent, char *key, int *value);

int		trap_AAS_AreaReachability(int areanum);

int		trap_AAS_AreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags);
int		trap_AAS_EnableRoutingArea( int areanum, int enable );
int		trap_AAS_PredictRoute(void /*struct aas_predictroute_s*/ *route, int areanum, vec3_t origin,
							int goalareanum, int travelflags, int maxareas, int maxtime,
							int stopevent, int stopcontents, int stoptfl, int stopareanum);

int		trap_AAS_AlternativeRouteGoals(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags,
										void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals,
										int type);
int		trap_AAS_Swimming(vec3_t origin);
int		trap_AAS_PredictClientMovement(void /* aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize);


void	trap_EA_Say(int client, char *str);
void	trap_EA_SayTeam(int client, char *str);
void	trap_EA_Command(int client, char *command);

void	trap_EA_Action(int client, int action);
void	trap_EA_Gesture(int client);
void	trap_EA_Talk(int client);
void	trap_EA_Attack(int client);
void	trap_EA_Use(int client);
void	trap_EA_Respawn(int client);
void	trap_EA_Crouch(int client);
void	trap_EA_MoveUp(int client);
void	trap_EA_MoveDown(int client);
void	trap_EA_MoveForward(int client);
void	trap_EA_MoveBack(int client);
void	trap_EA_MoveLeft(int client);
void	trap_EA_MoveRight(int client);
void	trap_EA_SelectWeapon(int client, int weapon);
void	trap_EA_Jump(int client);
void	trap_EA_DelayedJump(int client);
void	trap_EA_Move(int client, vec3_t dir, float speed);
void	trap_EA_View(int client, vec3_t viewangles);

void	trap_EA_EndRegular(int client, float thinktime);
void	trap_EA_GetInput(int client, float thinktime, void /* struct bot_input_s */ *input);
void	trap_EA_ResetInput(int client);


int		trap_BotLoadCharacter(char *charfile, float skill);
void	trap_BotFreeCharacter(int character);
float	trap_Characteristic_Float(int character, int index);
float	trap_Characteristic_BFloat(int character, int index, float min, float max);
int		trap_Characteristic_Integer(int character, int index);
int		trap_Characteristic_BInteger(int character, int index, int min, int max);
void	trap_Characteristic_String(int character, int index, char *buf, int size);

int		trap_BotAllocChatState(void);
void	trap_BotFreeChatState(int handle);
void	trap_BotQueueConsoleMessage(int chatstate, int type, char *message);
void	trap_BotRemoveConsoleMessage(int chatstate, int handle);
int		trap_BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm);
int		trap_BotNumConsoleMessages(int chatstate);
void	trap_BotInitialChat(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int		trap_BotNumInitialChats(int chatstate, char *type);
int		trap_BotReplyChat(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int		trap_BotChatLength(int chatstate);
void	trap_BotEnterChat(int chatstate, int client, int sendto);
void	trap_BotGetChatMessage(int chatstate, char *buf, int size);
int		trap_StringContains(char *str1, char *str2, int casesensitive);
int		trap_BotFindMatch(char *str, void /* struct bot_match_s */ *match, unsigned long int context);
void	trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size);
void	trap_UnifyWhiteSpaces(char *string);
void	trap_BotReplaceSynonyms(char *string, unsigned long int context);
int		trap_BotLoadChatFile(int chatstate, char *chatfile, char *chatname);
void	trap_BotSetChatGender(int chatstate, int gender);
void	trap_BotSetChatName(int chatstate, char *name, int client);
void	trap_BotResetGoalState(int goalstate);
void	trap_BotRemoveFromAvoidGoals(int goalstate, int number);
void	trap_BotResetAvoidGoals(int goalstate);
void	trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal);
void	trap_BotPopGoal(int goalstate);
void	trap_BotEmptyGoalStack(int goalstate);
void	trap_BotDumpAvoidGoals(int goalstate);
void	trap_BotDumpGoalStack(int goalstate);
void	trap_BotGoalName(int number, char *name, int size);
int		trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		trap_BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory, int travelflags);
int		trap_BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime);
int		trap_BotTouchingGoal(vec3_t origin, void /* struct bot_goal_s */ *goal);
int		trap_BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal);
int		trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal);
int		trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal);
int		trap_BotGetLevelItemGoal(int index, char *classname, void /* struct bot_goal_s */ *goal);
float	trap_BotAvoidGoalTime(int goalstate, int number);
void	trap_BotSetAvoidGoalTime(int goalstate, int number, float avoidtime);
void	trap_BotInitLevelItems(void);
void	trap_BotUpdateEntityItems(void);
int		trap_BotLoadItemWeights(int goalstate, char *filename);
void	trap_BotFreeItemWeights(int goalstate);
void	trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child);
void	trap_BotSaveGoalFuzzyLogic(int goalstate, char *filename);
void	trap_BotMutateGoalFuzzyLogic(int goalstate, float range);
int		trap_BotAllocGoalState(int state);
void	trap_BotFreeGoalState(int handle);

void	trap_BotResetMoveState(int movestate);
void	trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags);
int		trap_BotMoveInDirection(int movestate, vec3_t dir, float speed, int type);
void	trap_BotResetAvoidReach(int movestate);
void	trap_BotResetLastAvoidReach(int movestate);
int		trap_BotReachabilityArea(vec3_t origin, int testground);
int		trap_BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target);
int		trap_BotPredictVisiblePosition(vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target);
int		trap_BotAllocMoveState(void);
void	trap_BotFreeMoveState(int handle);
void	trap_BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove);
void	trap_BotAddAvoidSpot(int movestate, vec3_t origin, float radius, int type);

int		trap_BotChooseBestFightWeapon(int weaponstate, int *inventory);
void	trap_BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo);
int		trap_BotLoadWeaponWeights(int weaponstate, char *filename);
int		trap_BotAllocWeaponState(void);
void	trap_BotFreeWeaponState(int weaponstate);
void	trap_BotResetWeaponState(int weaponstate);

int		trap_GeneticParentsAndChildSelection(int numranks, float *ranks, int *parent1, int *parent2, int *child);

void	trap_SnapVector( float *v );


// OSP declarations used by the id code, with the argument names / types of the call sites.
// Only initialised (data) globals and functions are declared here: an extern
// of a bss global changes lcc's bss order in its defining file (see the cvar list above).

// OSP's freeze tag is team deathmatch with server_freezetag set (the
// expansion is visible in AddScore's code)
#define	IS_FREEZETAG	( g_gametype.integer == GT_TEAM && server_freezetag.integer )

// OSP's offhand grapple button (BUTTON_AFFIRMATIVE's bit; ClientThink_real)
#define	BUTTON_HOOK		32

// bg_promode.c (CPM / promode settings, initialised data)
extern	int		pm_armorPromode;
extern	float	armorKoeff[3];
extern	float	modeUnk_2744;	// 24 in VQ3 mode: knockback lift of radius damage on others (G_RadiusDamage)
extern	float	modeUnk_2748, modeUnused8, modeUnk_2784, modeUnk_2788, modeUnk_278c, modeUnk_279c;	// knockback scale per MOD (G_Damage)
extern	int		modeBackpacks;	// drop weapons with the player's ammo (TossClientItems)
extern	float	modeUnk_27b8;	// battlesuit damage scale (G_Damage)
extern	float	modeUnk_27dc;	// respawn delay, 1700 (player_die)
extern	float	modeSlimeDamage, modeLavaDamage;	// OSP: damage per waterlevel (P_WorldEffects; VQ3 10 / 30)
extern	int		modePainDebounce;	// OSP: pain sound interval (P_DamageFeedback; VQ3 700, PROMODE / CQ3 100)
extern	int		modeForceRespawn;	// OSP: g_forcerespawn of PROMODE / CQ3 (3 / 10; ClientThink_real uses it when level.physicsMode is set)
extern	int		modeUnk_27f4;	// knockback scale of MOD_SLIME / MOD_LAVA (G_Damage)
extern	int		modeRespawnPowerup, modeRespawnBattlesuit, modeRespawnPowerupCTF;	// powerup respawn times (Pickup_Powerup)
extern	float	modeAmmoBoxBullets;	// machinegun ammo box (Pickup_Ammo)
extern	int		modeAmmoBoxSlugs, modeRespawnAmmo, modeWeaponAmmoMG;	// railgun ammo box, ammo respawn time (Pickup_Ammo), machinegun weapon ammo (Pickup_Weapon)
extern	int		modeRespawnHealth, modeMegaHealthWait;	// health respawn time; mega health timer starts below 100 health (Pickup_Health)
extern	float	modeMGDamage, modeMGDamageCTF, modeMGDamageTeam;	// machinegun damage in PROMODE / CQ3 (FireWeapon)
extern	float	modeRocketSpeed, modePlasmaDamage;	// fire_rocket, fire_plasma
extern	int		modeNoThruFloors;	// splash never goes through floors, whatever server_thrufloors says (G_MissileImpact)
extern	float	modeShotgunDamage;	// damage per pellet (ShotgunPellet)
extern	float	modeShotgunKoeff;		// pellet spread (ShotgunPattern)
extern	int		modeShotgunNumberOfPellets;
extern	int		modeMaxAmmoShotgun, modeMaxAmmoGrenade, modeMaxAmmoRocket, modeMaxAmmoRail;	// ammo limits (Add_Ammo)

// g_osp_admin.c
extern	int		osp_adminLogDisabled;	// set when admin_log is "none" or can't be opened; G_UpdateCvars clears it
void	OSP_AdminLogPrintf( const char *text );	// one finished line (callers pass va() strings)

// g_osp_bot.c
extern	int		osp_botAutoloadDone;	// (G_CheckBotSpawn)
char	*OSP_BotNameFromHash( const char *name );	// "name" of g_botInfos[ OSP_BotNameHash( name, g_numBots ) ] (static buffer)
int		OSP_BotNameHash( const char *s, int mod );	// ( sum of the characters of s ) % mod
void	OSP_BotAutoload( void );

// g_osp_cmds.c
void	OSP_Cmd_Players_f( gentity_t *ent );
void	OSP_Cmd_Coach_f( gentity_t *ent, qboolean fHelp );	// "coach" 1; 0 (SpectatorThink, ClientEndFrame, SpectatorClientEndFrame): no "?" help, refreshes the coach view
char	*OSP_ChatTokens( gentity_t *ent, int color, const char *text );	// chat text with the #-tokens expanded (G_Say)
int		OSP_ClientCommand( gentity_t *ent, char *cmd, qboolean fRefCommand );	// nonzero when cmd was an OSP command (ClientCommand, OSP_Cmd_Ref_f); fRefCommand unused
int		OSP_CmdHelpCheck( gentity_t *ent, int cmdIndex );	// nonzero when "<cmd> ?" printed the help of command cmdIndex (ClientCommand)
void	OSP_Cmd_UInfo_f( gentity_t *ent );
void	OSP_Cmd_Stats_f( gentity_t *ent, qboolean fDump );	// "stats" 0, "statsdump" 1
void	OSP_Cmd_StatsAll_f( gentity_t *ent, int team );	// "statsall" 0, "statsred" TEAM_RED, "statsblue" TEAM_BLUE
void	OSP_Cmd_TopShots_f( gentity_t *ent, qboolean fBest );	// "topshots" 1, "bottomshots" 0
void	OSP_Cmd_GetStatsInfo_f( gentity_t *ent );
// OSP: signatures from OSP_ClientCommand's calls
void	OSP_Cmd_Help_f( gentity_t *ent );	// "help" / "commands" / "?"
int		OSP_CmdFloodCheck( gentity_t *ent );	// 0 (and "Wait another %.1fs ...") while the 5 s command delay runs, else 1 and a new delay
void	OSP_Cmd_SpecOnly_f( gentity_t *ent );
void	OSP_Cmd_Lock_f( gentity_t *ent, char *cmd );	// "lock" / "unlock"
void	OSP_Cmd_SpecLock_f( gentity_t *ent, char *cmd );	// "speclock" / "specunlock"
void	OSP_Cmd_Ready_f( gentity_t *ent, qboolean ready );	// "ready" 1, "notready" / "unready" 0
void	OSP_Cmd_Invite_f( gentity_t *ent );
void	OSP_Cmd_SpecInvite_f( gentity_t *ent );
void	OSP_Cmd_CoachInvite_f( gentity_t *ent );
void	OSP_Cmd_CoachDecline_f( gentity_t *ent );
void	OSP_Cmd_CoachKick_f( gentity_t *ent );
void	OSP_Cmd_Remove_f( gentity_t *ent );
void	OSP_Cmd_Resign_f( gentity_t *ent );
void	OSP_Cmd_TeamReady_f( gentity_t *ent );
void	OSP_Cmd_Captains_f( gentity_t *ent );
void	OSP_Cmd_Timeout_f( gentity_t *ent, qboolean fPause );	// "time" / "pause" / "timeout" 1, "timein" / "unpause" 0
void	OSP_Cmd_Joincode_f( gentity_t *ent );
void	OSP_Cmd_Drop_f( gentity_t *ent );
void	OSP_StopCountdown( void );	// g_osp_main.c (OSP: OSP_Cmd_Ready_f)

// g_osp_config.c
typedef struct {
	int		gametype;	// gametype the mode is played as (5 = clan arena)
	char	*name;
} ospMode_t;
extern	ospMode_t	osp_modes[];	// indexed by g_gametype
extern	int			osp_numModes;	// 8
char	*OSP_ModeName( int mode );
void	OSP_SetGametype( int mode );
void	OSP_LoadModeMaplist( int mode, char *map );	// OSP: mode's map settings and map list; nextmap = map (NULL: the current map) when it is in the list
int		OSP_ModeAvailable( int mode );
char	*OSP_CustomModeConfig( int mode );	// config of a loaded custom mode, "" otherwise	// nonzero when mode is enabled in server_availmodes or is a custom mode
char	*OSP_ReadConfigLine( char *buf, int size, char *line, int lineSize );	// OSP: copies the next non-comment line of buf (size bytes left) to line; returns the position after it, NULL at the end

// g_osp_freeze.c
int		is_spectator( gclient_t *client );
int		is_body( gentity_t *ent );
void	FT_BodyFree( gentity_t *ent );
void	FT_Thaw( gentity_t *ent );
int		is_body_freeze( gentity_t *ent );
int		FT_RespawnFrozen( gentity_t *ent );
int		FT_BodyKnockback( gentity_t *targ, gentity_t *attacker, vec3_t dir, int mod, int knockback );	// nonzero when targ is a frozen player hit by a client: his body is pushed instead (G_Damage); was FT_MapNeedsBodyFix
void	player_freeze( gentity_t *self, gentity_t *attacker, int mod );
void	FT_CheckRoundEnd( void );
void	FT_TeamWins( int team );	// team scores the freeze-tag round
void	FT_SpectatorEndFrameHelper( gentity_t *ent, gclient_t *cl );	// copies the followed player's state (SpectatorClientEndFrame)
int		FT_RespawnPlayer( gentity_t *ent );	// nonzero when the follow request was handled by freeze tag (Cmd_FollowCycle_f)

// g_osp_main.c
int		OSP_IsRoundBased( int gametype );
void	OSP_SetPhysicsMode( void );
void	OSP_SetInstagib( int changed );	// match_instagib (G_RegisterCvars 0, G_UpdateCvars 1)
void	OSP_HidePlayersInServerinfo( int hide );	// SVF_NOSERVERINFO on the clients while a match runs (1.27l+ engines)
void	OSP_UpdateScoreCvars( char *scoreTime );	// Score_Time (+ Score_Red / Score_Blue) serverinfo cvars ("EndOfMatch" ExitLevel, "Timelimit", "Win: <name>" CheckExitRules)
void	OSP_ExitLevelSession( void );
void	OSP_ExitLevelHelper( void );
void	OSP_InitGame( void );
void	OSP_InitItemNames( void );
void	OSP_SetRestart( void );
void	OSP_Respawn( gentity_t *ent );
void	OSP_DamageStats( gentity_t *targ, gentity_t *attacker, int damage, int mod );
void	OSP_ResetClientStats( int clientNum );
void	OSP_HideItems( int type, int tag );	// hides the items of giType type / giTag tag (IT_WEAPON includes the ammo)
void	OSP_ShowItems( int type, int tag );		// shows them again
void	OSP_ReplaceSpawnItem( char *classname );
int		OSP_ItemAllowed( gentity_t *ent );	// nonzero when the item may spawn (instagib, item_disable / powerup_disable)
void	OSP_FollowPowerupPickup( gentity_t *ent );
void	OSP_RegisterStartWeapons( void );
qboolean OSP_RadiusDamage( vec3_t origin, gentity_t *inflictor, float damage, float radius, gentity_t *ignore, int mod, vec3_t point );	// G_RadiusDamage measured from point (G_MissileImpact)
void	OSP_PrintWeaponStats( gentity_t *ent, gentity_t *player );	// player's weapon stats to ent ("xstats1" for OSP clients >= 1007)
void	OSP_LogWeaponStats( gentity_t *ent );
void	OSP_SortClients( void );
void	OSP_StartCountdown( void );
void	OSP_DelayPrintf( int type, int time, gentity_t *ent );	// prints message type (0 welcome, 1 last match result) to ent at time
void	OSP_ResetPlayerReady( void );
void	OSP_SetSpawnWeapons( gentity_t *ent );
void	OSP_SelectSpawnWeapon( gentity_t *ent );
int		OSP_PM_CanFire( void );	// nonzero when firing is allowed (clan arena warmup needs match_cawarmupfire; ClientEvents, PM_Weapon)
int		OSP_FollowHelper( gentity_t *ent, int clientNum, int team );	// nonzero when ent may not follow clientNum (SpectatorClientEndFrame)
int		OSP_ClientScore( gentity_t *ent );	// score flags column of the scoreboard (DeathmatchScoreboardMessage)
int		OSP_TeamPlayerCount( int team );	// number of connected clients playing or coaching for team (OSP_ClientTeam; SetTeam in round-based modes) - misnamed
int		OSP_FollowNext( gentity_t *ent, int dir, int team );	// next client ent may follow in direction dir (team: pers.followTeam), -1 if none (Cmd_FollowCycle_f)
int		OSP_ChatFilter( const char *text );	// 0 when the (normalized) text contains a filtered word (G_Say)
void	OSP_GlobalSound( char *sound );	// plays a sound for everybody (Cmd_CallVote_f)
int		OSP_MOTDHelper( char *src, int srcSize, char *dst, int dstSize, int maxVisible );	// copy with color codes not counted; returns the visible characters copied
void	OSP_BatcherInfo( gentity_t *ent );
void	OSP_Batcher( gentity_t *ent );
int		OSP_CheckEngineVersion( char *version );	// nonzero for a 1.27l+ server engine
void	OSP_StatsDump( gentity_t *ent );	// cg_autoAction 1: "condump stats/..." on the client
void	OSP_Cmd_AutoRecord( int clientNum, qboolean record, qboolean fClient );	// demo (record) or screenshot; "autoscreenshot": ( clientNum, 0, 1 )
char	*OSP_DemoName( int clientNum );
void	OSP_SendAuthRequest( gentity_t *ent );	// "psohtua <challenge>" when server_ospauth and no request is pending
void	OSP_CheckAuthTimeout( gentity_t *ent );	// ClientCommand, while pers.authTime > 0 and server_ospauth
void	OSP_PrintMatchResult( gentity_t *ent );	// "wins the match" / "leads" / "Match tied" (OSP_Scores_CA)
void	OSP_Cmd_ClientAuth_f( gentity_t *ent );	// "htuatneilc" ("clientauth" reversed)
// bg_auth.c (OSP: prototypes for OSP_Cmd_ClientAuth_f)
void	BG_OSPAuthInit( void );
void	BG_OSPAuthUpdateName( char *str );
void	BG_OSPAuthGetSecret( char *secret );

void	OSP_AutoScreenshot( void );
void	OSP_ClearClients( void );
void	OSP_AutoRecordStart( void );	// countdown started: server_record / cg_autoAction 4 demos
void	OSP_AutoRecordStop( void );
void	OSP_MatchEndStats( int scores, qboolean allowDump );	// end of match: weapon stats (scores 0) or the scoreboard (scores 1) to every client
void	OSP_RestoreDefaultSettings( void );	// default_change: restores default_settings (OSP_SaveDefaultSettings)
void	OSP_SaveDefaultSettings( void );	// stores the current settings in default_settings unless set
void	OSP_PrintMatchResult( gentity_t *ent );	// team scores to ent: "wins the match" / "leads" / "Match tied"
void	OSP_CA_RoundEnd( void );
void	OSP_BuzzerSound( void );
extern	int		osp_pausedTime;		// time spent paused (G_RunFrame)
extern	int		osp_matchState;		// 0 waiting for players, 1 countdown, 2 match in progress (CheckTournament)
void	OSP_UnpauseComplete( void );
void	OSP_RoundReset( void );
void	OSP_CheckReady( void );

// g_osp_maps.c
extern	int		osp_mapIndexSet;		// next map chosen (OSP_SetMapIndex)
extern	int		osp_mapChangeReason;	// OSP_NextMap's reason (1 voted map, 2 referee, 3 default mode rotation), 0 = normal map end
void	OSP_NextMap( int reason );
void	OSP_SetMapIndex( int index );
void	OSP_SetMapSessionCount( int count );	// "mapsessioncount": count, "none" for 0
void	OSP_LoadMaplist( void );
void	OSP_MapRestart( void );
extern	int		osp_numMaps;
extern	int		osp_mapIndex;			// osp_maps index of the current / next map
extern	int		osp_mapFileListState;	// 0 not checked yet, 1 map_autoload file list loaded, -1 none (old engine or map_autoload 0)
void	OSP_MapCheckThink( gentity_t *ent );	// "delay_mapcheck" think: OSP_LoadMaplist once the file system is ready
void	OSP_DelayMapCheck( int time );
int		OSP_MapExists( char *list, int size, int count, char *map );	// map is in the maps/*.bsp file list (marks the entry); unused size
int		OSP_ParseMapEntry( char *buf, int size, ospMap_t *map );	// next maps.txt line: 1 entry, -1 no entry, 100 last entry, 0 end
void	OSP_SaveMapSession( void );	// map list -> mapsessioncount / mapsession<n> cvars
int		OSP_GetMapFileList( char *buf, int size );
int		OSP_FindMapFile( char *map );	// maps/<map>.bsp exists (callvote map)
void	OSP_Cmd_MapList_f( gentity_t *ent );	// "maplist"
char	*OSP_ParseTabs( char *buf, int size, qboolean reset, qboolean spaces );	// next tab (or space) separated token of buf, NULL at the end
int		OSP_LoadMapSession( qboolean verbose );	// map list from the mapsession<n> cvars, 0 when there is none
int		OSP_ValidateMap( gentity_t *ent, char *map, qboolean setIndex );	// nonzero when map (name or 1-based number) is in the map list

// g_osp_multiview.c
void	OSP_MV_WriteSession( void );
void	OSP_MV_RemoveAll( gentity_t *ent );
void	OSP_MV_Validate( void );
void	OSP_MV_ReadSession( int clientNum );
void	OSP_Cmd_ViewFollow_f( gentity_t *ent );
void	OSP_MV_UpdatePortal( gentity_t *ent );
void	OSP_Cmd_ViewNext_f( gentity_t *ent, int dir );	// cycles the multiview windows (Cmd_FollowCycle_f)
void	OSP_MV_SendViewList( gentity_t *ent );

void	OSP_Cmd_ViewAdd_f( gentity_t *ent );	// "viewadd <player>"
void	OSP_Cmd_ViewRemove_f( gentity_t *ent );	// "viewremove [player]" (the active window without an argument)
void	OSP_Cmd_ViewFrame_f( gentity_t *ent );	// "viewframe <window>": sets the active window
void	OSP_Cmd_ViewCycle_f( gentity_t *ent, int dir );	// "viewcyclenext" 1 / "viewcycleprev" -1: the active window shows the next player
void	OSP_Cmd_ViewAll_f( gentity_t *ent );	// "viewall"
void	OSP_Cmd_ViewTeam_f( gentity_t *ent, int team, qboolean fValidate );	// "viewred" / "viewblue": every player of team (followTeam = team)
void	OSP_MV_AddView( gentity_t *ent, int pid, qboolean fVerbose );	// new window showing pid (portal entity)
void	OSP_MV_Start( gentity_t *ent );	// first window: upside-down view at viewcam 0 / the intermission point, SPECTATOR_MULTIVIEW
void	OSP_MV_RemoveView( gentity_t *ent, int pid, int verbose );	// verbose: 0 silent, 1 view list only, 2 + messages

// g_osp_referee.c
// OSP: argOffset is 0 for "r_<cmd> <arg>" and 1 for "ref <cmd> <arg>" (argument
// trap_Argv( argOffset + 1 ))
int		OSP_RefCommand( gentity_t *ent, char *cmd, int argOffset );	// nonzero when a referee command was handled (ClientCommand passes 0, OSP_Cmd_Ref_f 1)
void	OSP_Cmd_Ref_f( gentity_t *ent, qboolean fValue );	// "ref" / "admin" / "referee" (ClientCommand passes 0; nonzero: a referee's "ref" does nothing)
void	OSP_Ref_Help( gentity_t *ent );
void	OSP_Ref_Pause( gentity_t *ent );	// ent NULL: server console (OSP_Svcmd_Pause_f)
void	OSP_Ref_Ban( gentity_t *ent, int argOffset );	// ent NULL: server console "ban <id>"
void	OSP_Ref_Kick( gentity_t *ent, int argOffset );
void	OSP_Ref_Map( gentity_t *ent, int argOffset );
void	OSP_Ref_Lock( gentity_t *ent, int team );	// toggles the team lock (OSP_Cmd_Lock_f, OSP_RefCommand)
void	OSP_Ref_SpecLock( gentity_t *ent, int team );	// toggles the spectator lock (OSP_Cmd_SpecLock_f, OSP_RefCommand)
void	OSP_Ref_Restart( gentity_t *ent );	// ent unused
void	OSP_Ref_Put( gentity_t *ent, int team, int argOffset );
void	OSP_Ref_Remove( gentity_t *ent, int argOffset );
void	OSP_Ref_Abort( gentity_t *ent );
void	OSP_Ref_Promote( gentity_t *ent, int argOffset );
void	OSP_Ref_AllReady( gentity_t *ent );	// ent NULL: server console (OSP_Svcmd_AllReady_f)
void	OSP_Ref_MuteSpecs( gentity_t *ent );
void	OSP_Ref_AllCaptain( gentity_t *ent );

// g_osp_restrict.c
int		OSP_CheckClientFilters( char *name, int clientNum, qboolean nameOnly );	// 0 refused, 1 accepted, 2 spectator only, > 2 refused by a filter, 6 name changed too fast; nameOnly skips the address / password filters
void	OSP_LoadPlayerNames( void );
int		OSP_NormalizeName( char *in, int len, char *out, qboolean toLower );	// copy without color codes / control characters, returns its length
int		OSP_PlayerNameTeam( gclient_t *client, char *userinfo );	// player_file team of the userinfo name (sets client->sess.nameTeam)
void	OSP_SanitizeFilename( char *in, int len, char *out );	// file-name safe copy of a player name (OSP_DemoName)
extern	int		osp_numPlayerNames;	// entries loaded from player_file

// g_osp_svcmds.c
void	OSP_Svcmd_AllReady_f( void );
void	OSP_Svcmd_Pause_f( void );
void	OSP_Svcmd_LoadMaps_f( void );
void	OSP_Svcmd_BanList_f( void );

// g_osp_team.c
void	OSP_FlagReturnThink( gentity_t *ent );
int		OSP_WinningTeam( void );
int		OSP_CheckTeamJoin( int team, gentity_t *ent, int verbose );	// 0 when the team is locked or full (ent gets the message when verbose)
extern	char	*osp_teamNames[];	// indexed by team_t
void	OSP_SetCaptain( gentity_t *ent );	// passes ent's captaincy to another player of the team
void	OSP_ResetEmptyTeams( void );	// unused
void	OSP_TeamPrint( int team, char *cmd );	// server command to every connected player of team
void	OSP_PrintTimeRemaining( gentity_t *ent );
void	OSP_LogTeamScore( void );
int		OSP_ClientTeam( gentity_t *ent );	// the team ent plays or coaches for
void	OSP_Scores_TDM( gentity_t *ent, qboolean fDump );	// "scores" (ClientCommand passes 0)
void	OSP_Scores_FT( gentity_t *ent, qboolean fDump );
void	OSP_Scores_FFA( gentity_t *ent, qboolean fDump );
void	OSP_Scores_CTF( gentity_t *ent, qboolean fDump );
void	OSP_Scores_CA( gentity_t *ent, qboolean fDump );
void	OSP_ClearCaptains( void );	// clears sess.captain of every connected client
int		OSP_NumReady( void );
void	OSP_RoundStart( void );

// g_osp_viewcam.c
extern	int		osp_numSnapshots;	// snapshots taken on this map ("snapshot")
void	OSP_LoadViewcam( void );
void	OSP_ViewcamThink( gentity_t *ent, int rangeBonus );	// viewcam following a player: keeps or changes the camera (range + rangeBonus)
int		OSP_ViewcamUpdate( int cam, qboolean dynamic );	// rates camera cam, returns its activity
void	OSP_Cmd_Snapshot_f( gentity_t *ent );
void	OSP_Cmd_SnapshotDump_f( gentity_t *ent );
void	OSP_Cmd_VCProximity_f( gentity_t *ent );
void	OSP_Cmd_Range_f( gentity_t *ent );
void	OSP_Cmd_Viewcam_f( gentity_t *ent );
void	OSP_Cmd_VCFollow_f( gentity_t *ent );
int		OSP_ViewcamCyclePlayer( gentity_t *ent, int dir );
void	OSP_ViewcamCyclePosition( gentity_t *ent, int dir );
int		OSP_ViewcamRandomSpot( int moveClients );	// nonzero when it set the intermission point from a random viewcam spot (FindIntermissionPoint; moveClients: also move the clients there)
void	OSP_ViewcamRunFrame( void );

// g_osp_vote.c
void	OSP_Vote_Help( gentity_t *ent, qboolean fShowVote );	// callvote list (Cmd_CallVote_f passes 1: with header and usage; OSP_Ref_Help 0)
int		OSP_Vote_Parse( gentity_t *ent, char *cmd, char *arg, char *arg2, int arg2Size, qboolean fRefCommand );	// 0 unknown vote (help), < 0 refused, > 0 vote set up (Cmd_CallVote_f)
int		OSP_Vote_Map( gentity_t *ent, char *map, qboolean fRefCommand );	// map vote with map_queue, 0 when refused (Cmd_CallVote_f)
int		OSP_Vote_Execute( void );
// OSP: signatures from OSP_Vote_Parse's calls
typedef struct {
	int		gametypeMask;	// modes (1 << g_gametype) that list the vote in OSP_Vote_Help
	char	*name;
	char	*help;			// usage text after the name (OSP_Vote_Usage)
} ospVoteInfo_t;
extern	ospVoteInfo_t	osp_voteTable[];
int		OSP_Vote_Usage( gentity_t *ent, char *cmd, int vote );	// "<vote> ?" or no argument: prints the usage of osp_voteTable[vote], returns 1
void	OSP_Vote_Disabled( gentity_t *ent, char *name );	// "Sorry, <name> voting has been disabled"
int		OSP_Vote_ProModeOnly( gentity_t *ent, char *name );	// -1 (and a message) unless ProMode is on
void	OSP_Vote_CurrentValue( gentity_t *ent, char *name, char *value );	// "<name> is currently <value>"
char	*OSP_OvertimeModeName( qboolean fLong, int mode );	// match_overtime mode name
int		OSP_Vote_ItemToggle( gentity_t *ent, char *arg, char *arg2, int arg2Size, qboolean fRefCommand );	// powerup / item votes ("quad", "bfg", ...)
int		OSP_Vote_Toggle( gentity_t *ent, char *arg, char *arg2, int arg2Size, char *cmd, char *name, int curValue, int allowed, int vote, int voteType );	// 0/1 votes
void	OSP_PrintInvalidPlayerID( gentity_t *ent );
// OSP: the vote handlers run by OSP_Vote_Execute
void	OSP_Vote_Pmove( void );
void	OSP_Vote_Nextmap( void );
void	OSP_Vote_Referee( void );
void	OSP_Vote_Unreferee( void );
void	OSP_Vote_MapExec( void );
void	OSP_Vote_Mode( char *map );	// mode level.nextMode - 1 (map: osp_voteMap)
void	OSP_Vote_FallingDamage( void );
void	OSP_Vote_ItemSet( char *name, qboolean fPowerup, int bit, int tag, int type, qboolean enable );	// item_disable / powerup_disable bit, OSP_ShowItems / OSP_HideItems( type, tag )
void	OSP_Vote_Instagib( void );
void	OSP_Vote_SetCvar( char *name, char *cvar );	// "<cvar> <osp_voteValue>", prints ENABLED / DISABLED
void	OSP_Vote_SetValue( char *name, char *cvar );	// "<cvar> <osp_voteValue>", prints the value
void	OSP_Vote_SetValueBroadcast( char *name );	// runs level.voteString	// "Use the players command ..." (OSP: OSP_Cmd_ViewAdd_f, OSP_Cmd_ViewRemove_f, OSP_Cmd_VCFollow_f)
