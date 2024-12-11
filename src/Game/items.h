typedef enum {
    misc = 0,
    rifle,
    pistol,
    shotgun,
    explosive,
    armor,
    healing,
    food,
    currency,
    ammunition,
    item_category_count
} item_categories_t;

typedef enum {
    item_none = 0,
    item_assault_rifle,
    item_standard_caseless_rifle,
    item_green_herb,
    item_type_count
} item_type_t;

typedef struct
{
    action_type_t action;
    item_type_t type;
    item_categories_t category;
    const char *name;
    s32 range;
    s32 pellet_spread;
    s32 pushback;
    s32 damage_threshold;
    union
    {
        s32 damage;
        u16 healing;
    };
    s32 price;
    s32 durability;
    s32 quantity;
    s32 weight;
    s32 width;  // Width in grid cells (for inventory tetris)
    s32 height; // Height in grid cells
    v2s size;

    v2s area; // (1, 1) if not specified
    //target_flags_t target; // A list of valid targets. "Any" if not specified.

    const char *description;

    const char *ammo;
    s32 armor_penetration;

    const bitmap_t *animation_ranged;
    //const bitmap_t *animation;
} item_params_t;

typedef struct {
    const item_params_t *params;
    bitmap_t *icon;
    
    // NOTE(): Space occupied by the item in
    // the inventory grid. Parameterized as a rectangle.

    v2s size;
    union
    {
        struct { s32 x, y; };
        v2s index;
    };

    item_id_t ID;
} item_t;

static item_params_t _Global_Item_Data[item_type_count];

fn const item_params_t *GetItemParams(item_type_t type)
{
    return &_Global_Item_Data[type];
}

fn item_t MakeItemFromType(item_type_t Type)
{
    item_t Result = {0};
    Result.params = GetItemParams(Type);
    Result.size   = Result.params->size;
    return Result;  
}