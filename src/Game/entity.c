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

fn b32 IsAlive(const entity_t *Entity)
{
	b32 Result = false;
	if (Entity)
	{
		if (!(Entity->flags & entity_flags_deleted))
			Result = true;
	}
	return Result;
}