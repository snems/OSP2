/*
 * g_osp_svcmds.c: OSP's server console commands (called by ConsoleCommand).
 */

#include "g_local.h"

/*
==================
OSP_Svcmd_AllReady_f

"allready": starts the match (only during warmup)
==================
*/
void OSP_Svcmd_AllReady_f( void ) {
	if ( level.warmupTime == 0 ) {
		return;
	}
	OSP_Ref_AllReady( NULL );
}

/*
==================
OSP_Svcmd_Pause_f
==================
*/
void OSP_Svcmd_Pause_f( void ) {
	OSP_Ref_Pause( NULL );
}

/*
==================
OSP_Svcmd_LoadMaps_f
==================
*/
void OSP_Svcmd_LoadMaps_f( void ) {
	OSP_SetMapSessionCount( 0 );
	OSP_LoadMaplist();
}

/*
==================
OSP_Svcmd_BanList_f
==================
*/
void OSP_Svcmd_BanList_f( void ) {
	int		i;

	if ( level.numFilters == 0 ) {
		G_Printf( "No bans are currently defined.\n" );
		return;
	}

	for ( i = 0; i < level.numFilters; i++ ) {
		if ( osp_filters[i].type == 1 )
			G_Printf( "Ban entry: [NAME] name=%s, addr=%s, pass=%s\n", osp_filters[i].name, osp_filters[i].ip, osp_filters[i].password );
		else if ( osp_filters[i].type == 2 )
			G_Printf( "Ban entry: [TAG]   tag=%s, addr=%s, pass=%s\n", osp_filters[i].name, osp_filters[i].ip, osp_filters[i].password );
		else if ( osp_filters[i].type == 3 )
			G_Printf( "Ban entry: [ADDR] name=%s, addr=%s, pass=%s\n", osp_filters[i].name, osp_filters[i].ip, osp_filters[i].password );
		else if ( osp_filters[i].type == 4 )
			G_Printf( "Ban entry: [ADDRBAN] addr=%s, (player: %s)\n", osp_filters[i].ip, osp_filters[i].name );
		else if ( osp_filters[i].type == 5 )
			G_Printf( "Ban entry: [PASS] name=%s, addr=%s, pass=%s\n", osp_filters[i].name, osp_filters[i].ip, osp_filters[i].password );
		else
			G_Printf( "Ban entry: [UNKNOWN] name=%s, addr=%s, pass=%s\n", osp_filters[i].name, osp_filters[i].ip, osp_filters[i].password );
	}
}
