#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/threadmgr.h>

#include "list/linked_list.h"
#include "gui.h"

struct GUI_Window
{
    int dont_free;
    GUI_WindowCallbacks callbacks;
    void *userdata;
    LinkedListEntry *entry;
};

static SceKernelLwMutexWork gui_window_mutex = {0};
static LinkedList *gui_window_list = NULL;

static int Window_Close(GUI_Window *window);
static void Window_Destroy(GUI_Window *window);

static void freeWindowEntryData(void *data)
{
    if (data)
    {
        GUI_Window *window = (GUI_Window *)data;
        Window_Close(window);
        Window_Destroy(window);
    }
}

static LinkedList *NewWindowList()
{
    LinkedList *list = NewLinkedList();
    if (!list)
        return NULL;

    LinkedListSetFreeEntryDataCallback(list, freeWindowEntryData);

    return list;
}

static GUI_Window *Window_Create()
{
    GUI_Window *window = (GUI_Window *)calloc(1, sizeof(GUI_Window));
    return window;
}

static void Window_Destroy(GUI_Window *window)
{
    if (window && !window->dont_free)
        free(window);
}

static int Window_Open(GUI_Window *window)
{
    if (!window)
        return -1;

    if (window->callbacks.onOpen)
        window->callbacks.onOpen(window);

    return 0;
}

static int Window_Close(GUI_Window *window)
{
    if (!window)
        return -1;

    if (window->callbacks.onClose)
        window->callbacks.onClose(window);

    return 0;
}

static int Window_BeforeDraw(GUI_Window *window)
{
    if (!window)
        return -1;

    if (window->callbacks.onBeforeDraw)
        window->callbacks.onBeforeDraw(window);

    return 0;
}

static int Window_Draw(GUI_Window *window)
{
    if (!window)
        return -1;

    if (window->callbacks.onDraw)
        window->callbacks.onDraw(window);

    return 0;
}

static int Window_AfterDraw(GUI_Window *window)
{
    if (!window)
        return -1;

    if (window->callbacks.onAfterDraw)
        window->callbacks.onAfterDraw(window);

    return 0;
}

static int Window_Ctrl(GUI_Window *window)
{
    if (!window)
        return -1;

    if (window->callbacks.onCtrl)
        window->callbacks.onCtrl(window);

    return 0;
}

static int Window_Event(GUI_Window *window)
{
    if (!window)
        return -1;

    if (window->callbacks.onEvent)
        window->callbacks.onEvent(window);

    return 0;
}

static GUI_Window *Window_GetCurrent()
{
    return (GUI_Window *)LinkedListGetEntryData(LinkedListTail(gui_window_list));
}

GUI_Window *GUI_CreateWindow()
{
    return Window_Create();
}

void GUI_DestroyWindow(GUI_Window *window)
{
    Window_Destroy(window);
}

int GUI_OpenWindow(GUI_Window *window)
{
    if (!window)
        return -1;

    int ret = 0;
    sceKernelLockLwMutex(&gui_window_mutex, 1, NULL);
    window->entry = LinkedListAdd(gui_window_list, window);
    if (!window->entry)
        ret = -1;
    else
        Window_Open(window);
    sceKernelUnlockLwMutex(&gui_window_mutex, 1);
    return ret;
}

int GUI_CloseWindow(GUI_Window *window)
{
    if (!window)
        return -1;

    sceKernelLockLwMutex(&gui_window_mutex, 1, NULL);
    if (!LinkedListRemove(gui_window_list, window->entry))
        Window_Destroy(window);
    sceKernelUnlockLwMutex(&gui_window_mutex, 1);
    return 0;
}

int GUI_CloseAllWindows()
{
    sceKernelLockLwMutex(&gui_window_mutex, 1, NULL);
    LinkedListEmpty(gui_window_list);
    sceKernelUnlockLwMutex(&gui_window_mutex, 1);
    return 0;
}

int GUI_CloseOtherWindows()
{
    sceKernelLockLwMutex(&gui_window_mutex, 1, NULL);
    LinkedListEntry *entry = LinkedListPrev(LinkedListTail(gui_window_list));
    while (entry)
    {
        LinkedListEntry *prev = LinkedListPrev(entry);
        LinkedListRemove(gui_window_list, entry);
        entry = prev;
    }
    sceKernelUnlockLwMutex(&gui_window_mutex, 1);
    return 0;
}

