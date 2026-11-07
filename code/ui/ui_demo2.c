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
/*
=======================================================================

DEMOS MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_BACK0           "menu/art/back_0"
#define ART_BACK1           "menu/art/back_1"
#define ART_GO0             "menu/art/play_0"
#define ART_GO1             "menu/art/play_1"
// OSP has no frame art and uses vertical arrows (as the server browser)
#define ART_ARROWS          "menu/art/arrows_vert_0"
#define ART_ARROWUP         "menu/art/arrows_vert_top"
#define ART_ARROWDOWN       "menu/art/arrows_vert_bot"

#define MAX_DEMOS           128
#define NAMEBUFSIZE         ( MAX_DEMOS * 16 )

#define ID_BACK             10
#define ID_GO               11
#define ID_LIST             12
#define ID_RIGHT            13
#define ID_LEFT             14
// OSP ids (15 is unused)
#define ID_UP               16
#define ID_DOWN             17
#define ID_TIMEDEMO         18


typedef struct {
	menuframework_s menu;

	menutext_s      banner;


	menulist_s      list;

	menubitmap_s    arrows;
	menubitmap_s    back;
	menubitmap_s    go;
	menubitmap_s    up;         // OSP (id: left / right before back)
	menubitmap_s    down;
	menuradiobutton_s   timedemo;   // OSP "Time Demo:" radio button (cvar timedemo)

	int             numDemos;
	char            names[NAMEBUFSIZE];
	char*            demolist[MAX_DEMOS];
} demos_t;

static demos_t  s_demos;

// OSP: the demo file extension read by Demos_MenuEvent; set by Demos_MenuInit
// (dm_66 / dm_67).
static char*     demoExtension = "dm_48";



/*
===============
Demos_MenuEvent
===============
*/
static void Demos_MenuEvent(void* ptr, int event) {
	// OSP plays a demo on a double click in the list
	if (event == QM_DOUBLECLICK && ((menucommon_s*)ptr)->id == ID_LIST) {
		UI_ForceMenuOff ();
		trap_Cmd_ExecuteText(EXEC_APPEND, va("demo \"%s.%s\"\n",
		                                     s_demos.list.itemnames[s_demos.list.curvalue], demoExtension));
		return;
	}

	if (event != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s*)ptr)->id) {
		case ID_GO:
			UI_ForceMenuOff ();
			trap_Cmd_ExecuteText(EXEC_APPEND, va("demo \"%s.%s\"\n",
			                                     s_demos.list.itemnames[s_demos.list.curvalue], demoExtension));
			break;

		case ID_BACK:
			UI_PopMenu();
			break;

		case ID_UP:
			ScrollList_Key(&s_demos.list, K_UPARROW);
			break;

		case ID_DOWN:
			ScrollList_Key(&s_demos.list, K_DOWNARROW);
			break;

		case ID_TIMEDEMO:
			trap_Cvar_SetValue("timedemo", s_demos.timedemo.curvalue);
			break;
	}
}


/*
=================
UI_DemosMenu_Key
=================
*/
static sfxHandle_t UI_DemosMenu_Key(int key) {
	(void)Menu_ItemAtCursor(&s_demos.menu);
	return Menu_DefaultKey(&s_demos.menu, key);
}


/*
===============
Demos_GameVersion

OSP: the engine version as a number,
"Q3 1.31 ..." -> 131; 0 for an unknown version string.
===============
*/
static int Demos_GameVersion(void) {
	char    version[MAX_STRING_CHARS];

	trap_Cvar_VariableStringBuffer("version", version, sizeof(version));
	if (version[0] && !Q_strncmp(version, "Q3 1.", 5)) {
		return atoi(version + 5) + 100;
	}
	return 0;
}


