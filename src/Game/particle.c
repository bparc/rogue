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
	particle_type_combat_text,
	particle_type_text,
} particle_type_t;

typedef struct
{
	particle_type_t type;

	v2 p;
	f32 t;

	s32 number;

	const char *Text;
	v4 Color;

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
	{
		result = &particles->parts[particles->num++];
		ZeroStruct(result);
		result->p = p;
		result->type = type;
	}
	return result;
}

fn void CreateTextParticle(particles_t *Particles, v2 Pos, v4 Color, const char *Text)
{
	particle_t *Result = CreateParticle(Particles, Pos, particle_type_text);
	Result->Color = Color;
	Result->Text = Text;
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

fn void DrawParticleSystem(particles_t *particles, const assets_t *Assets, const camera_t *Camera, command_buffer_t *Out, f32 dt)
{
	for (s32 index = 0; index < particles->num; index++)
	{
		particle_t *particle = &particles->parts[index];
		particle->t += dt;
		if (particle->t < 1.0f)
		{
			f32 t = particle->t;
			v4 color = White();
			color.w = (1.0f - Smoothstep(t, 0.5f));
			v2 p = CameraToScreen(Camera, particle->p);
			p.y -= ((50.0f * t) + (t * t * t) * 20.0f);
			p.x += (Sine(t) * 2.0f - 1.0f) * 2.0f;

			switch (particle->type)
			{
			case particle_type_number:
				{	
					DrawFormat(Out, Assets->Font, p, color, "%i", particle->number);
				} break;
			case particle_type_combat_text:
				{
	                const char *text = "";
	                switch (particle->combat_text)
	                {
	                    case combat_text_critical: color = Yellow(); text = "CRITICAL"; break;
	                    case combat_text_hit:      text = "HIT";      break;
	                    case combat_text_miss:     color = LightGrey(); text = "MISS";     break;
	                    case combat_text_graze:    text = "GRAZE";    break;
	                    case combat_text_alerted:  text = "!"; color = White(); break;
	                    case combat_text_heal:     text = "HEAL"; color = Green(); break;
	                }

	                DrawFormat(Out, Assets->Font, p, color, "%s", text);
				} break;
			case particle_type_text:
				{
					DrawFormat(Out, Assets->Font, p, particle->Color, "%s", particle->Text);
				} break;
			}
			continue;
		}

		particles->parts[index--] = particles->parts[--particles->num];
	}
}