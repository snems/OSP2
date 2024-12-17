#include "cg_local.h"
#include "../qcommon/qcommon.h"
#include "cg_superhud_private.h"

static superhudElement_t* elementsHead;

static void CG_SHUDRoutenesDestroy(superhudElement_t* shud)
{
	superhudElement_t* tmp;

	while (shud)
	{
		if (shud->element.destroy)
		{
			shud->element.destroy(shud->element.context);
		}
		tmp = shud;
		shud = shud->next;
		Z_Free(tmp);
	}

}

qboolean CG_SHUDLoadConfigPrivate(const char* filename)
{
	fileHandle_t fileHandle;
	int rc;
	char* fileContent;
	int fileContentSize;
	qboolean filePrepared;
	qboolean eof = qfalse;
	configFileInfo_t finfo = {0, NULL, NULL};
	superhudElement_t* newElementsHead = NULL;
	superhudElement_t* newElementLast = NULL;
	superhudConfig_t* defaultElement = NULL;
	superhudConfigParseElement_t statusElement;
	superhudConfigParseCommand_t statusCommand;
	char superhudFilename[MAX_QPATH];
	int numberOfElements = 0;

	Com_sprintf(superhudFilename, MAX_QPATH, "hud/%s.cfg", filename);

	CG_Printf("^3SuperHUD: loading config file: %s\n", superhudFilename);

	rc = trap_FS_FOpenFile(superhudFilename, &fileHandle, FS_READ);
	if (rc < 0 || !fileHandle)
	{
		CG_Printf("^1SuperHUD: config file: %s is not found.\n", superhudFilename);
		return qfalse;
	}
	if (rc == 0)
	{
		CG_Printf("^1SuperHUD: config file: %s is empty.\n", superhudFilename);
		return qfalse;
	}

	fileContentSize = rc + 1;

	fileContent = Z_Malloc(fileContentSize);
	OSP_MEMORY_CHECK(fileContent);
	trap_FS_Read(fileContent, rc, fileHandle);
	trap_FS_FCloseFile(fileHandle);
	fileContent[fileContentSize - 1] = 0;

	CG_SHUDParserInit();

	filePrepared = CG_SHUDFileInfoInit(&finfo, fileContent);
	Z_Free(fileContent);
	if (!filePrepared)
	{
		CG_Printf("^1SuperHUD: config file: could not prepare %s for parsing.\n", superhudFilename);
		return qfalse;
	}

	do
	{
		statusElement = CG_SHUDFileInfoGetElementItem(&finfo);
		switch (statusElement.status)
		{
			case SUPERHUD_CONFIG_OK:
				break;
			case SUPERHUD_CONFIG_UNEXPECTED_CHARACTER:
				CG_Printf("^1SuperHUD: found unexpected character: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
				goto error_exit;
			case SUPERHUD_CONFIG_WRONG_ELEMENT_NAME:
				CG_Printf("^1SuperHUD: found unknown element name: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
				goto error_exit;
			case SUPERHUD_CONFIG_LOST_ELEMENT_BODY:
				CG_Printf("^1SuperHUD: found wrong wrong element body: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
				goto error_exit;
			case SUPERHUD_CONFIG_END_OF_FILE:
				eof = qtrue;
				break;
			case SUPERHUD_CONFIG_END_OF_ELEMENT:
			case SUPERHUD_CONFIG_WRONG_COMMAND_NAME:
				CG_Printf("^1SuperHUD: found command name or end of element: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
				goto error_exit;
		}
		if (statusElement.status == SUPERHUD_CONFIG_OK)
		{
			qboolean eoe = qfalse; //end of element
			qboolean is_default = qfalse;
			// parse element body
			if (!newElementsHead)
			{
				newElementsHead = Z_Malloc(sizeof(*newElementsHead));
				OSP_MEMORY_CHECK(newElementsHead);
			}
			if (!newElementLast)
			{
				newElementLast = newElementsHead;
			}
			else
			{
				newElementLast->next = Z_Malloc(sizeof(*newElementsHead));
				OSP_MEMORY_CHECK(newElementLast->next);
				newElementLast = newElementLast->next;
			}

			is_default = strcmp(statusElement.item->name, "!default") == 0;

			// if this element is not default and we already got default, set it
			if (!is_default && defaultElement)
			{
				memcpy(&newElementLast->config, defaultElement, sizeof(newElementLast->config));
			}

			do
			{
				statusCommand = CG_SHUDFileInfoGetCommandItem(&finfo);
				switch (statusCommand.status)
				{
					case SUPERHUD_CONFIG_OK:
						break;
					case SUPERHUD_CONFIG_UNEXPECTED_CHARACTER:
						CG_Printf("^1SuperHUD: found unexpected character: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
						goto error_exit;
					case SUPERHUD_CONFIG_END_OF_FILE:
						CG_Printf("^1SuperHUD: found unexpected end of file: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
						goto error_exit;
					case SUPERHUD_CONFIG_END_OF_ELEMENT:
						eoe = qtrue;
						break;
					case SUPERHUD_CONFIG_WRONG_ELEMENT_NAME:
					case SUPERHUD_CONFIG_LOST_ELEMENT_BODY:
						CG_Printf("^1SuperHUD: found unexpected result: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
						goto error_exit;
					case SUPERHUD_CONFIG_WRONG_COMMAND_NAME:
						CG_Printf("^3SuperHUD: found unknown command name: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
						continue;
				}
				if (statusCommand.status == SUPERHUD_CONFIG_OK)
				{
					superhudConfigParseStatus_t parseStatus;
					parseStatus = statusCommand.item->parse(&finfo, &newElementLast->config);
					switch (parseStatus)
					{
						case SUPERHUD_CONFIG_OK:
							break;
						case SUPERHUD_CONFIG_UNEXPECTED_CHARACTER:
							CG_Printf("^1SuperHUD: found unexpected character: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
							goto error_exit;
						case SUPERHUD_CONFIG_END_OF_FILE:
							CG_Printf("^1SuperHUD: found unexpected end of file: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
							goto error_exit;
						case SUPERHUD_CONFIG_END_OF_ELEMENT:
							CG_Printf("^1SuperHUD: found unexpected end of element: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
							goto error_exit;
						case SUPERHUD_CONFIG_WRONG_ELEMENT_NAME:
						case SUPERHUD_CONFIG_WRONG_COMMAND_NAME:
						case SUPERHUD_CONFIG_LOST_ELEMENT_BODY:
							CG_Printf("^1SuperHUD: found unexpected result: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
							goto error_exit;
							break;
					}
				}
			}
			while (!eoe);
			++numberOfElements;
			memcpy(&newElementLast->element, statusElement.item, sizeof(newElementLast->element));

			//set default
			if (is_default)
			{
				defaultElement = &newElementLast->config;
			}
		}
	}
	while (!eof);

	CG_Printf("^3SuperHUD: loaded %d counfigured element%s\n", numberOfElements, numberOfElements > 1 ? "s" : "");
	if (numberOfElements == 0)
	{
		CG_Printf("^3SuperHUD: ignored empty config file\n");
		goto error_exit;
	}
	else
	{
		int numberOfElementsCreated = 0;

		CG_Printf("^3SuperHUD: creating HUD routines... ");
		newElementLast = newElementsHead;

		while (newElementLast)
		{
			if (newElementLast->element.create)
			{
				newElementLast->element.context = newElementLast->element.create(&newElementLast->config);
				if (newElementLast->element.context)
				{
					++numberOfElementsCreated;
				}
			}
			newElementLast = newElementLast->next;
		}
		CG_Printf("^3%d drawable element%s\n", numberOfElementsCreated, numberOfElementsCreated > 1 ? "s" : "");
	}

	CG_Printf("^3SuperHUD: sorting HUD routines... \n");
	{
		superhudElement_t* prev;
		superhudElement_t* current;
		superhudElement_t* next;
		int swapped;

		do
		{
			swapped = 0;
			prev = NULL;
			current = newElementsHead;
			next = newElementsHead->next;

			while (current && next)
			{
				if (next->element.order < current->element.order)
				{
					//swap current and next
					// prev->current->next->after next
					if (prev)
					{
						prev->next = next;
					}
					else
					{
						newElementsHead = next;
					}

					current->next = next->next;
					next->next = current;
					++swapped;
				}

				prev = current;
				current = next;
				next = current->next;
			}
		}
		while (swapped);


	}

	CG_Printf("^3SuperHUD: installing new routines... ");

	CG_SHUDRoutenesDestroy(elementsHead);
	elementsHead = newElementsHead;

	CG_Printf("^3OK\n");

	CG_SHUDFileInfoTeardown(&finfo);
	return qtrue;

error_exit:
	CG_SHUDRoutenesDestroy(newElementsHead);
	CG_SHUDFileInfoTeardown(&finfo);
	return qfalse;
}

/*
 * Load SuperHUD config
 */
void CG_SHUDLoadConfig(void)
{
	if (!CG_SHUDLoadConfigPrivate(ch_file.string))
	{
		CG_Printf("^1SuperHUD: couldn't load file %s, going to load default.cfg\n", ch_file.string);
		if (!CG_SHUDLoadConfigPrivate("default"))
		{
			CG_Printf("^1SuperHUD: couldn't default config, going to disable SuperHUD\n", ch_file.string);
			trap_Cvar_Set("cg_shud", "0");
		}
	}
}

void CG_SHUDRoutine(void)
{
	superhudElement_t* last = elementsHead;
	int vflags;
	qboolean skip;
	const qboolean is_dead = cg.predictedPlayerState.pm_type == PM_DEAD || cg.predictedPlayerState.pm_type == PM_FREEZE;
	const qboolean is_intermission = cg.predictedPlayerState.pm_type == PM_INTERMISSION;
	const qboolean is_team_game = cgs.gametype >= GT_TEAM;
	const qboolean is_spectator = CG_IsSpectator();

	CG_DrawCrosshair();

	if (CG_DrawIntermission() == 0)
	{
		CG_OSPDrawCenterString();
	}

	while (last)
	{
		// check visibility
		vflags = last->config.visiblity.isSet ? last->config.visiblity.value : last->element.visibility;

		skip = (!(vflags & SE_IM) && is_intermission) ||
		       ((vflags & SE_TEAM_ONLY) && (!is_team_game)) ||
		       (!(vflags & SE_DEAD) && is_dead) ||
		       (!(vflags & SE_SPECT) && is_spectator) ||
		       ((vflags & SE_DEMO_HIDE) && cg.demoPlayback)
		       ;

		if (!skip && last->element.routine)
		{
			last->element.routine(last->element.context);
		}
		last = last->next;
	}
}

void CG_SHUDEventFrag(const char* message)
{
	superhudGlobalContext_t* ctx = CG_SHUDGetContext();
	Q_strncpyz(ctx->fragmessage.message, message, sizeof(ctx->fragmessage.message));
	ctx->fragmessage.time = cg.time;

	if (cgs.gametype < GT_TEAM)
	{
		ctx->rankmessage.time = cg.time;
		Com_sprintf(ctx->rankmessage.message, sizeof(ctx->rankmessage.message), "%s place with %i", CG_PlaceString(cg.snap->ps.persistant[PERS_RANK] + 1), cg.snap->ps.persistant[PERS_SCORE]);
	}
	else
	{
		ctx->rankmessage.time = 0;
		ctx->rankmessage.message[0] = 0;
	}
}

void CG_SHUDEventChat(const char* message)
{
	superhudGlobalContext_t* ctx = CG_SHUDGetContext();
	int index;

	index = ctx->chat.index % SHUD_MAX_CHAT_LINES;
	Q_strncpyz(ctx->chat.line[index].message, message, MAX_SAY_TEXT);
	ctx->chat.line[index].time = cg.time;
	++ctx->chat.index;
}

void CG_SHUDEventTeamChat(const char* message)
{
	int len = 0;
	char* loc_start;
	char* loc_end;
	char* p;
	int size = 1024;
	int index;
	superhudGlobalContext_t* ctx = CG_SHUDGetContext();

	if (customLocationsEnabled != 0)
	{
		char* cloc_begin, *cloc_end;
		int free_left;
		vec3_t cloc;
		if (CG_CustomLocationsTeamChatCode(message, cloc, &cloc_begin, &cloc_end))
		{
			int location_len;
			const char* location_name;
			char* tmp;

			location_name = CG_CustomLocationsGetName(cloc);
			location_len = strlen(location_name); //size of message without location

			tmp = Z_Malloc(size);
			OSP_MEMORY_CHECK(tmp);

			Q_strncpyz(tmp, cloc_end, size);
			free_left = size - (cloc_begin - message);
			Q_strncpyz(cloc_begin, location_name, free_left);
			free_left -= location_len;
			Q_strncpyz(cloc_begin + location_len, tmp, free_left);

			Z_Free(tmp);
		}
	}

	if (ch_FilterLocationsTeamchat.integer)
	{
		loc_start = strchr(message, '(');
		if (loc_start)
		{
			loc_start = strchr(loc_start + 1, '(');
			if (loc_start)
			{
				loc_end = strchr(loc_start, ')');
				if (loc_end)
				{
					strcpy(loc_start - 1, loc_end + 1);
				}
			}
		}
	}

	index = ctx->chat.index % SHUD_MAX_CHAT_LINES;

	p = ctx->chat.line[index].message;
	(*(p) = 0);
	while (*message)
	{
		if (Q_IsColorString(message))
		{
			*p++ = *message++;
			*p++ = *message++;
			continue;
		}
		*p++ = *message++;
		len++;
	}

	*p = 0;

	ctx->chat.line[index].time = cg.time;
	++ctx->chat.index;
}

void CG_SHUDEventObituaries(int attacker, int target, int mod)
{
	superhudGlobalContext_t* ctx = CG_SHUDGetContext();
	int index;
	const char* attackerName;
	const char* targetName;
	int attackerTeam;
	int targetTeam;

	index = ctx->obituaries.index % SHUD_MAX_OBITUARIES_LINES;

	attackerTeam = cgs.clientinfo[attacker].team;
	targetTeam = cgs.clientinfo[target].team;

	ctx->obituaries.line[index].attacker = attacker;
	ctx->obituaries.line[index].target = target;
	ctx->obituaries.line[index].mod = mod;
	ctx->obituaries.line[index].time = cg.time;

	ctx->obituaries.line[index].attackerTeam = attackerTeam;
	ctx->obituaries.line[index].targetTeam = targetTeam;

	++ctx->obituaries.index;
}

void CG_SHUDEventTempAccGetStats(playerState_t* ps, playerState_t* ops)
{
	static float lastAccuracy = 0.0f;
	static float currentAccuracy = 0.0f;

	static int lastHitCount = 0;
	static int lastFireTime = 0;
	static int lastWeapon = WP_NONE;
	static int detectedDamage = 0;
	static int currentHits;
	int weapon = ps->weapon;
	int newHits;
	superhudGlobalContext_t* ctx = CG_SHUDGetContext();
	superhudTempAccEntry_t* tempAcc = &ctx->tempAcc;

	if ((weapon != WP_LIGHTNING && lastWeapon == WP_LIGHTNING) || (cg.time - lastFireTime > 1000) || (ps->stats[STAT_HEALTH] <= 0))
	{
		if (tempAcc->totalAmmoUsed > 0 || tempAcc->totalHits > 0)
		{
			lastAccuracy = tempAcc->totalHits > 0 ? ((float)tempAcc->totalHits / (float)tempAcc->totalAmmoUsed) * 100.0f : 0.0f;

			tempAcc->lastAccuracy = (lastAccuracy != 0.0f) ? lastAccuracy : tempAcc->lastAccuracy;
			tempAcc->lastTotalAmmoUsed = tempAcc->totalAmmoUsed;
			tempAcc->lastTotalHits = tempAcc->totalHits;

			tempAcc->totalAmmoUsed = 0;
			tempAcc->totalHits = 0;
			tempAcc->accuracy = 0.0f;
			tempAcc->currentAccuracy = 0.0f;
		}
		lastHitCount = ps->persistant[PERS_HITS];
		detectedDamage = 0;
	}

	lastWeapon = weapon;

	if (weapon != WP_LIGHTNING)
	{
		return;
	}

	if (ps->weaponstate == WEAPON_FIRING)
	{
		lastFireTime = cg.time;

		if (ps->ammo[weapon] < ops->ammo[weapon])
		{
			int ammoUsed = ops->ammo[weapon] - ps->ammo[weapon];
			tempAcc->totalAmmoUsed += ammoUsed;
		}

		currentHits = ps->persistant[PERS_HITS];
		if (currentHits > lastHitCount)
		{
			int delta = currentHits - lastHitCount;

			if (detectedDamage == 0)
			{
				detectedDamage = delta;
			}

			newHits = detectedDamage > 0 ? delta / detectedDamage : 1;

			tempAcc->totalHits += newHits;
			lastHitCount = currentHits;
		}

		tempAcc->accuracy = tempAcc->totalHits > 0 ? ((float)tempAcc->totalHits / (float)tempAcc->totalAmmoUsed) * 100.0f : 0.0f;

		tempAcc->currentAccuracy = tempAcc->accuracy;
		if (currentAccuracy != 0.0f)
		{
			tempAcc->lastAccuracy = currentAccuracy;
		}
	}
}
