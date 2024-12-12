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
    p = IsoToScreen(p);
    v2 A[4], B[4];
    MakeIsoRect(A, p.x, p.y, sz);
    MakeIsoRect(B, p.x - height, p.y - height, sz);
    DrawQuad(out, A[0], A[1], A[2], A[3], color);
    if (height != 0)
    {
        DrawQuadv(out, B, color);
        DrawQuad(out, A[3], B[3], B[2], A[2], Scale3(color, 0.8f));
        DrawQuad(out, B[2], B[1], A[1], A[2], Scale3(color, 0.8f));
    }
}

fn void RenderIsoTile(command_buffer_t *out, map_t *Map, v2s offset, v4 color, s32 Filled, f32 height)
{
    v2 p = MapToScreen(Map, offset);
    p = ScreenToIso(p);
    if (Filled)
        RenderIsoCubeFilled(out, p, Map->tile_sz, height, color);
    else
        RenderIsoCube(out, p, Map->tile_sz, height, color);
}

fn void RenderIsoTileArray(command_buffer_t *Out, map_t *Map, v2s Center, v2s *Tiles, s32 Count, v4 Color)
{
    for (int32_t Index = 0; Index < Count; Index++)
    {
        RenderIsoTile(Out, Map, IntAdd(Center, Tiles[Index]), Color, true, 0);
    }
}

fn void RenderIsoTileArea(command_buffer_t *out, map_t *Map, v2s min, v2s max, v4 color)
{
    for (int y = min.y; y < max.y; y++)
    {
        for (int x = min.x; x < max.x; x++)
            RenderIsoTile(out, Map, V2S(x,y), color, true, 0);
    }
}

fn void RenderRange(command_buffer_t *out, map_t *Map,v2s center, int radius, v4 color)
{
    for (s32 y = center.y - radius; y <= center.y + radius; ++y)
    {
        for (s32 x = center.x - radius; x <= center.x + radius; ++x)
        {
            v2s target = V2S(x, y);

            if (IsInsideCircle(target, V2S(1,1), center, radius)) {
                if (GetTile(Map, x, y) != 0 && IsTraversable(Map, target)) { // this doesn't work, its supposed to not draw range area over grids where floor isnt placed
                    if (IsLineOfSight(Map, center, target)) {
                        RenderIsoTile(out, Map, V2S(x,y), A(color, 0.5f), true, 0);
                    }
                }
            }
        }
    }
}

fn void RenderRangedAnimation(command_buffer_t *out, v2 from, v2 to, const bitmap_t *bitmap, f32 t)
{
    v2 target_center = to;
    v2 bitmap_p = Lerp2(from, target_center, t);
    v2 bitmap_sz = bitmap->scale;

    bitmap_p = ScreenToIso(bitmap_p);
    bitmap_p = Sub(bitmap_p, Scale(bitmap_sz, 0.5f));
    DrawBitmap(out, bitmap_p, bitmap_sz,  PureWhite(), bitmap);
}

fn void RenderTileAlignedBitmap(command_buffer_t *out, map_t *Map, v2s offset, bitmap_t *bitmap, v4 color)
{
	v2 p = MapToScreen(Map, offset);
	p = ScreenToIso(p);
	p = Sub(p, Scale(bitmap->scale, 0.5f));
	DrawBitmap(out, p, bitmap->scale, color, bitmap);
}

fn void RenderHealthPoints(command_buffer_t *out, v2 p, assets_t *assets, entity_t *entity)
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

fn void RenderDiegeticText(const camera_t *Camera, const bmfont_t *Font, v2 p, v2 offset, v4 color, const char *format, ...)
{
    char string[256] = "";
    va_list args = {0};
    va_start(args, format);
    vsnprintf(string, sizeof(string), format, args);
    va_end(args);

    command_buffer_t *out = Debug.out_top;
    v2 screen_p = CameraToScreen(Camera, p);
    screen_p = Add(screen_p, offset);
    DrawText(out, Font, screen_p, string, color);
}

fn void RenderPath(command_buffer_t *out, map_t *Map, path_t *Path, v4 Color)
{
    for (int32_t Index = 0; Index < Path->length; Index++)
    {
        v2s TileIndex = Path->tiles[Index].p;
        RenderIsoTile(out, Map, TileIndex, Color, true, 0);
    }
}

fn void RenderRangeMap(command_buffer_t *out, map_t *TileMap, range_map_t *Map)
{
    v2s Min = Map->Min;
    v2s Max = IntAdd(Min, Map->Size);

    for (s32 Y = Min.y; Y <= Max.y; Y++)
    {
        for (s32 X = Min.x; X <= Max.x; X++)
        {
            v2s Index = V2S(X, Y);
            range_map_cell_t *Cell = GetRangeMapCell(Map, Index);
            if (Cell)
            {
                if (Cell->Filled)
                {
                    RenderIsoTile(out, TileMap, Index, W(Orange(), 0.5f), true, 0);
                }

                RenderIsoTile(out, TileMap, Index, Red(), false, 0);
            }
        }
    }

    RenderIsoTile(out, TileMap, Map->From, Yellow(), true, 0);
}

fn void RenderHitChance(command_buffer_t *out, assets_t *assets, v2 p, s32 hit_chance)
{
    v2 screen_position = ScreenToIso(p);

    char hit_chance_text[16];
    snprintf(hit_chance_text, sizeof(hit_chance_text), "%d%%", hit_chance);

    screen_position.x += 25;
    screen_position.y -= 20;

    DrawText(out, assets->Font, screen_position, hit_chance_text, White());
}