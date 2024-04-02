#ifndef __M_SLIDING_MENU_H__
#define __M_SLIDING_MENU_H__

typedef enum SlidingMenuModeType
{
    TYPE_SLIDING_MENU_MODE_LEFT = 0x0001,
    TYPE_SLIDING_MENU_MODE_RIGHT = 0x0002,
    TYPE_SLIDING_MENU_MODE_TOP = 0x0004,
    TYPE_SLIDING_MENU_MODE_BOTTOM = 0x0008,
} SlidingMenuModeType;

typedef enum SlidingMenuChoiceType
{
    TYPE_SLIDING_MENU_CHOICE_NONE,
    TYPE_SLIDING_MENU_CHOICE_SINGLE,
    TYPE_SLIDING_MENU_CHOICE_MULTIPLE,
} SlidingMenuChoiceType;

typedef struct SlidingMenu SlidingMenu;

SlidingMenu *SlidingMenu_Create();
void SlidingMenu_Destroy(SlidingMenu *slidingMenu);

int SlidingMenu_Show(SlidingMenu *slidingMenu);
int SlidingMenu_Dismiss(SlidingMenu *slidingMenu);
int SlidingMenu_Open(SlidingMenu *slidingMenu);  // 和SlidingMenu_Dismiss不同的地方在于没有打开动画，直接打开
int SlidingMenu_Close(SlidingMenu *slidingMenu); // 和SlidingMenu_Dismiss不同的地方在于没有关闭动画，直接关闭

int SlidingMenu_SetAutoFree(SlidingMenu *slidingMenu, int auto_free);
int SlidingMenu_SetData(SlidingMenu *slidingMenu, void *data);
int SlidingMenu_SetMode(SlidingMenu *slidingMenu, SlidingMenuModeType mode);
int SlidingMenu_SetChoiceType(SlidingMenu *slidingMenu, SlidingMenuChoiceType type);
int SlidingMenu_SetItems(SlidingMenu *slidingMenu, char *const *items, int n_items);
int SlidingMenu_SetOnItemClickListener(SlidingMenu *slidingMenu, int (*onItemClickListener)(SlidingMenu *slidingMenu, int which));
int SlidingMenu_SetOnSelectChangedListener(SlidingMenu *slidingMenu, int (*onSelectChangedListener)(SlidingMenu *slidingMenu));
int SlidingMenu_SetOnCloseListener(SlidingMenu *slidingMenu, int (*onCloseListener)(SlidingMenu *slidingMenu));
int SlidingMenu_SetFreeDataCallback(SlidingMenu *slidingMenu, void (*freeData)(void *data));
int SlidingMenu_SetItemSeclected(SlidingMenu *slidingMenu, int id, int seclected);
int SlidingMenu_SetFocusPos(SlidingMenu *slidingMenu, int focus_pos);

void *SlidingMenu_GetData(SlidingMenu *slidingMenu);
int SlidingMenu_GetItemsLength(SlidingMenu *slidingMenu);
int SlidingMenu_IsItemSeclected(SlidingMenu *slidingMenu, int id);

#endif
