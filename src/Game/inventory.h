#define MAX_INVENTORY_SIZE 10
#define BACKPACK_GRID_X_SIZE 16
#define BACKPACK_GRID_Y_SIZE 8

typedef struct
{
    item_id_t ID;
} layout_cell_t;

typedef struct {
    s32 item_count;
    item_t items[BACKPACK_GRID_X_SIZE * BACKPACK_GRID_Y_SIZE];

    union
    {
        struct { s32 x; s32 y; };
        v2s size;
    };

    layout_cell_t layout[BACKPACK_GRID_Y_SIZE][BACKPACK_GRID_X_SIZE]; // For inventory tetris

    s32 carried_weight;
    s32 max_carry_weight;

    item_t equipped_weapon;
    item_t equipped_armor;
    
    u64 *global_item_count; // NOTE(): Not too thrilled about this pointer, but getting rid of it would
    // be would be kind of annoying...
} inventory_t;

fn void SetupInventory(inventory_t *inventory, u64 *IDGen)
{
    ZeroStruct(inventory);
    inventory->item_count = 0;
    inventory->x = BACKPACK_GRID_X_SIZE;
    inventory->y = BACKPACK_GRID_Y_SIZE;
    inventory->global_item_count = IDGen;
}

#undef BACKPACK_GRID_X_SIZE
#undef BACKPACK_GRID_Y_SIZE
#undef MAX_INVENTORY_SIZE

fn item_id_t Eq_AllocateID(inventory_t *inventory);

fn void Eq_MoveItem(inventory_t *Eq, item_t Source, v2s Dest);
fn item_t *Eq_GetItem(inventory_t *inventory, item_id_t ID);
fn item_t *Eq_AddItem(inventory_t *inventory, item_type_t type);
fn b32 Eq_RemoveItem(inventory_t *inventory, item_id_t ID);

fn b32 Eq_FindVacantSpace(const inventory_t *Eq, v2s *Index, v2s RequiredSpace);
fn b32 Eq_IsSpaceFree(const inventory_t *eq, v2s offset, v2s size);
fn b32 Eq_IsSpaceOccupied(const inventory_t *eq, v2s offset, v2s size);

fn void Eq_OccupySpace(inventory_t *eq, v2s offset, v2s size, item_id_t ID);
fn void Eq_FreeSpace(inventory_t *eq, v2s min, v2s size);