fn v2 ScreenToIso(v2 p);
fn v2 IsoToScreen(v2 p);

fn void RenderIsoCube(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color);
fn void RenderIsoCubeCentered(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color);
fn void RenderIsoCubeFilled(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color);

fn void RenderIsoTile(command_buffer_t *out, map_t *map, v2s offset, v4 color, s32 Filled, f32 height);
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