fn inline void SetupPlayer(game_state_t *World, entity_t *Entity);

fn entity_t *CreatePlayer(game_state_t *state, v2s p)
{
    entity_t *result = 0;
    if (state->Player == 0)
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

        state->Player = result->id;
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

fn void CreateContainer(game_state_t *state, v2s position)
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
            if (RandomChance(35))
                Eq_AddItem(&Container->inventory, item_assault_rifle);

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

fn void CreateScene(game_state_t *Scene, map_layout_t *Layout)
{   
    CreateMapFromLayout(Scene->map, Layout);

    for (s32 Index = 1; Index < Layout->PlacedRoomCount; Index++)
    {
        v2s Pos = GetRoomPosition(Layout, &Layout->PlacedRooms[Index]);
        CreateSlime(Scene, Add32(Pos, V2S(7, 8)));
        CreateSlime(Scene, Add32(Pos, V2S(14, 5)));
    }

    // 
    if (Layout->PlacedRoomCount)
    {
        const room_t *StartingRoom = &Layout->PlacedRooms[0];
        GenerateRoomInterior(Scene->map, *StartingRoom, Layout->ChunkSize);

        v2s Pos = GetRoomPosition(Layout, StartingRoom);
        CreatePlayer(Scene,    Add32(Pos, V2S(1, 1)));
        CreateContainer(Scene, Add32(Pos, V2S(2, 2)));
        CreateContainer(Scene, Add32(Pos, V2S(2, 5)));
    }
}