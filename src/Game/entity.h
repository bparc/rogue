typedef enum
{
	entity_flags_controllable = 1 << 0,
	entity_flags_hostile = 1 << 1,
	entity_flags_deleted = 1 << 2,
} entity_flags_t;

typedef enum
{
	RenderFlags_DisableAutoAnimation = 1 << 0,
} render_flags_t;

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
	u8 render_flags; // flags that last for a duration of a one frame

	inventory_t *inventory;

	v2s p; // A position on the tile Map.
	v2 deferred_p;

	v2s size; //size in squares, 1x1, 2x1, 1x2, etc

	f32 blink_time;
	s16 health;
	s16 max_health;
	u16 attack_dmg;

	s32 ranged_accuracy;
	s32 melee_accuracy;
	s32 evasion;
	f32 hitchance_boost_multiplier;
	b32 has_hitchance_boost;

	status_effect_t StatusEffect;

	// NOTE(): AI stuff
	b32 Alerted;
} entity_t;

typedef struct
{
	union
	{
		entity_t entity;
		entity_t;
	};
	f32 time_remaining;
} evicted_entity_t;

fn b32 IsAlive(const entity_t *Entity);
fn b32 IsHostile(const entity_t *entity);
fn b32 IsPlayer(const entity_t *entity);

typedef struct
{
    s32 ID;
    inventory_t inventory;
} container_t;

typedef struct
{
	u64 TotalEntityCount;
	s32 EntityCount;
	entity_t entities[1024];

	s32 ContainerCount;
	container_t Containers[64];
} entity_storage_t;