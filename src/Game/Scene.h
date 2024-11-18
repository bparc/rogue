fn container_t *PushContainer(game_world_t *state);
fn container_t *GetContainer(game_world_t *state, v2s position);

fn inline void SetupPlayer(game_world_t *World, entity_t *Entity);

// NOTE(): Entities
fn entity_t *CreatePlayer(game_world_t *state, v2s p);

fn entity_t *CreateSlime(game_world_t *state, v2s p);
fn entity_t *CreateBigSlime(game_world_t *state, v2s p);

// NOTE(): Map Objects
fn void CreatePoisonTrap(game_world_t *state, v2s p);
fn void CreateContainer(game_world_t *state, v2s position);

fn void CreateScene(game_world_t *Scene, map_layout_t *Layout)
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
    }
}