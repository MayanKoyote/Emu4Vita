#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/io/fcntl.h>

#include "list/file_list.h"
#include "activity/activity.h"
#include "setting/setting.h"
#include "emu/emu.h"
#include "gui/gui.h"
#include "utils.h"
#include "file.h"
#include "config.h"
#include "lang.h"

static int onStartActivity(GUI_Activity *activity);
static int onFinishActivity(GUI_Activity *activity);
static int onBeforeDrawActivity(GUI_Activity *activity);
static int onDrawActivity(GUI_Activity *activity);
static int onCtrlActivity(GUI_Activity *activity);

static GUI_ButtonInstruction button_instructions[] = {
    {LANG_LOCAL_BUTTON_CANCEL, LANG_PARENT_DIR, 1},
    {LANG_LOCAL_BUTTON_ENTER, LANG_OPEN_DIR, 1},
    {LANG_LOCAL_BUTTON_Y, LANG_OPTION_MENU, 1},
    {LANG_LOCAL_BUTTON_HOME, LANG_SETTING_MENU, 1},
    {LANG_LOCAL_BUTTON_SELECT, LANG_ABOUT, 1},
    {LANG_LOCAL_BUTTON_START, LANG_CHANGE_DIR, 1},
    {LANG_NULL, LANG_NULL, 0},
};

GUI_Activity browser_activity = {
    LANG_APP_TITLE,       // Title
    button_instructions,  // Button instructions
    NULL,                 // Wallpaper
    0,                    // Disable draw statusbar
    1,                    // Disable home event
    onStartActivity,      // Start callback
    onFinishActivity,     // Finish callback
    onBeforeDrawActivity, // Before draw callback
    onDrawActivity,       // Draw callback
    NULL,                 // After callback
    onCtrlActivity,       // Ctrl callback
    NULL,                 // Event callback
    NULL,                 // User data
};

enum OptionItemIndex
{
    INDEX_OPTION_ITEM_LOAD_GAME,
    INDEX_OPTION_ITEM_DELETE_GAME,
    INDEX_OPTION_ITEM_DELETE_AUTO_SAVESTATE,
    INDEX_OPTION_ITEM_DELETE_AUTO_SAVEFILE,
    INDEX_OPTION_ITEM_DELETE_CACHE_FILES,
};

static int option_items[] = {
    LANG_OPTION_MENU_START_GAME,
    LANG_OPTION_MENU_DELETE_GAME,
    LANG_OPTION_MENU_DELETE_AUTO_SAVESTATE,
    LANG_OPTION_MENU_DELETE_AUTO_SAVEFILE,
    LANG_OPTION_MENU_DELETE_CACHE_FILES,
};
#define N_OPTION_ITEMS (sizeof(option_items) / sizeof(int))

#define MAX_DIR_LEVELS 128

#define LAYOUT_CHILD_MARGIN 2

#define PATH_VIEW_PADDING_L 10
#define PATH_VIEW_PADDING_T 6
#define PATH_TEXT_COLOR COLOR_SPRING_GREEN

#define LISTVIEW_PADDING_L 2
#define LISTVIEW_PADDING_T 2
#define ITEMVIEW_PADDING_L 8
#define ITEMVIEW_PADDING_T 6

#define ITEMVIEW_COLOR_BG 0
#define ITEMVIEW_COLOR_FOCUS GUI_DEF_COLOR_FOCUS
#define FOLDER_TEXT_COLOR COLOR_WHITE
#define FILE_TEXT_COLOR COLOR_GREEN

#define PREVIEW_VIEW_PADDING 8
#define PREVIEW_REFRESH_DELAY_FRAMES 6

// File list
static LinkedList *file_list = NULL;
static int list_sort_mode = SORT_BY_NAME;

// Position
static int focus_pos_saves[MAX_DIR_LEVELS] = {0};
static int dir_level = 0;

// Preview
static int preview_need_refresh = 1;
static int preview_refresh_delay_frames = 0;

static Layout *browser_layout = NULL;
static TextView *path_textview = NULL;
static ListView *browser_listview = NULL;
static ImageView *preview_imageview = NULL;

static int onGetListLength(void *list)
{
    return LinkedListGetLength((LinkedList *)list);
}

