fn void ActionMenu(entity_t *user, game_world_t *state, command_buffer_t *out, assets_t *assets, const client_input_t *input, turn_queue_t *queue) {
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

    slot_bar_t *Bar = &state->slot_bar;

    for (s32 i = 0; i < ArraySize(Bar->slots); i++)
    {
        slot_t *slot = &Bar->slots[i];
        action_t *action = &slot->action;
        const action_params_t *params = GetParameters(action->type);

        v2 slot_offset = V2(i * (slot_size.x + padding), 0.0f);
        v2 slot_p = Add(slot_start_pos, slot_offset);

        v4 border_color = (Bar->selected_slot == (i + 1)) ? Blue() : Black();
        DrawRectOutline(out, slot_p, slot_size, border_color);

        if (action->icon)
        {
            v4 color = PureWhite();

            if (queue->action_points < GetAPCost(action->type))
                color = RGB(30, 30, 30);
            DrawBitmap(out, slot_p, slot_size, color, action->icon);
        }
        else
        {
            if (params->name)
                DrawText(out, assets->Font, Add(slot_p, V2(4, 2)), params->name, White());
        }

        if (IsKeyPressed(input, key_code_1 + (u8)i))
            Bar->selected_slot = i + 1;
    }
}