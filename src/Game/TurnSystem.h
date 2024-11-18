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
	// NOTE(): Stores the turns as an list of entity ids
// in a *reverse* order (the last turn in the queue will be executed first).
	s32 num;
	entity_id_t entities[64];
	entity_storage_t *storage;

	// NOTE(): Turn State
	v2s starting_p;
	interpolator_state_t interp_state;
	f32 time; // NOTE(): A variable within 0.0 to 1.0 range for interpolating values.

	s32 max_action_points;
	s32 action_points;
	s32 movement_points;
	s32 turn_inited;
	f32 seconds_elapsed; // NOTE(): Seconds elapsed from the start of the turn.

	s32 action_count;
	async_action_t actions[1];

	// NOTE(): Some additional book-keeping
	// for the animation system to use.
	entity_id_t prev_turn_entity;
	// NOTE(): A small buffer that stores
	// copies of deleted entities for
	// a brief amount of time.
	#define ENTITY_EVICTION_SPEED 2.0f
	s32 num_evicted_entities;
	evicted_entity_t evicted_entities[8];

	s32 free_camera_mode_enabled;
	s32 god_mode_enabled;

	s32 break_mode_enabled;
	interpolator_state_t requested_state;
	s32 request_step;

	// NOTE(): AI stuff
	path_t path;
} turn_queue_t;

fn void SetupTurn(turn_queue_t *queue, s32 MovementPointCount)
{
	queue->movement_points = MovementPointCount;
	queue->action_points = 3;
	queue->interp_state = interp_request;
	queue->time = 0.0f;
	queue->seconds_elapsed = 0.0f;

	queue->turn_inited = true;
}

fn void AcceptTurn(turn_queue_t *queue, entity_t *entity);;
fn void Brace(turn_queue_t *queue, entity_t *entity);

fn void PushTurn(turn_queue_t *queue, entity_t *entity);
fn void ClearTurnQueue(turn_queue_t *queue);
fn void QueryAsynchronousAction(turn_queue_t *queue, action_type_t type, entity_id_t target, v2s target_p);

fn s32 CountHostiles(turn_queue_t *Queue);

fn s32 ConsumeMovementPoints(turn_queue_t *queue, s32 count);
fn s32 ConsumeActionPoints(turn_queue_t *queue, s32 count);
fn entity_t *GetActiveUnit(const turn_queue_t *queue);

fn b32 IsActionQueueCompleted(const turn_queue_t *queue);