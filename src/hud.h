fn void RenderSlotBar(game_world_t *state, command_buffer_t *out, assets_t *assets) {
    v2 action_bar_size = V2(540.0f, 60.0f);
    v2 slot_size = V2(50.0f, 50.0f);
    f32 padding = 10.0f;
    f32 total_width = 9 * slot_size.x + 8 * padding;

    f32 screen_width = 1600.0f;
    f32 screen_height = 900.0f;
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

        if (state->slot_bar.slots[i].icon) {
            DrawBitmap(out, slot_pos, slot_size, PureWhite(), state->slot_bar.slots[i].icon);
        } else {
            char *label;
            switch(state->slot_bar.slots[i].action) {
                case action_ranged_attack:
                    label = "attack";
                break;
                default:
                    label = "none";
                break;
            }
            if (label[0] != '\0') {
                v2 text_pos = slot_pos;
                text_pos.x += 1;

                DrawText(out, assets->Font, text_pos, label, Black());
            }
        }
    }
}

// todo: waiting for comabt revamp before finishing this method
/*fn void ActivateSlotAction(game_world_t *state, s32 slot_number, command_buffer_t *out) {
    if (slot_number < 1 || slot_number > 9)
        return;

    slot_t *slot = &state->slot_bar.slots[slot_number - 1];

    switch(slot->action) {
        case action_ranged_attack:
            PerformAttack(state, out);
            break;
        case action_none:
        default:
            break;
    }
}*/