// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

#ifndef SCRIPT_CMD_SUCCESS
	#define	SCRIPT_CMD_SUCCESS	true
#endif

#ifndef SCRIPT_CMD_FAILURE
	#define	SCRIPT_CMD_FAILURE	false
#endif

/*==========================================
 * CreativeSD: Queue System
 *------------------------------------------*/
BUILDIN(queue_create)
{
	struct queue_data *queue = NULL;
	const char *event = "", *name = "";
	int queue_id = script_getnum(st,2);

	name = script_getstr(st,3);
	if( script_hasdata(st,4) )
	{
		event = script_getstr(st,4);
		check_event(st, event);
	}

	if( (queue = queue_search(queue_id)) != NULL )
	{
		safestrncpy(queue->name, name, sizeof(queue->name));
		safestrncpy(queue->join_event, event, sizeof(queue->join_event));
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,queue_create(queue_id, name, event));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_delete)
{
	struct queue_data *queue = NULL;
	int queue_id = script_getnum(st,2);
	
	if( (queue = queue_search(queue_id)) == NULL )
	{
		script_pushint(st,0);
		ShowError("script:queue_delete: queue (%d) does not exist\n", queue_id);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,queue_delete(queue_id));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_change)
{
	struct queue_data *queue = NULL;
	const char* change = "";
	int queue_id = script_getnum(st,2), type = script_getnum(st,3);

	if( (queue = queue_search(queue_id)) == NULL )
	{
		script_pushint(st,0);
		ShowError("script:queue_change_event: queue (%d) does not exist\n", queue_id);
		return SCRIPT_CMD_SUCCESS;
	}

	change = script_getstr(st,4);
	if( type )
	{
		check_event(st, change);
		safestrncpy(queue->join_event, change, sizeof(queue->name));
	}
	else {
		safestrncpy(queue->name, change, sizeof(queue->name));
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_check)
{
	struct queue_data *queue = queue_search(script_getnum(st,2));
	script_pushint(st,queue==NULL?0:1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_size)
{
	struct queue_data *queue = queue_search(script_getnum(st,2));
	script_pushint(st,(queue==NULL?0:queue->count));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_join)
{
	struct map_session_data *sd = NULL;
	struct queue_data *queue = NULL;
	int queue_id = script_getnum(st,2);
	int flag = 1, check_delay = 1, delay = 0;

	if( (queue = queue_search(queue_id)) == NULL )
	{
		script_pushint(st,0);
		ShowError("script:queue_join: queue (%d) does not exist\n", queue_id);
		return SCRIPT_CMD_SUCCESS;
	}

	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);

	// Delay
	if( script_hasdata(st,4) )
		check_delay = script_getnum(st,4);

	if (script_hasdata(st,5))
		sd = map->id2sd(script_getnum(st,5));
	else
		sd = script->rid2sd(st);

	if (!sd)
	{
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}
	
	if (sd->queue_id && !queue_leave(sd,2) ) {
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}
	
	if( battle_config.queue_only_towns && !queue_check_map(sd) ) {
		script_pushint(st,-2);
		return SCRIPT_CMD_SUCCESS;
	}

	if( check_delay )
	{
		delay = sd->status.queue_delay-(int)time(NULL);
		if( delay > battle_config.queue_join_delay ) {
			sd->status.queue_delay = (int)time(NULL)+battle_config.queue_join_delay;
			delay = sd->status.queue_delay-(int)time(NULL);
		}

		if( delay > 0 ) {
			script_pushint(st,-3);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	
	script_pushint(st, queue_join(sd,queue_id,flag));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_leave)
{
	struct map_session_data *sd = NULL;
	struct queue_data *queue = NULL;
	int queue_id = 0, flag = 2;

	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);

	if( script_hasdata(st,3) )
		sd = map->id2sd(script_getnum(st,3));
	else
		sd = script->rid2sd(st);
	
	if( !sd || !sd->queue_id )
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	
	queue_id = sd->queue_id;
	
	if( (queue = queue_search(queue_id)) == NULL )
	{
		if( sd->queue_id == queue_id ) sd->queue_id = 0;
		script_pushint(st,1);
		ShowError("script:queue_leave: queue (%d) does not exist\n", queue_id);
		return SCRIPT_CMD_SUCCESS;
	}
	
	script_pushint(st, queue_leave(sd,flag));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_clean)
{
	struct map_session_data *sd;
	struct queue_data *queue = queue_search(script_getnum(st,2));
	int i = 0, flag = 0, delay = -1;

	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
	if( script_hasdata(st,4) )
		delay = script_getnum(st,4);

	if( queue == NULL ) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	while(i < MAX_QUEUE_PLAYERS )
	{
		if( (sd = queue->players[0].sd) != NULL )
		{
			queue_leave(sd,flag);
			if( delay >= 0 )
				sd->status.queue_delay = (int)time(NULL)+delay;
		}
		i++;
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_get_players)
{
	struct map_session_data *sd = NULL;
	struct queue_data *queue = queue_search(script_getnum(st,2));
	int i, j = 0;

	if ( queue == NULL)
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	for (i = 0; i < MAX_QUEUE_PLAYERS; i++) {
		sd = queue->players[i].sd;
		if (sd == NULL || queue->players[i].char_id <= 0)
			continue;

		script->set_reg(st, NULL, reference_uid(script->add_str(".@queue_cid"), j), ".@queue_cid", (void*)queue->players[i].char_id, NULL);
		script->set_reg(st, NULL, reference_uid(script->add_str(".@queue_aid"), j), ".@queue_aid", (void*)queue->players[i].sd->status.account_id, NULL);
		script->set_reg(st, NULL, reference_uid(script->add_str(".@queue_player$"), j), ".@queue_player$", (void*)queue->players[i].sd->status.name, NULL);
		j++;
	}

	script->set_reg(st, NULL, script->add_str(".@queue_count"), ".@queue_count", (void*)j, NULL);
	script_pushint(st, j);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_get_data)
{
	struct queue_data *queue = NULL;
	int queue_id = script_getnum(st,2), type = script_getnum(st,3);

	if( (queue = queue_search(queue_id)) == NULL )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch( type )
	{
		case 0: script_pushstrcopy(st, queue->name); break;
		case 1: script_pushstrcopy(st, queue->join_event); break;
		case 2: script_pushint(st, queue->count); break;
		default:
			ShowError("script:queue_get_data: invalid parameter (%d).\n", type);
			script_pushint(st,0);
			break;
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_char_info)
{
	struct map_session_data *sd = NULL;
	struct queue_data *queue = NULL;
	int type = script_getnum(st,2), delay = 0;

	if (script_hasdata(st,3))
		sd = map->id2sd(script_getnum(st,3));
	else
		sd = script->rid2sd(st);

	if( !sd ) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}
	
	if( sd->queue_id && (queue = queue_search(sd->queue_id)) == NULL )
		sd->queue_id = 0;

	// Update Delay.
	delay = sd->status.queue_delay-(int)time(NULL);
	if( delay > battle_config.queue_join_delay )
		sd->status.queue_delay = (int)time(NULL)+battle_config.queue_join_delay;

	switch( type )
	{
		case 0: script_pushint(st, sd->queue_id); break;
		case 1: script_pushint(st, sd->status.queue_delay); break;
		case 2: script_pushint(st, sd->queue_nomove); break;
		case 3:
			delay = sd->status.queue_delay-(int)time(NULL);
			script_pushint(st, (delay>0?delay:0));
			break;
		case 4: script_pushstrcopy(st, queue_delay(sd->status.queue_delay)); break;
		default:
			ShowError("script:queue_getcharinfo: invalid parameter (%d).\n", type);
			script_pushint(st,0);
			break;
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN(queue_set_delay)
{
	struct map_session_data *sd = NULL;
	struct queue_data *queue = NULL;
	int delay = script_getnum(st,2);

	if (script_hasdata(st,3))
		sd = map->id2sd(script_getnum(st,3));
	else
		sd = script->rid2sd(st);

	if( !sd ) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	sd->status.queue_delay = (int)time(NULL)+delay;

	// Update Delay.
	delay = sd->status.queue_delay-(int)time(NULL);
	if( delay > battle_config.queue_join_delay )
		sd->status.queue_delay = (int)time(NULL)+battle_config.queue_join_delay;

	script_pushint(st,sd->status.queue_delay);
	return SCRIPT_CMD_SUCCESS;
}