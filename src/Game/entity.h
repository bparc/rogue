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

	inventory_t *inventory;

	v2s p; // A position on the tile Map.
	v2 deferred_p;

	v2s size; //size in squares, 1x1, 2x1, 1x2, etc

	f32 blink_time;
	u16 health;
	u16 max_health;
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

fn b32 IsAlive(entity_t *Entity);
fn b32 IsHostile(const entity_t *entity);
fn b32 IsPlayer(const entity_t *entity);

fn void TakeHP(entity_t *entity, s16 damage);
fn void Heal(entity_t *entity, s16 healed_hp);

typedef struct
{
	u64 IDPool;
	s32 EntityCount;
	entity_t entities[1024];

	s32 ContainerCount;
	container_t Containers[64];
} entity_storage_t;

fn entity_t *GetEntity(entity_storage_t *storage, entity_id_t id);
fn entity_t *GetEntityFromPosition(entity_storage_t *storage, v2s p);

fn entity_t *FindClosestHostile(entity_storage_t *storage, v2s player_pos);
fn entity_t *FindClosestPlayer(entity_storage_t *storage, v2s p);
