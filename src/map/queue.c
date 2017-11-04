// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

#define BRATHENA_CORE

#include "queue.h"

#include "common/cbasetypes.h"
#include "common/timer.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"

#include "map/battle.h"
#include "map/clif.h"
#include "map/map.h"
#include "map/npc.h"
#include "map/pc.h"

#include <string.h>
#include <stdio.h>

struct DBMap* queue_db;
static unsigned int queue_counter = 0;

struct queue_data* queue_search(int queue_id)
{
	struct queue_data* queue;
	struct DBIterator *iter = db_iterator(queue_db);

	for( queue = dbi_first(iter); dbi_exists(iter); queue = dbi_next(iter) )
	{
		if( queue->queue_id == queue_id )
			break;
	}
	dbi_destroy(iter);
	return queue;
}

int queue_create(int queue_id, const char *name, const char *event)
{
	struct queue_data *queue;
	queue_counter++;
	
	CREATE(queue, struct queue_data, 1);
	queue->queue_id = queue_id;
	queue->count = 0;
	memset(&queue->players, 0, sizeof(queue->players));
	safestrncpy(queue->name, name, sizeof(queue->name));
	safestrncpy(queue->join_event, event, sizeof(queue->join_event));
	idb_put(queue_db, queue_counter, queue);
	ShowStatus("queue_create: The queue (%d: %s) was successfully created.\n", queue->queue_id, queue->name);
	return true;
}

int queue_delete(int queue_id)
{
	int i;
	struct map_session_data *sd;
	struct queue_data *queue = queue_search(queue_id);
	char name[QUEUE_NAME_LENGTH];

	if( queue == NULL ) return 0;
	for( i = 0; i < MAX_QUEUE_PLAYERS; i++ )
	{
		if( (sd = queue->players[i].sd) == NULL )
			continue;

		sd->queue_id = 0;
	}
	queue_counter--;
	safestrncpy(name, queue->name, sizeof(name));
	idb_remove(queue_db, queue_id);
	ShowStatus("queue_delete: The row (%d: %s) was successfully removed.\n", queue->queue_id, name);
	return true;
}

int queue_join(struct map_session_data *sd, int queue_id, int flag) {
	int i;
	struct queue_data *queue = queue_search(queue_id);

	if (queue == NULL || sd == NULL)
		return false;

	ARR_FIND(0, MAX_QUEUE_PLAYERS, i, (queue->players[i].sd == NULL || queue->players[i].sd == 0));
	if (i == MAX_QUEUE_PLAYERS)
		return false; // No free slots

	if( sd->queue_id )
		queue_leave(sd,0);

	sd->queue_id = queue_id;
	sd->status.queue_delay = (int)time(NULL)+battle_config.queue_join_delay;
	queue->players[i].sd = sd;
	queue->players[i].char_id = sd->status.char_id;
	queue->count++;
	ShowInfo("Player '%s' joined in queue (%d: %s).\n", sd->status.name, queue_id, queue->name);

	if( battle_config.queue_notify == 1 || battle_config.queue_notify == 3 )
		queue_join_notify(queue_id, sd);

	if( queue->join_event[0] && flag )
	{
		if( !npc->event_do(queue->join_event) )
			ShowError("queue_join: '%s' not found in (queue_id: %d)!\n", queue->join_event, queue_id);
	}
	return true;
}

int queue_leave(struct map_session_data *sd, int flag)
{
	struct queue_data *queue = NULL;
	int queue_id, i, size;
	
	if( sd == NULL)
		return false;

	queue_id = sd->queue_id;
	queue = queue_search(queue_id);

	if( queue == NULL )
		return false;

	size = queue->count;
	ARR_FIND( 0, size, i, queue->players[i].sd == sd );
	if( i < size )
	{
		memmove(&queue->players[i], &queue->players[i+1], sizeof(queue->players[0])*(queue->count-i));
		memset(&queue->players[queue->count], 0, sizeof(queue->players));
		queue->count--;
		sd->queue_id = 0;
		ShowInfo("Player '%s' removed from queue (%d: %s).\n", sd->status.name, queue_id, queue->name);
		if( flag && battle_config.queue_notify >= 2 )
			queue_leave_notify(queue_id, sd, flag);
		return true;
	}
	return false;
}

