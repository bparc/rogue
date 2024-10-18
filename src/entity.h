typedef u64 entity_id_t;

typedef enum
{
	entity_flags_controllable = 1 << 0,
	entity_flags_hostile = 1 << 1,
	entity_flags_deleted = 1 << 2,
} entity_flags_t;

typedef enum {
	static_entity_flags_trap = 1 << 0,
	static_entity_flags_stepon_trigger = 1 << 1,
} static_entity_flags;

typedef struct
{
	u8 flags;
	entity_id_t id;
	v2s p; // A position on the tile map.
	v2 deferred_p;

	v2s size; //size in squares, 1x1, 2x1, 1x2, etc

	f32 blink_time;
	u16 health;
	u16 max_health;
	u16 attack_dmg;

	status_effect_t status_effects[MAX_STATUS_EFFECTS];

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

// NOTE(): Lifetime management:
fn entity_t *CreateEntity(entity_storage_t *storage, v2s p, v2s size, u8 flags, u16 health_points, u16 attack_dmg, const map_t *map, u16 max_health_points);
fn void RemoveEntity(entity_t *entity);
fn entity_t *GetEntity(entity_storage_t *storage, entity_id_t id);

// NOTE():
fn void InflictDamage(entity_t *entity, u16 damage);

// NOTE(): Spatial queries:
fn entity_t *GetEntityByPosition(entity_storage_t *storage, v2s p);
fn entity_t *FindClosestHostile(entity_storage_t *storage, v2s player_pos);

// NOTE(): Type queries:
fn b32 IsHostile(const entity_t *entity);
fn b32 IsTrap(const static_entity_t *entity);