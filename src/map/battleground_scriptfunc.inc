// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: BattleGround Queue Expansive
 *------------------------------------------*/
BUILDIN(bg_kick_team)
{
	int i, bg_id = script_getnum(st,2);
	struct map_session_data *sd;
	struct battleground_data *bgd = bg->team_search(bg_id);
	
	if( bg == NULL )
		script_pushint(st,0); // Equipe não foi encontrada ou a id é inválida.
	else {
		for( i = 0; i < MAX_BG_MEMBERS; i++ )
		{
			if( (sd = bgd->members[i].sd) == NULL )
				continue;
			else
				bg->team_leave(sd, BGTL_LEFT);
		}
		
		script_pushint(st,1);
	}
	return true;
}

BUILDIN(bg_team_size)
{
	struct battleground_data *bgd = bg->team_search(script_getnum(st,2));
	struct map_session_data *sd = NULL;
	int i, j = 0;

	if ( bgd == NULL)
	{
		script_pushint(st, 0);
		return true;
	}

	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		sd = bgd->members[i].sd;
		if (sd == NULL)
			continue;

		j++;
	}
	script_pushint(st, j);
	return true;
}

BUILDIN(bg_get_team)
{
	struct battleground_data *bgd = bg->team_search(script_getnum(st,2));
	struct map_session_data *sd = NULL;
	int bg_id = script_getnum(st, 2);
	int i, j = 0;

	if ( bgd == NULL)
	{
		script_pushint(st,0);
		return true;
	}

	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		sd = bgd->members[i].sd;
		if (sd == NULL)
			continue;

		script->set_reg(st, NULL, reference_uid(script->add_str(".@bg_members_cid"), j), ".@bg_members_cid", (void*)sd->status.char_id, NULL);
		script->set_reg(st, NULL, reference_uid(script->add_str(".@bg_members_aid"), j), ".@bg_members_aid", (void*)sd->status.account_id, NULL);
		j++;
	}

	script->set_reg(st, NULL, script->add_str(".@bg_members_count"), ".@bg_members_count", (void*)j, NULL);
	script_pushint(st, j);
	return true;
}

BUILDIN(bg_set_respawn)
{
	struct battleground_data *bgd = bg->team_search(script_getnum(st,2));

	if( bgd == NULL )
	{
		script_pushint(st, 0);
		return true;
	}

	bgd->respawn_x = script_getnum(st,3);
	bgd->respawn_y = script_getnum(st,4);
	script_pushint(st,1);
	return true;
}

BUILDIN(bg_attach_respawn)
{
	struct map_session_data *sd = NULL;
	struct battleground_data *bgd = NULL;
	int timer_r = 1;

	if( script_hasdata(st,2) )
		timer_r = script_getnum(st,2);

	if (script_hasdata(st,3))
		sd = map->charid2sd(script_getnum(st,3));
	else
		sd = script->rid2sd(st);

	if( sd == NULL || !sd->bg_id || (bgd = bg->team_search(sd->bg_id)) == NULL )
	{
		script_pushint(st, 0);
		return true;
	}

	sd->bg_respawn_timer_count = 10;
	sd->bg_respawn_timer = timer->add(timer->gettick()+timer_r, bg->respawn_timer, sd->bl.id, 0);
	script_pushint(st,1);
	return true;
}

BUILDIN(bg_attach_respawn_all)
{
	struct map_session_data *sd = NULL;
	struct battleground_data *bgd = bg->team_search(script_getnum(st,2));
	int timer_r = 1, i;

	if( bgd == NULL )
	{
		script_pushint(st,0);
		return true;
	}

	if( script_hasdata(st,3) )
		timer_r = script_getnum(st,3);

	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		sd = bgd->members[i].sd;
		if (sd == NULL)
			continue;

		sd->bg_respawn_timer_count = 10;
		sd->bg_respawn_timer = timer->add(timer->gettick()+timer_r, bg->respawn_timer, sd->bl.id, 0);
	}
	script_pushint(st,1);
	return true;
}

