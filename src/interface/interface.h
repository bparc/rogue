typedef struct
{
    const virtual_controls_t *Cons;
    const client_input_t *Input;
	command_buffer_t *Out;
    bmfont_t *Font;
    f32 DeltaTime;
    
    s32 DraggedItemID;
    b32 InventoryOpened;
    
    inventory_t *DraggedContainer;
    item_t DraggedItem;
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

	container_t *OpenedContainer;  
} interface_t;

fn void CloseContainer(interface_t *In);
fn void OpenContainer(interface_t *In, container_t *Container);

fn void OpenInventory(interface_t *In)
{
	In->InventoryOpened = true;
}

fn void CloseInventory(interface_t *In)
{
	In->InventoryOpened = false;
	CloseContainer(In);
}

fn void ToggleInventory(interface_t *In)
{
	In->InventoryOpened = !In->InventoryOpened;
}

fn void BeginItemDrag(interface_t *In, const item_t *Item, inventory_t *SourceContainer)
{
	In->DraggedContainer = SourceContainer;
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