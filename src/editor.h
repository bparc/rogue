typedef struct
{
	s32 delete_me;
	s32 inited;
} editor_state_t;


int IsValidEntity(s32 globalX, s32 globalY, char Data[globalY][globalX], s32 startX, s32 startY, v2s size, char entityChar);