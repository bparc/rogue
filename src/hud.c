fn void DefaultActionBar(slot_bar_t *bar)
{
    // NOTE(Arc): I'm calling this from Setup()!

    for (s32 i = 0; i < 9; i++) {
        bar->slots[i].action = action_none;
        bar->slots[i].icon = NULL;
    }

    bar->slots[0].action = action_ranged_attack;
    bar->slots[1].action = action_melee_attack;
    bar->slots[2].action = action_throw;
    bar->slots[3].action = action_push;
    bar->slots[4].action = action_heal_self;

    bar->selected_slot = 1;
}

fn void RenderSlotBar(game_world_t *state, command_buffer_t *out, assets_t *assets, const client_input_t *input) {
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

        if (state->slot_bar.slots[i].icon)
        {
            DrawBitmap(out, slot_pos, slot_size, PureWhite(), state->slot_bar.slots[i].icon);
        }
        else
        {
            action_type_t type = state->slot_bar.slots[i].action;
            v2 text_p = Add(slot_pos, V2(2.0f, 0.0f));
            DrawText(out, assets->Font, text_p, action_type_t_names[type], Black());
        }

        if (IsKeyPressed(input, key_code_1 + (u8)i))
            state->slot_bar.selected_slot = i + 1;
    }
}

fn void 
HUD(command_buffer_t *out, game_world_t *state, turn_queue_t *queue, entity_storage_t *storage, assets_t *assets, const client_input_t *input)
{
    f32 y = 100.0f;
    // Renderowanie kolejki
    for (s32 index = queue->num - 1; index >= 0; index--)
    {
        entity_t *entity = GetEntity(storage, queue->entities[index]);
        if (entity)
        {
            v2 frame_sz = V2(64.f, 64.f);
            bitmap_t *bitmap = IsHostile(entity) ? &assets->Slime : &assets->Player[0];         
            v4 frame_color = (index == (queue->num - 1)) ? Red() : Black();

            v2 p = V2(8.0f, 100.0f + y);
            DrawRect(out, p, frame_sz, Black());
            DrawBitmap(out, p, frame_sz, PureWhite(), &assets->CombatUI.action_bar_elements[0]);
            DrawRectOutline(out, p, frame_sz, frame_color);
            if (bitmap)
                DrawBitmap(out, Add(p, V2(0.0f, 5.0f)), frame_sz, PureWhite(), bitmap);
            y += (frame_sz.y + 5.0f);
        }
    }

    //if ((state->cursor->active == false)) //NOTE(): Hide the action bar if the cursor is open?
        RenderSlotBar(state, out, assets, input);
}

//fn void ActivateSlotAction(entity_t *user, entity_t *target, action_type_t action);
// NOTE(Arc): This has to be implemented in "cursor.c" for now.
