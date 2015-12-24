// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * proto.h: Constants for use with client/server protocol.
 */

#ifndef _PROTO_H
#define _PROTO_H

/* Sync/login message types */
enum {
   AP_PING          = 1,
   AP_LOGIN         = 2,
   AP_REGISTER      = 3,
   AP_REQ_GAME      = 4,
   AP_REQ_ADMIN     = 5,
   AP_RESYNC        = 6,
   AP_GETCLIENT     = 7,
   AP_GETRESOURCE   = 8,
   AP_GETALL        = 9,
   AP_REQ_MENU      = 10,
   AP_ADMINNOTE     = 11,

   AP_GETLOGIN      = 21,
   AP_GETCHOICE     = 22,
   AP_LOGINOK       = 23,
   AP_LOGINFAILED   = 24,
   AP_GAME          = 25,
   AP_ADMIN         = 26,
   AP_ACCOUNTUSED   = 27,
   AP_TOOMANYLOGINS = 28,
   AP_TIMEOUT       = 29,
   AP_CREDITS       = 30,
   AP_DOWNLOAD      = 31,
   AP_UPLOAD        = 32,
   AP_NOCREDITS     = 33,
   AP_MESSAGE       = 34,
   AP_DELETERSC     = 35,
   AP_DELETEALLRSC  = 36,
   AP_NOCHARACTERS  = 37,
   AP_GUEST         = 38,
   AP_SERVICEREPORT = 39,
};


/* Game message types */
enum {   
   BP_ECHO_PING             = 1,
   BP_RESYNC                = 2,
   BP_PING                  = 3,
   BP_ROUNDTRIP1            = 4,
   BP_ROUNDTRIP2            = 5,
   BP_SYSTEM                = 6,

   BP_LOGOFF                = 20,
   BP_WAIT                  = 21,
   BP_UNWAIT                = 22,
   BP_CHANGE_PASSWORD       = 23,
   BP_AD_SELECTED           = 24,
   
   BP_CHANGE_RESOURCE       = 30,
   BP_SYS_MESSAGE           = 31,
   BP_MESSAGE               = 32,

   BP_SEND_PLAYER           = 40,
   BP_SEND_STATS            = 41,
   BP_SEND_ROOM_CONTENTS    = 42,
   BP_SEND_OBJECT_CONTENTS  = 43,
   BP_SEND_PLAYERS          = 44,
   BP_SEND_CHARACTERS       = 45,
   BP_USE_CHARACTER         = 46,
   BP_DELETE_CHARACTER      = 47,
   BP_NEW_CHARINFO          = 48,
   BP_SEND_CHARINFO         = 49,
   BP_SEND_SPELLS           = 50,
   BP_SEND_SKILLS           = 51,
   BP_SEND_STAT_GROUPS      = 52,
   BP_SEND_ENCHANTMENTS     = 53,
   BP_REQ_QUIT              = 54,
   BP_SAY_BLOCKED           = 55,
   BP_CHARINFO_OK           = 56,
   BP_CHARINFO_NOT_OK       = 57,
   BP_LOAD_MODULE           = 58,
   BP_UNLOAD_MODULE         = 59,

   BP_REQ_ADMIN             = 60,
   BP_REQ_DM                = 61,
   BP_REQ_ADMIN_QUEST       = 62,

   BP_EFFECT                = 70,
   
   BP_MAIL                  = 80,
   BP_REQ_GET_MAIL          = 81,
   BP_SEND_MAIL             = 82,
   BP_DELETE_MAIL           = 83,
   BP_DELETE_NEWS           = 84,
   BP_REQ_ARTICLES          = 85,
   BP_REQ_ARTICLE           = 86,
   BP_POST_ARTICLE          = 87,

   BP_REQ_LOOKUP_NAMES      = 88,

   BP_ACTION                = 90,
   BP_REQ_MOVE              = 100,
   BP_REQ_TURN              = 101,
   BP_REQ_GO                = 102,
   BP_REQ_ATTACK            = 103,
   BP_REQ_SHOOT             = 104,
   BP_REQ_CAST              = 105,
   BP_REQ_USE               = 106,
   BP_REQ_UNUSE             = 107,
   BP_REQ_APPLY             = 108,
   BP_REQ_ACTIVATE          = 109,
   BP_SAY_TO                = 110,
   BP_SAY_GROUP             = 111,
   BP_REQ_PUT               = 112,
   BP_REQ_GET               = 113,
   BP_REQ_GIVE              = 114,
   BP_REQ_TAKE              = 115,
   BP_REQ_LOOK              = 116,
   BP_REQ_INVENTORY         = 117,
   BP_REQ_DROP              = 118,
   BP_REQ_HIDE              = 119,
   BP_REQ_OFFER             = 120,
   BP_ACCEPT_OFFER          = 121,
   BP_CANCEL_OFFER          = 122,
   BP_REQ_COUNTEROFFER      = 123,
   BP_REQ_BUY               = 124,
   BP_REQ_BUY_ITEMS         = 125,
   BP_CHANGE_DESCRIPTION    = 126,
   BP_REQ_INVENTORY_MOVE    = 127,

