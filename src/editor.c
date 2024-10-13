
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
		'#','#','S','#','#','#','#','#','#','T','#','#','#','#','W','#','#','S','#',' ',
		'#','#','#','S','#','#','#','#','#','#','#','#','#','#','W','#','#','#','#',' ',
		'#','#','#','#','#','#','#','#','B','B','#','#','#','#','#','W','#','#','#',' ',
		' ','#','#','#','#','#','#','T','B','B','#','#','#',' ',' ',' ','#','#','#',' ',
		' ','#','#','#','#','T','#','#','#','#','#',' ',' ',' ',' ',' ',' ','#','#',' ',
		' ',' ',' ','#','#','#','#','T','#','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ','#','#','#','#','#','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ','#','#','#','#','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ','T','T','T',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
	};

	ClearMap(world->map);

	int visited[Y][X] = {false}; //needed for compound tiles larger than 1x1

	for (s32 y = 0; y < Y; y++) {
		for (s32 x = 0; x < X; x++) {

			u8 value = 1;

			if (visited[y][x]) {
				
				SetTileValueI(world->map, x, y, value);
				continue;
			};  // can only fire for >1x1

			
			switch (Data[y][x]) {
				case '#': value = 1; break;
				case 'W': value = 2; break;
				case 'S':
					CreateSlimeI(world, x, y);
					break;
				case 'B': {
					v2s size = V2S(2,2);
					// big slimes can only be 2x2
					if (IsValidEntity(X, Y, (char *)Data, x, y, size, 'B')) {
						//exclude the whole square
						for (int j = y; j < y + 2; j++) {
							for (int i = x; i < x + 2; i++) {
								visited[j][i] = true; //dont rerun those
							}
						}
						CreateBigSlimeI(world, x, y); //big slimes are 2x2

					} else {
						printf("Invalid big slime at position (%d, %d)\n", x, y);
						visited[y][x] = true;
					}
					break;
				}
				case 'T': 
				CreatePoisonTrapI(world, x, y);
				 break;
				default:
					value = 0; break;
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

int IsValidEntity(s32 globalX, s32 globalY, char *Data, s32 startX, s32 startY, v2s size, char entityChar) {
    s32 n = size.x;
    s32 m = size.y;

    // within bounds?
    if (startX < 0 || startY < 0 || startX + n > globalX || startY + m > globalY) {
        return false;  // Out of bounds
    }

    // is entire requested size filled with entityChar?
    for (int y = startY; y < startY + m; y++) {
        for (int x = startX; x < startX + n; x++) {
            if (Data[y * globalX + x] != entityChar) {
                return false;  // Mismatch found
            }
        }
    }

    return true; 
}
