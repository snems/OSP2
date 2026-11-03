/*
 * g_osp_bot.c: OSP's bot helpers: bot names from a hash of unknown names, bot_autoload.
 */

#include "g_local.h"

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t	bot_autoload;

// OSP: (the only data of this file): set by G_CheckBotSpawn
// when it runs OSP_BotAutoload, 5 seconds into the map
int		osp_botAutoloadDone = 0;

// g_bot.c (OSP: no longer static there)
extern int		g_numBots;
extern char		*g_botInfos[];
void	G_AddBot( const char *name, float skill, const char *team, int delay, char *altname );

/*
===============
OSP_BotNameFromHash

The name of the bot chosen by the hash of name (G_AddBot maps unknown bot names to a real bot).
===============
*/
char *OSP_BotNameFromHash( const char *name ) {
	static char	botName[128];
	int			n;

	n = OSP_BotNameHash( name, g_numBots );
	strcpy( botName, Info_ValueForKey( g_botInfos[n], "name" ) );
	return botName;
}

/*
===============
OSP_BotNameHash
===============
*/
int OSP_BotNameHash( const char *s, int mod ) {
	int		i;
	int		sum;

	sum = 0;
	for ( i = 0; i < strlen( s ); i++ ) {
		sum += ( (const unsigned char *)s )[i];
	}
	return sum % mod;
}

/*
===============
OSP_BotAutoload

Adds the bots listed in bot_autoload (separated by spaces) once, then sets it to "[none]".
===============
*/
void OSP_BotAutoload( void ) {
	char	*s, *p;
	char	buf[MAX_STRING_CHARS];

	if ( !bot_autoload.string[0] || !Q_stricmp( bot_autoload.string, "[none]" ) ) {
		return;
	}

	Q_strncpyz( buf, bot_autoload.string, sizeof( buf ) );
	s = buf;
	do {
		p = strchr( s, ' ' );
		if ( p ) {
			*p = 0;
			p++;
		}
		G_AddBot( s, 0, 0, 0, 0 );
		s = p;
	} while ( s );

	trap_Cvar_Set( "bot_autoload", "[none]" );
}