   BP_PLAYER                = 130,
   BP_STAT                  = 131,
   BP_STAT_GROUP            = 132,
   BP_STAT_GROUPS           = 133,
   BP_ROOM_CONTENTS         = 134,
   BP_OBJECT_CONTENTS       = 135,
   BP_PLAYERS               = 136,
   BP_PLAYER_ADD            = 137,
   BP_PLAYER_REMOVE         = 138,
   BP_CHARACTERS            = 139,
   BP_CHARINFO              = 140,
   BP_SPELLS                = 141,
   BP_SPELL_ADD             = 142,
   BP_SPELL_REMOVE          = 143,
   BP_SKILLS                = 144,
   BP_SKILL_ADD             = 145,
   BP_SKILL_REMOVE          = 146,
   BP_ADD_ENCHANTMENT       = 147,
   BP_REMOVE_ENCHANTMENT    = 148,
   BP_QUIT                  = 149,
   BP_BACKGROUND            = 150,
   BP_PLAYER_OVERLAY        = 151,
   BP_ADD_BG_OVERLAY        = 152,
   BP_REMOVE_BG_OVERLAY     = 153,
   BP_CHANGE_BG_OVERLAY     = 154,
   BP_USERCOMMAND           = 155,
   BP_REQ_STAT_CHANGE       = 156,
   BP_CHANGED_STATS         = 157,
   BP_CHANGED_STATS_OK      = 158,
   BP_CHANGED_STATS_NOT_OK	 = 159,

   BP_PASSWORD_OK           = 160,
   BP_PASSWORD_NOT_OK       = 161,
   BP_ADMIN                 = 162,

   BP_PLAY_WAVE             = 170,
   BP_PLAY_MUSIC            = 171,
   BP_PLAY_MIDI             = 172,
   BP_STOP_WAVE             = 173,

   BP_LOOK_NEWSGROUP        = 180,
   BP_ARTICLES              = 181,
   BP_ARTICLE               = 182,

   BP_LOOKUP_NAMES          = 190,

   BP_MOVE                  = 200,
   BP_TURN                  = 201,
   BP_SHOOT                 = 202,
   BP_USE                   = 203,
   BP_UNUSE                 = 204,
   BP_USE_LIST              = 205,
   BP_SAID                  = 206,
   BP_LOOK                  = 207,
   BP_INVENTORY             = 208,
   BP_INVENTORY_ADD         = 209,
   BP_INVENTORY_REMOVE      = 210,
   BP_OFFER                 = 211,
   BP_OFFER_CANCELED        = 212,
   BP_OFFERED               = 213,
   BP_COUNTEROFFER          = 214,
   BP_COUNTEROFFERED        = 215,
   BP_BUY_LIST              = 216,
   BP_CREATE                = 217,
   BP_REMOVE                = 218,
   BP_CHANGE                = 219,
   BP_LIGHT_AMBIENT         = 220,
   BP_LIGHT_PLAYER          = 221,
   BP_LIGHT_SHADING         = 222,
   BP_SECTOR_MOVE           = 223,
   BP_SECTOR_LIGHT          = 224,
   BP_WALL_ANIMATE          = 225,
   BP_SECTOR_ANIMATE        = 226,
   BP_CHANGE_TEXTURE        = 227,
   BP_INVALIDATE_DATA       = 228,
   BP_RADIUS_SHOOT          = 229,
   BP_REQ_DEPOSIT           = 230,
   BP_WITHDRAWAL_LIST       = 231,
   BP_REQ_WITHDRAWAL        = 232,
   BP_REQ_WITHDRAWAL_ITEMS  = 233,
   BP_XLAT_OVERRIDE         = 234,
   BP_WALL_SCROLL           = 235,
   BP_SECTOR_SCROLL         = 236,
   BP_SET_VIEW              = 237,
   BP_RESET_VIEW            = 238,
   BP_SECTOR_CHANGE         = 239,
};

