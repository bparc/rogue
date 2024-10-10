fn void EstablishTurnOrder(game_world_t *World, turn_queue_t *queue, entity_storage_t *storage)
{
	DefaultTurnOrder(queue, storage);
}

fn s32 BeginTurn(game_world_t *World, entity_t *requestee)
{
	s32 action_point_count = 8;
	if (IsHostile(requestee))
		action_point_count = 1 + (rand() % 2);

	return action_point_count;
}

fn s32 Decide(game_world_t *World, entity_t *requestee)
{
	s32 cost = 1;
	MoveEntity(World->map, requestee, cardinal_directions[rand() % 4]);
	return (cost);
}