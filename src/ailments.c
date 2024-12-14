fn inline void _StatusEffectText(game_state_t *State, entity_t *Entity, status_effect_type_t Type)
{
    CreateTextParticle(&State->ParticleSystem, Entity->deferred_p, status_effect_Colors[Type], status_effect_Text[Type]);
}

fn void AilmentEvaluate(game_state_t *State, entity_t *Entity, status_effect_t *Effect)
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
        EndTurn(State, Entity);
    } break;
    case status_effect_stun:
    {
        RemoveMovementRange(State);
    } break;
    }

    _StatusEffectText(State, Entity, Effect->type);

    if (--Effect->duration <= 0)
    {
        RemoveStatusEffect(State, Entity);
    }
}

fn void AilmentPreDamage(game_state_t *State, status_effect_type_t Type, entity_t *DamageTarget, s32 *Damage)
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