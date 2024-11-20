typedef enum {
    combat_text_critical,
    combat_text_hit,
    combat_text_miss,
    combat_text_graze,
    combat_text_alerted,
    combat_text_heal,
} combat_text_type_t;

typedef enum
{
	particle_type_none,
	particle_type_number,
	particle_type_combat_text
} particle_type_t;

typedef struct
{
	v2 p;
	f32 t;
	particle_type_t type;
	s32 number;
	combat_text_type_t combat_text;
} particle_t;

typedef struct
{
	s32 num;
	particle_t parts[64];
} particles_t;

fn particle_t *CreateParticle(particles_t *particles, v2 p, particle_type_t type)
{
	particle_t *result = 0;
	if (particles->num < ArraySize(particles->parts))
		result = &particles->parts[particles->num++];
	if (result)
	{
		ZeroStruct(result);
		result->p = p;
		result->type = type;
	}
	return result;
}

fn void CreateCombatText(particles_t *particles, v2 p, combat_text_type_t text_type) {
	particle_t *result = CreateParticle(particles, p, particle_type_combat_text);
	result->combat_text = text_type;

	v2 random_offset = {0};
    random_offset.x = RandomFloat();
    random_offset.y = RandomFloat();
    random_offset = Normalize(random_offset);
    random_offset = Scale(random_offset, 5.0f);
    result->p = Add(result->p, random_offset);
}

fn void CreateDamageNumber(particles_t *particles, v2 p, s32 number)
{
	particle_t *result = CreateParticle(particles, p, particle_type_number);
	result->number = number;

	v2 random_offset = {0};
	random_offset.x = RandomFloat();
	random_offset.y = RandomFloat();
	random_offset = Normalize(random_offset);
	random_offset = Scale(random_offset, 5.0f);
	result->p = Add(result->p, random_offset);
}