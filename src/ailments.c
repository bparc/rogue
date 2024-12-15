fn void StatusEffects_Evaluate(game_state_t *State, entity_t *Entity, status_effect_t *Effect)
{
    DebugLog("...");

    switch (Effect->type)
    {
    case status_effect_poison:
    {
        InflictDamage(State, Entity, Entity, 4, "poison");
    } break;
    case status_effect_freeze:
    {
        EndTurn(State);
    } break;
    case status_effect_stun:
    {
        RemoveMovementRange(State);
    } break;
    }

    CreateTextParticle(&State->ParticleSystem, Entity->deferred_p, status_effect_Colors[Effect->type], status_effect_Text[Effect->type]);

    if (--Effect->duration <= 0)
    {
        RemoveStatusEffect(State, Entity);
    }
}

fn void StatusEffects_PreDamage(game_state_t *State, status_effect_type_t Type, entity_t *DamageTarget, s32 *Damage)
{
    switch (Type)
    {
    case status_effect_freeze:
        {
            CreateTextParticle(&State->ParticleSystem, DamageTarget->deferred_p, Blue(), "SHATTER");
            RemoveStatusEffect(State, DamageTarget);
            *Damage *= 2;
        } break;
    }
}