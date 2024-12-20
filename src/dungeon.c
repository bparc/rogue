fn entity_t *CreateSlime(game_state_t*State, v2s p)
{
	u16 slime_hp = 54;
	u16 slime_max_hp = 54;
	u16 slime_attack_dmg = 6;
	s32 slime_accuracy = 30; // Applying this value for both melee and ranged accuracy
	s32 slime_evasion = 80;
	entity_t *result = CreateEntity(State, p, V2S(1, 1),  entity_flags_hostile, slime_hp, slime_attack_dmg, &State->Map,
	slime_max_hp, slime_accuracy, slime_evasion, MAX_SLIME_ACTION_POINTS, MAX_SLIME_MOVEMENT_POINTS, 1);
    result->enemy_type = enemy_slime;
    return result;
}

fn entity_t *CreateBigSlime(game_state_t *State, v2s p)
{
	u16 slime_hp = 400;
	u16 slime_max_hp = 400;
	u16 slime_attack_dmg = 25;
	s32 slime_accuracy = 45; // Applying this value for both melee and ranged accuracy
	s32 slime_evasion = 40;
	entity_t *result = CreateEntity(State, p, V2S(2, 2),  entity_flags_hostile, slime_hp, slime_attack_dmg, &State->Map,
	slime_max_hp, slime_accuracy, slime_evasion, MAX_SLIME_ACTION_POINTS, MAX_SLIME_MOVEMENT_POINTS, 1);
    result->enemy_type = enemy_slime_large;
    return result;
}

fn void CreatePoisonTrap(game_state_t *State, v2s p)
{
    
}

fn void CreateRandomLoot(game_state_t *State, v2s position)
{
    container_t *Container = CreateContainer(State, position);
    if (Container)
    {
        s32 GreenHerbCount = 1 + (rand() % 4);
        while (GreenHerbCount--)
        {
            Eq_AddItem(&Container->inventory, item_green_herb);
        }
    }
}

fn void CreateRoomInterior(game_state_t *State, room_t room, v2s chunk_size)
{
    map_t *Map = &State->Map;

#define X DUNGEON_ROOM_SIZE_Y
#define Y DUNGEON_ROOM_SIZE_X
    
    s32 RandomRoomIndex = rand() % ArraySize(Dungeon_RoomPresets);
    char *Data = Dungeon_RoomPresets[RandomRoomIndex];

    v2s min = IntMul(room.ChunkAt, chunk_size);
    for (s32 y = 2; y < Y - 1; y++)
    {
        for (s32 x = 2; x < X - 1; x++)
        {
            v2s TileIndex = {x, y};
            TileIndex = IntAdd(min, TileIndex);

            u8 value = tile_floor;
            switch (Data[y * X + x])
            {
            case 'W': value = tile_wall; break;
            case 'S': CreateSlime(State, TileIndex); break;
            case 'C': CreateRandomLoot(State, TileIndex); break;
            case 'P': CreatePoisonTrap(State, TileIndex); break;
            }
            SetTileValue(Map, TileIndex, value);
        }
    }
#undef X
#undef Y
}

fn void CreateDungeon(game_state_t *State, map_layout_t *Layout)
{   
    CreateMapFromLayout(&State->Map, Layout);

    for (s32 Index = 1; Index < Layout->PlacedRoomCount; Index++)
    {
        room_t *Room = &Layout->PlacedRooms[Index];
        v2s Pos = GetRoomPosition(Layout, Room);
        CreateRoomInterior(State, *Room, Layout->ChunkSize);
    }

    // 
    if (Layout->PlacedRoomCount)
    {
        room_t *StartingRoom = &Layout->PlacedRooms[0];

        v2s Pos = GetRoomPosition(Layout, StartingRoom);
        CreatePlayer(State, IntAdd(Pos, V2S(2, 2)));
        //CreateRandomLoot(State, IntAdd(Pos, V2S(2, 2)));
        //CreateRandomLoot(State, IntAdd(Pos, V2S(2, 5)));
    }
}