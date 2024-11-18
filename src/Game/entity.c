fn entity_t *CreateEntity(entity_storage_t *storage, v2s p, v2s size, u8 flags, u16 health_points, u16 attack_dmg, const map_t *map, u16 max_health_points, s32 accuracy, s32 evasion, s32 remaining_action_points, s32 remaining_movement_points, f32 hitchance_boost_multiplier)
{
	entity_t *result = 0;
	if (storage->EntityCount < ArraySize(storage->entities))
		result = &storage->entities[storage->EntityCount++];
	if (result)
	{
		ZeroStruct(result); //memset macro
		result->p = p;
		result->deferred_p = GetTileCenter(map, result->p);

		result->size = size;

		result->id = (0xff + (storage->IDPool++));
		result->flags = flags;
		result->health = health_points;
		result->attack_dmg = attack_dmg;
		result->max_health = max_health_points;
		result->melee_accuracy = accuracy;
		result->ranged_accuracy = accuracy;
		result->evasion = evasion;
		result->remaining_action_points = remaining_action_points;
		result->remaining_action_points = remaining_movement_points;
		result->hitchance_boost_multiplier = hitchance_boost_multiplier;

		// Initializing status effects to status_effect_none (which is 1)
		for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
			result->status_effects[i].type = status_effect_none;
			result->status_effects[i].remaining_turns = 0;
		}
	}
	
	return result;
}

fn static_entity_t * CreateStaticEntity(entity_storage_t *storage, v2s p, v2s size, u8 flags, status_effect_t status_effects[MAX_STATUS_EFFECTS])
{
  	static_entity_t *result = 0;
	if (storage->StaticEntityCount < ArraySize(storage->static_entities))
		result = &storage->static_entities[storage->StaticEntityCount++];
	if (result)
	{
		ZeroStruct(result); //memset macro
		result->p = p;

		result->size = size;
		result->flags = flags;

		//array cpy im killing myself
        for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
            result->status_effects[i] = status_effects[i];
        }
	}
	
	return result;
}

fn entity_t *GetEntityByPosition(entity_storage_t *storage, v2s p)
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
			f32 distance = DistanceV2S(entity->p, player_pos);
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
			f32 distance = DistanceV2S(entity->p, p);
			if (distance < nearest_distance)
			{
				nearest_player = entity;
				nearest_distance = distance;
			}
		}
	}

	return nearest_player;
}

fn v2s GetDirectionToClosestPlayer(entity_storage_t *storage, v2s p) {
	entity_t *nearest_player = FindClosestPlayer(storage, p);

	v2s direction = Sub32(nearest_player->p, p);

	if (direction.x != 0) direction.x = (direction.x > 0) ? 1 : -1;
	if (direction.y != 0) direction.y = (direction.y > 0) ? 1 : -1;

	return direction;
}

fn entity_t *EntityFromIndex(entity_storage_t *storage, s32 index)
{
	if ((index >= 0) && (index < storage->EntityCount))
		return &storage->entities[index];
	return NULL;
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

fn entity_t *DEBUGGetPlayer(entity_storage_t *storage)
{
	for (s32 index = 0; index < storage->EntityCount; index++)
		if (IsPlayer(&storage->entities[index]))
			return &storage->entities[index];
	return 0;
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

fn void ApplyTrapEffects(tile_t *tile, entity_t *entity) {
    switch(tile->trap_type) {
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

fn void ApplyTileEffects(map_t *map, entity_t *entity)
{
    tile_t *tile = GetTile(map, entity->p.x, entity->p.y);

    if (tile->trap_type != trap_type_none)
    {
        ApplyTrapEffects(tile, entity);
    }
    // todo: cover mechanics stuff
    //if (tile->cover_type != cover_type_none) {
    //  ApplyCoverBonus(entity);
    //}
}

fn void ProcessStatusEffects(entity_t *entity)
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