static void *onGetHeadListEntry(void *list)
{
    return LinkedListHead((LinkedList *)list);
}

static void *onGetNextListEntry(void *list, void *entry, int id)
{
    return LinkedListNext((LinkedListEntry *)entry);
}

static void *onCreateItemView(void *data)
{
    ItemView *itemView = NewItemView();
    if (!itemView)
        return NULL;

    LayoutParamsSetPadding(itemView, ITEMVIEW_PADDING_L, ITEMVIEW_PADDING_L, ITEMVIEW_PADDING_T, ITEMVIEW_PADDING_T);
    LayoutParamsSetLayoutSize(itemView, TYPE_LAYOUT_PARAMS_MATH_PARENT, TYPE_LAYOUT_PARAMS_WRAP_CONTENT);

    if (data)
    {
        FileListEntryData *e_data = (FileListEntryData *)LinkedListGetEntryData((LinkedListEntry *)data);
        ItemViewSetNameText(itemView, e_data->name);
        if (e_data->is_folder)
            ItemViewSetNameTextColor(itemView, FOLDER_TEXT_COLOR);
        else
            ItemViewSetNameTextColor(itemView, FILE_TEXT_COLOR);
    }

    return itemView;
}

static int onSetItemViewData(void *itemView, void *data)
{
    return ItemViewSetData((ItemView *)itemView, data);
}

static int onBeforeDrawItemView(ListView *listView, void *itemView, int id)
{
    if (!listView || !itemView)
        return -1;

    int focus_pos = ListViewGetFocusPos(listView);
    if (focus_pos == id)
        ItemViewSetBgColor((ItemView *)itemView, ITEMVIEW_COLOR_FOCUS);
    else
        ItemViewSetBgColor((ItemView *)itemView, ITEMVIEW_COLOR_BG);

    return 0;
}

