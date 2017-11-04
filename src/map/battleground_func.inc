// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: BattleGround Queue Expansive
 *------------------------------------------*/
// Atualiza Patentes
int bg_refresh_patent(int bg_id)
{
	struct battleground_data *bgd = bg->team_search(bg_id);
	struct map_session_data *sd;
	char output[256];
	int i, j = 0;
	int army;

	if (bgd == NULL)
	{
		ShowError("bg_refresh_patent: Team not found.\n");
		return false;
	}

	army = bgd->army;
	if (army < 0 || army > 1)
	{
		ShowError("bg_refresh_patent: Unknown Army.\n");
		return false;
	}

	for (i = 0; i < MAX_BG_MEMBERS; i++)
	{
		if ((sd = bgd->members[i].sd) == NULL)
			continue;

		sd->bg_position = (j >= MAX_BG_POSITION ? (MAX_BG_POSITION - 1) : j);

		sprintf(output, "Batalhas Campais: Você é %s do %s.", bg_pos_name[sd->bg_position], bg_army[army]);
		clif->messagecolor_self(sd->fd, COLOR_ORANGE, output);
		if (j == 0 )
		{
			bgd->master_id = sd->status.char_id;
			bg->change_skill(sd, true);
		}

		j++;
	}
	return true;
}

// Adiciona ou remove as habilidades das Batalhas Campais.
void bg_change_skill(struct map_session_data *sd, int flag)
{
	if (!flag)
	{
		// Remove Habilidades.
		pc->skill(sd, BG_EMERGENCYCALL, 0, 0);
		pc->skill(sd, BG_RESTORE, 0, 0);
		pc->skill(sd, BG_BATTLEORDER, 0, 0);
		pc->skill(sd, BG_REGENERATION, 0, 0);
	}
	else {
		// Adiciona Habilidades.
		pc->skill(sd, BG_EMERGENCYCALL, 1, 0);
		pc->skill(sd, BG_RESTORE, 1, 0);
		pc->skill(sd, BG_BATTLEORDER, 1, 0);
		pc->skill(sd, BG_REGENERATION, 5, 0);
	}
}

// Bloqueio de Habilidades
void bg_block_skill(struct map_session_data *sd, int time)
{
	uint16 skill_id[] = { BG_BATTLEORDER, BG_REGENERATION, BG_RESTORE, BG_EMERGENCYCALL };
	int bs;
	for (bs = 0; bs < 4; bs++)
		skill->blockpc_start(sd, skill_id[bs], time);
}

// Reportagem de AFK
int bg_report_afk(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd;

	sd = map->id2sd(id);
	if (sd)
	{
		if (sd->bg_afk_timer != INVALID_TIMER && sd->bg_id && !sd->npc_id && DIFF_TICK32(sockt->last_tick, sd->idletime))
		{
			bg->team_leave(sd, BGTL_AFK);
			if (battle_config.bg_afk_warp_save_point)
				pc->setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
			else {
				unsigned short map_index = mapindex->name2id("bat_room");
				if (map_index)
					pc->setpos(sd, map_index, 154, 150, CLR_TELEPORT);
			}
		}
		sd->bg_afk_timer = INVALID_TIMER;
		return 1;
	}
	return 0;
}

// Altera o Capitão da Batalha.
int bg_change_master(int bg_id, int char_id, struct map_session_data *sd)
{
	struct battleground_data *bgd = bg->team_search(bg_id);
	struct map_session_data *tsd;
	int i, x1, y1, x2, y2, m_afk, n_afk, j = 0;
	if (bg == NULL)
		return 0;

	for (i = 0; i < MAX_BG_MEMBERS; i++)
	{
		if ((tsd = bgd->members[i].sd) == NULL)
			continue;

		if (tsd->status.char_id == char_id)
		{
			m_afk = bgd->members[0].afk;
			n_afk = bgd->members[i].afk;
			x1 = bgd->members[0].x;
			y1 = bgd->members[0].y;
			x2 = bgd->members[i].x;
			y2 = bgd->members[i].y;

			bgd->members[0].sd = tsd;
			bgd->members[0].x = x2;
			bgd->members[0].y = y2;
			bgd->members[0].afk = m_afk;

			bgd->members[i].sd = sd;
			bgd->members[i].x = x1;
			bgd->members[i].y = x2;
			bgd->members[i].afk = n_afk;
			bg_refresh_patent(bg_id);
			bg_change_skill(sd,0);
			return 1;
		}
		j++;
	}
	return 0;
}

// Respawn automático.
int bg_respawn_timer(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd;
	struct battleground_data *bgd;
	char cutin[NAME_LENGTH];
	char sound[NAME_LENGTH];
	sd = map->id2sd(id);

	if (sd)
	{
		if (sd->bg_id && (bgd = bg_team_search(sd->bg_id)) != NULL && sd->bg_respawn_timer != INVALID_TIMER )
		{
			if( sd->bg_respawn_timer_count < 0 )
			{
				clif->cutin(sd,"",255);
				sd->bg_respawn_timer = INVALID_TIMER;
				sd->bg_respawn_timer_count = 0;
				pc->percentheal(sd,100,100);
				pc->setpos(sd, bgd->mapindex, bgd->respawn_x, bgd->respawn_y, CLR_OUTSIGHT);
			}
			else {
				sd->bg_respawn_timer_count--;
				sprintf(cutin,"respawn_%d",sd->bg_respawn_timer_count+1);
				sprintf(sound,"respawn_%s.wav",sd->bg_respawn_timer_count<0?"go":"beep");
				clif->cutin(sd,cutin,2);
				clif->soundeffect(sd,&sd->bl,sound,0);
				sd->bg_respawn_timer = timer->add(timer->gettick() + 1000, bg->respawn_timer, sd->bl.id, 0);
			}
		}
		else {
			sd->bg_respawn_timer = INVALID_TIMER;
			sd->bg_respawn_timer_count = 0;
		}
		return 1;
	}
	return 0;
}

void bg_respawn_timer_delete(struct map_session_data *sd)
{
	sd->bg_respawn_timer = INVALID_TIMER;
	sd->bg_respawn_timer_count = 0;
	clif->cutin(sd,"",255);
}

int bg_digit_timer(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd = NULL;
	struct battleground_data *bgd = bg->team_search(id);
	int i;
	
	if( bgd == NULL && bgd->timerdigit != INVALID_TIMER )
		return 0;

	for (i = 0; i < MAX_BG_MEMBERS; i++)
	{
		if ((sd = bgd->members[i].sd) == NULL)
			continue;

		if( bgd->timerdigit_count > 0 )
			clif->showdigit(sd, (unsigned char)2, -(bgd->timerdigit_count));
		else
			clif->showdigit(sd, (unsigned char)3, 0);
	}

	if( bgd->timerdigit_count <= 0 )
		bgd->timerdigit = INVALID_TIMER;
	else {
		bgd->timerdigit_count--;
		bgd->timerdigit = timer->add(timer->gettick() + 1000, bg->digit_timer, bgd->bg_id, 0);
	}

	return 1;
}