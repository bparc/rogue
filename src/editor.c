
fn void MakeSimpleMap(map_t *map)
{
	#define X 20
	#define Y 20
	// # = Traversables.
	// W = Walls.
	char Data[Y][X] =
	{
		' ',' ',' ',' ',' ',' ',' ',' ',' ','#','#','#','#','#','#',' ','#','#','#',' ',
		' ',' ','#','#','#',' ',' ',' ',' ',' ','#','#','#','#',' ',' ','#','#','#',' ',
		' ','#','#','#','#','#',' ',' ',' ',' ','#','#','#',' ',' ',' ','#','#','#',' ',
		'#','#','#','#','#','#','#',' ',' ',' ','#','#','#',' ',' ',' ','#','#','#',' ',
		'#','#','#','#','#','#','#',' ',' ',' ','#','#','#',' ',' ',' ','#','#','#',' ',
		'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#',' ',
		'#','#','#','#','#','#','#','#','#','#','#','#','#','#','W','#','#','#','#',' ',
		'#','#','#','#','#','#','#','#','#','#','#','#','#','#','W','#','#','#','#',' ',
		'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#',' ',
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

	ClearMap(map);
	for (s32 y = 0; y < Y; y++)
	{
		for (s32 x = 0; x < X; x++)
		{
			u8 value = 0;
			switch (Data[y][x])
			{
			case '#': value = 1; break;
			case 'W': value = 2; break;
			}
			SetTileValueI(map, x, y, value);
		}
	}
	#undef X
	#undef Y
}

fn void Editor(editor_state_t *editor, game_world_t *state, command_buffer_t *out, const client_input_t *input)
{
	if ((editor->inited == false))
	{
		MakeSimpleMap(state->map);
		editor->inited = true;
	}
}