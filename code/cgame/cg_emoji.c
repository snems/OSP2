#include "cg_local.h"
#include "../qcommon/qcommon.h"

#define CG_EMOJI_PATH "gfx/emoji"
#define CG_EMOJI_CONFIG_NAME "gfx/emoji/emoji.cfg"
#define CG_EMOJI_HASH_SIZE 512

typedef struct emojiTable_s emojiTable_t;

struct emojiTable_s
{
	emoji_t emoji;
	emojiTable_t* next;
	emojiTable_t* prev;
};

static  emojiTable_t* emojiTable[CG_EMOJI_HASH_SIZE];

const emoji_t* CG_OSPGetEmoji(const char* name)
{
	char emoji_name[MAX_STRING_CHARS];
	const char* second_colon;
	int emoji_len;
	emojiTable_t* target;

	// If first character not EMOJI_CHAR it is not emoji
	if (name[0] != EMOJI_CHAR)
	{
		return NULL;
	}

	second_colon = strchr(&name[1], EMOJI_CHAR);

	// Not emoji if there is no second colon or no text
	if (!second_colon || second_colon - name == 1)
	{
		return NULL;
	}

	emoji_len = second_colon - name + 1;

	// Emoji name too long
	if (emoji_len > MAX_STRING_CHARS - 1)
	{
		return NULL;
	}

	memcpy(emoji_name, name, emoji_len);

	emoji_name[emoji_len] = 0;

	Q_strlwr(emoji_name);

	target = emojiTable[Com_GenerateHashValue(emoji_name, CG_EMOJI_HASH_SIZE)];

	while (target && Q_stricmp(emoji_name, target->emoji.name))
	{
		target = target->next;
	}

	if (target == NULL)
	{
		return NULL;
	}

	return Q_stricmp(emoji_name, target->emoji.name) == 0 ? &target->emoji : NULL;
}

void CG_OSPAddEmoji(const char* name)
{
	/* store in the hash table */
	emojiTable_t** node;
	emojiTable_t* new_emoji;
	char shadername[MAX_NAME_LENGTH];

	new_emoji = (emojiTable_t*)Z_Malloc(sizeof(emojiTable_t));
	OSP_MEMORY_CHECK(new_emoji);

	Q_strncpyz(shadername, va("%s/%s", CG_EMOJI_PATH, name), MAX_NAME_LENGTH);
	Q_strncpyz(new_emoji->emoji.name, va("%c%s%c", EMOJI_CHAR, name, EMOJI_CHAR), MAX_NAME_LENGTH);

	Q_strlwr(new_emoji->emoji.name);

	new_emoji->emoji.shader = trap_R_RegisterShader(shadername);
	if (!new_emoji->emoji.shader)
	{
		CG_Printf("Couldn't load emoji token: \"%s\" shader name: \"%s\"\n", name, shadername);
		Z_Free(new_emoji);
		return;
	}

	new_emoji->emoji.len = strlen(new_emoji->emoji.name);

	node = &emojiTable[Com_GenerateHashValue(new_emoji->emoji.name, CG_EMOJI_HASH_SIZE)];
	if (*node == NULL)
	{
		*node = new_emoji;
	}
	else
	{
		//collision
		while ((*node)->next)
		{
			node = &(*node)->next;
		}
		new_emoji->prev = *node;
		(*node)->next = new_emoji;
	}
}

static const char* CG_OSPStringFindIfChar(const char* begin, const char* end, qboolean(*pred)(char string))
{
	while (begin < end)
	{
		if (pred(*begin))
		{
			return begin;
		}
		++begin;
	}
	return end;
}

static const char* CG_OSPStringFindIfNotChar(const char* begin, const char* end, qboolean(*pred)(char string))
{
	while (begin < end)
	{
		if (!pred(*begin))
		{
			return begin;
		}
		++begin;
	}
	return end;
}

static qboolean CG_OSPEmojiIsCharAllowed(char c)
{
	return (c >= 'a' && c <= 'z')
	       || (c >= 'A' && c <= 'Z')
	       || (c >= '0' && c <= '9')
	       ||  c == '_'
	       ||  c == '-'
	       ;

}

void CG_OSPLoadEmoji(void)
{
	char text[16384];
	const char* begin;
	const char* end;
	char emoji_name[MAX_INFO_VALUE];
	fileHandle_t filehandle;
	int file_size;

	file_size = trap_FS_FOpenFile(CG_EMOJI_CONFIG_NAME, &filehandle, 0);
	if (file_size <= 0)
	{
		CG_Printf("\n*** ERROR: Could not open emoji config \"%s\"\n", CG_EMOJI_CONFIG_NAME);
	}
	else if (file_size >= 16384)
	{
		CG_Printf("\n*** Emoji config file \"%s\" too big! (%d bytes, max = %d)\n", CG_EMOJI_CONFIG_NAME, file_size, 16383);
		trap_FS_FCloseFile(filehandle);
	}
	else if (filehandle)
	{
		trap_FS_Read(text, file_size, filehandle);
		trap_FS_FCloseFile(filehandle);
		/* Split file content to strings */
		begin = &text[0];
		end = &text[file_size];
		while (begin < end)
		{
			const char* name_begin;
			const char* name_end;
			int name_size;

			name_begin = CG_OSPStringFindIfChar(begin, end, CG_OSPEmojiIsCharAllowed);
			name_end = CG_OSPStringFindIfNotChar(name_begin, end, CG_OSPEmojiIsCharAllowed);

			name_size = name_end - name_begin;
			if (name_size > 0)
			{
				memcpy(emoji_name, name_begin, name_size);
				emoji_name[name_size] = 0;
				CG_OSPAddEmoji(emoji_name);
			}
			begin = name_end;
		}
	}
}





