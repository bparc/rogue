fn inline void SetupPlayer(game_state_t *World, entity_t *Entity);

fn entity_t *CreatePlayer(game_state_t *State, v2s p)
{
    entity_t *result = 0;
    if (State->Players[0] == 0)
    {
        u16 player_health = 62;
        u16 player_max_health = 62;
        u16 attack_dmg = 8; // What does this do now?
        s32 player_accuracy = 75; // Applying this value for both melee and ranged accuracy
        s32 player_evasion = 20;
        result = CreateEntity(&State->Units, p, V2S(1, 1), entity_flags_controllable,
            player_health, attack_dmg, &State->Map, player_max_health, player_accuracy, player_evasion,
            MAX_PLAYER_ACTION_POINTS, MAX_PLAYER_MOVEMENT_POINTS, 1);
        
	    result->inventory = PushStruct(inventory_t, State->Memory);
        SetupInventory(result->inventory);
        SetupPlayer(State, result);

        State->Players[0] = result->id;
    }
    else
    {
        DebugLog("player already created!");
    }
    return result;
}

fn entity_t *CreateSlime(game_state_t*State, v2s p)
{
	u16 slime_hp = 54;
	u16 slime_max_hp = 54;
	u16 slime_attack_dmg = 6;
	s32 slime_accuracy = 30; // Applying this value for both melee and ranged accuracy
	s32 slime_evasion = 80;
	entity_t *result = CreateEntity(&State->Units, p, V2S(1, 1),  entity_flags_hostile, slime_hp, slime_attack_dmg, &State->Map,
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
	entity_t *result = CreateEntity(&State->Units, p, V2S(2, 2),  entity_flags_hostile, slime_hp, slime_attack_dmg, &State->Map,
	slime_max_hp, slime_accuracy, slime_evasion, MAX_SLIME_ACTION_POINTS, MAX_SLIME_MOVEMENT_POINTS, 1);
    result->enemy_type = enemy_slime_large;
    return result;
}

fn void CreatePoisonTrap(game_state_t *State, v2s p)
{
	u8 flags = static_entity_flags_trap | static_entity_flags_stepon_trigger;

	status_effect_t status_effects = {0};
	status_effects.type = status_effect_poison;
	status_effects.remaining_turns = 3;
	status_effects.damage = 1;
}

fn void CreateRandomLoot(game_state_t *State, v2s position)
{
    b32 Result = false;
    if (InMapBounds(&State->Map, position))
    {
        s32 Index = GetTileIndex(&State->Map, position);
        container_t *Container = PushContainer(&State->Units);
        if (Container)
        {
            State->Map.container_ids[Index] = Container->ID;
            SetupInventory(&Container->inventory);

            // randomize loot
            s32 GreenHerbCount = 1 + (rand() % 4);
            while (GreenHerbCount--)
                Eq_AddItem(&Container->inventory, item_green_herb);

            Result = true;
        }
    }
}

fn container_t *GetContainer(game_state_t *State, v2s position)
{
    entity_storage_t *Storage = &State->Units;
    map_t *Map = &State->Map;

    container_t *Result = 0;
    if (InMapBounds(&State->Map, position))
    {
        s32 TileIndex = GetTileIndex(&State->Map, position);
        s32 ContainerIndex = (Map->container_ids[TileIndex] - 1);
        if ((ContainerIndex >= 0) && (ContainerIndex < Storage->ContainerCount))
            Result = &Storage->Containers[ContainerIndex];
    }
    return Result;
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
        StartingRoom->Visited = true;
        //CreateRandomLoot(State, IntAdd(Pos, V2S(2, 2)));
        //CreateRandomLoot(State, IntAdd(Pos, V2S(2, 5)));
    }
}