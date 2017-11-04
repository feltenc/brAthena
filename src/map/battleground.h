/****************************************************************************!
*                _           _   _   _                                       *
*               | |__  _ __ / \ | |_| |__   ___ _ __   __ _                  *
*               | '_ \| '__/ _ \| __| '_ \ / _ \ '_ \ / _` |                 *
*               | |_) | | / ___ \ |_| | | |  __/ | | | (_| |                 *
*               |_.__/|_|/_/   \_\__|_| |_|\___|_| |_|\__,_|                 *
*                                                                            *
*                            www.brathena.org                                *
******************************************************************************
* src/map/battleground.h                                                     *
******************************************************************************
* Copyright (c) brAthena Dev Team                                            *
* Copyright (c) Hercules Dev Team                                            *
* Copyright (c) Athena Dev Teams                                             *
*                                                                            *
* Licenciado sob a licen�a GNU GPL                                           *
* Para mais informa��es leia o arquivo LICENSE na ra�z do emulador           *
*****************************************************************************/

#ifndef MAP_BATTLEGROUND_H
#define MAP_BATTLEGROUND_H

#include "map/map.h" // EVENT_NAME_LENGTH
#include "common/cbasetypes.h"
#include "common/db.h"
#include "common/mmo.h" // struct party

struct block_list;
struct map_session_data;

/**
 * Defines
 **/
#define MAX_BG_MEMBERS 30
#define BG_DELAY_VAR_LENGTH 30

// [CreativeSD]: Names & Version
#define MAX_BG_POSITION 4
static const char bg_version[7] = "6.0.01";
static const char bg_army[2][NAME_LENGTH] = { "Ex�rcito de Guillaume", "Ex�rcito de Croix" };
static const char bg_pos_name[MAX_BG_POSITION][NAME_LENGTH] = { "Capit�o", "Tenente", "Sargento", "Soldado" };

/**
 * Enumerations
 **/
enum bg_queue_types {
	BGQT_INVALID    = 0x0,
	BGQT_INDIVIDUAL = 0x1,
	BGQT_PARTY      = 0x2,
	/* yup no 0x3 */
	BGQT_GUILD      = 0x4,
};

enum bg_team_leave_type {
	BGTL_LEFT = 0x0,
	BGTL_QUIT = 0x1,
	BGTL_AFK  = 0x2,
};

struct battleground_member_data {
	unsigned short x, y;
	struct map_session_data *sd;
	unsigned afk : 1;
	struct point source;/* where did i come from before i join? */
};

struct battleground_data {
	unsigned int bg_id;
	unsigned char count;
	int army, master_id, timerdigit_count, timerdigit; // [CreativeSD]
	struct battleground_member_data members[MAX_BG_MEMBERS];
	// BG Cementery & Respawn
	unsigned short mapindex, x, y, respawn_x, respawn_y;
	// Logout Event
	char logout_event[EVENT_NAME_LENGTH];
	char die_event[EVENT_NAME_LENGTH];
	// Without Event
	char without_event[EVENT_NAME_LENGTH];
};

struct bg_arena {
	char name[NAME_LENGTH];
	unsigned char id;
	char npc_event[EVENT_NAME_LENGTH];
	short min_level, max_level;
	short prize_win, prize_loss, prize_draw;
	short min_players;
	short max_players;
	short min_team_players;
	char delay_var[NAME_LENGTH];
	unsigned short maxDuration;
	int queue_id;
	int begin_timer;
	int fillup_timer;
	int game_timer;
	unsigned short fillup_duration;
	unsigned short pregame_duration;
	bool ongoing;
	enum bg_queue_types allowed_types;
};

struct battleground_interface {
	bool queue_on;
	/* */
	int mafksec, afk_timer_id;
	char gdelay_var[BG_DELAY_VAR_LENGTH];
	/* */
	struct bg_arena **arena;
	unsigned char arenas;
	/* */
	DBMap *team_db; // int bg_id -> struct battleground_data*
	unsigned int team_counter; // Next bg_id
	/* */
	void (*init) (bool minimal);
	void (*final) (void);
	/* */
	struct bg_arena *(*name2arena) (char *name);
	void (*queue_add) (struct map_session_data *sd, struct bg_arena *arena, enum bg_queue_types type);
	enum BATTLEGROUNDS_QUEUE_ACK (*can_queue) (struct map_session_data *sd, struct bg_arena *arena, enum bg_queue_types type);
	int (*id2pos) (int queue_id, int account_id);
	void (*queue_pc_cleanup) (struct map_session_data *sd);
	void (*begin) (struct bg_arena *arena);
	int (*begin_timer) (int tid, int64 tick, int id, intptr_t data);
	void (*queue_pregame) (struct bg_arena *arena);
	int (*fillup_timer) (int tid, int64 tick, int id, intptr_t data);
	void (*queue_ready_ack) (struct bg_arena *arena, struct map_session_data *sd, bool response);
	void (*match_over) (struct bg_arena *arena, bool canceled);
	void (*queue_check) (struct bg_arena *arena);
	struct battleground_data* (*team_search) (int bg_id);
	struct map_session_data* (*getavailablesd) (struct battleground_data *bgd);
	bool (*team_delete) (int bg_id);
	bool (*team_warp) (int bg_id, unsigned short map_index, short x, short y);
	void (*send_dot_remove) (struct map_session_data *sd);
	bool (*team_join) (int bg_id, struct map_session_data *sd, int flag);
	int (*team_leave) (struct map_session_data *sd, enum bg_team_leave_type flag);
	bool (*member_respawn) (struct map_session_data *sd);
	int (*create) (unsigned short map_index, short rx, short ry, short rsx, short rsy, int army, const char *ev, const char *dev, const char *wev);
	int (*team_get_id) (struct block_list *bl);
	bool (*send_message) (struct map_session_data *sd, const char *mes);
	int (*send_xy_timer_sub) (DBKey key, DBData *data, va_list ap);
	int (*send_xy_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*afk_timer) (int tid, int64 tick, int id, intptr_t data);
	/* */
	enum bg_queue_types (*str2teamtype) (const char *str);
	/*==========================================
	 * CreativeSD: BattleGround Queue Expansive
	 *------------------------------------------*/
#include "map/battleground_def.inc"
	/* */
	void (*config_read) (void);
};

struct battleground_interface *bg;

#ifdef BRATHENA_CORE
void battleground_defaults(void);
#endif // BRATHENA_CORE

#endif /* MAP_BATTLEGROUND_H */