static int createLayout()
{
    int layout_x = 0, layout_y = 0;
    int available_w = 0, available_h = 0;
    int layout2_available_w = 0, layout2_available_h = 0;

    if (browser_layout)
        return 0;

    // 获取主布局的可用尺寸和位置
    GUI_GetActivityLayoutPosition(&browser_activity, &layout_x, &layout_y);
    GUI_GetActivityAvailableSize(&browser_activity, &available_w, &available_h);

    // 主布局
    browser_layout = NewLayout();
    if (!browser_layout)
        return -1;
    LayoutParamsSetAutoFree(browser_layout, 0);
    LayoutParamsSetOrientation(browser_layout, TYPE_LAYOUT_PARAMS_ORIENTATION_VERTICAL);
    LayoutParamsSetLayoutPosition(browser_layout, layout_x, layout_y);
    LayoutParamsSetAvailableSize(browser_layout, available_w, available_h);
    LayoutParamsSetLayoutSize(browser_layout, TYPE_LAYOUT_PARAMS_MATH_PARENT, TYPE_LAYOUT_PARAMS_MATH_PARENT);
    LayoutParamsSetPadding(browser_layout, GUI_DEF_MAIN_LAYOUT_PADDING, GUI_DEF_MAIN_LAYOUT_PADDING, GUI_DEF_MAIN_LAYOUT_PADDING, GUI_DEF_MAIN_LAYOUT_PADDING);

    // 文件列表父路径
    path_textview = NewTextView();
    if (!path_textview)
        return -1;
    LayoutParamsSetLayoutSize(path_textview, TYPE_LAYOUT_PARAMS_MATH_PARENT, GUI_GetLineHeight() + PATH_VIEW_PADDING_T * 2);
    LayoutParamsSetPadding(path_textview, PATH_VIEW_PADDING_L, PATH_VIEW_PADDING_L, PATH_VIEW_PADDING_T, PATH_VIEW_PADDING_T);
    TextViewSetBgColor(path_textview, GUI_DEF_COLOR_BG);
    TextViewSetSingleLine(path_textview, 1);
    TextViewSetTextColor(path_textview, PATH_TEXT_COLOR);
    TextViewSetText(path_textview, "");
    LayoutAddView(browser_layout, path_textview);

    // 文件列表和预览图布局
    Layout *layout2 = NewLayout();
    LayoutParamsSetOrientation(layout2, TYPE_LAYOUT_PARAMS_ORIENTATION_HORIZONTAL);
    LayoutParamsSetLayoutSize(layout2, TYPE_LAYOUT_PARAMS_MATH_PARENT, TYPE_LAYOUT_PARAMS_MATH_PARENT);
    LayoutParamsSetMargin(layout2, 0, 0, LAYOUT_CHILD_MARGIN, 0);
    LayoutAddView(browser_layout, layout2);

    // 先更新主布局以获取layout2的可用尺寸用来确定预览图的视图尺寸
    LayoutParamsUpdate(browser_layout);
    LayoutParamsGetAvailableSize(layout2, &layout2_available_w, &layout2_available_h);

    // 预览图
    preview_imageview = NewImageView();
    if (!preview_imageview)
        return -1;
    LayoutParamsSetLayoutSize(preview_imageview, layout2_available_h, layout2_available_h);
    LayoutParamsSetPadding(preview_imageview, PREVIEW_VIEW_PADDING, PREVIEW_VIEW_PADDING, PREVIEW_VIEW_PADDING, PREVIEW_VIEW_PADDING);
    ImageViewSetBgColor(preview_imageview, GUI_DEF_COLOR_BG);
    LayoutAddView(layout2, preview_imageview);

    // 文件列表
    browser_listview = NewListView();
    if (!browser_listview)
        return -1;
    LayoutParamsSetOrientation(browser_listview, TYPE_LAYOUT_PARAMS_ORIENTATION_VERTICAL);
    LayoutParamsSetLayoutSize(browser_listview, TYPE_LAYOUT_PARAMS_MATH_PARENT, TYPE_LAYOUT_PARAMS_MATH_PARENT);
    LayoutParamsSetMargin(browser_listview, 0, LAYOUT_CHILD_MARGIN, 0, 0);
    LayoutParamsSetPadding(browser_listview, LISTVIEW_PADDING_L, LISTVIEW_PADDING_L, LISTVIEW_PADDING_T, LISTVIEW_PADDING_T);
    ListViewSetBgColor(browser_listview, GUI_DEF_COLOR_BG);
    ListViewCallbacks callbacks;
    memset(&callbacks, 0, sizeof(ListViewCallbacks));
    callbacks.onCreateItemView = onCreateItemView;
    callbacks.onSetItemViewData = onSetItemViewData;
    callbacks.onBeforeDrawItemView = onBeforeDrawItemView;
    callbacks.onGetListLength = onGetListLength;
    callbacks.onGetHeadListEntry = onGetHeadListEntry;
    callbacks.onGetNextListEntry = onGetNextListEntry;
    ListViewSetList(browser_listview, file_list, &callbacks);
    LayoutAddViewAbove(layout2, browser_listview, preview_imageview);

    // 更新文件列表和预览图布局
    LayoutParamsUpdate(layout2);

    return 0;
}

static int destroyLayout()
{
    LayoutParamsDestroy(browser_layout);
    browser_layout = NULL;
    path_textview = NULL;
    preview_imageview = NULL;
    browser_listview = NULL;

    return 0;
}

int Browser_CurrentPathIsGame()
{
    int focus_pos = ListViewGetFocusPos(browser_listview);
    LinkedListEntry *entry = LinkedListFindByNum(file_list, focus_pos);
    FileListEntryData *data = (FileListEntryData *)LinkedListGetEntryData(entry);
    if (data)
        return !data->is_folder;

    return 0;
}

int Browser_GetCurrentRomType()
{
    int focus_pos = ListViewGetFocusPos(browser_listview);
    LinkedListEntry *entry = LinkedListFindByNum(file_list, focus_pos);
    FileListEntryData *data = (FileListEntryData *)LinkedListGetEntryData(entry);
    if (!data)
        return -1;

    return data->rom_type;
}

int Browser_MakeCurrentFileName(char *name)
{
    int focus_pos = ListViewGetFocusPos(browser_listview);
    LinkedListEntry *entry = LinkedListFindByNum(file_list, focus_pos);
    FileListEntryData *data = (FileListEntryData *)LinkedListGetEntryData(entry);
    if (!data || !data->name)
        return -1;

    snprintf(name, MAX_NAME_LENGTH, "%s", data->name);
    return 0;
}

