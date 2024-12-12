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

typedef enum
{
	system_any_evicted  = 1 << 1,
} system_event_t;

typedef struct
{
	memory_t *Memory;
	assets_t *Assets;
	log_t *Log;

	particles_t ParticleSystem;
	interface_t GUI;
	
	cursor_t Cursor;

	map_layout_t MapLayout;
	map_t Map;

	camera_t Camera;
	slot_bar_t Bar;

	system_event_t Events;
	b32 EncounterModeEnabled;

	entity_storage_t Units;
	entity_id_t Players[1];

	// enemy
	float EnemyAnimationTime;
	s32 EnemyInited;
	path_t EnemyPath;
	
	// phase
	range_map_t EffectiveRange;
	#define MAX_PER_PHASE 64
	s32 PhaseSize;
	s32 QueueSize;
	entity_id_t Queue[MAX_PER_PHASE]; // // NOTE(): Stores the System as an list of entity ids in a *reverse* order (the last turn in the queue will be executed first).
	entity_id_t Phase[MAX_PER_PHASE];
	#undef MAX_PER_PHASE

	// turn
	s32 TurnInited;
	s32 ActionPoints;
	f32 SecondsElapsed; // NOTE(): Seconds elapsed from the start of the turn.

	// actions
	s32 ActionCount;
	async_action_t Actions[1];

	// NOTE(): Some additional book-keeping
	// for the animation system to use.
	entity_id_t PrevTurnEntity;
	#define ENTITY_EVICTION_SPEED 2.0f
	s32 EvictedEntityCount;
	evicted_entity_t EvictedEntities[8]; // // NOTE(): A small buffer that stores copies of deleted entities for a brief amount of time.
} game_state_t;

fn void SetupTurn(game_state_t *queue, s32 ActionPointCount);

fn void ClearTurnQueue(game_state_t *queue);
fn void PushTurn(game_state_t *queue, entity_t *entity);

fn entity_t *GetActive(const game_state_t *System);
fn int32_t IsActive(const game_state_t *System, entity_id_t id);

fn void InteruptTurn(game_state_t *Queue, entity_t *Entity);
fn void AcceptTurn(game_state_t *queue, entity_t *entity);

fn b32 IsActionQueueCompleted(const game_state_t *queue);
fn void QueryAsynchronousAction(game_state_t *System, action_type_t type, entity_id_t target, v2s target_p);
fn void CommitAction(game_state_t *state, entity_t *user, entity_t *target, action_t *action, v2s target_p);
fn void UseItem(game_state_t *State, entity_t *Entity, inventory_t *Eq, item_t Item);

fn void Brace(game_state_t *queue, entity_t *entity);
fn s32 ConsumeActionPoints(game_state_t *queue, s32 count);

fn b32 IsCellEmpty(game_state_t *System, v2s p);
fn b32 ChangeCell(game_state_t *System, entity_t *Entity, v2s NewCell);
fn b32 MakeMove(game_state_t *System, entity_t *entity, v2s offset);
fn b32 Launch(game_state_t *System, v2s source, entity_t *target, u8 push_distance, s32 strength);