fn void ActivateSlotAction(game_world_t *state, entity_t *user, entity_t *target, action_t *action);
fn void HandleAttack(game_world_t *state, entity_t *user, entity_t *target, action_type_t action_type);

#define BASE_HIT_CHANCE 50
#define MELEE_BONUS 15
#define GRAZE_THRESHOLD 10
#define MAX_EFFECTIVE_RANGE 3
#define DISTANCE_PENALTY_PER_TILE 9
#define CRITICAL_HIT_CHANCE 10
#define CRITICAL_DAMAGE_MULTIPLIER 2

fn s32 CalculateHitChance(const entity_t *user, const entity_t *target, action_type_t action_type);

fn void PushEntity(game_world_t *state, v2s source, entity_t *target, u8 push_distance, s32 strength);