BUILDIN(bg_viewpoint)
{
	struct battleground_data *bgd;
	struct map_session_data *sd = NULL;
	int bg_id, type, x, y, id, color, i;
	
	bg_id = script_getnum(st,2);
	type = script_getnum(st,3);
	x = script_getnum(st,4);
	y = script_getnum(st,5);
	id = script_getnum(st,6);
	color = script_getnum(st,7);

	bgd = bg->team_search(bg_id);

	if (bgd == NULL)
	{
		script_pushint(st, 0);
		return true;
	}
	
	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		sd = bgd->members[i].sd;
		if (sd == NULL)
			continue;

		clif->viewpoint(sd, st->oid, type, x, y, id, color);
	}

	script_pushint(st,1);
	return true;
}

BUILDIN(bg_areapercentheal)
{
	const char *str;
	int16 m, x0, y0, x1, y1;
	int bg_id, hp, sp;
	int i = 0;
	struct battleground_data *bgd = NULL;
	struct map_session_data *sd = NULL;

	bg_id = script_getnum(st,2);
	str = script_getstr(st,3);

	if( (bgd = bg->team_search(bg_id)) == NULL || (m = map->mapname2mapid(str)) < 0 ) {
		script_pushint(st,0);
		return true;
	}

	x0 = script_getnum(st,4);
	y0 = script_getnum(st,5);
	x1 = script_getnum(st,6);
	y1 = script_getnum(st,7);
	hp = script_getnum(st,8);
	sp = script_getnum(st,9);

	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		sd = bgd->members[i].sd;
		if (sd == NULL)
			continue;
		if (sd->bl.m != m || sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1)
			continue;

		pc->percentheal(sd,hp,sp);
	}
	return true;
}

BUILDIN(bg_team_getxy)
{
	struct battleground_data *bgd = bg->team_search(script_getnum(st,2));

	if( bgd == NULL )
		return true;

	if( !script_getnum(st,3) )
		script_pushint(st,bgd->x);
	else
		script_pushint(st,bgd->y);
	return true;
}

BUILDIN(bg_respawn_getxy)
{
	struct battleground_data *bgd = bg->team_search(script_getnum(st,2));

	if( bgd == NULL )
		return true;

	if( !script_getnum(st,3) )
		script_pushint(st,bgd->respawn_x);
	else
		script_pushint(st,bgd->respawn_y);
	return true;
}

BUILDIN(bg_refresh_patent)
{
	struct battleground_data *bgd;
	int bg_id = script_getnum(st,2);

	if (!battle_config.bg_name_position)
		return true;

	bgd = bg->team_search(bg_id);

	if (bgd == NULL)
		return true;

	bg->refresh_patent(bg_id);
	return true;
}

BUILDIN(bg_message)
{
	const char *msg;
	int i;
	struct map_session_data *sd;
	struct battleground_data *bgd = bg->team_search(script_getnum(st,2));

	if (bg == NULL)
		return true;

	msg = script_getstr(st,3);
	for (i = 0; i < MAX_BG_MEMBERS; i++)
	{
		if ((sd = bgd->members[i].sd) == NULL)
			continue;
	
		clif->messagecolor_self(sd->fd, COLOR_ORANGE, msg);
	}
	return true;
}

BUILDIN(bg_digit_timer)
{
	struct map_session_data* sd = NULL;
	struct battleground_data *bgd = bg->team_search(script_getnum(st,2));
	int value = 0;

	if (bgd == NULL)
		return true;

	if( script_hasdata(st,3) )
		value = script_getnum(st,3);

	if( value > 0 ) {
		bgd->timerdigit_count = value+1;
		bgd->timerdigit = timer->add(timer->gettick() + 1, bg->digit_timer, bgd->bg_id, 0);
	}
	else {
		bgd->timerdigit_count = 0;
		bgd->timerdigit = timer->add(timer->gettick(), bg->digit_timer, bgd->bg_id, 0);
	}
	return true;
}

BUILDIN(bg_console)
{
	const char *str;
	str=script_getstr(st,2);
	ShowMessage(CL_WHITE"[BattleGround]:"CL_RESET" %s\n",str);
	return true;
}