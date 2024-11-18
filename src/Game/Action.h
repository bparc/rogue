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

typedef enum
{
	target_self	   = 1 << 0,
	//target_field   = 1 << 1,
	//target_hostile = 1 << 2,
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

	target_flags_t target; // A list of valid targets. "Any" if not specified.

	const bitmap_t *animation_ranged;
	//const bitmap_t *animation;
} action_params_t;

typedef struct {
	action_type_t type;
	const action_params_t *params;
	bitmap_t *icon;
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

static action_params_t _Global_Action_Data[action_type_count];
static const char *_Global_Action_Names[action_type_count];

static inline const action_params_t *GetParameters(action_type_t type)
{
	return &_Global_Action_Data[type];
}

fn inline s32 GetAPCost(action_type_t type)
{
	s32 result = GetParameters(type)->cost;
	return result;
}

fn inline b32 IsTargetSelf(action_type_t type)
{
	return _Global_Action_Data[type].target & target_self;
}

fn inline b32 IsTargetAny(action_type_t type)
{
	s32 result = (GetParameters(type)->target == 0);
	return result;
}

fn const char *NameFromActionType(const char *name, memory_t *memory)
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

void DefaultActionValues(void)
{
	for (s32 index = 0; index < ArraySize(_Global_Action_Data); index++)
	{
		action_params_t *Params = &_Global_Action_Data[index];
		Params->type = (action_type_t)index;
		if (Params->name == NULL)
			Params->name = _Global_Action_Names[index];
		if (!Params->range)
			Params->range = 2;

	}
}

fn void SetupActionDataTable(memory_t *memory, const assets_t *assets)
{
	#define ACT(Type) \
	_Global_Action_Names[action_##Type] = NameFromActionType(#Type, memory); \
	_Global_Action_Data[action_##Type]  = (action_params_t)

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
		.value 	= 10,
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
	};
	ACT(slash)
	{
		.damage = 9,
		.cost   = 3,
		.flags  = action_display_move_name,
	};
	ACT(dash)
	{
		.mode   = action_mode_dash,
		.flags  = action_display_move_name,
		.range  = 5,
		.cost   = 1,
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