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
    item_type_t type;
    item_categories_t category;
    const char *name;
    s32 id; // Needs to be unique for each weapon
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
    target_flags_t target; // A list of valid targets. "Any" if not specified.

    const char *description;

    const char *ammo;
    s32 armor_penetration;

    const bitmap_t *animation_ranged;
    //const bitmap_t *animation;
} item_params_t;

typedef enum s32 item_id_t;

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
static const char *_Global_Item_Names[item_type_count];

fn const item_params_t *GetItemParams(item_type_t type)
{
    return &_Global_Item_Data[type];
}

fn const char *NameFromItemType(const char *name, memory_t *memory)
{
    s32 length = StringLength(name);
    char *result = PushSize(memory, (length + 1));
    result[length] = 0;
    if (length)
        result[0] = ToUpper(result[0]);

    s32 at = 0;
    while (at < length)
    {
        char *ch = &result[at];
        *ch = ToUpper(name[at]);

        if (*ch == '_')
            *ch = ' ';

        at++;
    }

    return result;
}

void DefaultItemValues(void)
{
    for (s32 index = 0; index < ArraySize(_Global_Item_Data); index++)
    {
        item_params_t *Params = &_Global_Item_Data[index];
        Params->type = (item_type_t)index;
        Params->category = (item_categories_t)index;
        if (Params->name == NULL)
            Params->name = _Global_Item_Names[index];
        if (IsZero(Params->size))
            Params->size = V2S(1, 1);
    }
}

fn void SetupItemDataTable(memory_t *memory, const assets_t *assets)
{
#define ITM(Type) \
_Global_Item_Names[item_##Type] = NameFromItemType(#Type, memory); \
_Global_Item_Data[item_##Type]  = (item_params_t)

    ITM(standard_caseless_rifle)
    {
        .id = 1,
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
        .id = 3,
        .name = "Green Herb",
        .size = V2S(1, 1),
    };

    ITM(assault_rifle)
    {
        .id = 0,
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
