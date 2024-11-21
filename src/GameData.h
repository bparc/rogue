void DefaultItemValues(void)
{
    for (s32 index = 0; index < ArraySize(_Global_Item_Data); index++)
    {
        item_params_t *Params = &_Global_Item_Data[index];
        Params->type = (item_type_t)index;
        Params->category = (item_categories_t)index;
        if (Params->name == NULL)
            Params->name = "Not Set";
        if (IsZero(Params->size))
            Params->size = V2S(1, 1);
    }
}

fn void SetupItemDataTable(memory_t *memory, const assets_t *assets)
{
#define ITM(Type) _Global_Item_Data[item_##Type]  = (item_params_t)

    ITM(standard_caseless_rifle)
    {
        .name = "MilSpec Penetrator",
        .category = ammunition,
        .ammo = "7.62x51mm_caseless",
        .armor_penetration = 12,
        .price = 15,
        .quantity = 30,
        .description = "Common military-grade caseless rifle round for versatile use and easy logistics.",
        .size = V2S(1, 1),
    };

    ITM(green_herb)
    {
        .name = "Green Herb",
        .size = V2S(1, 1),
        .action = action_heal,
    };

    ITM(assault_rifle)
    {
        .size = V2S(5, 2),
        .name = "T-17 Assault Rifle",
        .category = rifle,
        .range = 25,
        .damage = 18,
        .price = 300,
        .durability = 120,
        .ammo = "7.62x51mm_caseless", // Weapon will borrow armor penetration from its ammo type
        .description = "A rugged and reliable assault rifle designed for military use, compatible with 7.62x51mm caseless rounds. Effective at any range.",
    };


#undef ITM

    DefaultItemValues();
}

void DefaultActionValues(void)
{
    for (s32 index = 0; index < ArraySize(_Global_Action_Data); index++)
    {
        action_params_t *Params = &_Global_Action_Data[index];
        Params->type = (action_type_t)index;
        if (Params->name == NULL)
            Params->name = "Not Set";
        if (!Params->range)
            Params->range = 2;
        if (!Params->target)
            Params->target = target_hostile;
    }
}

fn void SetupActionDataTable(memory_t *memory, const assets_t *assets)
{
    #define ACT(Type) _Global_Action_Data[action_##Type]  = (action_params_t)

    ACT(melee_attack)
    {
        .damage = 3,
        .cost   = 1,
    };
    ACT(ranged_attack)
    {
        .range  = 5,
        .cost   = 1,
        .damage = 4,
    };
    ACT(heal)
    {
        .mode = action_mode_heal,
        .cost = 2,
        .damage = 10,
        .value  = 10,
        .target = target_self,
    };
    ACT(push)
    {
        .range = 2,
        .cost  = 3,
        .pushback = 2,
    };
    ACT(throw)
    {
        .animation_ranged = &assets->PlayerGrenade,
        .damage = 20,
        .range  = 6,
        .area   = {2, 2},
        .cost   = 1,
        .target = target_field,
    };
    ACT(slash)
    {
        .damage = 9,
        .cost   = 3,
        .flags  = action_display_move_name,
    };
    ACT(dash)
    {
        .name = "Dash",
        .mode   = action_mode_dash,
        .flags  = action_display_move_name,
        .range  = 5,
        .cost   = 1,
        .target = target_field,
    };
    ACT(slime_ranged)
    {
        .animation_ranged = &assets->SlimeBall,
        .range  = 5,
        .damage = 3,
    };
    #undef ACT

    DefaultActionValues();
}