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
	camera_t *Camera = &Game->Camera;

	v2 Focus = {0};
	if (TrackedEntity)
	{
		Focus = TrackedEntity->deferred_p;
	}

	if (IsCursorEnabled(&Game->Cursor))
	{
		Focus = GetTileCenter(&Game->Map, Game->Cursor.p);		
	}

	Camera->p = CameraTracking(Camera->p, Focus, GetViewport(Input), dt);

	if (Input->wheel)
	{
		Camera->zoom += (Input->wheel > 0 ? 1.0f : -1.0f) * 0.2f;
	}

	if (Camera->zoom <= 0.0f)
		Camera->zoom = 0.0f;
}