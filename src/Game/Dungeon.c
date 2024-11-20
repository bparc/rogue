fn inline void SetupPlayer(game_state_t *World, entity_t *Entity);

fn entity_t *CreatePlayer(game_state_t *state, v2s p)
{
    entity_t *result = 0;
    if (state->player == 0)
    {
        u16 player_health = 62;
        u16 player_max_health = 62;
        u16 attack_dmg = 8;
        s32 player_accuracy = 75; // Applying this value for both melee and ranged accuracy
        s32 player_evasion = 20;
        result = CreateEntity(state->storage, p, V2S(1, 1), entity_flags_controllable,
            player_health, attack_dmg, state->map, player_max_health, player_accuracy, player_evasion,
            MAX_PLAYER_ACTION_POINTS, MAX_PLAYER_MOVEMENT_POINTS, 1);
        
	    result->inventory = PushStruct(inventory_t, state->memory);
        SetupInventory(result->inventory);
        SetupPlayer(state, result);

        state->player = result->id;
    }
    else
    {
        DebugLog("player already created!");
    }
    return result;
}

fn entity_t *CreateSlime(game_state_t *state, v2s p)
{
	u16 slime_hp = 54;
	u16 slime_max_hp = 54;
	u16 slime_attack_dmg = 6;
	s32 slime_accuracy = 30; // Applying this value for both melee and ranged accuracy
	s32 slime_evasion = 80;
	entity_t *result = CreateEntity(state->storage, p, V2S(1, 1),  entity_flags_hostile, slime_hp, slime_attack_dmg, state->map,
	slime_max_hp, slime_accuracy, slime_evasion, MAX_SLIME_ACTION_POINTS, MAX_SLIME_MOVEMENT_POINTS, 1);
    result->enemy_type = enemy_slime;
    return result;
}

fn entity_t *CreateBigSlime(game_state_t *state, v2s p)
{
	u16 slime_hp = 400;
	u16 slime_max_hp = 400;
	u16 slime_attack_dmg = 25;
	s32 slime_accuracy = 45; // Applying this value for both melee and ranged accuracy
	s32 slime_evasion = 40;
	entity_t *result = CreateEntity(state->storage, p, V2S(2, 2),  entity_flags_hostile, slime_hp, slime_attack_dmg, state->map,
	slime_max_hp, slime_accuracy, slime_evasion, MAX_SLIME_ACTION_POINTS, MAX_SLIME_MOVEMENT_POINTS, 1);
    result->enemy_type = enemy_slime_large;
    return result;
}

fn void CreatePoisonTrap(game_state_t *state, v2s p)
{
	u8 flags = static_entity_flags_trap | static_entity_flags_stepon_trigger;

	status_effect_t status_effects = {0};
	status_effects.type = status_effect_poison;
	status_effects.remaining_turns = 3;
	status_effects.damage = 1;

	status_effect_t effects[MAX_STATUS_EFFECTS] = {status_effects, 0, 0};
	CreateStaticEntity(state->storage, p, V2S(1,1), flags, effects);
}

fn void CreateRandomLoot(game_state_t *state, v2s position)
{
    b32 Result = false;
    if (InMapBounds(state->map, position))
    {
        s32 Index = GetTileIndex(state->map, position);
        container_t *Container = PushContainer(state->storage);
        if (Container)
        {
            state->map->container_ids[Index] = Container->ID;
            SetupInventory(&Container->inventory);

            // randomize loot
            s32 GreenHerbCount = 1 + (rand() % 4);
            while (GreenHerbCount--)
                Eq_AddItem(&Container->inventory, item_green_herb);

            Result = true;
        }
    }
}

fn container_t *GetContainer(game_state_t *state, v2s position)
{
    entity_storage_t *Storage = state->storage;
    map_t *Map = state->map;

    container_t *Result = 0;
    if (InMapBounds(state->map, position))
    {
        s32 TileIndex = GetTileIndex(state->map, position);
        s32 ContainerIndex = (Map->container_ids[TileIndex] - 1);
        if ((ContainerIndex >= 0) && (ContainerIndex < Storage->ContainerCount))
            Result = &Storage->Containers[ContainerIndex];
    }
    return Result;
}

fn void CreateRoomInterior(game_state_t *Scene, room_t room, v2s chunk_size)
{
    map_t *Map = Scene->map;
    entity_storage_t *Storage = Scene->storage;

#define X DUNGEON_ROOM_SIZE_Y
#define Y DUNGEON_ROOM_SIZE_X
    
    s32 RandomRoomIndex = rand() % ArraySize(Dungeon_RoomPresets);
    char *Data = Dungeon_RoomPresets[RandomRoomIndex];

    v2s min = Mul32(room.ChunkAt, chunk_size);
    for (s32 y = 1; y < Y - 1; y++)
    {
        for (s32 x = 1; x < X - 1; x++)
        {
            v2s TileIndex = {x, y};
            TileIndex = Add32(min, TileIndex);

            u8 value = tile_floor;
            switch (Data[y * X + x])
            {
            case 'W': value = tile_wall; break;
            case 'S': CreateSlime(Scene, TileIndex); break;
            case 'C': CreateRandomLoot(Scene, TileIndex); break;
            case 'P': CreatePoisonTrap(Scene, TileIndex); break;
            }
            SetTileValue(Map, TileIndex, value);
        }
    }
#undef X
#undef Y
}

fn void CreateDungeon(game_state_t *Scene, map_layout_t *Layout)
{   
    CreateMapFromLayout(Scene->map, Layout);

    for (s32 Index = 1; Index < Layout->PlacedRoomCount; Index++)
    {
        room_t *Room = &Layout->PlacedRooms[Index];
        v2s Pos = GetRoomPosition(Layout, Room);
        CreateRoomInterior(Scene, *Room, Layout->ChunkSize);
    }

    // 
    if (Layout->PlacedRoomCount)
    {
        const room_t *StartingRoom = &Layout->PlacedRooms[0];

        v2s Pos = GetRoomPosition(Layout, StartingRoom);
        CreatePlayer(Scene, Add32(Pos, V2S(1, 1)));
        //CreateRandomLoot(Scene, Add32(Pos, V2S(2, 2)));
        //CreateRandomLoot(Scene, Add32(Pos, V2S(2, 5)));
    }
}