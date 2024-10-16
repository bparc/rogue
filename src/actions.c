fn action_params_t DefineNoneAction() {
    action_params_t none_action = {
        .range = 0.0f,
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
        .range = 1.0f,
        .target = 0,
        .area_of_effect = {1, 1},
        .action_point_cost = 3,
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
        .range = 5.0f,
        .target = 0,
        .area_of_effect = {1, 1},
        .action_point_cost = 4,
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
        .range = 4.0f,
        .target = 0,
        .area_of_effect = {3, 3},
        .action_point_cost = 6,
        .accuracy = 0.75f,
        .damage = 50.0f,
        .is_healing = false,
        .is_status_effect = false,
        .status_effect = status_effect_none,
        .name = "Throw"
    };

    return throw_action;
}

fn action_params_t DefinePushAction(entity_t *user) {
    action_params_t push_action = {
        .range = 1.0f,
        .target = 0,
        .area_of_effect = {1, 1},
        .action_point_cost = 2,
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
        .range = 1.0f,
        .target = 0,
        .area_of_effect = {1, 1},
        .action_point_cost = 6,
        .accuracy = 1.0f,
        .damage = 50.0f,
        .is_healing = true,
        .is_status_effect = false,
        .status_effect = status_effect_none,
        .name = "Heal"
    };

    return heal_action;
}
