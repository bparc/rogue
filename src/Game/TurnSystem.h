typedef enum
{
	interp_request,
	interp_transit,
	interp_action,
	interp_accept,
	interp_wait_for_input,
} interpolator_state_t;

const char *interpolator_state_t_names[] = {
	"Request",
	"Transit",
	"Attack",
	"Accept",
	"Break",
};

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

	entity_storage_t *storage;
	map_t *map;

	// NOTE(): Stores the turns as an list of entity ids
	// in a *reverse* order (the last turn in the queue will be executed first).
	s32 QueueSize;
	entity_id_t Queue[64];
	entity_id_t Player;

	// NOTE(): Turn State
	v2s starting_p;
	interpolator_state_t interp_state;
	f32 time; // NOTE(): A variable within 0.0 to 1.0 range for interpolating values.

	s32 turn_inited;
	s32 max_action_points;
	s32 action_points;
	s32 movement_points;
	f32 seconds_elapsed; // NOTE(): Seconds elapsed from the start of the turn.

	s32 action_count;
	async_action_t actions[1];

	// Some additional book-keeping
	// for the animation system to use.
	entity_id_t prev_turn_entity;
	// NOTE(): A small buffer that stores
	// copies of deleted entities for
	// a brief amount of time.
	#define ENTITY_EVICTION_SPEED 2.0f
	s32 num_evicted_entities;
	evicted_entity_t evicted_entities[8];

	// modes
	s32 free_camera_mode_enabled;
	s32 god_mode_enabled;

	s32 break_mode_enabled;
	interpolator_state_t requested_state;
	s32 request_step;

	// NOTE(): AI stuff
	path_t path;
} turn_system_t;

fn void SetupTurn(turn_system_t *queue, s32 MovementPointCount)
{
	queue->movement_points = MovementPointCount;
	queue->action_points = 3;
	queue->interp_state = interp_request;
	queue->time = 0.0f;
	queue->seconds_elapsed = 0.0f;

	queue->turn_inited = true;
}

// turn management
fn void ClearTurnQueue(turn_system_t *queue);
fn void PushTurn(turn_system_t *queue, entity_t *entity);

fn void InteruptTurn(turn_system_t *Queue, entity_t *Entity);
fn void AcceptTurn(turn_system_t *queue, entity_t *entity);

// animation system
fn void QueryAsynchronousAction(turn_system_t *queue, action_type_t type, entity_id_t target, v2s target_p);
fn b32 IsActionQueueCompleted(const turn_system_t *queue);

// resources
fn void Brace(turn_system_t *queue, entity_t *entity);
fn s32 ConsumeMovementPoints(turn_system_t *queue, s32 count);
fn s32 ConsumeActionPoints(turn_system_t *queue, s32 count);
fn entity_t *GetActiveUnit(const turn_system_t *queue);

// grid-based movement
fn b32 IsCellEmpty(turn_system_t *System, v2s p);
fn b32 Move(turn_system_t *System, entity_t *entity, v2s offset);
fn b32 Launch(turn_system_t *System, v2s source, entity_t *target, u8 push_distance, s32 strength);