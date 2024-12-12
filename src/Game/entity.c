fn entity_t *GetEntityFromPosition(entity_storage_t *storage, v2s p)
{
	for (s32 index = 0; index < storage->EntityCount; index++) {
        entity_t *entity = &storage->entities[index];

        // "real" pos
        v2s topLeft = entity->p; 
        v2s bottomRight = V2S( entity->p.x + entity->size.x - 1, entity->p.y + entity->size.y - 1 );  // bottom right corner (faster? then looping twice)

        if (p.x >= topLeft.x && p.x <= bottomRight.x &&
            p.y >= topLeft.y && p.y <= bottomRight.y) {
            return entity;  // point within bounds
        }
    }
    return 0; 
}

fn entity_t *FindClosestHostile(entity_storage_t *storage, v2s player_pos)
{
	entity_t *nearest_enemy = 0;
	f32 nearest_distance = FLT_MAX;

	for (s32 i = 0; i < storage->EntityCount; i++)
	{
		entity_t *entity = &storage->entities[i];
		if (IsHostile(entity))
		{
			f32 distance = IntDistance(entity->p, player_pos);
			if (distance < nearest_distance)
			{
				nearest_enemy = entity;
				nearest_distance = distance;
			}
		}
	}

	return nearest_enemy;
}

fn entity_t *FindClosestPlayer(entity_storage_t *storage, v2s p) {
	entity_t *nearest_player = 0;
	f32 nearest_distance = FLT_MAX;

	for (s32 i = 0; i < storage->EntityCount; i++)
	{
		entity_t *entity = &storage->entities[i];
		if (IsPlayer(entity))
		{
			f32 distance = IntDistance(entity->p, p);
			if (distance < nearest_distance)
			{
				nearest_player = entity;
				nearest_distance = distance;
			}
		}
	}

	return nearest_player;
}

fn entity_t *GetEntity(entity_storage_t *storage, entity_id_t id)
{
	if (id > 0)
	{
		for (s32 index = 0; index < storage->EntityCount; index++)
		{
			entity_t *entity = &storage->entities[index];
			if (entity->id == id)
				return entity;
		}
	}
	return (0);
}

fn b32 IsHostile(const entity_t *entity)
{
	if (entity)
		return entity->flags & entity_flags_hostile;
	return false;
}

fn b32 IsPlayer(const entity_t *entity)
{
	if (entity)
		return (entity->flags & entity_flags_controllable);
	return false;
}

fn void TakeHP(entity_t *entity, s16 damage)
{
    if (entity)
    {
        entity->blink_time = 1.0f;
        if (damage >= entity->health)
        {
            entity->health = 0;
            entity->flags |= entity_flags_deleted;
        }
        else
        {
            entity->health -= damage;
        }
    }
}

fn void Heal(entity_t *entity, s16 amount) {
    if (entity)
        entity->health = Min16U(entity->max_health, entity->health + amount);
}

fn void AddStatusEffect(entity_t *entity, status_effect_type_t status_effect, s32 duration) {
    for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {

        s32 status_effect_type_value = entity->status_effects[i].type;

        if (entity->status_effects[i].type == status_effect_none) {
            switch (status_effect) {
                case status_effect_poison:
                    entity->status_effects[i].type = status_effect_poison;
                    entity->status_effects[i].remaining_turns = duration;
                    break;
                default:
                    break;
            }
        }
    }
}

fn void StepOnTile(map_t *Map, entity_t *entity)
{
	tile_t *tile = GetTile(Map, entity->p.x, entity->p.y);
	if (tile && (tile->trap_type != trap_type_none))
	{
    	switch(tile->trap_type)
    	{
    	    case trap_type_physical:
    	        TakeHP(entity, 25);
    	        break;
    	    case trap_type_poison:
    	        AddStatusEffect(entity, status_effect_poison, 3);
    	        break;
    	    default:
    	        break;
    	}
	}
}

fn void EvaluateStatusEffects(entity_t *entity)
{
    for (int i = 0; i < MAX_STATUS_EFFECTS; ++i) {
        if (entity->status_effects[i].type != status_effect_none) {
            switch (entity->status_effects[i].type) {
                case status_effect_poison:
                    TakeHP(entity, 10);
                    break;
                default:
                    break;
            }

            entity->status_effects[i].remaining_turns--;

            if (entity->status_effects[i].remaining_turns == 0) {
                DebugLog("Changing status effect to none, applying value: %d", status_effect_none);
                entity->status_effects[i].type = status_effect_none;
            }
        }
    }
}

fn b32 IsAlive(entity_t *Entity)
{
	b32 Result = false;
	if (Entity)
	{
		if (!(Entity->flags & entity_flags_deleted))
			Result = true;
	}
	return Result;
}