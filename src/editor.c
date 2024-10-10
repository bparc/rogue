
fn void MakeSimpleMap(game_world_t *world)
{
	#define X 20
	#define Y 20
	// # = Traversables
	// W = Walls
    // S = Slime
	char Data[Y][X] =
	{
		' ',' ',' ',' ',' ',' ',' ',' ',' ','#','#','#','#','#','#',' ','#','#','#',' ',
		' ',' ','#','#','#',' ',' ',' ',' ',' ','#','#','#','#',' ',' ','#','#','#',' ',
		' ','#','#','#','#','#',' ',' ',' ',' ','#','#','#',' ',' ',' ','#','#','#',' ',
		'#','#','#','#','#','#','#',' ',' ',' ','#','#','#',' ',' ',' ','#','#','#',' ',
		'#','#','S','#','#','#','#',' ',' ',' ','#','#','#',' ',' ',' ','#','#','#',' ',
		'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#',' ',
		'#','#','S','#','#','#','#','#','#','#','#','#','#','#','W','#','#','S','#',' ',
		'#','#','#','S','#','#','#','#','#','#','#','#','#','#','W','#','#','#','#',' ',
		'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','W','#','#','#',' ',
		' ','#','#','#','#','#','#','#','#','#','#','#','#',' ',' ',' ','#','#','#',' ',
		' ','#','#','#','#','#','#','#','#','#','#',' ',' ',' ',' ',' ',' ','#','#',' ',
		' ',' ',' ','#','#','#','#','#','#','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ','#','#','#','#','#','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ','#','#','#','#','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ','#','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
	};

	ClearMap(world->map);
	for (s32 y = 0; y < Y; y++)
	{
		for (s32 x = 0; x < X; x++)
		{
			u8 value = 1;
			switch (Data[y][x])
			{
			case '#': value = 1; break;
			case 'W': value = 2; break;
			case 'S': CreateSlimeI(world, x, y); break;
			default: value = 0; break;
			}
			SetTileValueI(world->map, x, y, value);
		}
	}
	#undef X
	#undef Y
}

fn void ReloadAssets(assets_t *assets, log_t *log)
{
	LoadAssets(assets);
	LogLn(log, "editor: reloading \"assets/\"");
}

fn void Editor(editor_state_t *editor, game_world_t *state, command_buffer_t *out, const client_input_t *input, log_t *log, assets_t *assets)
{
	if ((editor->inited == false))
	{
		MakeSimpleMap(state);
		editor->inited = true;
	}

	s32 count = input->char_count;
	while (count--)
	{
		switch(ToUpper(input->char_queue[count]))
		{
			#if ENABLE_ASSET_RELOADING
			// TODO(): We should technically free the OpenGL handles here, but since this is strictly a debug feature - it doesn't really matter.
		case 'R': ReloadAssets(assets, log); break;
			#endif
		}
	}	
}