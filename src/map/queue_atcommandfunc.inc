// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: Queue System
 *------------------------------------------*/
ACMD(queue)
{
	int count = 0;
	char output[128];
	nullpo_retr(-1, sd);

	if( sd == NULL )
		return false;

	clif->message(fd, "Filas de Espera:");
	count = queue_atcommand_list(sd);
	
	if( !count )
		clif->message(fd, "Nenhuma Fila de Espera encontrada.");
	else {
		sprintf(output, "Total de %d Filas de Espera.", count);
		clif->message(fd, output);
	}
	return true;
}

ACMD(queuelist)
{
	struct queue_data *queue = NULL;
	struct map_session_data *pl_sd = NULL;
	int queue_id, i=0, j=0;
	char output[128];

	nullpo_retr(-1, sd);

	if( sd == NULL )
		return false;

	if (!message || !*message || sscanf(message, "%d", &queue_id) < 1 || queue_id == 0 || (queue = queue_search(queue_id)) == NULL) {
		clif->message(fd, "Você deve informar a Fila de Espera em que deseja saber os jogadores registrados:");
		queue_atcommand_list(sd);
		return false;
	}

	sprintf(output, "Jogadores aguardando na Fila de Espera em %s:", queue->name);
	clif->message(fd,output);
	for (i = 0; i < MAX_QUEUE_PLAYERS; i++) {
		pl_sd = queue->players[i].sd;
		if (pl_sd == NULL || queue->players[i].char_id <= 0)
			continue;

		sprintf(output, "   (%d) %s | Classe: %s | Nível: %d/%d", pl_sd->status.char_id, pl_sd->status.name, pc->job_name(pl_sd->status.class_), pl_sd->status.base_level, pl_sd->status.job_level);
		clif->message(fd,output);
		j++;
	}

	if( !j ) {
		clif->message(fd,"Nenhum jogador registrado nessa Fila de Espera.");
	}
	return true;
}

ACMD(queuekick)
{
	struct queue_data *queue = NULL;
	struct map_session_data *pl_sd = NULL;
	char output[128];

	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif->message(fd, "Você deve entrar com o nome ou id da conta do personagem (usando: @queuekick <nome do jogador/id>).");
		return false;
	}

	if((pl_sd=map->nick2sd((char *)message)) == NULL && (pl_sd=map->charid2sd(atoi(message))) == NULL)
	{
		clif->message(fd, "O personagem não foi encontrado.");
		return false;
	}

	if( !pl_sd->queue_id || (queue = queue_search(pl_sd->queue_id)) == NULL )
	{
		clif->message(fd, "O jogador não está alistado em uma Lista de Espera para você remove-lo.");
		return false;
	}

	if( queue_leave(pl_sd,3) ) {
		sprintf(output, "O jogador '%s' foi removido da Fila de Espera em %s com sucesso:", pl_sd->status.name, queue->name);
		clif->message(fd, output);
		return true;
	}
	
	sprintf(output, "Houve uma falha ao remover o jogador '%s' da Fila de Espera em %s com sucesso:", pl_sd->status.name, queue->name);
	clif->message(fd, output);
	return false;
}

ACMD(queuesetdelay)
{
	int value = 0, delay = 0;
	char output[128];

	nullpo_retr(-1, sd);

	if( sd == NULL )
		return false;

	if (!message || !*message || sscanf(message, "%d", &value) < 1) {
		clif->message(fd, "Você deve entrar com o valor do delay em segundos (usando: @queuesetdelay <delay>).");
		return false;
	}

	if( value > battle_config.queue_join_delay )
		value = battle_config.queue_join_delay;

	sd->status.queue_delay = (int)time(NULL)+value;
	delay = sd->status.queue_delay-(int)time(NULL);
	clif->message(fd, "Delay para entrada em Fila de Espera alterado com sucesso.");
	if( delay > 0 ) {
		sprintf(output, "Delay da Fila de Espera: %s.", queue_delay(sd->status.queue_delay));
		clif->message(fd, output);
	}
	return true;
}

ACMD(queuedelay)
{
	int delay = 0;
	char output[128];

	nullpo_retr(-1, sd);

	if( sd == NULL )
		return false;

	// Update Delay.
	delay = sd->status.queue_delay-(int)time(NULL);
	if( delay > battle_config.queue_join_delay )
		sd->status.queue_delay = (int)time(NULL)+battle_config.queue_join_delay;

	delay = sd->status.queue_delay-(int)time(NULL);
	if( delay <= 0 )
		clif->message(fd, "Você não está com tempo de espera para entrar em uma Fila de Espera e pode registrar-se normalmente.");
	else {
		sprintf(output, "Delay Fila de Espera: %s.", queue_delay(sd->status.queue_delay));
		clif->message(fd, output);
	}
	return true;
}

ACMD(queuenomove)
{
	nullpo_retr(-1, sd);

	if( sd == NULL )
		return false;

	sd->queue_nomove = sd->queue_nomove ? 0 : 1;
	if( sd->queue_nomove )
		clif->message(fd, "Você não será movido para outra fila de espera caso a que você esteja expire.");
	else
		clif->message(fd, "Você será movido para outra fila de espera caso a que você esteja expire.");
	return true;
}