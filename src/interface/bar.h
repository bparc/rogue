typedef struct {
	action_t action;
	item_id_t AssignedItem;
} slot_t;

#define MAX_SLOTS 9
typedef struct {
	slot_t slots[MAX_SLOTS];
	s32 selected_slot;
} slot_bar_t;

fn void SetupActionBar(slot_bar_t *bar, assets_t *assets);
fn slot_t *GetSlot(slot_bar_t *Bar, s32 Index);
fn action_t GetEquippedAction(const slot_bar_t *menu, entity_t *user);

fn void AssignItem(slot_bar_t *Bar, item_id_t ItemID, s8 Index);