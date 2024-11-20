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
	// context
	log_t 		   	 *log;
	particles_t 	 *particles;
	entity_storage_t *storage;
	map_t 			 *map;
	entity_id_t 	 player;

	// queue
	s32 QueueSize;
	entity_id_t Queue[64]; // // NOTE(): Stores the turns as an list of entity ids in a *reverse* order (the last turn in the queue will be executed first).

	// turn state
	s32 turn_inited;
	s32 action_points, initial_action_points;
	s32 movement_points;
	f32 seconds_elapsed; // NOTE(): Seconds elapsed from the start of the turn.

	s32 action_count;
	async_action_t actions[1];

	// animation system
	interpolator_state_t interp_state;
	v2s starting_p;
	f32 time; // NOTE(): A variable within 0.0 to 1.0 range for interpolating values.

	// NOTE(): Some additional book-keeping
	// for the animation system to use.
	entity_id_t prev_turn_entity;
	#define ENTITY_EVICTION_SPEED 2.0f
	s32 num_evicted_entities;
	evicted_entity_t evicted_entities[8]; // // NOTE(): A small buffer that stores copies of deleted entities for a brief amount of time.

	// debug modes
	s32 free_camera_mode_enabled;
	s32 god_mode_enabled;

	s32 break_mode_enabled;
	interpolator_state_t requested_state;
	s32 request_step;

	// AI
	path_t path;
} turn_system_t;

fn b32 GodModeDisabled(turn_system_t *System)
{
	return (!System->god_mode_enabled);
}

fn void SetupTurn(turn_system_t *queue, s32 MovementPointCount, s32 ActionPointCount)
{
	queue->initial_action_points = ActionPointCount;
	queue->action_points = queue->initial_action_points;
	
	queue->movement_points = MovementPointCount;
	
	queue->interp_state = interp_request;
	queue->time = 0.0f;
	queue->seconds_elapsed = 0.0f;

	queue->turn_inited = true;
}

// turn management
fn void ClearTurnQueue(turn_system_t *queue);
fn void PushTurn(turn_system_t *queue, entity_t *entity);

fn entity_t *PullUntilActive(turn_system_t *System);
fn entity_t *GetActive(const turn_system_t *System);
fn int32_t IsActive(const turn_system_t *System, entity_id_t id);

fn void InteruptTurn(turn_system_t *Queue, entity_t *Entity);
fn void AcceptTurn(turn_system_t *queue, entity_t *entity);

// actions
fn void QueryAsynchronousAction(turn_system_t *System, action_type_t type, entity_id_t target, v2s target_p);
fn void CommitAction(turn_system_t *state, entity_t *user, entity_t *target, action_t *action, v2s target_p);
fn void UseItem(turn_system_t *State, entity_t *Entity, inventory_t *Eq, item_t Item);

// animation system
fn void QueryAsynchronousAction(turn_system_t *queue, action_type_t type, entity_id_t target, v2s target_p);
fn b32 IsActionQueueCompleted(const turn_system_t *queue);

// resources
fn void Brace(turn_system_t *queue, entity_t *entity);
fn s32 ConsumeMovementPoints(turn_system_t *queue, s32 count);
fn s32 ConsumeActionPoints(turn_system_t *queue, s32 count);

// grid-based movement
fn b32 IsCellEmpty(turn_system_t *System, v2s p);
fn b32 MakeMove(turn_system_t *System, entity_t *entity, v2s offset);
fn b32 Launch(turn_system_t *System, v2s source, entity_t *target, u8 push_distance, s32 strength);