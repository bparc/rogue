#define MAX_STATUS_EFFECTS 3


typedef enum
{
	entity_flags_controllable = 1 << 0,
	entity_flags_hostile = 1 << 1,
	entity_flags_deleted = 1 << 2,
} entity_flags_t;

typedef enum
{
	static_entity_flags_trap = 1 << 0,
	static_entity_flags_stepon_trigger = 1 << 1,
} static_entity_flags;

typedef enum
{
	enemy_none,
	enemy_slime,
	enemy_slime_large,
	enemy_type_count,
} enemy_type_t;

typedef struct
{
	entity_id_t id;
	u8 enemy_type;
	u8 flags;

	v2s p; // A position on the tile map.
	v2 deferred_p;

	v2s size; //size in squares, 1x1, 2x1, 1x2, etc

	f32 blink_time;
	u16 health;
	u16 max_health;
	u16 attack_dmg;

	s32 ranged_accuracy;
	s32 melee_accuracy;
	s32 evasion;
	s32 remaining_action_points;
	f32 hitchance_boost_multiplier;
	b32 has_hitchance_boost;

	status_effect_t status_effects[MAX_STATUS_EFFECTS];

	s32 DEBUG_step_count;
} entity_t;

typedef struct
{
	u8 flags;
	v2s p; // A position on the tile map.
	//wont move
	v2s size; //size in squares, 1x1, 2x1, 1x2, etc

	status_effect_t status_effects[MAX_STATUS_EFFECTS];
} static_entity_t;

typedef struct
{
	s32 num;
	s32 statics_num;
	entity_t entities[1024];
	static_entity_t static_entities[1024];
	u64 next_id;
} entity_storage_t;

// NOTE(): Lifetime Management
fn entity_t *CreateEntity(entity_storage_t *storage, v2s p, v2s size, u8 flags, u16 health_points, u16 attack_dmg, const map_t *map, u16 max_health_points, s32 accuracy, s32 evasion, s32 remaining_action_points, s32 remaining_movement_points, f32 hitchance_boost_multiplier);
fn static_entity_t * CreateStaticEntity(entity_storage_t *storage, v2s p, v2s size, u8 flags, status_effect_t status_effects[MAX_STATUS_EFFECTS]);

fn entity_t *EntityFromIndex(entity_storage_t *storage, s32 index);
fn entity_t *GetEntity(entity_storage_t *storage, entity_id_t id);

// NOTE(): Spatial
fn entity_t *GetEntityByPosition(entity_storage_t *storage, v2s p);
fn entity_t *FindClosestHostile(entity_storage_t *storage, v2s player_pos);
fn entity_t *FindClosestPlayer(entity_storage_t *storage, v2s p);
fn v2s GetDirectionToClosestPlayer(entity_storage_t *storage, v2s p);

// NOTE(): Type Queries
fn b32 IsHostile(const entity_t *entity);
fn b32 IsPlayer(const entity_t *entity);
fn entity_t *DEBUGGetPlayer(entity_storage_t *storage);