int Browser_MakeCurrentFilePath(char *path)
{
    FileListData *ls_data = (FileListData *)LinkedListGetListData(file_list);
    int focus_pos = ListViewGetFocusPos(browser_listview);
    LinkedListEntry *entry = LinkedListFindByNum(file_list, focus_pos);
    FileListEntryData *e_data = (FileListEntryData *)LinkedListGetEntryData(entry);
    if (!ls_data || !e_data || !e_data->name)
        return -1;

    snprintf(path, MAX_PATH_LENGTH, "%s%s", ls_data->path, e_data->name);
    return 0;
}

GUI_Texture *GetDefaultPreviewTexture()
{
    char path[MAX_PATH_LENGTH];
    MakePreviewPath(path, "png");
    GUI_Texture *texture = GUI_LoadPNGFile(path);
    if (!texture)
    {
        MakePreviewPath(path, "jpg");
        texture = GUI_LoadJPEGFile(path);
    }
    return texture;
}

static void updatePreviewImageView()
{
    switch (app_config.preview_style)
    {
    case TYPE_PREVIEW_SCALE_TYPE_FIT_XY:
        ImageViewSetScaleType(preview_imageview, TYPE_IMAGE_SCALE_FIT_XY);
        break;
    case TYPE_PREVIEW_SCALE_TYPE_FIT_CENTER_CROP:
        ImageViewSetScaleType(preview_imageview, TYPE_IMAGE_SCALE_FIT_CENTER_CROP);
        break;
    case TYPE_PREVIEW_SCALE_TYPE_FIT_CENTER_INSIDE:
    default:
        ImageViewSetScaleType(preview_imageview, TYPE_IMAGE_SCALE_FIT_CENTER_INSIDE);
        break;
    }
    LayoutParamsUpdate(preview_imageview);
}

static void updatePreview()
{
    GUI_Texture *texture = NULL;

    if (CurrentPathIsGame())
    {
        switch (app_config.preview_path)
        {
        case TYPE_PREVIEW_PATH_DEFAULT:
            texture = GetDefaultPreviewTexture();
            break;
        case TYPE_PREVIEW_PATH_SAVESTATE:
            texture = Emu_GetStateScreenshotTexture(-1);
            break;
        case TYPE_PREVIEW_PATH_AUTO:
        default:
            if (misc_config.auto_save_load)
                texture = Emu_GetStateScreenshotTexture(-1);
            if (!texture)
                texture = GetDefaultPreviewTexture();
            break;
        }
    }

    ImageViewSetTexture(preview_imageview, texture);
    updatePreviewImageView();
}

static void checkPreviewUpdate()
{
    if (preview_need_refresh)
    {
        // 销毁旧预览图的texture
        GUI_Texture *texture = (GUI_Texture *)ImageViewGetTexture(preview_imageview);
        if (texture)
        {
            ImageViewSetTexture(preview_imageview, NULL);
            GUI_DestroyTexture(texture);
        }

        if (preview_refresh_delay_frames > 0)
        {
            preview_refresh_delay_frames--;
        }
        else
        {
            updatePreview();
            preview_need_refresh = 0;
        }
    }
}

static void moveFileListPos(int type)
{
    static int old_dir_level = -1;
    static int old_focus_pos = -1;

    ListViewMoveFocusPos(browser_listview, type);
    int focus_pos = ListViewGetFocusPos(browser_listview);

    if (old_dir_level != dir_level || old_focus_pos != focus_pos)
    {
        Browser_RequestRefreshPreview(0);
        Setting_SetStateSelectId(0);
    }

    if (CurrentPathIsGame())
    {
        button_instructions[1].instruction = LANG_START_GAME;
        button_instructions[2].visibility = 1;
    }
    else
    {
        button_instructions[1].instruction = LANG_OPEN_DIR;
        button_instructions[2].visibility = 0;
    }

    old_dir_level = dir_level;
    old_focus_pos = focus_pos;
}