int queue_clean(int queue_id, int flag)
{
	struct map_session_data *sd;
	struct queue_data *queue = queue_search(queue_id);
	int i;
	
	if( queue == NULL )
		return false;
	else {
		for( i = 0; i < MAX_QUEUE_PLAYERS; i++ )
		{
			if( (sd = queue->players[i].sd) == NULL )
				continue;
			else
				queue_leave(sd,flag);
		}
	}
	return true;
}

int queue_check_map(struct map_session_data *sd)
{
	int m = map->list[sd->bl.m].index;
	if( mapindex->name2id("prontera") ==  m ||
		mapindex->name2id("morocc") == m ||
		mapindex->name2id("geffen") == m ||
		mapindex->name2id("payon") == m ||
		mapindex->name2id("alberta") == m ||
		mapindex->name2id("izlude") == m ||
		mapindex->name2id("aldebaran") == m ||
		mapindex->name2id("xmas") == m ||
		mapindex->name2id("comodo") == m ||
		mapindex->name2id("yuno") == m ||
		mapindex->name2id("amatsu") == m ||
		mapindex->name2id("gonryun") == m ||
		mapindex->name2id("umbala") == m ||
		mapindex->name2id("niflheim") == m ||
		mapindex->name2id("louyang") == m ||
		mapindex->name2id("new_1-1") == m ||
		mapindex->name2id("jawaii") == m ||
		mapindex->name2id("ayothaya") == m ||
		mapindex->name2id("einbroch") == m ||
		mapindex->name2id("lighthalzen") == m ||
		mapindex->name2id("einbech") == m ||
		mapindex->name2id("hugel") == m ||
		mapindex->name2id("rachel") == m ||
		mapindex->name2id("veins") == m ||
		mapindex->name2id("moscovia") == m ||
		mapindex->name2id("mid_camp") == m ||
		mapindex->name2id("manuk") == m ||
		mapindex->name2id("splendide") == m ||
		mapindex->name2id("brasilis") == m ||
		mapindex->name2id("dicastes01") == m ||
		mapindex->name2id("mora") == m ||
		mapindex->name2id("dewata") == m ||
		mapindex->name2id("malangdo") == m ||
		mapindex->name2id("malaya") == m ||
		mapindex->name2id("eclage") == m
	) {
		return true;
	}
	return false;
}

int queue_atcommand_list(struct map_session_data *sd)
{
	struct queue_data* queue = NULL;
	struct DBIterator *iter = db_iterator(queue_db);
	char output[128];
	int count = 0;

	for( queue = dbi_first(iter); dbi_exists(iter); queue = dbi_next(iter) )
	{
		sprintf(output, "    %d - %s", queue->queue_id, queue->name);
		clif->message(sd->fd, output);
		count++;
	}
	dbi_destroy(iter);
	return count;
}

void queue_join_notify(int queue_id, struct map_session_data *sd)
{
	struct queue_data *queue = NULL;
	struct map_session_data *pl_sd;
	char output[256];
	int i;

	queue = queue_search(queue_id);
	if( queue == NULL )
		return;

	for( i = 0; i < MAX_QUEUE_PLAYERS; i++ )
	{
		if( (pl_sd = queue->players[i].sd) == NULL || pl_sd == sd )
			continue;

		sprintf(output, "O Jogador '%s' ingressou na Fila de Espera em %s.", sd->status.name, queue->name);
		clif->messagecolor_self(pl_sd->fd, COLOR_BABYBLUE, output);
	}

	sprintf(output, "Você ingressou na Fila de Espera em %s.", queue->name);
	clif->messagecolor_self(sd->fd, COLOR_BABYBLUE, output);
}