int GUI_BeforeDrawWindow()
{
    sceKernelLockLwMutex(&gui_window_mutex, 1, NULL);

    LinkedListEntry *entry = LinkedListHead(gui_window_list);
    while (entry)
    {
        LinkedListEntry *next = LinkedListNext(entry);
        GUI_Window *window = (GUI_Window *)LinkedListGetEntryData(entry);
        Window_BeforeDraw(window);
        entry = next;
    }

    sceKernelUnlockLwMutex(&gui_window_mutex, 1);
    return 0;
}

int GUI_DrawWindow()
{
    sceKernelLockLwMutex(&gui_window_mutex, 1, NULL);

    LinkedListEntry *entry = LinkedListHead(gui_window_list);
    while (entry)
    {
        LinkedListEntry *next = LinkedListNext(entry);
        GUI_Window *window = (GUI_Window *)LinkedListGetEntryData(entry);
        Window_Draw(window);
        entry = next;
    }

    sceKernelUnlockLwMutex(&gui_window_mutex, 1);
    return 0;
}

int GUI_AfterDrawWindow()
{
    sceKernelLockLwMutex(&gui_window_mutex, 1, NULL);

    LinkedListEntry *entry = LinkedListHead(gui_window_list);
    while (entry)
    {
        LinkedListEntry *next = LinkedListNext(entry);
        GUI_Window *window = (GUI_Window *)LinkedListGetEntryData(entry);
        Window_AfterDraw(window);
        entry = next;
    }

    sceKernelUnlockLwMutex(&gui_window_mutex, 1);
    return 0;
}

int GUI_CtrlWindow()
{
    sceKernelLockLwMutex(&gui_window_mutex, 1, NULL);
    int ret = Window_Ctrl(Window_GetCurrent());
    sceKernelUnlockLwMutex(&gui_window_mutex, 1);
    return ret;
}

int GUI_EventWindow()
{
    sceKernelLockLwMutex(&gui_window_mutex, 1, NULL);
    int ret = Window_Event(Window_GetCurrent());
    sceKernelUnlockLwMutex(&gui_window_mutex, 1);
    return ret;
}

int GUI_SetWindowAutoFree(GUI_Window *window, int auto_free)
{
    if (!window)
        return -1;

    window->dont_free = !auto_free;

    return 0;
}

int GUI_SetWindowData(GUI_Window *window, void *data)
{
    if (!window)
        return -1;

    window->userdata = data;

    return 0;
}

int GUI_SetWindowCallbacks(GUI_Window *window, GUI_WindowCallbacks *callbacks)
{
    if (!window)
        return -1;

    if (callbacks)
        memcpy(&window->callbacks, callbacks, sizeof(GUI_WindowCallbacks));
    else
        memset(&window->callbacks, 0, sizeof(GUI_WindowCallbacks));

    return 0;
}

int GUI_GetWindowCount()
{
    return LinkedListGetLength(gui_window_list);
}

void *GUI_GetWindowData(GUI_Window *window)
{
    return window ? window->userdata : NULL;
}

int GUI_GetWindowLayoutPosition(int *x, int *y)
{
    if (x)
        *x = 0;
    if (y)
        *y = 0;
    return 0;
}

int GUI_GetWindowAvailableSize(int *w, int *h)
{
    if (w)
        *w = GUI_SCREEN_WIDTH;
    if (h)
        *h = GUI_SCREEN_HEIGHT;
    return 0;
}

int GUI_InitWindow()
{
    gui_window_list = NewWindowList();
    if (!gui_window_list)
        return -1;

    sceKernelCreateLwMutex(&gui_window_mutex, "gui_window_mutex", 2, 0, NULL);

    return 0;
}

int GUI_DeinitWindow()
{
    sceKernelLockLwMutex(&gui_window_mutex, 1, NULL);
    LinkedListDestroy(gui_window_list);
    gui_window_list = NULL;
    sceKernelUnlockLwMutex(&gui_window_mutex, 1);
    sceKernelDeleteLwMutex(&gui_window_mutex);

    return 0;
}
