typedef struct
{
	v2s p;
	b32 active; // If active, the player input is redirected to the cursor.

	entity_id_t Target;
} cursor_t;

// fn void DoCursor();