void queue_leave_notify(int queue_id, struct map_session_data *sd, int flag)
{
	struct queue_data *queue = NULL;
	struct map_session_data *pl_sd;
	char output[256];
	int i;

	if( flag <= 0 || flag >= 8 )
		return;

	queue = queue_search(queue_id);
	if( queue == NULL )
		return;

	for( i = 0; i < MAX_QUEUE_PLAYERS && flag < 5; i++ )
	{
		if( (pl_sd = queue->players[i].sd) == NULL || pl_sd == sd )
			continue;

		switch(flag)
		{
			case 1:
				sprintf(output, "O Jogador '%s' saiu da Fila de Espera em %s por não estar em uma Cidade.", sd->status.name, queue->name);
				break;
			case 2:
				sprintf(output, "O Jogador '%s' saiu da Fila de Espera em %s.", sd->status.name, queue->name);
				break;
			case 3:
				sprintf(output, "O Jogador '%s' foi expluso da Fila de Espera em %s.", sd->status.name, queue->name);
				break;
			case 4:
				sprintf(output, "O jogador '%s' foi removido da Fila de Espera em %s por desconectar-se.", sd->status.name, queue->name);
				break;
		}
		clif->messagecolor_self(pl_sd->fd, COLOR_BABYBLUE, output);
	}

	switch(flag)
	{
		case 1:
			sprintf(output, "Você foi removido da Fila de Espera em %s por ter saido da Cidade.", queue->name);
			break;
		case 2:
			sprintf(output, "Você saiu da Fila de Espera em %s.", queue->name);
			break;
		case 3:
			sprintf(output, "Você foi expulso da Fila de Espera em %s.", queue->name);
			break;
		case 5:
			sprintf(output, "Você foi removido da Fila de Espera em %s por não atender os requisitos.", queue->name);
			break;
		case 6:
			sprintf(output, "Você foi removido da Fila de Espera em %s e deve aguardar %s para registrar-se novamente.", queue->name, queue_delay(sd->status.queue_delay));
			break;
		case 7:
			sprintf(output, "Você foi removido da Fila de Espera em %s porque ela expirou, mas você já pode registrar-se em uma nova.", queue->name);
			break;
		default:
			return; // Nenhuma mensagem para o jogador.
	}

	clif->messagecolor_self(sd->fd, COLOR_BABYBLUE, output);
}

const char* queue_delay(int delay) {
	int timer = delay-(int)time(NULL);
	int sec = 0, min = 0, hour = 0, days = 0;
	static char output[128];

	if( timer < 60 ) {
		sprintf(output, "%d segundos", timer);
		return ((const char*)output);
	}
	
	min = timer/60;
	sec = timer - (min*60);

	if( min < 60 ) {
		sprintf(output, "%d minutos e %d segundos", min, sec);
		return ((const char*)output);
	}

	hour = min/60;
	min = min - (hour*60);

	if( hour < 24 ) {
		sprintf(output, "%d horas %d minutos e %d segundos", hour, min, sec);
		return ((const char*)output);
	}

	days = hour/24;
	hour = hour - (days*24);

	sprintf(output, "%d dias %d horas %d minutos e %d segundos", days, hour, min, sec);
	return ((const char*)output);
}

void do_init_queue(void)
{
	queue_db = idb_alloc(DB_OPT_RELEASE_DATA);
	ShowMessage(CL_WHITE"[Queue System]:"CL_RESET" System Queue successfully initialized.\n");
	ShowMessage(CL_WHITE"[Queue System]:"CL_RESET" by (c) CreativeSD, suport in "CL_GREEN"www.creativesd.com.br"CL_RESET"\n");
}

void do_final_queue(void)
{
	queue_db->destroy(queue_db, NULL);
}