static int dirLeveForward(int pos)
{
    if (dir_level < MAX_DIR_LEVELS - 1)
    {
        focus_pos_saves[dir_level] = pos;
        dir_level++;
    }

    // 返回新光标位置
    return 0;
}

static int dirLeveBackward()
{
    FileListData *ls_data = (FileListData *)LinkedListGetListData(file_list);
    RemoveEndSlash(ls_data->path);

    char *p;
    p = strrchr(ls_data->path, '/'); // ux0:abc/cde
    if (p)
    {
        p[1] = '\0';
        dir_level--;
        goto DIR_UP_RETURN;
    }

    p = strrchr(ls_data->path, ':'); // ux0:abc || ux0:
    if (p)
    {
        if (strlen(ls_data->path) - ((p + 1) - ls_data->path) > 0) // strlen(ux0:abc) - (p(:)+1 - p(u)) > 0 for skip ux0:
        {
            p[1] = '\0';
            dir_level--;
            goto DIR_UP_RETURN;
        }
    }

    strcpy(ls_data->path, HOME_PATH);
    dir_level = 0;

DIR_UP_RETURN:
    if (dir_level < 0)
        dir_level = 0;

    // 返回旧光标位置
    return focus_pos_saves[dir_level];
}

static int getPosByName(const char *name)
{
    int length = LinkedListGetLength(file_list);
    int pos = FileListGetNumberByName(file_list, name);
    if (pos < 0 || pos > length - 1)
        return 0;

    return pos;
}

static int refreshFileList(int pos, int update_views)
{
    int ret = 0, res = 0;
    FileListData *ls_data = (FileListData *)LinkedListGetListData(file_list);
    int focus_pos = pos;

    do
    {
        LinkedListEmpty(file_list);
        res = FileListGetEntries(file_list, ls_data->path, list_sort_mode);
        if (res < 0)
        {
            ret = res;
            focus_pos = dirLeveBackward();
        }
    } while (res < 0);

    if (update_views)
    {
        TextViewSetText(path_textview, ls_data->path);
        LayoutParamsUpdate(path_textview);
        ListViewRefreshtList(browser_listview);
        LayoutParamsUpdate(browser_listview);
        ListViewSetFocusPos(browser_listview, focus_pos);
        moveFileListPos(TYPE_MOVE_NONE);
    }

    return ret;
}

static int changeToDir(char *lastdir)
{
    if (!CheckFolderExist(lastdir))
        return -1;

    FileListData *ls_data = (FileListData *)LinkedListGetListData(file_list);
    dir_level = 0;
    strcpy(ls_data->path, HOME_PATH);

    int fcous_pos = 0;
    int i;
    for (i = 0; i < strlen(lastdir) + 1; i++)
    {
        if (lastdir[i] == ':' || lastdir[i] == '/')
        {
            char ch = lastdir[i + 1];
            lastdir[i + 1] = '\0';

            char ch2 = lastdir[i];
            lastdir[i] = '\0';

            char *p = strrchr(lastdir, '/');
            if (!p)
                p = strrchr(lastdir, ':');
            if (!p)
                p = lastdir - 1;

            lastdir[i] = ch2;

            refreshFileList(0, 0);
            fcous_pos = getPosByName(p + 1);

            strcpy(ls_data->path, lastdir);

            lastdir[i + 1] = ch;

            dirLeveForward(fcous_pos);
        }
    }
    refreshFileList(fcous_pos, 1);

    return 0;
}

static void backToParentDir()
{
    if (dir_level > 0)
    {
        refreshFileList(dirLeveBackward(), 1);
    }
}

static void enterToChildDir(LinkedListEntry *entry)
{
    FileListData *ls_data = (FileListData *)LinkedListGetListData(file_list);
    FileListEntryData *e_data = (FileListEntryData *)LinkedListGetEntryData(entry);
    if (!ls_data || !e_data)
        return;

    if (dir_level == 0)
    {
        strcpy(ls_data->path, e_data->name);
    }
    else
    {
        if (dir_level > 1)
            AddEndSlash(ls_data->path);
        strcat(ls_data->path, e_data->name);
    }

    refreshFileList(dirLeveForward(ListViewGetFocusPos(browser_listview)), 1);
}

