fn v2 CameraTracking(v2 p, v2 player_world_pos, v2 viewport, f32 dt)
{
	v2 player_iso_pos = ScreenToIso(player_world_pos);
	
	v2 screen_center = Scale(viewport, 0.5f);
	v2 camera_offset = Sub(screen_center, player_iso_pos);
	p = Lerp2(p, camera_offset, 5.0f * dt);
	return p;
}

fn void Camera(game_state_t *Game, entity_t *TrackedEntity, const client_input_t *Input, f32 dt)
{
	camera_t *Camera = Game->Camera;
	turn_system_t *System = Game->System;
	map_t *Map = Game->Map;

	// NOTE(): We're focusing the Camera either on a cursor or on a player position,
	// depending on the current mode.
	v2 focus_p = Game->cursor->active ? GetTileCenter(Map, Game->cursor->p) : TrackedEntity->deferred_p;
	
	Game->Camera->p = CameraTracking(Game->Camera->p, focus_p, GetViewport(Input), dt);

	f32 delta = 0.0f;
	if (Input->wheel)
		delta = Input->wheel > 0 ? 1.0f : -1.0f;
	delta *= 0.2f;

	Game->Camera->zoom += delta;
	if (Game->Camera->zoom <= 0.0f)
		Game->Camera->zoom = 0.0f;
}