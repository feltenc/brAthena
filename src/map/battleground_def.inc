// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: BattleGround Queue Expansive
 *------------------------------------------*/
int (*refresh_patent) (int bg_id);
void (*change_skill) (struct map_session_data *sd, int flag);
void (*block_skill) (struct map_session_data *sd, int time);
int (*report_afk) (int tid, int64 tick, int id, intptr_t data);
int (*change_master) (int bg_id, int char_id, struct map_session_data *sd);
int (*respawn_timer) (int tid, int64 tick, int id, intptr_t data);
void (*respawn_timer_delete) (struct map_session_data *sd);
int (*digit_timer) (int tid, int64 tick, int id, intptr_t data);