

const char *action_type_t_names[] =
{
	"None",
	"Melee",
	"Ranged",
	"Throw",
	"Push",
	"Heal",
};

typedef enum
{
	target_self	   = 1 << 0,
	target_field   = 1 << 1,
	target_hostile = 1 << 2,
} target_flags_t;

typedef enum {
	action_none = 0,
	action_melee_attack,
	action_ranged_attack,
	action_throw,
	action_push,
	action_heal,
	action_slash,
	action_dash,
	action_freeze,
	action_slime_ranged,
	action_type_count,
} action_type_t;

typedef enum
{
	action_display_move_name = 1 << 0, // NOTE(): Is in effect only if "animation_ranged" is set to NULL.
} action_flags_t;

typedef enum
{
	action_mode_damage, // NOTE(): action_mode_damage is default.
	action_mode_custom,
	action_mode_heal,
	action_mode_dash,
} action_mode_t; // NOTE(): Basic mode of interaction.

typedef struct
{
	action_mode_t mode;
	action_type_t type;
	const char *name;
	s32 range;
	s32 flags;
	s32 pushback;
	union
	{
	s32 damage;
	s32 value;
	};
	s32 cost;
	v2s area; // (1, 1) if not specified
	status_effect_type_t status_effect;

	target_flags_t target; // A list of valid targets. "Any" if not specified.

	const bitmap_t *animation_ranged;
	//const bitmap_t *animation;
} action_params_t;

typedef struct
{
	action_type_t type;
	const action_params_t *params;
	
	bitmap_t *icon;
} action_t;

typedef struct {
	action_t action;
	item_id_t AssignedItem;
} slot_t;

#define MAX_SLOTS 9
typedef struct {
	slot_t slots[MAX_SLOTS];
	s32 selected_slot;
} slot_bar_t;

fn slot_t *GetSlot(slot_bar_t *Bar, s32 Index);
static action_params_t _Global_Action_Data[action_type_count];

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

static inline const action_params_t *GetActionParams(action_type_t type)
{
	return &_Global_Action_Data[type];
}

static action_t ActionFromType(action_type_t Type)
{
	action_t Result = {0};
	Result.type = Type;
	Result.params = GetActionParams(Type);
	return Result;
}

fn inline s32 GetAPCost(action_type_t type)
{
	s32 result = GetActionParams(type)->cost;
	return result;
}

fn inline b32 IsTargetSelf(action_type_t type)
{
	return _Global_Action_Data[type].target & target_self;
}

fn inline b32 IsTargetHostile(action_type_t type)
{
	return _Global_Action_Data[type].target & target_hostile;
}

fn inline b32 IsTargetField(action_type_t type)
{
	return _Global_Action_Data[type].target & target_field;
}