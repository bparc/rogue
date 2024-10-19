fn void DefaultActionBar(slot_bar_t *bar, assets_t *assets)
{
    // NOTE(Arc): I'm calling this from Setup()!

    for (s32 i = 0; i < 9; i++) {
        bar->slots[i].action.type = action_none;
        bar->slots[i].action.icon = NULL;
    }

  

    bar->slots[0].action.type = action_ranged_attack;
    bar->slots[0].action.icon = &assets->CombatUI.action_bar_icons[1][0];

    bar->slots[1].action.type = action_melee_attack;
    bar->slots[1].action.icon = &assets->CombatUI.action_bar_icons[0][0];

    bar->slots[2].action.type = action_throw;
    bar->slots[2].action.icon = &assets->CombatUI.action_bar_icons[2][0];

    bar->slots[3].action.type = action_push;
    bar->slots[3].action.icon = &assets->CombatUI.action_bar_icons[3][0];

    bar->slots[4].action.type = action_heal_self;
    bar->slots[4].action.icon = &assets->CombatUI.action_bar_icons[4][0];

    bar->selected_slot = 1;
}

fn void ActionMenu(game_world_t *state, command_buffer_t *out, assets_t *assets, const client_input_t *input) {
    v2 action_bar_size = V2(540.0f, 60.0f);
    v2 slot_size = V2(50.0f, 50.0f);
    f32 padding = 10.0f;
    f32 total_width = 9 * slot_size.x + 8 * padding;

    f32 screen_width = input->viewport[0];
    f32 screen_height = input->viewport[1];
    f32 start_pos_x = (screen_width - action_bar_size.x) / 2.0f; // 530 pikseli
    f32 start_pos_y = screen_height - action_bar_size.y - 10.0f; // 830 pikseli
    v2 action_bar_pos = V2(start_pos_x, start_pos_y); // (530, 830)

    DrawBitmap(out, action_bar_pos, action_bar_size, PureWhite(), &assets->CombatUI.action_bar_full);

    v2 slot_start_pos = Add(action_bar_pos, V2(5.0f, 5.0f)); // (535, 835)

    for (s32 i = 0; i < ArraySize(state->slot_bar.slots); i++) {
        v2 slot_offset = V2(i * (slot_size.x + padding), 0.0f);
        v2 slot_pos = Add(slot_start_pos, slot_offset);

        v4 border_color = (state->slot_bar.selected_slot == (i + 1)) ? Blue() : Black();

        DrawRectOutline(out, slot_pos, slot_size, border_color);

        if (state->slot_bar.slots[i].action.icon)
        {
            DrawBitmap(out, slot_pos, slot_size, PureWhite(), state->slot_bar.slots[i].action.icon);
        }
        else
        {
            action_type_t type = state->slot_bar.slots[i].action.type;
            v2 text_p = Add(slot_pos, V2(2.0f, 0.0f));
            DrawText(out, assets->Font, text_p, action_type_t_names[type], Black());
        }

        if (IsKeyPressed(input, key_code_1 + (u8)i))
            state->slot_bar.selected_slot = i + 1;
    }
}

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

fn void TurnQueue(command_buffer_t *out, game_world_t *state, turn_queue_t *queue, assets_t *assets, cursor_t *cursor)
{
    entity_storage_t *storage = queue->storage;

    v2 sz = V2(64.f, 64.f);
    f32 spacing = sz.y + 5.0f;
    f32 top = 100.0f;
    f32 min = 8.0f;

    f32 x = min;
    f32 y = top;

    f32 fade_out_time = 0.5f;
    if ((queue->prev_turn_entity > 0) &&
        (queue->seconds_elapsed < fade_out_time))
    {
        f32 t = 1.0f - (queue->seconds_elapsed / fade_out_time);
        entity_t *entity = GetEntity(storage, queue->prev_turn_entity);
        DrawTurnQueuePicFadeOut(out, x, &y, sz, entity, assets, t, spacing);
    }

    for (s32 index = queue->num - 1; index >= 0; index--)
    {
        entity_id_t ID = queue->entities[index];
        entity_t *entity = GetEntity(storage, ID);
        if (entity)
        {
            DrawTurnQueuePic(out, x, y, sz, 1.0f, entity, assets);
            if (entity->id == cursor->target_id)
                DrawRectOutline(out, V2(x, y), sz, Orange());

            y += spacing;
        }
        else
        {
            evicted_entity_t *evicted = GetEvictedEntity(queue, ID);
            if (evicted)
                DrawTurnQueuePicFadeOut(out, x, &y, sz, &evicted->entity, assets, evicted->time_remaining, spacing);
        }
    }

    DrawRectOutline(out, V2(x, 100.0f), sz, Red());
}

fn void HUD(command_buffer_t *out, game_world_t *state, turn_queue_t *queue, entity_storage_t *storage, assets_t *assets, const client_input_t *input)
{
    TurnQueue(out, state, queue, assets, state->cursor);
    ActionMenu(state, out, assets, input);
}

//fn void ActivateSlotAction(entity_t *user, entity_t *target, action_type_t action);
// NOTE(Arc): This has to be implemented in "cursor.c" for now.
