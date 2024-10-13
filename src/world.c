// NOTE(): Isometric.
fn v2 ScreenToIso(v2 p)
{
	v2 result = {0};
	result.x = Dot(p, V2(+0.50f,-0.50f));
	result.y = Dot(p, V2(+0.25f,+0.25f));
	return result;
}

fn v2 IsoToScreen(v2 p)
{
	v2 result = {0};
	result.x = Dot(p, V2(+2.0f,+4.0f));
	result.y = Dot(p, V2(-2.0f,+4.0f));
	result = Scale(result, 0.5f);
	return result;
}

fn v2 MapToScreen(const map_t *map, v2s p)
{
	v2 result = SV2(p.x, p.y);
	result = Mul(result, map->tile_sz);
	return (result);
}

fn void MakeIsoRect(v2 points[4], f32 x, f32 y, v2 sz)
{
	v2 min = V2(x, y);
	v2 max = Add(min, sz);
	points[0] = V2(min.x, min.y);
	points[1] = V2(max.x, min.y);
	points[2] = V2(max.x, max.y);
	points[3] = V2(min.x, max.y);
	points[0] = ScreenToIso(points[0]);
	points[1] = ScreenToIso(points[1]);
	points[2] = ScreenToIso(points[2]);
	points[3] = ScreenToIso(points[3]);
}

fn void RenderIsoCube(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color)
{
	p = IsoToScreen(p);
	v2 A[4], B[4];
	MakeIsoRect(A, p.x, p.y, sz);
	MakeIsoRect(B, p.x - height, p.y - height, sz);
	DrawLineLoop(out, A, 4, color);
	DrawLineLoop(out, B, 4, color);
	DrawLine(out, A[0], B[0], color);
	DrawLine(out, A[1], B[1], color);
	DrawLine(out, A[2], B[2], color);
	DrawLine(out, A[3], B[3], color);
}

fn void RenderIsoCubeCentered(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color)
{
	p.y -= sz.y * 0.25f;
	RenderIsoCube(out, p, sz, height, color);
}

fn void RenderIsoCubeFilled(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color)
{
	//Assert((0 && "Not implemented!"));
	p = IsoToScreen(p);
	v2 A[4], B[4];
	MakeIsoRect(A, p.x, p.y, sz);
	MakeIsoRect(B, p.x - height, p.y - height, sz);
	DrawQuad(out, A[0], A[1], A[2], A[3], color);
	if (height != 0)
	{
		DrawQuadv(out, B, Red());
		DrawQuad(out, A[3], B[3], B[2], A[2], Green());
		DrawQuad(out, B[2], B[1], A[1], A[2], Blue());
	}
}

// todo: Add animation when chunk of health is lost, add art asset
fn void RenderHealthBars(command_buffer_t *out, v2 position, assets_t *assets, entity_storage_t *storage) {
	for (s32 index = 0; index < storage->num; index++) {
		entity_t *entity = &storage->entities[index];

		f32 health_percentage = (f32)entity->health / entity->max_health;

		v2 bar_size = V2(40, 5);
		v2 bar_position = Sub(position, V2(8.0f, 10.0f));

		DrawRect(out, bar_position, bar_size, Black());
		v2 health_bar_size = V2(bar_size.x * health_percentage, bar_size.y);
		DrawRect(out, bar_position, health_bar_size, Green());
	}
}

fn void RenderIsoTile(command_buffer_t *out, const map_t *map, v2s offset, v4 color, s32 Filled, f32 height)
{
	v2 p = MapToScreen(map, offset);
	p = ScreenToIso(p);
	if (Filled)
		RenderIsoCubeFilled(out, p, map->tile_sz, height, color);
	else
		RenderIsoCube(out, p, map->tile_sz, height, color);
}

//NOTE: static combat items
fn static_entity_t * CreateEffectTile(entity_storage_t *storage, v2s p, v2s size, u8 flags, status_effect_t status_effects[MAX_STATUS_EFFECTS]) {
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

// NOTE(): Entities.
fn entity_t *CreateEntity(entity_storage_t *storage, v2s p, v2s size, u8 flags, u16 health_points, u16 attack_dmg, const map_t *map, u16 max_health_points)
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

		result->id = storage->next_id++;
		result->flags = flags;
		result->health = health_points;
		result->attack_dmg = attack_dmg;
		result->max_health = max_health_points;
	}
	
	return result;
}

fn void CreateSlimeI(game_world_t *state, s32 x, s32 y)
{
	u16 slime_hp = 100;
	u16 slime_max_hp = 100;
	u16 slime_attack_dmg = 10;
	CreateEntity(state->storage, V2S(x, y), V2S(1, 1),  entity_flags_hostile, slime_hp, slime_attack_dmg, state->map, slime_max_hp);
}

fn void CreateBigSlimeI(game_world_t *state, s32 x, s32 y)
{
	u16 slime_hp = 400;
	u16 slime_max_hp = 400;
	u16 slime_attack_dmg = 25;
	CreateEntity(state->storage, V2S(x, y), V2S(2, 2),  entity_flags_hostile, slime_hp, slime_attack_dmg, state->map, slime_max_hp);
}

