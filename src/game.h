typedef struct
{
	f32 elapsed;
	action_t action_type;

	entity_id_t target_id;
	v2s target_p;
} async_action_t;

typedef struct
{
	union
	{
		entity_t entity;
		entity_t;
	};
	f32 time_remaining;
} evicted_entity_t;

typedef struct
{
	memory_t *Memory;
	assets_t *Assets;
	log_t *Log;

	particles_t ParticleSystem;
	
	u64 GlobalItemCount;
	interface_t GUI;
	slot_bar_t Bar;

	cursor_t Cursor;

	map_layout_t MapLayout;
	map_t Map;

	camera_t Camera;

	b32 EncounterModeEnabled;
	entity_storage_t Units;
	entity_id_t Players[1];

	// enemy

	float EnemyLerp;
	s32 EnemyInited;
	path_t EnemyPath;
	
	// phase

	range_map_t EffectiveRange;
	#define MAX_PER_PHASE 64
	s32 PhaseSize;
	s32 QueueSize;
	entity_id_t Queue[MAX_PER_PHASE]; // // NOTE(): Queue encoded as an list of entity ids in a *reverse* order (the last turn in the queue will be executed first).
	entity_id_t Phase[MAX_PER_PHASE];
	#undef MAX_PER_PHASE

	// turn

	s32 TurnInited;
	s32 ActionPoints;
	f32 SecondsElapsed; // NOTE(): Seconds elapsed from the start of the turn.

	// actions

	s32 ActionCount;
	async_action_t Actions[1];

	// NOTE(): An additional book-keeping
	// for the animation system to use.

	entity_id_t PrevTurnEntity;
	#define ENTITY_EVICTION_SPEED 2.0f
	s32 EvictedEntityCount;
	evicted_entity_t EvictedEntities[8]; // // NOTE(): A small buffer that stores copies of deleted entities for a brief amount of time.
} game_state_t;

// =======
// turn
// ======
fn void PushTurn(game_state_t *queue, entity_t *entity);

fn void InteruptTurn(game_state_t *State, entity_t *Entity);
fn void EndTurn(game_state_t *queue, entity_t *entity);

fn int32_t IsActive(const game_state_t *State, entity_id_t id);
fn entity_t *GetActive(const game_state_t *State);

// ======
// objects
// ======
fn entity_t *CreateEntity(game_state_t *State, v2s p, v2s size, u8 flags, u16 health_points, u16 attack_dmg, const map_t *Map, u16 max_health_points, s32 accuracy, s32 evasion, s32 remaining_action_points, s32 remaining_movement_points, f32 hitchance_boost_multiplier);

fn container_t *CreateContainer(game_state_t *State, v2s Pos);
fn container_t *GetContainer(game_state_t *State, v2s position);

fn inventory_t *CreateInventory(game_state_t *State);

// ======
// actions
// ======
fn b32 IsActionQueueCompleted(const game_state_t *queue);
fn void QueryAsynchronousAction(game_state_t *State, action_type_t type, entity_id_t target, v2s target_p);
fn void CommitAction(game_state_t *State, entity_t *user, entity_t *target, action_t *action, v2s target_p);

fn void UseItem(game_state_t *State, entity_t *Entity, inventory_t *Eq, item_t Item);
fn void Brace(game_state_t *queue, entity_t *entity);
fn s32 ConsumeActionPoints(game_state_t *queue, s32 count);

// ======
// grid
// ======
fn b32 IsCellEmpty(game_state_t *State, v2s p);
fn b32 ChangeCell(game_state_t *State, entity_t *Entity, v2s NewCell);
fn b32 MakeMove(game_state_t *State, entity_t *entity, v2s offset);
fn b32 Launch(game_state_t *State, v2s source, entity_t *target, u8 push_distance, s32 strength);