// User commands (in BP_USERCOMMAND message)
enum {
   UC_SEND_QUIT = 1,
   UC_LOOK_PLAYER = 2,
   UC_CHANGE_URL = 3,
   UC_SPELL_SCHOOLS = 4,
   UC_REST = 5,
   UC_STAND = 6,

   UC_REQ_PREFERENCES = 7,

   UC_SUICIDE = 8,

   UC_SEND_PREFERENCES = 9,

   UC_REQ_GUILDINFO = 10,
   UC_GUILDINFO = 11,
   UC_INVITE = 12,
   UC_EXILE = 13,
   UC_RENOUNCE = 14,
   UC_ABDICATE = 15,
   UC_VOTE = 16,
   UC_SET_RANK = 17,
   UC_GUILD_ASK = 18,
   UC_GUILD_CREATE = 19,
   UC_DISBAND = 20,
   UC_REQ_GUILD_LIST = 21,
   UC_GUILD_LIST = 22,
   UC_MAKE_ALLIANCE = 23,
   UC_END_ALLIANCE = 24,
   UC_MAKE_ENEMY = 25,
   UC_END_ENEMY = 26,
   UC_GUILD_HALLS = 27,
   UC_ABANDON_GUILD_HALL = 28,
   UC_GUILD_RENT = 29,
   UC_GUILD_SET_PASSWORD = 30,
   UC_GUILD_SHIELD = 31,
   UC_GUILD_SHIELDS = 32,
   UC_CLAIM_SHIELD = 33,

   UC_RECEIVE_PREFERENCES = 34,

   UC_DEPOSIT = 35,
   UC_WITHDRAW = 36,
   UC_BALANCE = 37,

   UC_APPEAL = 40,
   UC_REQ_RESCUE = 41,

   UC_MINIGAME_START        = 45,
   UC_MINIGAME_STATE        = 46,
   UC_MINIGAME_MOVE         = 47,
   UC_MINIGAME_PLAYER       = 48,
   UC_MINIGAME_RESET_PLAYERS= 49,

   UC_REQ_TIME = 60,
};

// Login error action constants
#define LA_NOTHING   0
#define LA_LOGOFF    1

/* Constants for <say_info> for BP_SAY_TO message */
enum { SAY_NORMAL = 1, SAY_YELL = 2, SAY_EVERYONE = 3, SAY_GROUP = 4, SAY_RESOURCE = 5,
       SAY_EMOTE = 6, SAY_MESSAGE = 7, SAY_DM = 9, SAY_GUILD = 10 };

/* Constants for <attack_info> for BP_REQ_ATTACK message */
enum { ATTACK_NORMAL = 1, };

/* Tags for upper 4 bits of objects */
enum { CLIENT_TAG_NORMAL = 0, CLIENT_TAG_NUMBER = 1};

/* char info each stat type */
enum { INFO_SINGLE = 1, INFO_MULTIPLE = 2, INFO_OVERLAYS = 3, INFO_FILTERS = 4};

/* These constants tell how an object should be animated */
enum {
   ANIMATE_NONE          = 1, // Don't animate at all
   ANIMATE_CYCLE         = 2, // Cycle continuously through all groups of bitmaps
   ANIMATE_ONCE          = 3, // Perform a one-time animation
   ANIMATE_FLOOR_LIFT    = 4, // Change floor height
   ANIMATE_CEILING_LIFT  = 5, // Change ceiling height
   ANIMATE_ROOM_BITMAP   = 6, // Used internally by client (change bitmap groups)
   ANIMATE_SCROLL        = 7, // Used internally by client (animate texture offsets)
   ANIMATE_FLICKER       = 8, // Used internally by client (flicker lights)
   ANIMATE_TRANSLATION   = 9, // Palette translation
   ANIMATE_EFFECT        = 10,// Use special effects like translucency
};

/* server angle value that equals 360 degrees */
#define MAX_ANGLE 4096

/* Max # of player overlays allowed */
#define NUM_PLAYER_OVERLAYS 2

/* Constants for stat type STAT and STAT_GROUP messages */
enum { STAT_INT = 1, STAT_RES = 2, };

/* Statistics group types */
enum {
   STATS_NUMERIC    = 1,    // Numeric values
   STATS_LIST       = 2,    // Items in a list
};