/*
===============
Demos_MenuInit
===============
*/
static void Demos_MenuInit(void) {
	int     i;
	int     len;
	char*    demoname;

	memset(&s_demos, 0,sizeof(demos_t));
	s_demos.menu.key = UI_DemosMenu_Key;

	Demos_Cache();

	// OSP, the demo protocol of the running engine
	if (Demos_GameVersion() > 130) {
		demoExtension = "dm_67";
	} else if (Demos_GameVersion() > 127) {
		demoExtension = "dm_66";
	}

	s_demos.menu.fullscreen = qtrue;
	s_demos.menu.wrapAround = qtrue;

	s_demos.banner.generic.type     = MTYPE_BTEXT;
	s_demos.banner.generic.x        = 320;
	s_demos.banner.generic.y        = 16;
	s_demos.banner.string           = "DEMOS";
	s_demos.banner.color            = color_white;
	s_demos.banner.style            = UI_CENTER;

	s_demos.arrows.generic.type     = MTYPE_BITMAP;
	s_demos.arrows.generic.name     = ART_ARROWS;
	s_demos.arrows.generic.flags    = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_demos.arrows.generic.callback = Demos_MenuEvent;
	s_demos.arrows.generic.x        = 592;
	s_demos.arrows.generic.y        = 176;
	s_demos.arrows.width            = 64;
	s_demos.arrows.height           = 128;

	s_demos.up.generic.type         = MTYPE_BITMAP;
	s_demos.up.generic.flags        = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_MOUSEONLY;
	s_demos.up.generic.callback     = Demos_MenuEvent;
	s_demos.up.generic.id           = ID_UP;
	s_demos.up.generic.x            = s_demos.arrows.generic.x;
	s_demos.up.generic.y            = 176;
	s_demos.up.width                = 64;
	s_demos.up.height               = 64;
	s_demos.up.focuspic             = ART_ARROWUP;

	s_demos.down.generic.type       = MTYPE_BITMAP;
	s_demos.down.generic.flags      = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_MOUSEONLY;
	s_demos.down.generic.callback   = Demos_MenuEvent;
	s_demos.down.generic.id         = ID_DOWN;
	s_demos.down.generic.x          = s_demos.arrows.generic.x;
	s_demos.down.generic.y          = 240;
	s_demos.down.width              = 64;
	s_demos.down.height             = 64;
	s_demos.down.focuspic           = ART_ARROWDOWN;

	s_demos.timedemo.generic.type       = MTYPE_RADIOBUTTON;
	s_demos.timedemo.generic.name       = "Time Demo:";
	s_demos.timedemo.generic.flags      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_demos.timedemo.generic.callback   = Demos_MenuEvent;
	s_demos.timedemo.generic.id         = ID_TIMEDEMO;
	s_demos.timedemo.generic.x          = 340;
	s_demos.timedemo.generic.y          = 440;
	s_demos.timedemo.curvalue           = Com_Clamp(0, 1, trap_Cvar_VariableValue("timedemo"));

	s_demos.back.generic.type       = MTYPE_BITMAP;
	s_demos.back.generic.name       = ART_BACK0;
	s_demos.back.generic.flags      = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_demos.back.generic.id         = ID_BACK;
	s_demos.back.generic.callback   = Demos_MenuEvent;
	s_demos.back.generic.x          = 0;
	s_demos.back.generic.y          = 480-64;
	s_demos.back.width              = 128;
	s_demos.back.height             = 64;
	s_demos.back.focuspic           = ART_BACK1;

	s_demos.go.generic.type         = MTYPE_BITMAP;
	s_demos.go.generic.name         = ART_GO0;
	s_demos.go.generic.flags        = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_demos.go.generic.id           = ID_GO;
	s_demos.go.generic.callback     = Demos_MenuEvent;
	s_demos.go.generic.x            = 640;
	s_demos.go.generic.y            = 480-64;
	s_demos.go.width                = 128;
	s_demos.go.height               = 64;
	s_demos.go.focuspic             = ART_GO1;

	s_demos.list.generic.type       = MTYPE_SCROLLLIST;
	s_demos.list.generic.flags      = QMF_PULSEIFFOCUS;
	s_demos.list.generic.callback   = Demos_MenuEvent;
	s_demos.list.generic.id         = ID_LIST;
	s_demos.list.generic.x          = 64;
	s_demos.list.generic.y          = 76;
	s_demos.list.width              = 64;
	s_demos.list.height             = 20;
	s_demos.list.numitems           = trap_FS_GetFileList("demos", demoExtension, s_demos.names, NAMEBUFSIZE);
	s_demos.list.itemnames          = (const char**)s_demos.demolist;
	s_demos.list.columns            = 1;

	if (!s_demos.list.numitems) {
		strcpy(s_demos.names, "No Demos Found.");
		s_demos.list.numitems = 1;

		//degenerate case, not selectable
		s_demos.go.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
	}
	else if (s_demos.list.numitems > MAX_DEMOS)
		s_demos.list.numitems = MAX_DEMOS;

	demoname = s_demos.names;
	for (i = 0; i < s_demos.list.numitems; i++) {
		s_demos.list.itemnames[i] = demoname;

		// strip extension
		len = strlen(demoname);
		if (!Q_stricmp(demoname +  len - 5,demoExtension))
			demoname[len-6] = '\0';

		demoname += len + 1;
	}

	Menu_AddItem(&s_demos.menu, &s_demos.banner);

	Menu_AddItem(&s_demos.menu, &s_demos.list);
	Menu_AddItem(&s_demos.menu, &s_demos.arrows);
	Menu_AddItem(&s_demos.menu, &s_demos.up);
	Menu_AddItem(&s_demos.menu, &s_demos.down);
	Menu_AddItem(&s_demos.menu, &s_demos.timedemo);
	Menu_AddItem(&s_demos.menu, &s_demos.back);
	Menu_AddItem(&s_demos.menu, &s_demos.go);
}

/*
=================
Demos_Cache
=================
*/
void Demos_Cache(void) {
	trap_R_RegisterShaderNoMip(ART_BACK0);
	trap_R_RegisterShaderNoMip(ART_BACK1);
	trap_R_RegisterShaderNoMip(ART_GO0);
	trap_R_RegisterShaderNoMip(ART_GO1);
	trap_R_RegisterShaderNoMip(ART_ARROWS);
	trap_R_RegisterShaderNoMip(ART_ARROWUP);
	trap_R_RegisterShaderNoMip(ART_ARROWDOWN);
}

/*
===============
UI_DemosMenu
===============
*/
void UI_DemosMenu(void) {
	Demos_MenuInit();
	UI_PushMenu(&s_demos.menu);
}
