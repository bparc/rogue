typedef struct
{
	memory_t *Memory;
	assets_t *Assets;
	log_t *Log;

	min_queue_t FloodQueue;
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
	entity_id_t ActiveUnit;

	// enemy

	float EnemyLerp;
	s32 EnemyInited;
	path_t EnemyPath;
	
	// phase

	#define MAX_PER_PHASE 64
	s32 PhaseSize;
	s32 QueueSize;
	entity_id_t Queue[MAX_PER_PHASE]; // // NOTE(): Queue encoded as an list of entity ids in a *reverse* order (the last turn in the queue will be executed first).
	entity_id_t Phase[MAX_PER_PHASE];
	#undef MAX_PER_PHASE

	// current turn

	range_map_t EffectiveRange;
	s32 TurnInited;
	s32 ActionPoints;
	f32 SecondsElapsed; // NOTE(): Seconds elapsed from the start of the turn.

	// action animator

	s32 ActionCount;
	async_action_t Actions[1];

	// NOTE(): An additional book-keeping
	// for the animation system to use.

	entity_id_t PrevTurnEntity;
	#define ENTITY_EVICTION_SPEED 2.0f
	s32 EvictedEntityCount;
	evicted_entity_t EvictedEntities[8]; // // NOTE(): A small buffer that stores copies of deleted entities for a brief amount of time.
} game_state_t;

// =============
// turn-based
// ============

fn b32 SetupNewTurn(game_state_t *State, s32 ActionPointCount);

fn void PushTurn(game_state_t *queue, entity_t *entity);
fn void EndTurn(game_state_t *State);
fn void InteruptTurn(game_state_t *State, entity_t *Entity);

fn int32_t IsActive(const game_state_t *State, entity_id_t id);
fn entity_t *GetActive(const game_state_t *State);

fn s32 ConsumeActionPoints(game_state_t *State, s32 count);

fn void RemoveMovementRange(game_state_t *State);
fn void CreateMovementRange(game_state_t *State, entity_t *Entity, s32 Range);

// =============
// animator
// ============

// The animation system is responsible for animating and rendering unit actions (such as
// attacking). It also takes care of actually
// commiting an action to the game logic.

fn b32 IsActionQueueCompleted(const game_state_t *State);
fn void QueryAsynchronousAction(game_state_t *State, action_type_t type, entity_id_t target, v2s target_p);
fn void AnimateAction(const game_state_t *State, const entity_t *Entity, async_action_t *Act, command_buffer_t *Out, f32 dt);

// ============
// objects
// ============

fn entity_t *CreateEntity(game_state_t *State, v2s p, v2s size, u8 flags, u16 health_points, u16 attack_dmg, const map_t *Map, u16 max_health_points, s32 accuracy, s32 evasion, s32 remaining_action_points, s32 remaining_movement_points, f32 hitchance_boost_multiplier);
fn entity_t *GetEntity(entity_storage_t *storage, entity_id_t id);
fn entity_t *GetEntityFromPosition(entity_storage_t *storage, v2s p);

fn entity_t *CreatePlayer(game_state_t *State, v2s p);
fn entity_t *FindClosestPlayer(entity_storage_t *storage, v2s p);
fn entity_t *FindClosestHostile(entity_storage_t *storage, v2s player_pos);

fn container_t *CreateContainer(game_state_t *State, v2s Pos);
fn container_t *GetContainer(game_state_t *State, v2s position);
fn container_t *GetAdjacentContainer(game_state_t *State, v2s Cell);

fn b32 GetAdjacentDoor(game_state_t *State, v2s Cell, v2s *DoorCell);

fn inventory_t *CreateInventory(game_state_t *State);

// ============
// combat
// ============

fn s32 CalculateHitChance(const entity_t *user, const entity_t *target, action_type_t action_type);

// Immediately executes an action (modifies the game logic).
fn void CommitCombatAction(game_state_t *State, entity_t *user, entity_t *target, action_t *action, v2s target_p);

// Schedules a deferred action that will be executed by the animation system. (See: QueryAsynchronousAction()/AnimateAction())
fn void CombatAction(game_state_t *State, entity_t *User, v2s Target, const action_params_t *Action);

// Inflicts a damage from one unit to another. The "user" can be the same as the "target".
fn void InflictDamage(game_state_t *State, entity_t *user, entity_t *target, s32 damage, const char *damage_type_prefix);

fn void UseItem(game_state_t *State, entity_t *Entity, inventory_t *Eq, item_t Item);

fn void AddStatusEffect(game_state_t *State, entity_t *Entity, status_effect_type_t status_effect, s32 duration);
fn void RemoveStatusEffect(game_state_t *State, entity_t *Entity);

fn void Brace(game_state_t *State, entity_t *entity);

// ============
// ailments.c
// ============

// Routines that implement all of status effect logic.

// Triggers before the start of every turn.
fn void StatusEffects_Evaluate(game_state_t *State, entity_t *Entity, status_effect_t *Effect);

// Triggers right before an entity inflicts a damage. Modifies that
// damage based on the status effect type.
fn void StatusEffects_PreDamage(game_state_t *State, status_effect_type_t Type, entity_t *DamageTarget, s32 *Damage);

// ============
// movement
// ============

fn b32 ChangeCell(game_state_t *State, entity_t *Entity, v2s NewCell);
fn b32 MakeMove(game_state_t *State, entity_t *entity, v2s offset);
fn b32 Launch(game_state_t *State, v2s source, entity_t *target, u8 push_distance, s32 strength);

// ============
// grid
// ============

fn b32 IsCellEmpty(game_state_t *State, v2s p);