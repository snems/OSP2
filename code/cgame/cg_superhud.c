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

/*
 * Load SuperHUD config
 */
void CG_SHUDLoadConfig(void)
{
	//read fileneame into local buffer
	fileHandle_t fileHandle;
	int rc;
	//configFileInfo_t cf;
	char c;
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

	Com_sprintf(superhudFilename, MAX_QPATH, "hud/%s.cfg", ch_file.string);

	CG_Printf("^3SuperHUD: loading config file: %s\n", superhudFilename);

	rc = trap_FS_FOpenFile(superhudFilename, &fileHandle, FS_READ);
	if (rc < 0 || !fileHandle)
	{
		CG_Printf("^1SuperHUD: config file: %s is not found.\n", superhudFilename);
		return;
	}
	if (rc == 0)
	{
		CG_Printf("^1SuperHUD: config file: %s is empty.\n", superhudFilename);
		return;
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
		return;
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
				CG_Printf("^1SuperHUD: found wrong element name1: %s\n", finfo.last_line->line);
				CG_Printf("^1SuperHUD: found wrong element name1: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
				goto error_exit;
			case SUPERHUD_CONFIG_LOST_ELEMENT_BODY:
				CG_Printf("^1SuperHUD: found wrong wrong element body: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
				goto error_exit;
			case SUPERHUD_CONFIG_END_OF_FILE:
				eof = qtrue;
				break;
			case SUPERHUD_CONFIG_END_OF_ELEMENT:
			case SUPERHUD_CONFIG_WRONG_COMMAND_NAME:
				CG_Printf("^1SuperHUD: found unexpected character: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
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
				//memcpy(&newElementLast->config, defaultElement, sizeof(newElementLast->config));
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
						CG_Printf("^1SuperHUD: found wrong command name: file %s, line %d, pos %d.\n", superhudFilename, finfo.last_line->line_number, finfo.pos);
						goto error_exit;
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
		
		while(newElementLast)
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

	CG_Printf("^3SuperHUD: installing new routines... ");

	CG_SHUDRoutenesDestroy(elementsHead);
	elementsHead = newElementsHead;

	CG_Printf("^3OK\n");

	CG_SHUDFileInfoTeardown(&finfo);
	return;

error_exit:
	CG_SHUDRoutenesDestroy(newElementsHead);
	CG_SHUDFileInfoTeardown(&finfo);
	return;
}

void CG_SHUDRoutine(void)
{
	superhudElement_t* last = elementsHead;

	while(last)
	{
		if (last->element.routine)
		{
			last->element.routine(last->element.context);
		}
		last = last->next;
	}
  CG_OSPDrawCrosshair();

	if (CG_DrawIntermission() == 0)
	{
		CG_OSPDrawCenterString();
	}
}

void CG_SHUDFragEvent(const char *message)
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

