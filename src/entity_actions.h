typedef enum {
    status_effect_none = 1 << 0,
    status_effect_poison = 1 << 1,
	status_effect_instant_damage = 1 << 2,
} status_effect_type_t;

const char *action_type_t_names[] =
{
	"None",
	"Melee",
	"Ranged",
	"Throw",
	"Push",
	"Heal",
};

typedef struct {
	s32 range;
	entity_id_t target;
	v2s area_of_effect;
	s32 action_point_cost;
	f32 accuracy;
	f32 damage;
	b32 is_healing;
	b32 is_status_effect;
	status_effect_type_t status_effect;
	const char *name;
} action_params_t;

typedef enum {
	action_none = 0,
	action_melee_attack,
	action_ranged_attack,
	action_throw,
	action_push,
	action_heal_self,
} action_type_t;

typedef struct {
	action_type_t type;
	bitmap_t *icon;
	action_params_t params;
} action_t;

typedef struct {
    status_effect_type_t type;
    s32 remaining_turns;
	s32 damage;
} status_effect_t;

typedef struct {
	action_t action;
} slot_t;

#define MAX_SLOTS 9
typedef struct {
	slot_t slots[MAX_SLOTS];
	s32 selected_slot;
} slot_bar_t;