/* Enchantment types */
enum {
   ENCHANT_PLAYER = 1,      // Player enchantment
   ENCHANT_ROOM   = 2,      // Room enchantment
};

/* Hotspots for player overlays; each one specifies a certain point on the screen */
#define HOTSPOT_PLAYER_MAX 9
enum {
   HOTSPOT_NW       = 1,
   HOTSPOT_N        = 2,
   HOTSPOT_NE       = 3,
   HOTSPOT_E        = 4,
   HOTSPOT_SE       = 5,
   HOTSPOT_S        = 6,
   HOTSPOT_SW       = 7,
   HOTSPOT_W        = 8,
   HOTSPOT_CENTER   = 9,
};


/* Remote view flag values */
#define REMOTE_VIEW_MOVE	 0x00000001
#define REMOTE_VIEW_TURN	 0x00000002
#define REMOTE_VIEW_TILT	 0x00000004
#define REMOTE_VIEW_CAST	 0x00000008
#define REMOTE_VIEW_LOOK	 0x00000010
#define REMOTE_VIEW_TOP		 0x00000100
#define REMOTE_VIEW_BOTTOM	 0x00000200
#define REMOTE_VIEW_MID		 0x00000400
#define REMOTE_VIEW_SPECIFIED	 0x00000800
#define REMOTE_VIEW_CONTROL	 0x00010000
#define REMOTE_VIEW_ESC_CANCELS	 0x00020000
#define REMOTE_VIEW_VALID_HEIGHT 0x00040000
#define REMOTE_VIEW_VALID_LIGHT  0x00080000


/* Object flag values and masks */
#define OF_PLAYER        0x00000004    // Set if object is a player
#define OF_ATTACKABLE    0x00000008    // Set if object is legal target for an attack
#define OF_GETTABLE      0x00000010    // Set if player can try to pick up object
#define OF_CONTAINER     0x00000020    // Set if player can put objects inside this one
#define OF_NOEXAMINE     0x00000040    // Set if player CAN'T examine object
#define OF_ITEM_MAGIC    0x00000080    // Set for magic item to color in lists
#define OF_HANGING       0x00000100    // Set for ceiling-pinned objects.
#define OF_OFFERABLE     0x00000200    // Set if object can be offered to
#define OF_BUYABLE       0x00000400    // Set if object can be bought from
#define OF_ACTIVATABLE   0x00000800    // Set if object can be activated
#define OF_APPLYABLE     0x00001000    // Set if object can be applied to another object

#define OF_BOUNCING      0x00010000    // If both flags on then object is bouncing
#define OF_FLICKERING    0x00020000    // For players or objects if holding a flickering light.
#define OF_FLASHING      0x00040000    // For players or objects if flashing with light.
#define OF_PHASING       0x00080000    // For players or objects if phasing translucent/solid.

#define GetItemFlags(flags)   ((flags))

// Drawing effects. Separate from object flags.
#define NUM_DRAW_EFFECTS 256       // # of object drawing effects.
enum {
   DRAWFX_DRAW_PLAIN    = 0x00,    // No special drawing effects
   DRAWFX_TRANSLUCENT25 = 0x01,    // Set if object should be drawn at 25% opacity
   DRAWFX_TRANSLUCENT50 = 0x02,    // Set if object should be drawn at 50% opacity
   DRAWFX_TRANSLUCENT75 = 0x03,    // Set if object should be drawn at 75% opacity
   DRAWFX_BLACK         = 0x04,    // Set if object should be drawn all black
   DRAWFX_INVISIBLE     = 0x05,    // Set if object should be drawn with invisibility effect
   DRAWFX_TRANSLATE     = 0x06,    // Reserved (used internally by client)
   DRAWFX_DITHERINVIS   = 0x07,    // Haze (dither with transparency) 50% of pixels
   DRAWFX_DITHERTRANS   = 0x08,    // Dither (with two translates) 50% of pixels
   DRAWFX_DOUBLETRANS   = 0x09,    // Translate twice each pixel, plus lighting
   DRAWFX_SECONDTRANS   = 0x0A,    // Ignore per-overlay xlat and use only secondary xlat
   DRAWFX_DITHERGREY    = 0x0B,    // Haze (dither with transparency) 50% of pixels, greyscale other 50%
};

