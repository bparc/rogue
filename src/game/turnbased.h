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
	// context
	log_t 		   	 *log;
	particles_t 	 *particles;
	entity_storage_t *storage;
	map_t 			 *Map;
	entity_id_t 	 player;
	memory_t *Memory;
	
	system_event_t Events;
	b32 EncounterModeEnabled;

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
	s32 turn_inited;
	s32 action_points, initial_action_points;
	f32 seconds_elapsed; // NOTE(): Seconds elapsed from the start of the turn.

	s32 action_count;
	async_action_t actions[1];

	// NOTE(): Some additional book-keeping
	// for the animation system to use.
	entity_id_t prev_turn_entity;
	#define ENTITY_EVICTION_SPEED 2.0f
	s32 num_evicted_entities;
	evicted_entity_t evicted_entities[8]; // // NOTE(): A small buffer that stores copies of deleted entities for a brief amount of time.

	// debug modes
	s32 god_mode_enabled;

	s32 break_mode_enabled;
	s32 request_step;
} turn_system_t;

fn b32 GodModeDisabled(turn_system_t *System)
{
	return (!System->god_mode_enabled);
}

fn void SetupTurn(turn_system_t *queue, s32 ActionPointCount)
{
	queue->initial_action_points = ActionPointCount;
	queue->action_points = queue->initial_action_points;

	queue->seconds_elapsed = 0.0f;

	queue->EnemyAnimationTime = 0.0f;
	queue->EnemyInited = false;

	queue->turn_inited = true;
}

// turn management
fn void ClearTurnQueue(turn_system_t *queue);
fn void PushTurn(turn_system_t *queue, entity_t *entity);

fn entity_t *Pull(turn_system_t *System);
fn entity_t *GetActive(const turn_system_t *System);
fn int32_t IsActive(const turn_system_t *System, entity_id_t id);

fn void InteruptTurn(turn_system_t *Queue, entity_t *Entity);
fn void AcceptTurn(turn_system_t *queue, entity_t *entity);

// actions
fn b32 IsActionQueueCompleted(const turn_system_t *queue);
fn void QueryAsynchronousAction(turn_system_t *System, action_type_t type, entity_id_t target, v2s target_p);
fn void CommitAction(turn_system_t *state, entity_t *user, entity_t *target, action_t *action, v2s target_p);
fn void UseItem(turn_system_t *State, entity_t *Entity, inventory_t *Eq, item_t Item);


// resources
fn void Brace(turn_system_t *queue, entity_t *entity);
fn s32 ConsumeActionPoints(turn_system_t *queue, s32 count);

// grid-based movement
fn b32 IsCellEmpty(turn_system_t *System, v2s p);
fn b32 ChangeCell(turn_system_t *System, entity_t *Entity, v2s NewCell);
fn b32 MakeMove(turn_system_t *System, entity_t *entity, v2s offset);
fn b32 Launch(turn_system_t *System, v2s source, entity_t *target, u8 push_distance, s32 strength);