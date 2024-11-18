fn void RenderIsoCube(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color);
fn void RenderIsoCubeCentered(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color);
fn void RenderIsoCubeFilled(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color);

fn void RenderIsoTile(command_buffer_t *out, map_t *map, v2s offset, v4 color, s32 Filled, f32 height);
fn void RenderIsoTile32(command_buffer_t *out, map_t *map, s32 x, s32 y, v4 color);

fn void RenderIsoTileArea(command_buffer_t *out, map_t *map, v2s min, v2s max, v4 color);
fn void RenderRange(command_buffer_t *out, map_t *map,v2s center, int radius, v4 color);

fn void DrawRangedAnimation(command_buffer_t *out, v2 from, v2 to, const bitmap_t *bitmap, f32 t);

fn void RenderTileAlignedBitmap(command_buffer_t *out, map_t *map, v2s offset, bitmap_t *bitmap, v4 color)
{
	v2 p = MapToScreen(map, offset);
	p = ScreenToIso(p);
	p = Sub(p, Scale(bitmap->scale, 0.5f));
	DrawBitmap(out, p, bitmap->scale, color, bitmap);
}

fn void RenderHPBar(command_buffer_t *out, v2 p, assets_t *assets, entity_t *entity)
{
    // todo: Add animation when chunk of health is lost, add art asset
    f32 health_percentage = (f32)entity->health / entity->max_health;

    v2 bar_size = V2(16*4, 2*4);
    v2 bar_p = p;

    DrawRect(out, bar_p, bar_size, Black());
    v2 health_bar_size = V2(bar_size.x * health_percentage, bar_size.y);
            
    DrawRect(out, bar_p, health_bar_size, Green());
    DrawRectOutline(out, bar_p, health_bar_size, Black());
    DrawFormat(out, assets->Font, Add(bar_p, V2(4.0f,-10.0f)), White(), "%i/%i", entity->health,entity->max_health);
}