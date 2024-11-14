typedef u64 entity_id_t;
#include "actions.h"
#include "items.h"
#include "inventory.h"
#include "inventory.c"

#include "entity.h"
#include "entity.c"
#include "actions.c"

#include "turn_system.h"
#include "cursor.h"

#include "draw.h"
#include "draw.c"

typedef enum {
    combat_text_critical,
    combat_text_hit,
    combat_text_miss,
    combat_text_graze
} combat_text_type_t;

typedef enum
{
	particle_type_none,
	//particle_type_bitmap,
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

typedef struct
{
    s32 DraggedItemID;
    b32 inventory_visible;

    item_t DraggedItem;
    //v2s DraggedItemSz;
    s32 OriginalX;
    s32 OriginalY;

    v2 Cursor;
    s32 Interact[2];
    s32 Buttons[2];

    v2 ClickOffset;
    item_id_t ContextMenuItem;
    b32 ContextMenuOpened;
    b32 CloseContextMenu;
    f32 ContextMenuT;
} interface_t;

fn void BeginInterface(interface_t *In, const client_input_t *Input)
{
	In->Cursor = GetCursorOffset(Input);
	In->Interact[0] = (!In->Buttons[0] && Input->mouse_buttons[0]);
	In->Interact[1] = (!In->Buttons[1] && Input->mouse_buttons[1]);

	In->Buttons[0] = Input->mouse_buttons[0];
	In->Buttons[1] = Input->mouse_buttons[1];
}

fn void EndInterface(interface_t *In)
{

}

fn void BeginItemDrag(interface_t *In, const item_t *Item)
{
    In->DraggedItemID = Item->ID;
    In->DraggedItem = *Item;
    In->OriginalX = Item->x;
    In->OriginalY = Item->y;
}

fn void DiscardInput(interface_t *In)
{
	In->Interact[1] = false;
    In->Interact[0] = false;
}

fn void OpenContextMenu(interface_t *In, item_id_t Item)
{
	if (!In->ContextMenuOpened)
	{
		In->ContextMenuT = 0.0f;
    	In->ContextMenuItem = Item;
    	In->ContextMenuOpened = true;
    	In->ClickOffset = In->Cursor;
    	In->CloseContextMenu = false;
    	DiscardInput(In);
	}
}

typedef struct
{
	assets_t *assets;
	log_t *log;
	cursor_t *cursor;
	entity_storage_t *storage;
	particles_t *particles;
	interface_t *interface;
	
	slot_bar_t slot_bar;
	turn_queue_t *turns;

	map_t *map;
	camera_t *camera;
	
	memory_t *memory;
} game_world_t;
typedef game_world_t video_game_t;

fn entity_t *CreateSlime(game_world_t *state, v2s p);
fn void CreateBigSlime(game_world_t *state, v2s p);
fn void CreatePoisonTrap(game_world_t *state, v2s p);

fn b32 IsWorldPointEmpty(game_world_t *state, v2s p);
fn b32 Move(game_world_t *world, entity_t *entity, v2s offset);
#include "world.c"

#include "game.h"
#include "game.c"
#include "cursor.c"
#include "enemy.c"
#include "turn_system.c"
#include "hud.c"

fn void Setup(game_world_t *state, memory_t *memory, log_t *log, assets_t *assets)
{

	state->memory = memory;
	state->assets = assets;
	state->log = log;
	state->camera 		= PushStruct(camera_t, memory);
	state->cursor 		= PushStruct(cursor_t, memory);
	state->turns  		= PushStruct(turn_queue_t, memory);
	state->storage 		= PushStruct(entity_storage_t, memory);
	state->particles 	= PushStruct(particles_t, memory);
	state->interface 	= PushStruct(interface_t, memory);

	state->map = CreateMap(1024, 1024, memory, TILE_PIXEL_SIZE);
	
	SetupItemDataTable(memory, assets);
	SetupActionDataTable(memory, assets);

	DefaultActionBar(&state->slot_bar,  assets);

	state->camera->p = V2(0, 0);
	state->camera->zoom = 2.0f;
	state->camera->viewport = V2(1600.0f, 900.0f);

	state->interface->inventory_visible = true;
}

fn void BeginGameWorld(game_world_t *state)
{
	//DebugPrint("Moves: %i", state->turns->action_points);
}

fn void EndGameWorld(game_world_t *state)
{

}

fn void Update(game_world_t *state, f32 dt, client_input_t input, log_t *log, assets_t *assets, virtual_controls_t cons, command_buffer_t *out)
{
	BeginGameWorld(state);
	TurnSystem(state, state->storage, state->map, state->turns, dt, &input, cons, log, out, assets);	
	EndGameWorld(state);
	HUD(Debug.out, state, state->turns, state->storage, assets, &input, &cons, dt);
}

fn inline void RenderEntity(command_buffer_t *out, const entity_t *entity, f32 alpha, assets_t *assets, const map_t *map)
{
	v2 p = entity->deferred_p;
	bitmap_t *bitmap = IsHostile(entity) ? &assets->Slime : &assets->Player[0];
	v2 bitmap_p = p;
	// TODO(): Still somewhat hard-coded.
	v2 cube_bb_sz = V2(24, 24);
	if ((entity->size.x == 2) && (entity->size.y == 2))
	{
		bitmap = &assets->SlimeBig;
		bitmap_p = Add(bitmap_p, Scale(map->tile_sz, 0.5f));
		p = Add(p, Scale(map->tile_sz, 0.5f));
		cube_bb_sz = V2(64.0f, 64.0f);
	}
	// NOTE(): Bitmap
	v4 bitmap_color = PureWhite();
	v2 bitmap_sz = bitmap->scale;
	bitmap_p = ScreenToIso(bitmap_p);
	bitmap_p = Sub(bitmap_p, Scale(bitmap_sz, 0.5f)); //center bitmap
	bitmap_p.y -= bitmap_sz.y * 0.25f; //center bitmap "cube"

	// NOTE(): Flickering
	if (entity->blink_time > 0)
	{
		f32 t = entity->blink_time;
		t = t * t;
		f32 blink_rate = 0.4f;
		if (fmodf(t, blink_rate) >= (blink_rate * 0.5f))
			bitmap_color = Red();
		else
			bitmap_color = A(Blue(), 0.9f);
	}

	bitmap_color.w = alpha;
	DrawBitmap(out, bitmap_p, bitmap_sz, bitmap_color, bitmap);
	RenderIsoCubeCentered(out, ScreenToIso(p), cube_bb_sz, 50, Pink());
}

fn v2s ViewportToMap(const game_world_t *World, v2 p)
{
	p = Sub(p, World->camera->p);
	p = IsoToScreen(p);
	p = Div(p, World->map->tile_sz);
	v2s result = {0};
	result.x = (s32)p.x;
	result.y = (s32)p.y;
	return result;
}

fn inline void RenderTile(command_buffer_t *out, map_t *map, s32 x, s32 y, assets_t *assets)
{
	v2s at = {x, y};
	if (!IsEmpty(map, at))
	{
		const tile_t *Tile = GetTile(map, x, y);
		RenderIsoTile(out, map, at, White(), false, 0);

		if (IsTraversable(map, at))
		{
			bitmap_t *bitmap = PickTileBitmap(map, x, y, assets);
			RenderTileAlignedBitmap(out, map, at, bitmap, PureWhite());
			
			// Draw temporary blood overlay
			if (Tile->blood)
			{
				v4 color = (Tile->blood == blood_red) ? Red() : Green();
				RenderTileAlignedBitmap(out, map, at, bitmap, A(color, 0.8f));
			}
		}
		if (IsWall(map, at))
			RenderIsoTile(out, map, at, White(), true, 15);
		if (GetTileValue(map, at.x, at.y) == 5)
			RenderIsoTile(out, map, at, Red(), true, 25);

		if ((Tile->trap_type != trap_type_none))
			RenderTileAlignedBitmap(out, map, at, &assets->Traps[(Tile->trap_type - 1)], PureWhite());	
	}
}

fn inline void ClipToViewport(game_world_t *World, map_t *map, bb_t clipplane, command_buffer_t *out)
{
	v2 viewport = Sub(clipplane.max, clipplane.min);
	assets_t *assets = World->assets;

	// NOTE(): Tessalate the clipplane into a parallelogram and two
	// triangles.
	v2s top_corner = ViewportToMap(World, TopMaxCorner(clipplane));
	v2s bot_corner = ViewportToMap(World, BotMinCorner(clipplane));

	f32 projection_coefficient = 0.5f;
	viewport = Ratio(viewport, 1.0f / projection_coefficient);

	v2s min1 = ViewportToMap(World, clipplane.min);
	v2s max1 = ViewportToMap(World, Add(clipplane.min, viewport));

	v2s min2 = ViewportToMap(World, clipplane.max);
	v2s max2 = ViewportToMap(World, Sub(clipplane.max, viewport));

	#if 0
	DrawRectOutline(Debug.out_top, clipplane.min, Sub(clipplane.max, clipplane.min), Orange());
	DrawLine(Debug.out_top, clipplane.min, Add(clipplane.min, viewport), Red());
	DrawLine(Debug.out_top, clipplane.max, Sub(clipplane.max, viewport), Red());
	#endif

	// NOTE(): Sort by Y
	v2s top_min;
    v2s top_max;
    v2s bot_min;
    v2s bot_max;
    if(min1.y<min2.y)
    {
        top_min = min1;
        top_max = max1;
        bot_min = max2;
        bot_max = min2;
    }
    else
    {
        top_min = max2;
        top_max = min2;
        bot_min = min1;
        bot_max = max1;
    }

    // NOTE(): Top triangle
    v2s min = top_min;
    v2s max = top_max;
    v2s corner = top_corner;
    for(s32 y = 0; y < min.y - corner.y; y++)
    for(s32 x = -y; x <= y; x++)
            RenderTile(out,map,corner.x+x,corner.y+y,assets);
    // NOTE(): Parallelogram
    if(min1.y<min2.y)
    {
        for(s32 y = 0; y <= min2.y - min.y; y++)
        for(s32 x = 0; x <= max.x-min.x;x++)
        	RenderTile(out, map,min.x+x+y,min.y+y,assets);
    }
    else
    {
        for(s32 y = 0; y <= min1.y - min.y; y++)
        for(s32 x = 0; x <= max.x-min.x;x++)
           RenderTile(out,map,(min.x+x)-y,min.y+y,assets);
    }
    // NOTE(): Bot triangle
    min = bot_min;
    max = bot_max;
    corner = bot_corner;
    for(s32 y = 1; y < (corner.y - max.y); y++)
    for(s32 x = y; x < (max.x - min.x)-y; x++)
    	RenderTile(out,map,min.x+x,min.y+y,assets);
}

fn void DrawFrame(game_world_t *state, command_buffer_t *out, f32 dt, assets_t *assets, v2 viewport)
{
	map_t *map = state->map;
	entity_storage_t *storage = state->storage;
	entity_t *player = DEBUGGetPlayer(storage);
	particles_t *particles = state->particles;
	command_buffer_t *out_top = Debug.out;
	turn_queue_t *queue = state->turns;

	//DrawRect(out, V2(0.0f, 0.0f), V2(1000.0f, 1000.0f), SKY_COLOR); // NOTE(): Background

	bb_t view = {0.0f};
	view.min = V2(0.0f, 0.0f);
	view.max = Add(view.min, viewport);
	view = Shrink(view, 64.0f * 3.0f);

	ClipToViewport(state, map, view, out);

	// NOTE(): Entities
	for (s32 index = 0; index < storage->num; index++)
	{
		entity_t *entity = &storage->entities[index];

		// NOTE(): The "deferred_p"s of the 'active' no-player entities are
		// animated directly in TurnKernel() to allow for
		// more explicit controls over the entity animation in that section of the code-base.
		if ((entity->flags & entity_flags_controllable) ||
			(IsEntityActive(state->turns, storage, entity->id) == false))
		{
			entity->deferred_p = Lerp2(entity->deferred_p, GetTileCenter(state->map, entity->p), 10.0f * dt);
		}
		entity->blink_time = MaxF32(entity->blink_time - (dt * 1.5f), 0.0f);

		RenderEntity(out, entity, 1.0f, assets, map);

		v2 screen_p = CameraToScreen(state->camera, Sub(entity->deferred_p, V2(60.0f, 60.0f)));
		screen_p.x -= 25.0f;
		RenderHPBar(out_top, screen_p, assets, entity);
	}
	for (s32 index = 0; index < queue->num_evicted_entities; index++)
	{
		evicted_entity_t *entity = &queue->evicted_entities[index];
		entity->deferred_p = Sub(entity->deferred_p, Scale(V2(10.0f, 10.0f), dt));
		RenderEntity(out, &entity->entity, entity->time_remaining, assets, map);
	}
 	// NOTE(): Particles
	for (s32 index = 0; index < particles->num; index++)
	{
		particle_t *particle = &particles->parts[index];
		particle->t += dt;
		if (particle->t < 1.0f)
		{
			switch (particle->type)
			{
			case particle_type_number:
				{	
					f32 t = particle->t;
					v4 color = White();
					color.w = (1.0f - Smoothstep(t, 0.5f));

					v2 p = CameraToScreen(state->camera, particle->p);
					p.y -= ((50.0f * t) + (t * t * t) * 20.0f);
					p.x += (Sine(t) * 2.0f - 1.0f) * 2.0f;

					DrawFormat(out_top, assets->Font, p, color, "%i", particle->number);
				} break;
			case particle_type_combat_text:
				{
					f32 t = particle->t;
					v4 color = White();
					color.w = (1.0f - Smoothstep(t, 0.5f));

					v2 p = CameraToScreen(state->camera, particle->p);
					p.y -= ((50.0f * t) + (t * t * t) * 20.0f);
					p.x += (Sine(t) * 2.0f - 1.0f) * 2.0f;

	                const char *text = "";
	                switch (particle->combat_text)
	                {
	                    case combat_text_critical: color = Yellow(); text = "CRITICAL"; break;
	                    case combat_text_hit:      text = "HIT";      break;
	                    case combat_text_miss:     color = LightGrey(); text = "MISS";     break;
	                    case combat_text_graze:    text = "GRAZE";    break;
	                }

	                DrawFormat(out_top, assets->Font, p, color, "%s", text);
				} break;
			}
			continue;
		}

		particles->parts[index--] = particles->parts[--particles->num];
	}
}