typedef struct
{
	v2s p;
	b32 active; // If active, the player input is redirected to the cursor.

	entity_id_t Target;
} cursor_t;

fn void CloseCursor(cursor_t *Cursor)
{
	Cursor->active = false;
}
// fn void DoCursor();