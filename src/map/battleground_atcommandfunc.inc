// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: BattleGround Queue Expansive
 *------------------------------------------*/
ACMD(reloadbg)
{
	struct npc_data *nd = npc->name2id("BattleGround#queue_main");
	if (nd == NULL)
	{
		clif->message(sd->fd, "Impossível de recarregar as Batalhas Campais, o script principal não está ativado.");
		return true;
	}

	npc->event(sd, "BattleGround#queue_main::OnCmdReload", 0);
	clif->messagecolor_self(sd->fd, COLOR_DEFAULT, "Uma sinalização foi enviada para recarregar as Batalhas Campais.");
	return true;
}

ACMD(bgregister)
{
	struct npc_data *nd = npc->name2id("BattleGround#cmd_register");

	if (nd == NULL)
	{
		clif->message(sd->fd, "Não foi possível abrir o Registro das Batalhas Campais.");
		return true;
	}

	script->run(nd->u.scr.script, 0, sd->bl.id, nd->bl.id);
	return true;
}

ACMD(bgjoin)
{
	struct npc_data *nd = npc->name2id("BattleGround#cmd_join");

	if (nd == NULL)
	{
		clif->message(sd->fd, "Não foi possível utilizar o Registro rápido das Batalhas Campais.");
		return true;
	}

	script->run(nd->u.scr.script, 0, sd->bl.id, nd->bl.id);
	return true;
}

ACMD(bgleave)
{
	int16 m = sd->bl.m;
	if (sd->bg_id)
	{
		if (map->list[m].flag.battleground)
		{
			bg->team_leave(sd, BGTL_QUIT);
			if (!sd->bg_id)
			{
				if (battle_config.bg_afk_warp_save_point)
					pc->setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
				else {
					unsigned short map_index = mapindex->name2id("bat_room");
					if (map_index)
						pc->setpos(sd, map_index, 154, 150, CLR_TELEPORT);
				}
			}
			else {
				clif->message(sd->fd, "Não foi possível remove-lo da batalha atual.");
				return false;
			}
			return true;
		}
		else {
			// Não ativar labeis de eventos por estar fora da arena.
			bg->team_leave(sd, BGTL_LEFT);
			if ( sd->bg_id )
			{
				clif->message(sd->fd, "Removido da batalha com sucesso.");
				return true;
			}
			else {
				clif->message(sd->fd, "Não foi possível remove-lo da batalha atual.");
				return false;
			}
			return true;
		}
	}
	else {
		clif->message(sd->fd, "Você não está participando de uma Batalha Campais.");
		return false;
	}
}

ACMD(bgunregister)
{
	struct npc_data *nd = npc->name2id("BattleGround#cmd_unregister");

	if (nd == NULL)
	{
		clif->message(sd->fd, "Não foi possível abrir o Registro das Batalhas Campais.");
		return true;
	}

	script->run(nd->u.scr.script, 0, sd->bl.id, nd->bl.id);
	return true;
}

ACMD(bgreportafk)
{
	struct map_session_data *pl_sd = NULL;
	int64 tick;
	char output[128];

	if (!*message)
	{
		clif->message(fd, "Utilize @bgreportafk <player_name> para reportar um jogador que está ausente na batalha.");
		return false;
	}

	if ((pl_sd = map->nick2sd((char *)message)) == NULL && (pl_sd = map->charid2sd(atoi(message))) == NULL)
	{
		clif->messages(fd, "Jogador %s não encontrado", message);
		return false;
	}

	if (!pl_sd->bg_id)
	{
		clif->messages(fd, "O Jogador %s não está em batalha.", message);
		return false;
	}

	if (battle_config.bg_afk_team_report && sd->bg_id != pl_sd->bg_id)
	{
		clif->message(fd, "Você não está no mesmo Exército que o jogador.");
		return false;
	}

	if (pl_sd->npc_id)
	{
		clif->message(fd, "O Jogador não está ausente, no momento está integarindo com um NPC.");
		return false;
	}

	if (!DIFF_TICK32(sockt->last_tick, pl_sd->idletime))
	{
		clif->message(fd, "O Jogador não está ausente.");
		return false;
	}

	if (pl_sd->bg_afk_timer && pl_sd->bg_afk_timer != INVALID_TIMER)
	{
		clif->message(fd, "O Jogador já foi reportado ao sistema.");
		return false;
	}

	tick = timer->gettick();

	clif->message(fd, "O Jogador foi reportado com sucesso.");
	sprintf(output, "Batalhas Campais : Você foi reportado por ausência na batalha, você tem %d segundos para entrar em interação.", battle_config.bg_afk_timer);
	clif->messagecolor_self(pl_sd->fd, COLOR_ORANGE, output);

	pl_sd->bg_afk_timer = timer->add(tick + (battle_config.bg_afk_timer*1000), bg->report_afk, pl_sd->bl.id, 0);
	return true;
}

