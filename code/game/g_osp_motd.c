/*
 * g_osp_motd.c: the server's message of the day (server_motdfile, server_motd1..8).
 */

#include "g_local.h"

int OSP_LoadMOTD( void );

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t	server_motd8;
vmCvar_t	server_motd7;
vmCvar_t	server_motd6;
vmCvar_t	server_motd5;
vmCvar_t	server_motd4;
vmCvar_t	server_motd3;
vmCvar_t	server_motd2;
vmCvar_t	server_motd1;
vmCvar_t	server_motdfile;
// OSP: this file has no other bss.

/*
==================
OSP_SetMOTD

The 8 MOTD configstrings: server_motdfile, else the server_motd1..8 cvars
==================
*/
void OSP_SetMOTD( void ) {
	char	buf[192];

	if ( OSP_LoadMOTD() ) {
		return;
	}

	G_Printf( "\n>>> MOTD: Setting MOTD through cvars\n\n" );
	OSP_MOTDHelper( server_motd1.string, strlen( server_motd1.string ), buf, sizeof( buf ), 64 );
	trap_SetConfigstring( CS_OSP_MOTD + 0, buf );
	OSP_MOTDHelper( server_motd2.string, strlen( server_motd2.string ), buf, sizeof( buf ), 64 );
	trap_SetConfigstring( CS_OSP_MOTD + 1, buf );
	OSP_MOTDHelper( server_motd3.string, strlen( server_motd3.string ), buf, sizeof( buf ), 64 );
	trap_SetConfigstring( CS_OSP_MOTD + 2, buf );
	OSP_MOTDHelper( server_motd4.string, strlen( server_motd4.string ), buf, sizeof( buf ), 64 );
	trap_SetConfigstring( CS_OSP_MOTD + 3, buf );
	OSP_MOTDHelper( server_motd5.string, strlen( server_motd5.string ), buf, sizeof( buf ), 64 );
	trap_SetConfigstring( CS_OSP_MOTD + 4, buf );
	OSP_MOTDHelper( server_motd6.string, strlen( server_motd6.string ), buf, sizeof( buf ), 64 );
	trap_SetConfigstring( CS_OSP_MOTD + 5, buf );
	OSP_MOTDHelper( server_motd7.string, strlen( server_motd7.string ), buf, sizeof( buf ), 64 );
	trap_SetConfigstring( CS_OSP_MOTD + 6, buf );
	OSP_MOTDHelper( server_motd8.string, strlen( server_motd8.string ), buf, sizeof( buf ), 64 );
	trap_SetConfigstring( CS_OSP_MOTD + 7, buf );
}

/*
==================
OSP_LoadMOTD

MOTD lines (at most 8, 191 characters) from server_motdfile; 0 when there is none
==================
*/
int OSP_LoadMOTD( void ) {
	char			*p;
	int				lines;
	int				len;
	char			buf[MAX_OSP_FILE_CHARS];
	char			filename[MAX_QPATH];
	fileHandle_t	f;

	lines = 0;
	buf[0] = 0;
	filename[0] = 0;

	if ( server_motdfile.string[0] && Q_stricmp( server_motdfile.string, "none" ) ) {
		strcpy( filename, server_motdfile.string );
	} else {
		return 0;
	}

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		G_Printf( "\n>>> ERROR: Could not open MOTD config file \"%s\"", filename );
		return 0;
	}
	if ( len >= sizeof( buf ) ) {
		G_Printf( "\n>>> ERROR: MOTD config file \"%s\" too big! (%d bytes, max = %d)", filename, len, sizeof( buf ) - 1 );
		return 0;
	}

	if ( f ) {
		char	line[MAX_STRING_CHARS];

		G_Printf( "\nLoading MOTD config from \"%s\"\n", filename );
		trap_FS_Read( buf, len, f );
		p = buf;
		buf[len] = 0;
		lines = 0;
		do {
			line[0] = 0;
			p = OSP_ReadConfigLine( p, len - ( p - buf ), line, sizeof( line ) );
			if ( line[0] ) {
				line[191] = 0;
				trap_SetConfigstring( CS_OSP_MOTD + lines, line );
				G_Printf( "MOTD line[%d]: \"%s\"\n", lines, line );
				lines++;
			}
		} while ( p && lines < 8 );
	}

	G_Printf( "%d MOTD lines defined.\n\n", lines );
	trap_FS_FCloseFile( f );
	if ( !lines ) {
		return 0;
	}
	return 1;
}
