/*
fn void UseItem(game_state_t *State, entity_t *Entity, inventory_t *Eq, item_t Item)
{
    action_type_t Action = Item.params->action;
    if (Action != action_none)
    {
        CombatAction(State, Entity, Action, Entity->id, Entity->p);
        Eq_RemoveItem(Eq, Item.ID);
    }
}
*/

fn void AddStatusEffect(game_state_t *State, entity_t *Entity, status_effect_type_t Type, s32 Duration)
{
    status_effect_t *Effect = &Entity->StatusEffect;
    if ((Effect->duration <= 0) ||
        (Effect->type != Type))
    {
        ZeroStruct(Effect);
        Effect->duration = Duration;
        Effect->type = Type;
        
        _StatusEffectText(State, Entity, Effect->type);
    }
}

fn void RemoveStatusEffect(game_state_t *State, entity_t *Entity)
{
    ZeroStruct(&Entity->StatusEffect);
}

fn void CombatAction(game_state_t *State, entity_t *User, v2s Target, const action_params_t *Action)
{
	b32 TargetValid = true;

	#if 0
    if (IsTargetField(Action->type))
    {
    	TargetValid = (TargetValid && IsTraversable(&State->Map, Target));
    }

    if (IsTargetHostile(Action->type))
    {
    	TargetValid = (TargetValid && IsHostile(target));
    }
    #endif

    entity_t *Entity = GetEntityFromPosition(&State->Units, Target);

    if (!Entity)
    {
    	DebugWarning("TARGET INVALID (%i, %i)", Target.x, Target.y);
    }

    if (IsLineOfSight(&State->Map, User->p, Target) && Entity)
    {
       	if (ConsumeActionPoints(State, Action->cost))
       	{
       		QueryAsynchronousAction(State, Action->type, Entity ? Entity->id : 0, Target);
       	}
    }
}

fn void InflictDamage(game_state_t *State, entity_t *user, entity_t *target, s32 damage, const char *damage_type_prefix)
{
    damage = damage + (rand() % 3);

    if (target->StatusEffect.duration > 0)
    {
    	AilmentPreDamage(State, target->StatusEffect.type, target, &damage);
    }

    LogLn(State->Log, "%shit! inflicted %i %s of %sdamage upon the target!",
        damage_type_prefix, damage, damage == 1 ? "point" : "points", damage_type_prefix);
    
    TakeHP(target, (s16)damage);

    CreateDamageNumber(&State->ParticleSystem, Add(target->deferred_p, V2(-25.0f, -25.0f)), damage);

    if ((rand() / (float)RAND_MAX) < 20) {
        blood_type_t blood_type = (target->enemy_type == 0) ? blood_red : blood_green;
        hit_velocity_t hit_velocity = (rand() % 2 == 0) ? high_velocity : low_velocity; // Just a temporary thing until we add ammo types
        BloodSplatter(&State->Map, user->p, target->p, blood_type, hit_velocity);
    }
}

fn inline void _SingleTarget(game_state_t *State, entity_t *user, entity_t *target, const action_params_t *params)
{
	s32 chance = 100;
	CalculateHitChance(user, target, params->type);
    s32 roll = rand() % 100;
    s32 roll_crit = rand() % 100;
    b32 missed = !(roll < chance);
    b32 crited = (roll_crit < CRITICAL_DAMAGE_MULTIPLIER);
    b32 grazed = ((roll >= chance)) && (roll < (chance + GRAZE_THRESHOLD));

    if ((missed == false))
    {    
        if (crited) {
            InflictDamage(State, user, target, params->damage * CRITICAL_DAMAGE_MULTIPLIER, "critical ");
            CreateCombatText(&State->ParticleSystem, target->deferred_p, combat_text_critical);
        } else {
            InflictDamage(State, user, target, params->damage, "");
            CreateCombatText(&State->ParticleSystem, target->deferred_p, combat_text_hit);
        }
    }
    else
    {
        if (grazed) {
            InflictDamage(State, user, target, params->damage / 2, "graze ");
            CreateCombatText(&State->ParticleSystem, target->deferred_p, combat_text_graze);
        } else {
            LogLn(State->Log, "missed!");
            CreateCombatText(&State->ParticleSystem, target->deferred_p, combat_text_miss);
        }
    }

    DebugWarning("TODO(): IMPLEMENT STATUS EFFECTS FOR AOES");
  	AddStatusEffect(State, target, params->status_effect, 3);
}

fn inline void _AOE(game_state_t *State, entity_t *user, entity_t *target, const action_params_t *params, v2s TileIndex)
{
    const char *prefix = "blast ";
    s32 damage = params->damage;
    v2s area = params->area;
    s32 radius_inner = area.x;
    s32 radius_outer = area.x * (s32)2;
    v2s explosion_center = TileIndex;
    for (s32 i = 0; i < State->Units.EntityCount; i++)
    {
        entity_t *entity = &State->Units.entities[i];
        f32 distance = IntDistance(explosion_center, entity->p);
        
        if (distance <= radius_inner) {
            InflictDamage(State, user, entity, damage, prefix);
        } else if (distance <= radius_outer) {
            InflictDamage(State, user, entity, damage, prefix);
            Launch(State, explosion_center, entity, 2, 25);
        }
    }
}

fn void CommitCombatAction(game_state_t *State, entity_t *user, entity_t *target, action_t *action, v2s target_p)
{
    const action_params_t *params = GetActionParams(action->type);

    switch (params->mode)
    {
    case action_mode_damage:
    {
        if (IsZero(params->area))
        {
        	if (target)
        	{
            	_SingleTarget(State, user, target, params);
        	}
        }
        else
        {
            _AOE(State, user, target, params, target_p);
        }

        // NOTE(): Reset per-turn attack buffs/modifiers.
        user->has_hitchance_boost = false;
        user->hitchance_boost_multiplier = 1.0f;
    } break;
    case action_mode_heal:
    	{
    		Heal(target, (s16) params->value);
    		CreateCombatText(&State->ParticleSystem, target->deferred_p, combat_text_heal);
    	} break;
    case action_mode_dash: user->p = target_p; break;
    }
}