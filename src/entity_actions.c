fn action_params_t DefineNoneAction() {
    action_params_t none_action = {
        .range = 0,
        .target = 0,
        .area_of_effect = {1, 1},
        .action_point_cost = 0,
        .accuracy = 0.0f,
        .damage = 0.0f,
        .is_healing = false,
        .is_status_effect = false,
        .status_effect = status_effect_none,
        .name = "None"
    };

    return none_action;
}

fn action_params_t DefineMeleeAttack(entity_t *user) {
    action_params_t melee_attack = {
        .range = 2,
        .target = 0,
        .area_of_effect = {1, 1},
        .action_point_cost = 1,
        .accuracy = (f32)user->melee_accuracy,
        .damage = user->attack_dmg,
        .is_healing = false,
        .is_status_effect = false,
        .status_effect = status_effect_none,
        .name = "Melee Attack"
    };

    return melee_attack;
}

fn action_params_t DefineRangedAttack(entity_t *user) {
    action_params_t ranged_attack = {
        .range = 5,
        .target = 0,
        .area_of_effect = {1, 1},
        .action_point_cost = 1,
        .accuracy = (f32)user->ranged_accuracy,
        .damage = user->attack_dmg,
        .is_healing = false,
        .is_status_effect = false,
        .status_effect = status_effect_none,
        .name = "Ranged Attack"
    };

    return ranged_attack;
}

fn action_params_t DefineThrowAction(entity_t *user) {
    action_params_t throw_action = {
        .range = 6,
        .target = 0,
        .area_of_effect = {2, 2},
        .action_point_cost = 1,
        .accuracy = 1.0f,
        .damage = 10.0f,
        .is_healing = false,
        .is_status_effect = false,
        .status_effect = status_effect_none,
        .name = "Throw"
    };

    return throw_action;
}

fn action_params_t DefinePushAction(entity_t *user) {
    action_params_t push_action = {
        .range = 2,
        .target = 0,
        .area_of_effect = {1, 1},
        .action_point_cost = 4,
        .accuracy = (f32)user->melee_accuracy,
        .damage = 0.0f,
        .is_healing = false,
        .is_status_effect = false,
        .status_effect = status_effect_none,
        .name = "Push"
    };

    return push_action;
}

fn action_params_t DefineHealAction(entity_t *user) {
    action_params_t heal_action = {
        .range = 1,
        .target = 0,
        .area_of_effect = {1, 1},
        .action_point_cost = 2,
        .accuracy = 1.0f,
        .damage = 20.0f,
        .is_healing = true,
        .is_status_effect = false,
        .status_effect = status_effect_none,
        .name = "Heal"
    };

    return heal_action;
}

fn action_params_t DefineActionTypeParams(entity_t *user, action_t action) {

    switch (action.type) {
        case action_none:
            action.params = DefineNoneAction();
            break;
        case action_melee_attack:
            action.params = DefineMeleeAttack(user);
            break;
        case action_ranged_attack:
            action.params = DefineRangedAttack(user);
            break;
        case action_throw:
            action.params = DefineThrowAction(user);
            break;
        case action_push:
            action.params = DefinePushAction(user);
            break;
        case action_heal_self:
            action.params = DefineHealAction(user);
            break;
        default:
            break;
    }

    return action.params;
}

fn action_t GetEquippedAction(const slot_bar_t *menu, entity_t *user)
{
    action_t result = { .type = action_none };
    s32 index = (menu->selected_slot - 1);
    if ((index >= 0) && (index < ArraySize(menu->slots)))
    {
        result = menu->slots[index].action;
        result.params = DefineActionTypeParams(user, result);
    }
    return result;
}

fn s32 GetAPCost(action_t action, entity_t *entity)
{
    s32 result = 0;
    if (entity)
    {
        action_params_t params = DefineActionTypeParams(entity, action);
        result = params.action_point_cost;
    }
    return result;
}