ACMD(bgwaiting)
{
	struct npc_data *nd;
	struct script_state *st;
	int arg1 = 0, arg2 = 0, count = 0, nargs = 0 ;

	if (!strcmpi(info->command, "bgwaiting"))
	{
		nd = npc->name2id("BattleGround#cmd_waiting");
		if (nd == NULL)
		{
			clif->message(sd->fd, "Não foi possível abrir a Lista de Espera das Batalhas Campais.");
			return true;
		}
	}
	else
	{
		nd = npc->name2id("BattleGround#cmd_playing");
		if (nd == NULL)
		{
			clif->message(sd->fd, "Não foi possível abrir a Lista de Jogadores das Batalhas Campais.");
			return true;
		}
	}

	if (*message)
		count = sscanf(message, "%2d %1d", &arg1, &arg2);

	st = script->alloc_state(nd->u.scr.script, 0, sd->bl.id, nd->bl.id);

	if (count > 0)
		script->setd_sub(st, NULL, ".@atcmd_parameters", nargs++, (void *)h64BPTRSIZE(arg1), NULL);

	if (count > 1)
		script->setd_sub(st, NULL, ".@atcmd_parameters", nargs++, (void *)h64BPTRSIZE(arg2), NULL);

	script->setd_sub(st, NULL, ".@atcmd_numparameters", 0, (void *)h64BPTRSIZE(nargs), NULL);
	script->run_main(st);
	return true;
}

ACMD(bgranking)
{
	struct npc_data *nd;
	struct script_state *st;
	int arg1 = 0, arg2 = 0, arg3 = 0, count = 0, nargs = 0;

	nd = npc->name2id("BattleGround#cmd_ranking");
	if (nd == NULL)
	{
		clif->message(sd->fd, "Não foi possível abrir o Ranking das Batalhas Campais.");
		return true;
	}

	if (*message)
		count = sscanf(message, "%2d %1d %2d", &arg1, &arg2, &arg3);

	st = script->alloc_state(nd->u.scr.script, 0, sd->bl.id, nd->bl.id);

	if (count > 0)
		script->setd_sub(st, NULL, ".@atcmd_parameters", nargs++, (void *)h64BPTRSIZE(arg1), NULL);

	if (count > 1)
		script->setd_sub(st, NULL, ".@atcmd_parameters", nargs++, (void *)h64BPTRSIZE(arg2), NULL);

	if (count > 2)
		script->setd_sub(st, NULL, ".@atcmd_parameters", nargs++, (void *)h64BPTRSIZE(arg3), NULL);

	script->setd_sub(st, NULL, ".@atcmd_numparameters", 0, (void *)h64BPTRSIZE(nargs), NULL);
	script->run_main(st);
	return true;
}

ACMD(bgorder)
{
	struct battleground_data *bgd = NULL;
	nullpo_retr(-1,sd);

	if( !message || !*message )
	{
		clif->message(fd, "Você deve digitar a mensagem (usando: @order <mensagem>).");
		return false;
	}

	if( map->list[sd->bl.m].flag.battleground || !sd->bg_id )
	{
		bgd = bg->team_search(sd->bg_id);
		if( bgd == NULL && bgd->master_id != sd->status.char_id )
		{
			clif->message(fd, "Este comando é somente para os Capitões de Exército.");
			return false;
		}
		clif->broadcast2(&sd->bl, message, (int)strlen(message)+1, COLOR_ORANGE, 0x190, 12, 0, 0, BG);
	}
	else {
		clif->message(fd, "Você tem que estar em uma batlha para enviar uma mensagem para seu Exército.");
		return false;
	}

	return true;
}

ACMD(bgchangeleader)
{
	struct battleground_data *bgd = NULL;
	struct map_session_data *pl_sd = NULL;
	struct map_session_data *tsd = NULL;
	char output[128];
	int i, showlist = 0;

	nullpo_retr(-1, sd);

	if (!sd->bg_id)
	{
		clif->message(fd, "Você não está participando de uma Batalha.");
		return false;
	}

	bgd = bg->team_search(sd->bg_id);
	if (bgd == NULL || bgd->master_id != sd->status.char_id )
	{
		clif->message(fd, "Somente o Capitão do Exército pode utilizar este comando.");
		return false;
	}
	else if (bgd->count <= 1)
	{
		clif->message(fd, "Não há mais jogadores em seu Exército para você nomear como Capitão.");
		return false;
	}
	else if (!message || !*message)
		showlist = 1;
	else if ((pl_sd = map->nick2sd((char *)message)) == NULL || pl_sd->bg_id != sd->bg_id) {
		clif->message(fd, "Este jogador não está no mesmo time que você.");
		showlist = 1;
	}
	else {
		if (bg->change_master(sd->bg_id, pl_sd->status.char_id, sd))
		{
			sprintf(output, "O Jogador %s foi nomeado Capitão do Exército.", pl_sd->status.name);
			clif->message(fd, output);
			return false;
		}
		else {
			sprintf(output, "Não foi possível nomear o Jogador %s como Capitão do Exército.", pl_sd->status.name);
			clif->message(fd, output);
			showlist = 1;
		}
	}

	if (showlist)
	{
		clif->message(fd, "Você deve digitar o jogador que deseja (usando: @bgchangeleader <Nickname>).");
		for (i = 0; i < bgd->count; i++)
		{
			if (bgd->members[i].sd == sd)
				continue;

			tsd = bgd->members[i].sd;
			if (tsd == NULL)
				continue;

			sprintf(output, "Jogador: %s | Nível: %d/%d | Classe: %s", tsd->status.name, tsd->status.base_level, tsd->status.job_level, pc->job_name(tsd->status.class_));
			clif->message(fd, output);
		}
	}

	return true;
}