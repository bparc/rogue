fn entity_t *CreateEntity(entity_storage_t *storage, v2s p, v2s size, u8 flags, u16 health_points, u16 attack_dmg, const map_t *map, u16 max_health_points, s32 accuracy, s32 evasion, s32 remaining_action_points)
{
	entity_t *result = 0;
	if (storage->num < ArraySize(storage->entities))
		result = &storage->entities[storage->num++];
	if (result)
	{
		ZeroStruct(result); //memset macro
		result->p = p;
		result->deferred_p = GetTileCenter(map, result->p);

		result->size = size;

		result->id = (0xff + (storage->next_id++));
		result->flags = flags;
		result->health = health_points;
		result->attack_dmg = attack_dmg;
		result->max_health = max_health_points;
		result->melee_accuracy = accuracy;
		result->ranged_accuracy = accuracy;
		result->evasion = evasion;
		result->remaining_action_points = remaining_action_points;

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
	if (storage->statics_num < ArraySize(storage->static_entities))
		result = &storage->static_entities[storage->statics_num++];
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
	for (s32 index = 0; index < storage->num; index++) {
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

	for (s32 i = 0; i < storage->num; i++)
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

	for (s32 i = 0; i < storage->num; i++)
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
	if ((index >= 0) && (index < storage->num))
		return &storage->entities[index];
	return NULL;
}

fn entity_t *GetEntity(entity_storage_t *storage, entity_id_t id)
{
	if (id > 0)
	{
		for (s32 index = 0; index < storage->num; index++)
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
	for (s32 index = 0; index < storage->num; index++)
		if (IsPlayer(&storage->entities[index]))
			return &storage->entities[index];
	return 0;
}