// Minimap dot color bitfield. Now separate from object flags.
#define MM_NONE          0x00000000    // No dot (default for all objects)
#define MM_PLAYER        0x00000001    // Standard blue player dot
#define MM_ENEMY         0x00000002    // Enemy (halo or attackable) player
#define MM_FRIEND        0x00000004    // Friendly (guild ally) player
#define MM_GUILDMATE     0x00000008    // Guildmate player
#define MM_BUILDER_GROUP 0x00000010    // Player is in same building group
#define MM_MONSTER       0x00000020    // Default monster dot
#define MM_NPC           0x00000040    // NPC
#define MM_MINION_OTHER  0x00000080    // Set if monster is other's minion
#define MM_MINION_SELF   0x00000100    // Set if a monster is our minion
#define MM_TEMPSAFE      0x00000200    // Set if player has a temporary angel.
#define MM_MINIBOSS      0x00000400    // Set if mob is a miniboss (survival arena).
#define MM_BOSS          0x00000800    // Set if mob is a boss (survival arena).

/* Player name color sent as hex RGB value. Define constants
   for ease of use as needed. Requires OF_PLAYER boolean flag
   to be set to draw a name. */
#define NC_PLAYER        0xFFFFFF   // White name.
#define NC_SHADOW        0x000000   // Set if name should be drawn black.
#define NC_KILLER        0xFF0000   // Set if object is a killer.
#define NC_OUTLAW        0xFC9E00   // Set if object is an outlaw.
#define NC_DM            0x00FFFF   // Set if object is a DM player.
#define NC_CREATOR       0xFFFF00   // Set if object is a creator player.
#define NC_SUPER         0x00FF00   // Set if object is a "super DM".
#define NC_MODERATOR     0x0078FF   // Set if object is a "moderator".
#define NC_EVENTCHAR     0xFF00FF   // Set if object is an event character.
#define NC_DAENKS        0xB300B3   // Purple name color for Daenks.
#define NC_COLOR_MAX     0xFFFFFF   // Max color, defined for clarity.

/* Enum of object types. */
typedef enum {
   OT_NONE         = 0,   // Default for most objects.
   OT_KILLER       = 1,   // Set if object is a killer.
   OT_OUTLAW       = 2,   // Set if object is an outlaw.
   OT_DM           = 3,   // Set if object is a DM player.
   OT_CREATOR      = 4,   // Set if object is a creator player.
   OT_SUPER        = 5,   // Set if object is a "super DM".
   OT_MODERATOR    = 6,   // Set if object is a "moderator".
   OT_EVENTCHAR    = 7,   // Set if object is an event character.
} object_type;

/* How objects allow or disallow motion onto their square */
typedef enum {
   MOVEON_YES        = 0,   // Can always move on object
   MOVEON_NO         = 1,   // Can never move on object
   MOVEON_TELEPORTER = 2,   // Can move on object, but then kod will move you elsewhere
   MOVEON_NOTIFY     = 3,
} moveon_type;

// Client preferences defines. Set if:
#define CF_SAFETY_OFF    0x0001  // Player has safety off
#define CF_TEMPSAFE      0x0002  // Player has temp safety on death activated
#define CF_GROUPING      0x0004  // Player is grouping
#define CF_AUTOLOOT      0x0008  // Player is automatically picking up loot
#define CF_AUTOCOMBINE   0x0010  // Player automatically combines spell items
#define CF_REAGENTBAG    0x0020  // Player automatically puts items into reagent bag
#define CF_SPELLPOWER    0x0040  // Player gets spellpower readout from cast spells

/* Effect codes */
enum {
   EFFECT_INVERT        = 1,   // Invert screen
   EFFECT_SHAKE         = 2,   // Shake player's view
   EFFECT_PARALYZE      = 3,   // Paralyze player (no motion)
   EFFECT_RELEASE       = 4,   // Stop paralysis of player
   EFFECT_BLIND         = 5,   // Make player blind
   EFFECT_SEE           = 6,   // Remove blindness
   EFFECT_PAIN          = 7,   // Make player see they're hurting
   EFFECT_BLUR          = 8,   // Distort player's vision
   EFFECT_RAINING       = 9,   // Start rain
   EFFECT_SNOWING       = 10,  // Start snow
   EFFECT_CLEARWEATHER  = 11,  // Stop all weather effects
   EFFECT_SAND          = 12,  // Sandstorm
   EFFECT_CLEARSAND     = 13,  // Stop sandstorm
   EFFECT_WAVER         = 14,  // Wavering sideways
   EFFECT_FLASHXLAT     = 15,  // Flashes screen with a given XLAT number
   EFFECT_WHITEOUT	= 16,  // Got from full white and fade back to normal
   EFFECT_XLATOVERRIDE	= 17,  // Use this xlat at end over the whole screen
   EFFECT_FIREWORKS = 18,
};

