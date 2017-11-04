// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "map/map.h" // EVENT_NAME_LENGTH

// Está definição deve estar de acordo com o 'SCRIPT_MAX_ARRAYSIZE', caso você aumente esta definição,
// variáveis vetorizadas de scripts não irão aceitar a indexação e haverá perca de dados em processos de scripts.
//
// Is setting should be in accordance with the 'SCRIPT_MAX_ARRAYSIZE' if you increase this setting,
// variable vectorized scripts will not accept indexing and will miss data in scripting processes.
#define MAX_QUEUE_PLAYERS	127

#define	QUEUE_NAME_LENGTH	30

#ifndef COLOR_ORANGE
	#define COLOR_ORANGE  0xFFA500
#endif

#ifndef COLOR_BABYBLUE
	#define	COLOR_BABYBLUE	0x89cff0
#endif

struct queue_players {
	struct map_session_data *sd;
	int char_id;
};

struct queue_data {
	unsigned int queue_id;
	unsigned char count;
	char name[QUEUE_NAME_LENGTH];
	char join_event[EVENT_NAME_LENGTH];
	struct queue_players players[MAX_QUEUE_PLAYERS];
};

/* [CreativeSD]: Queue System */
struct queue_data *queue_search(int queue_id);
int queue_create(int queue_id, const char *name, const char *event);
int queue_delete(int queue_id);
int queue_join(struct map_session_data *sd, int queue_id, int flag);
int queue_leave(struct map_session_data *sd, int flag);
int queue_clean(int queue_id, int flag);
int queue_check_map(struct map_session_data *sd);
int queue_atcommand_list(struct map_session_data *sd);
void queue_join_notify(int queue_id, struct map_session_data *sd);
void queue_leave_notify(int queue_id, struct map_session_data *sd, int flag);
const char* queue_delay(int delay);

void do_init_queue(void);
void do_final_queue(void);

#endif /* _QUEUE_H_ */
