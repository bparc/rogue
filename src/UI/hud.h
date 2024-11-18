typedef struct
{
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

fn void OpenInventory(interface_t *In)
{
	In->InventoryOpened = true;
}

fn void CloseInventory(interface_t *In)
{
	In->InventoryOpened = false;
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