static void startGame(LinkedListEntry *entry)
{
    FileListData *ls_data = (FileListData *)LinkedListGetListData(file_list);
    FileListEntryData *e_data = (FileListEntryData *)LinkedListGetEntryData(entry);
    if (!ls_data || !e_data)
        return;

    EmuGameInfo info;
    snprintf(info.path, MAX_PATH_LENGTH, "%s%s", ls_data->path, e_data->name);
    info.rom_type = e_data->rom_type;
    info.state_num = -2;
    Emu_StartGame(&info);
}

static int onAlertDialogDeleteGame(AlertDialog *dialog, int which)
{
    char path[MAX_PATH_LENGTH];
    if (MakeCurrentGamePath(path) < 0)
        return -1;

    sceIoRemove(path);
#ifdef FBA_BUILD
    FileListData *ls_data = (FileListData *)LinkedListGetListData(file_list);
    char base_name[MAX_NAME_LENGTH];
    MakeBaseName(base_name, path, MAX_NAME_LENGTH);
    snprintf(path, MAX_PATH_LENGTH, "%s%s.dat", ls_data->path, base_name);
    sceIoRemove(path);
#endif
    refreshFileList(ListViewGetFocusPos(browser_listview), 1);
    Browser_RequestRefreshPreview(1);

    if (dialog)
    {
        AlertDialog *prev_dialog = AlertDialog_GetData(dialog);
        AlertDialog_Dismiss(prev_dialog);
        AlertDialog_Dismiss(dialog);
    }

    return 0;
}

static int onAlertDialogDeleteAutoState(AlertDialog *dialog, int which)
{
    Emu_DeleteState(-1);
    Browser_RequestRefreshPreview(1);

    if (dialog)
    {
        AlertDialog *prev_dialog = AlertDialog_GetData(dialog);
        AlertDialog_Dismiss(prev_dialog);
        AlertDialog_Dismiss(dialog);
    }

    return 0;
}

static int onAlertDialogDeleteAutoSrm(AlertDialog *dialog, int which)
{
    Emu_DeleteSrm();

    if (dialog)
    {
        AlertDialog *prev_dialog = AlertDialog_GetData(dialog);
        AlertDialog_Dismiss(prev_dialog);
        AlertDialog_Dismiss(dialog);
    }

    return 0;
}

static int onAlertDialogDeleteCacheFiles(AlertDialog *dialog, int which)
{
#if defined(WANT_EXT_ARCHIVE_ROM)
    char path[MAX_PATH_LENGTH];
    MakeCurrentGamePath(path);
    Archive_CleanCacheByPath(path);
#endif

    if (dialog)
    {
        AlertDialog *prev_dialog = AlertDialog_GetData(dialog);
        AlertDialog_Dismiss(prev_dialog);
        AlertDialog_Dismiss(dialog);
    }

    return 0;
}

