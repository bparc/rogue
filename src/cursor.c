fn void DrawHighlightArea(command_buffer_t *out, map_t *map,v2s center, int radius, v4 color) {
    for (s32 y = center.y - radius; y <= center.y + radius; ++y)
    {
        for (s32 x = center.x - radius; x <= center.x + radius; ++x)
        {

            v2s point = V2S(x,y);

            if (IsInsideCircle(point, V2S(1,1), center, radius))
            {
                RenderIsoTile(out, map, point, A(color, 0.5f), true, 0);
            }
        }
    }
}

fn void PushEntity(game_world_t *state, entity_t *user, entity_t *target, u8 push_distance) {
	v2s direction = SubS(target->p, user->p);

	if (direction.x != 0) direction.x = (direction.x > 0) ? 1 : -1;
	if (direction.y != 0) direction.y = (direction.y > 0) ? 1 : -1;

	const s32 PLAYER_STRENGTH = user->attack_dmg; // Just temp value for the demo, since we don't have RPG systems ready yet
	s32 damage_per_tile = PLAYER_STRENGTH / (target->size.x * target->size.y);
	s32 total_damage = 0;

	for (s32 i = 0; i < push_distance; ++i) {
		v2s next_pos = AddS(target->p, direction); // Slime is moved through each tile on the way

		if (!MoveFitsWithSize(state, target, next_pos)) {
			InflictDamage(target, (s16)damage_per_tile);
			break;
		} else {
			target->p = next_pos;
			ApplyTileEffects(next_pos, state, target);
		}
	}
}

#include "actions.c"

#define BASE_HIT_CHANCE 50
#define MELEE_BONUS 15
#define GRAZE_THRESHOLD 10
#define MAX_EFFECTIVE_RANGE 3
#define DISTANCE_PENALTY_PER_TILE 9
#define CRITICAL_HIT_CHANCE 10
#define CRITICAL_DAMAGE_MULTIPLIER 2
#define SKIP_TURN_HIT_CHANCE_INCREASE 15
fn s32 CalculateHitChance(entity_t *user, entity_t *target, action_type_t action_type) {
	s32 final_hit_chance;
	f32 distance = DistanceV2S(user->p, target->p);

	if (action_type != action_melee_attack) {
		final_hit_chance = user->ranged_accuracy - target->evasion + BASE_HIT_CHANCE;
		if (user->has_hitchance_boost) {
			final_hit_chance += SKIP_TURN_HIT_CHANCE_INCREASE;
			user->has_hitchance_boost = false;
		}

		if (distance > MAX_EFFECTIVE_RANGE) {
			s32 penalty = (s32) (distance - MAX_EFFECTIVE_RANGE) * DISTANCE_PENALTY_PER_TILE;
			final_hit_chance -= penalty;
		}

	} else {
		final_hit_chance = user->ranged_accuracy - target->evasion + BASE_HIT_CHANCE + MELEE_BONUS;
		if (user->has_hitchance_boost) {
			final_hit_chance += SKIP_TURN_HIT_CHANCE_INCREASE;
			user->has_hitchance_boost = false;
		}
	}

	if (final_hit_chance > 100) {
		final_hit_chance = 100;
	}

	return final_hit_chance;
}

// todo: add critical hits, distance measure
fn void HandleAttack(entity_t *user, entity_t *target, action_type_t action_type, game_world_t *state) {

	s32 final_hit_chance = CalculateHitChance(user, target, action_type);

	if (final_hit_chance < 0) final_hit_chance = 0;
	if (final_hit_chance > 100) final_hit_chance = 100;

	s32 roll = rand() % 100;
	s32 crit_roll = rand() % 100;

	if (roll < final_hit_chance) {

		if (crit_roll < CRITICAL_DAMAGE_MULTIPLIER) {
			s32 crit_damage = user->attack_dmg * CRITICAL_DAMAGE_MULTIPLIER;
			PushEntity(state, user, target, 2);
			InflictDamage(target, (u16)crit_damage);
			DebugLog("Critical Hit! Inflicted %i critical damage to target #%i", crit_damage, target->id);
		} else {
			InflictDamage(target, user->attack_dmg);
			DebugLog("Hit! Inflicted %i damage to target #%i", user->attack_dmg, target->id);
		}

	} else if (roll < final_hit_chance + GRAZE_THRESHOLD && roll >= final_hit_chance) {
		u16 graze_damage = user->attack_dmg / 2;
		InflictDamage(target, graze_damage);
		DebugLog("Grazing hit! Inflicted %i grazing damage to target #%i", graze_damage, target->id);
	} else {
		DebugLog("Missed! Ranged attack missed target #%i", target->id);
	}
}

fn void DrawHitChance(game_world_t *state, assets_t *assets, command_buffer_t *out, v2s enemy_position, s32 hit_chance)
{
	v2 screen_position = MapToScreen(state->map, enemy_position);
	screen_position = ScreenToIso(screen_position);

	char hit_chance_text[16];
	snprintf(hit_chance_text, sizeof(hit_chance_text), "%d%%", hit_chance);

	screen_position.x += 25;
	screen_position.y -= 20;

	DrawText(out, assets->Font, screen_position, hit_chance_text, White());
}

