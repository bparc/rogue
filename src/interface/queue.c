fn void DrawTurnQueuePic(command_buffer_t *out, f32 x, f32 y, v2 sz, f32 alpha, entity_t *entity, assets_t *assets)
{
    bitmap_t *bitmap = IsHostile(entity) ? &assets->Slime : &assets->Player[0];         
    v2 p = V2(x, y);

    v4 bitmap_color = PureWhite();
    bitmap_color.w = alpha;
    
    DrawBitmap(out, p, sz, bitmap_color, &assets->CombatUI.action_bar_elements[0]);
    if (bitmap)
        DrawBitmap(out, Add(p, V2(0.0f, 5.0f)), sz, bitmap_color, bitmap);
}

fn void DrawTurnQueuePicFadeOut(command_buffer_t *out, f32 x, f32 *y, v2 sz, entity_t *entity, assets_t *assets, f32 t, f32 y_spacing)
{
    t = t * t;

    v4 color = Black();
    color.w = t;
    x += (sz.x * (1.0f - t)); 
    DrawTurnQueuePic(out, -x, *y, sz, color.w, entity, assets);
    *y += y_spacing * t;
}

fn void TurnQueue(command_buffer_t *out, game_state_t *State, assets_t *assets, cursor_t *cursor)
{
    v2 sz = V2(64.f, 64.f);
    f32 spacing = sz.y + 5.0f;
    f32 top = 100.0f;
    f32 min = 8.0f;

    f32 x = min;
    f32 y = top;

    f32 fade_out_time = 0.5f;
    if ((State->PrevTurnEntity > 0) &&
        (State->SecondsElapsed < fade_out_time))
    {
        f32 t = 1.0f - (State->SecondsElapsed / fade_out_time);
        entity_t *entity = GetEntity(&State->Units, State->PrevTurnEntity);
        DrawTurnQueuePicFadeOut(out, x, &y, sz, entity, assets, t, spacing);
    }

    for (s32 index = State->QueueSize - 1; index >= 0; index--)
    {
        entity_id_t ID = State->Queue[index];
        entity_t *entity = GetEntity(&State->Units, ID);
        if (entity)
        {
            DrawTurnQueuePic(out, x, y, sz, 1.0f, entity, assets);
            if (entity->id == cursor->Target)
                DrawRectOutline(out, V2(x, y), sz, Orange());

            y += spacing;
        }
        else
        {
            evicted_entity_t *evicted = GetEvictedEntity(State, ID);
            if (evicted)
                DrawTurnQueuePicFadeOut(out, x, &y, sz, &evicted->entity, assets, evicted->time_remaining, spacing);
        }
    }

    DrawRectOutline(out, V2(x, 100.0f), sz, Red());
}