static int onOptionMenuAlertDialogPositiveClick(AlertDialog *dialog, int which)
{
    if (!dialog)
        return -1;

    int focus_pos = ListViewGetFocusPos(browser_listview);

    switch (which)
    {
    case INDEX_OPTION_ITEM_LOAD_GAME:
    {
        LinkedListEntry *entry = LinkedListFindByNum(file_list, focus_pos);
        FileListEntryData *data = (FileListEntryData *)LinkedListGetEntryData(entry);
        if (data && !data->is_folder)
        {
            AlertDialog_Close(dialog);
            startGame(entry);
        }
        break;
    }
    case INDEX_OPTION_ITEM_DELETE_GAME:
    {
        if (CurrentPathIsGame())
        {
            AlertDialog *ask_dialog = AlertDialog_Create();
            AlertDialog_SetTitle(ask_dialog, cur_lang[LANG_TIP]);
            AlertDialog_SetMessage(ask_dialog, cur_lang[LANG_MESSAGE_ASK_DELETE_GAME]);
            AlertDialog_SetPositiveButton(ask_dialog, cur_lang[LANG_CONFIRM], onAlertDialogDeleteGame);
            AlertDialog_SetNegativeButton(ask_dialog, cur_lang[LANG_CANCEL], AlertDialog_OnClickDismiss);
            AlertDialog_SetData(ask_dialog, dialog);
            AlertDialog_Show(ask_dialog);
        }
        break;
    }
    case INDEX_OPTION_ITEM_DELETE_AUTO_SAVESTATE:
    {
        if (CurrentPathIsGame())
        {
            AlertDialog *ask_dialog = AlertDialog_Create();
            AlertDialog_SetTitle(ask_dialog, cur_lang[LANG_TIP]);
            AlertDialog_SetMessage(ask_dialog, cur_lang[LANG_MESSAGE_ASK_DELETE_AUTO_STATE]);
            AlertDialog_SetPositiveButton(ask_dialog, cur_lang[LANG_CONFIRM], onAlertDialogDeleteAutoState);
            AlertDialog_SetNegativeButton(ask_dialog, cur_lang[LANG_CANCEL], AlertDialog_OnClickDismiss);
            AlertDialog_SetData(ask_dialog, dialog);
            AlertDialog_Show(ask_dialog);
        }
        break;
    }
    case INDEX_OPTION_ITEM_DELETE_AUTO_SAVEFILE:
    {
        if (CurrentPathIsGame())
        {
            AlertDialog *ask_dialog = AlertDialog_Create();
            AlertDialog_SetTitle(ask_dialog, cur_lang[LANG_TIP]);
            AlertDialog_SetMessage(ask_dialog, cur_lang[LANG_MESSAGE_ASK_DELETE_AUTO_SAVEFILE]);
            AlertDialog_SetPositiveButton(ask_dialog, cur_lang[LANG_CONFIRM], onAlertDialogDeleteAutoSrm);
            AlertDialog_SetNegativeButton(ask_dialog, cur_lang[LANG_CANCEL], AlertDialog_OnClickDismiss);
            AlertDialog_SetData(ask_dialog, dialog);
            AlertDialog_Show(ask_dialog);
        }
        break;
    }
    case INDEX_OPTION_ITEM_DELETE_CACHE_FILES:
    {
        if (CurrentPathIsGame())
        {
            AlertDialog *ask_dialog = AlertDialog_Create();
            AlertDialog_SetTitle(ask_dialog, cur_lang[LANG_TIP]);
            AlertDialog_SetMessage(ask_dialog, cur_lang[LANG_MESSAGE_ASK_DELETE_CACHE_FILES]);
            AlertDialog_SetPositiveButton(ask_dialog, cur_lang[LANG_CONFIRM], onAlertDialogDeleteCacheFiles);
            AlertDialog_SetNegativeButton(ask_dialog, cur_lang[LANG_CANCEL], AlertDialog_OnClickDismiss);
            AlertDialog_SetData(ask_dialog, dialog);
            AlertDialog_Show(ask_dialog);
        }
        break;
    }
    default:
        break;
    }

    return 0;
}

static void openOptionMenu()
{
    int focus_pos = ListViewGetFocusPos(browser_listview);
    LinkedListEntry *entry = LinkedListFindByNum(file_list, focus_pos);
    FileListEntryData *data = (FileListEntryData *)LinkedListGetEntryData(entry);
    if (!data || data->is_folder)
        return;

    AlertDialog *dialog = AlertDialog_Create();
    AlertDialog_SetTitle(dialog, cur_lang[LANG_MENU]);
    int n_items = N_OPTION_ITEMS;
    char **items = GetStringArrayByLangArray(option_items, n_items);
    AlertDialog_SetItems(dialog, items, n_items);
    free(items);
    AlertDialog_SetPositiveButton(dialog, cur_lang[LANG_CONFIRM], onOptionMenuAlertDialogPositiveClick);
    AlertDialog_SetNegativeButton(dialog, cur_lang[LANG_CANCEL], AlertDialog_OnClickDismiss);
    AlertDialog_Show(dialog);
}

static void onFileListViewItemClick(ListView *listView, void *itemView, int id)
{
    LinkedList *list = (LinkedList *)ListViewGetList(listView);
    LinkedListEntry *entry;
    if (itemView)
        entry = ItemViewGetData((ItemView *)itemView);
    else
        entry = LinkedListFindByNum(list, id);
    FileListEntryData *data = (FileListEntryData *)LinkedListGetEntryData(entry);
    if (data)
    {
        if (data->is_folder)
            enterToChildDir(entry);
        else
            startGame(entry);
    }
}

