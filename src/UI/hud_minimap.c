fn void MiniMap(command_buffer_t *out, map_layout_t *Gen, v2s PlayerP, assets_t *Assets)
{
    v2 ChunkSz = V2(16.0f, 16.0f);
    v2 ChunkHalfSz = Scale(ChunkSz, 0.5f);
    v2 GlobalOffset = V2(1325.0f, 10.0f);

    v2s PlayerChunkAt = Div32(PlayerP, V2S(20, 20));

#if 1
    for (s32 y = 0; y < Gen->ChunkCountY; y++)
    {
        for (s32 x = 0; x < Gen->ChunkCountX; x++)
        {
            b32 Occupied = Gen->OccupiedChunks[y][x];

            v2 p = V2((f32)x, (f32)y);
            p = Mul(p, ChunkSz);
            p = Add(p, GlobalOffset);

            bb_t Bounds = RectToBounds(p, ChunkSz);

            v4 color = White();

            DrawRect(out, Bounds.min, Sub(Bounds.max, Bounds.min), Black());
            Bounds = Shrink(Bounds, 4.0f);
            DrawRectOutline(out, Bounds.min, Sub(Bounds.max, Bounds.min), White());
        }
    }
#endif

    for (s32 index = 0; index < Gen->PlacedRoomCount; index++)
    {
        room_t *room = &Gen->PlacedRooms[index];

        v2s Chunk = room->ChunkAt;

        v2 p = V2((f32)Chunk.x, (f32)Chunk.y);
        p = Mul(p, ChunkSz);
        p = Add(p, GlobalOffset);

        bb_t Bounds = RectToBounds(p, ChunkSz);
        Bounds = Shrink(Bounds, 2.0f);
        v4 color = Blue();
        if (CompareVectors(PlayerChunkAt, Chunk))
            color = Green();

        DrawRect(out, Bounds.min, Sub(Bounds.max, Bounds.min), color);
        DrawFormat(out, Assets->Font, Add(Bounds.min, V2(4.0f, 0.0f)), White(), "%i", room->Index);
    }

    for (s32 index = 1; index < Gen->PlacedRoomCount; index++)
    {
        room_t *room = &Gen->PlacedRooms[index];

        v2s Chunk = room->ChunkAt;
        v2s PrevChunk = room->PrevChunk;

        v2 To = V2((f32)Chunk.x, (f32)Chunk.y);
        To = Mul(To, ChunkSz);
        To = Add(To, GlobalOffset);
        To = Add(To, ChunkHalfSz);

        v2 From = V2((f32)PrevChunk.x, (f32)PrevChunk.y);
        From = Mul(From, ChunkSz);
        From = Add(From, GlobalOffset);
        From = Add(From, ChunkHalfSz);
        
        DrawLine(out, From, To, Red());
    }
}