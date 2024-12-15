

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
	const action_params_t *Data;
} action_t;

typedef struct
{
	f32 Lerp;

	f32 elapsed;
	action_t action_type;

	entity_id_t target_id;
	v2s target_p;
} async_action_t;