fn void CreatePoisonTrapI(game_world_t *state, s32 x, s32 y) {
	u8 flags = static_entity_flags_trap | static_entity_flags_stepon_trigger;

	status_effect_t status_effects = {0};
	status_effects.type = status_effect_poison;
	status_effects.remaining_turns = 3;
	status_effects.damage = 1;

	status_effect_t effects[MAX_STATUS_EFFECTS] = {status_effects, 0, 0};
	CreateEffectTile(state->storage, V2S(x,y), V2S(1,1), flags, effects);
}

fn b32 IsHostile(const entity_t *entity)
{
	if (entity)
		return entity->flags & entity_flags_hostile;
	return 0;
}

fn b32 IsTrap(const static_entity_t *entity)
{
	if (entity)
		return entity->flags & static_entity_flags_trap;
	return 0;
}

fn entity_t *GetEntity(entity_storage_t *storage, entity_id_t id)
{
	for (s32 index = 0; index < storage->num; index++)
	{
		entity_t *entity = &storage->entities[id];
		if (entity->id == id)
			return entity;
	}
	return (0);
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

fn entity_t *FindNearestEnemy(entity_storage_t *storage, v2s player_pos)
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

fn void MoveEntity(map_t *map, entity_t *entity, v2s offset)
{
	entity->p = AddS(entity->p, offset);
	if (entity->p.x < 0)
		entity->p.x = 0;
	if (entity->p.y < 0)
		entity->p.y = 0;
	if (entity->p.x >= map->x)
		entity->p.x = map->x - 1;
	if (entity->p.y >= map->y)
		entity->p.y = map->y - 1;
}

fn v2 CameraTracking(v2 p, v2 player_world_pos, v2 viewport, f32 dt)
{
	v2 player_iso_pos = ScreenToIso(player_world_pos);
	
	v2 screen_center = Scale(Scale(viewport, 1.0f / (f32)VIEWPORT_INTEGER_SCALE), 0.5f);
	v2 camera_offset = Sub(screen_center, player_iso_pos);
	p = Lerp2(p, camera_offset, 5.0f * dt);
	return p;
}

// NOTE(): Turns
fn void PushTurn(turn_queue_t *queue, entity_t *entity)
{
	if (queue->num < ArraySize(queue->entities))
		queue->entities[queue->num++] = entity->id;
}

fn void DefaultTurnOrder(turn_queue_t *queue, entity_storage_t *storage)
{
	s32 player_count = 0;
	entity_t *players[16] = {0};

	for (s32 index = 0; index < storage->num; index++)
	{
		entity_t *entity = &storage->entities[index];
		if (IsHostile(entity))
			PushTurn(queue, entity);
		else
			players[player_count++] = entity;
	}

	Assert(player_count < ArraySize(players));
	for (s32 index = 0; index < player_count; index++)
		PushTurn(queue, players[index]);
}

fn entity_t *NextInOrder(turn_queue_t *queue, entity_storage_t *storage)
{
	entity_t *result = 0;
	if (queue->num > 0)
	{
		result = GetEntity(storage, queue->entities[queue->num - 1]);
		Assert(result);
	}
	return result;
}

fn int32_t IsEntityActive(turn_queue_t *queue, entity_storage_t *storage, entity_id_t id)
{
	entity_t *result = NextInOrder(queue, storage);
	if (result)
		return (result->id == id);
	return 0;
}

fn entity_t *PeekNextTurn(turn_queue_t *queue, entity_storage_t *storage)
{
	entity_t *result = 0;
	if (queue->num >= 2)
		result = GetEntity(storage, queue->entities[queue->num - 2]);
	return result;
}

fn void AcceptTurn(turn_queue_t *queue)
{
	DebugAssert(queue->turn_inited == true); // NOTE(): Propably a bug?

	Assert(queue->num > 0);
	queue->num--;
	queue->turn_inited = false;
}

fn void ConsumeActionPoints(turn_queue_t *queue, s32 count)
{
	queue->action_points--;
}

// NOTE(): World.
fn b32 IsWall(game_world_t *state, v2s p)
{
	b32 result = (GetTileValue(state->map, p.x, p.y) == 2);
	return result;
}

fn b32 IsOutOfBounds(game_world_t *state, v2s p) {
	b32 result = (GetTileValue(state->map, p.x, p.y) == 0);
	return result;
}

fn b32 IsOtherEntity(game_world_t *state, v2s p) {
	entity_t *collidingEntity = GetEntityByPosition(state->storage, p);
	return collidingEntity != NULL;
}

fn b32 IsWorldPointEmpty(game_world_t *state, v2s p) {

	entity_t *collidingEntity = GetEntityByPosition(state->storage, p);

	if (collidingEntity == NULL) {

		u8 tileValue = GetTileValue(state->map, p.x, p.y);
		b32 result = !(tileValue == 0 || tileValue == 2);
		//DebugLog("inside IsWorldPointEmpty, return value is = %d", result);
		return !(tileValue == 0 || tileValue == 2); // Wall or out of bounds
	}

	DebugLog("Return value is false, entity collision");
	return false; // Entity collision
}

// NOTE(): Cursor