static int onStartActivity(GUI_Activity *activity)
{
    browser_activity.wallpaper = GUI_GetImage(ID_GUI_IMAGE_WALLPAPER);

    // 先创建file_list，createLayout要用到
    file_list = NewFileList();
    if (!file_list)
        return -1;

    createLayout();

    // 跳转到最后游玩的游戏位置
    if (Browser_ChangeDirBySaveFile(LASTFILE_PATH) < 0)
    {
        // 如果失败打开home目录列表
        FileListData *ls_data = (FileListData *)LinkedListGetListData(file_list);
        strcpy(ls_data->path, HOME_PATH);
        refreshFileList(0, 1);
    }

    return 0;
}

static int onFinishActivity(GUI_Activity *activity)
{
    destroyLayout();
    LinkedListDestroy(file_list);
    file_list = NULL;
    return 0;
}

static int onBeforeDrawActivity(GUI_Activity *activity)
{
    if (!browser_activity.wallpaper)
        browser_activity.wallpaper = GUI_GetImage(ID_GUI_IMAGE_WALLPAPER);

    checkPreviewUpdate();

    return 0;
}

static int onDrawActivity(GUI_Activity *activity)
{
    LayoutParamsDraw(browser_layout);
    return 0;
}

static int onCtrlActivity(GUI_Activity *activity)
{
    if (hold_pad[PAD_UP] || hold2_pad[PAD_LEFT_ANALOG_UP])
    {
        moveFileListPos(TYPE_MOVE_UP);
    }
    else if (hold_pad[PAD_DOWN] || hold2_pad[PAD_LEFT_ANALOG_DOWN])
    {
        moveFileListPos(TYPE_MOVE_DOWN);
    }
    else if (hold_pad[PAD_LEFT])
    {
        moveFileListPos(TYPE_MOVE_LEFT);
    }
    else if (hold_pad[PAD_RIGHT])
    {
        moveFileListPos(TYPE_MOVE_RIGHT);
    }
    else if (released_pad[PAD_Y])
    {
        openOptionMenu();
    }
    else if (released_pad[PAD_CANCEL])
    {
        backToParentDir();
    }
    else if (released_pad[PAD_ENTER])
    {
        onFileListViewItemClick(browser_listview, NULL, ListViewGetFocusPos(browser_listview));
    }
    else if (released_pad[PAD_SELECT])
    {
        GUI_StartActivity(&about_activity);
    }
    else if (released_pad[PAD_START])
    {
        Browser_ChangeDirBySaveFile(LASTFILE_PATH);
    }
    else if (released_pad[PAD_HOME])
    {
        if (GUI_IsHomeKeyEnabled())
            Setting_OpenMenu();
    }

    return 0;
}

void Browser_RequestRefreshPreview(int urgent)
{
    if (urgent) // 立即更新
        preview_refresh_delay_frames = 0;
    else
        preview_refresh_delay_frames = PREVIEW_REFRESH_DELAY_FRAMES;

    preview_need_refresh = 1;
}

int Browser_ChangeDirByFilePath(const char *path)
{
    int ret;

    char lastdir[MAX_PATH_LENGTH];
    ret = MakeParentDir(lastdir, path, MAX_PATH_LENGTH);
    if (ret < 0)
        return ret;

    ret = changeToDir(lastdir);
    if (ret < 0)
        return ret;

    char name[MAX_NAME_LENGTH];
    ret = MakeFileName(name, path, MAX_NAME_LENGTH);
    if (ret >= 0)
    {
        ListViewSetFocusPos(browser_listview, getPosByName(name));
        moveFileListPos(TYPE_MOVE_NONE);
    }

    return 0;
}

int Browser_ChangeDirBySaveFile(const char *path)
{
    char lastfile[MAX_PATH_LENGTH];
    if (ReadFile(path, lastfile, sizeof(lastfile)) <= 0)
        return -1;
    lastfile[MAX_PATH_LENGTH - 1] = '\0';

    return Browser_ChangeDirByFilePath(lastfile);
}