#define GRENADE_EXPLOSION_RADIUS 3	// temp value
#define GRENADE_DAMAGE 50			// temp value
// todo: Make walls and out of boundary zone protect entities from explosions
fn void ActivateSlotAction(entity_t *user, entity_t *target, action_t *action,
v2s target_p, entity_storage_t *storage, game_world_t *state, turn_queue_t *queue)
{
    switch(action->type) {
        case action_ranged_attack:
        {
	        HandleAttack(user, target, action->type, state);
	        break;
        }
        case action_melee_attack:
        {
        	HandleAttack(user, target, action->type, state);
        	break;
        }
    	case action_throw:
    	{
    		s32 radius = action->params.area_of_effect.x;
			v2s explosion_center = state->cursor->p;
			for (s32 i = 0; i < storage->num; i++) {
				entity_t *entity = &storage->entities[i];
				if (IsInsideCircle(entity->p, entity->size, explosion_center, radius)) {
					InflictDamage(entity, (s16)action->params.damage);
				}
			}
			break;
		}
		case action_push:
    	{
			PushEntity(state, user, target, 4);
			break;
		}
        case action_heal_self:
        {
        	if (target == user)
        	{
        		Heal(target, (s16)action->params.damage);
        		DebugLog("healed up for %i hp", (s16)action->params.damage);
        	}
        }	break;
        case action_none:
        default:
            break;
    }

	DebugLog("Used %i action points", action->params.action_point_cost);
	queue->action_points -= action->params.action_point_cost;
	if (queue->action_points < 0) {
		queue->action_points = 0;
	}

}

fn void	DoCursor(
	command_buffer_t *out,
	entity_t *user, // the entity that currently uses the cursor
	virtual_controls_t cons,
	b32 move_requested, s32 direction, const v2s dirs[4], // the player wants to move
	turn_queue_t *queue, map_t *map, entity_storage_t *storage, log_t *log, cursor_t *cursor,
	slot_bar_t bar, game_world_t *state, assets_t *assets)
{
	cursor->target_id = 0;
	
	Assert(user);
	if ((cursor->active == false) && WentDown(cons.confirm))
	{
		// NOTE(): The cursor was just activated.
		// Setup a starting state.
		cursor->active = true;
		cursor->p = user->p;
	}

	if (cursor->active)
	{
		// NOTE(): Close the cursor, if needed.
		if (WentDown(cons.cancel))
			cursor->active = false;

		DebugAssert((bar.selected_slot >= 0) &&
		(bar.selected_slot < ArraySize(bar.slots))); // NOTE(): Validate, just in case.
		action_t equipped = bar.slots[bar.selected_slot - 1].action; // NOTE(): Slot IDs start from 1?
		equipped.params = DefineActionTypeParams(user, equipped);
		if (equipped.type == action_none)
		{
			DebugLog("An unsupported action was selected! Can't open the cursor!");
			cursor->active = false;
			return;
		}
		if ((equipped.type == action_heal_self)) // NOTE(): Some skills could activate directly from the bar?
		{
			if (WentDown(cons.confirm))
				if (queue->action_points >= equipped.params.action_point_cost) {
					ActivateSlotAction(user, user, &equipped, cursor->p, storage, state, queue);
				}
			else
				cursor->active = false;
			return;
		}

		// NOTE(): Draw the maximum range of the cursor.
		DrawHighlightArea(out, map, user->p, equipped.params.range, Pink());

		// NOTE() : Draw the explosion radius.
		if (equipped.type == action_throw) {
			DrawHighlightArea(out, map, cursor->p, equipped.params.area_of_effect.x, Red());
		}
		// NOTE(): Draw the cursor.
		RenderIsoTile(out, map, cursor->p, A(Pink(), 0.8f), true, 0);

		// NOTE(): Move the cursor.
		v2s requestedPos = AddS(cursor->p, move_requested ? dirs[direction] : V2S(0, 0));

		b32 in_range = false;
		in_range = IsInsideCircle(requestedPos, V2S(1,1), user->p, equipped.params.range);
		if (move_requested && in_range)
				cursor->p = requestedPos;
		// NOTE(): If the cursor somehow ended up out of its range -
		// move it back to the user.
		in_range = IsInsideCircle(cursor->p, V2S(1,1), user->p, equipped.params.range);
		if ((in_range == false))
			cursor->p = user->p;

		// NOTE(): Find the closest hostile to the cursor, draw
		// tiles underneath them, then snap to them if the button went down.
		entity_t *Enemy = FindClosestHostile(storage, cursor->p);
		if (Enemy)
		{ 
			if (IsInsideCircle(Enemy->p, Enemy->size, user->p, equipped.params.range))
			{

				RenderIsoTileArea(out, map, Enemy->p, AddS(Enemy->p, Enemy->size), A(Red(), 0.8f)); //render target for all size enemies
				if (WentDown(cons.snap_cursor))
					cursor->p = Enemy->p;
			}
		}

		// NOTE(): Get target.
		entity_t *target = GetEntityByPosition(storage, cursor->p);
		b32 target_valid = IsHostile(target);
		if (target_valid && target)
		{
			cursor->target_id = target->id;

			// NOTE(): Draw the hit chance
			s32 hit_chance = CalculateHitChance(user, target, equipped.type);
			DrawHitChance(state, assets, out, target->p, hit_chance);
		}
		// NOTE(): Pick the target under the cursor and perform the "slot action" on it.
		if (WentUp(cons.confirm))
		{

			if (equipped.type == action_throw) // NOTE(): "action_throw" can also target traversable tiles.
				target_valid |= IsTraversable(map, cursor->p); 

			b32 positioned_on_user = CompareVectors(cursor->p, user->p);
			if (target_valid && (positioned_on_user == false))
			{

				if (queue->action_points >= equipped.params.action_point_cost) {
					//ActivateSlotAction(user, target, equipped, cursor, storage, map);
					QueryAsynchronousAction(queue, equipped.type, target, cursor->p);
					cursor->active = false;
				}

			}
		}
	}
}