/* Room animation action codes */
enum {
   RA_NONE               = 0,  // Do nothing special
   RA_PASSABLE_END       = 1,  // Make wall passable when its animation ends
   RA_IMPASSABLE_END     = 2,  // Make wall impassable when its animation ends
   RA_INVISIBLE_END      = 3,  // Make wall passable when its animation ends, and
                               //   remove its normal wall texture
};

/* Object description flags (BP_LOOK) */
#define DF_EDITABLE    0x01    // Item has inscription/description that can be edited
#define DF_INSCRIBED   0x02    // Item has inscription/description

/* Flags for downloading files (AP_DOWNLOAD) */
enum {
   DF_RETRIEVE         = 0x00,   // Retrieve file
   DF_DELETE           = 0x01,   // Delete file

   DF_DIRRESOURCE      = 0x00,   // File's location is resource subdirectory
   DF_DIRCLIENT        = 0x04,   // File's location is client subdirectory
   DF_DIRWIN           = 0x08,   // File's location is Windows directory
   DF_DIRWINSYS        = 0x0c,   // File's location is Windows system directory
   DF_DIRHELP          = 0x10,   // File's location is help subdirectory
   DF_DIRMAIL          = 0x14,   // File's location is mail subdirectory
   DF_ADVERTISEMENT    = 0x18,   // Identifies file as an advertisement (goes in client dir)

   DF_GUEST            = 0x20,   // File should be downloaded by guests
};
#define DownloadCommand(z)  ((z) & 0x03)
#define DownloadLocation(z) ((z) & 0x1c)

/* Flags for playing sounds (BP_PLAY_WAVE) */
enum {
   SF_LOOP             = 0x01,      // Loop sound until player leaves room
   SF_RANDOM_PITCH	   = 0x02,		// Choose a random pitch for sound
   SF_RANDOM_PLACE     = 0x04,      // Kod already chose a random position
};

/* Flags for changing textures dynamically (BP_CHANGE_TEXTURE) */
enum {
   CTF_ABOVEWALL       = 0x01,      // Change above wall texture
   CTF_NORMALWALL      = 0x02,      // Change normal wall texture
   CTF_BELOWWALL       = 0x04,      // Change below wall texture
   CTF_FLOOR           = 0x08,      // Change floor texture
   CTF_CEILING         = 0x10,      // Change ceiling texture
   CTF_RESET           = 0x20,      // Reset to original ROO values
};

/* Types of sector lighting animation (BP_SECTOR_LIGHT) */
enum {
   SL_FLICKER_ON       = 1,         // Turn flickering on
   SL_FLICKER_OFF      = 2,         // Turn flickering off
};

/* Size in bytes of numbers in protocol */
#define SIZE_TYPE            1
#define SIZE_ID              4
#define SIZE_COORD           2
#define SIZE_LIST_LEN        2
#define SIZE_STRING_LEN      2
#define SIZE_SAY_INFO        1
#define SIZE_ATTACK_INFO     1
#define SIZE_NUM_RECIPIENTS  2
#define SIZE_NUM_STATS       1
#define SIZE_STAT_NUM        1
#define SIZE_STAT_TYPE       1
#define SIZE_GROUP           1
#define SIZE_AMOUNT          4
#define SIZE_COST            4
#define SIZE_ADMIN           1
#define SIZE_DOLLARS         2
#define SIZE_VALUE           4
#define SIZE_INFO_TYPE       1
#define SIZE_MULTI_CHOICES   1
#define SIZE_NEWSGROUP_ID    2
#define SIZE_TIME            4
#define SIZE_ARTICLE_INDEX   4
#define SIZE_LIGHT           1
#define SIZE_ANGLE           2
#define SIZE_ANIMATE         1
#define SIZE_ANIMATE_GROUP   2
#define SIZE_PART            1
#define SIZE_ACTION          1
#define SIZE_HOTSPOT         1
#define SIZE_FILTER          2
#define SIZE_PROJECTILE_FLAGS 2
#define SIZE_PROJECTILE_RESERVED 2

// new defines for dynamic lighting of d3d client
#define LIGHT_FLAG_NONE		0x0000
#define LIGHT_FLAG_ON		0x0001
#define LIGHT_FLAG_DYNAMIC	0x0002
#define LIGHT_FLAG_WAVERING	0x0004

#endif /* #ifndef _PROTO_H */
