#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "list/linked_list.h"
#include "gui.h"

struct GUI_Window
{
    int dont_free;
    GUI_WindowCallbacks callbacks;
    void *userdata;
};

static LinkedList *window_list = NULL;

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
    int ret = 0;
    GUI_LockDrawMutex();

    if (!window_list)
    {
        window_list = NewWindowList();
        if (!window_list)
            goto FAILED;
    }

    if (!LinkedListAdd(window_list, window))
        goto FAILED;

    Window_Open(window);

EXIT:
    GUI_UnlockDrawMutex();
    return ret;

FAILED:
    ret = -1;
    goto EXIT;
}

int GUI_CloseWindow(GUI_Window *window)
{
    GUI_LockDrawMutex();

    LinkedListEntry *entry = LinkedListFindByData(window_list, window);
    if (!LinkedListRemove(window_list, entry))
        Window_Destroy(window);

    GUI_UnlockDrawMutex();
    return 0;
}

int GUI_CloseWindowEx(GUI_WindowCloseType type, GUI_Window *window)
{
    switch (type)
    {
    case TYPE_WINDOW_CLOSE_SELF:
        return GUI_CloseWindow(window);

    case TYPE_WINDOW_CLOSE_PREV:
        return GUI_CloseWindow(GUI_GetPrevWindow(window));

    case TYPE_WINDOW_CLOSE_NEXT:
        return GUI_CloseWindow(GUI_GetNextWindow(window));

    case TYPE_WINDOW_CLOSE_ALL:
        GUI_LockDrawMutex();
        LinkedListEmpty(window_list);
        GUI_UnlockDrawMutex();
        break;

    case TYPE_WINDOW_CLOSE_ALL_PREV:
    {
        GUI_LockDrawMutex();
        LinkedListEntry *entry = LinkedListFindByData(window_list, window);
        entry = LinkedListPrev(entry);
        while (entry)
        {
            LinkedListEntry *prev = LinkedListPrev(entry);
            LinkedListRemove(window_list, entry);
            entry = prev;
        }
        GUI_UnlockDrawMutex();
        break;
    }

    case TYPE_WINDOW_CLOSE_ALL_NEXT:
    {
        GUI_LockDrawMutex();
        LinkedListEntry *entry = LinkedListFindByData(window_list, window);
        entry = LinkedListNext(entry);
        while (entry)
        {
            LinkedListEntry *next = LinkedListNext(entry);
            LinkedListRemove(window_list, entry);
            entry = next;
        }
        GUI_UnlockDrawMutex();
        break;
    }

    default:
        break;
    }

    return 0;
}

int GUI_BeforeDrawWindow()
{
    LinkedListEntry *entry = LinkedListHead(window_list);

    while (entry)
    {
        LinkedListEntry *next = LinkedListNext(entry);
        GUI_Window *window = (GUI_Window *)LinkedListGetEntryData(entry);
        Window_BeforeDraw(window);
        entry = next;
    }

    return 0;
}

int GUI_DrawWindow()
{
    LinkedListEntry *entry = LinkedListHead(window_list);

    while (entry)
    {
        LinkedListEntry *next = LinkedListNext(entry);
        GUI_Window *window = (GUI_Window *)LinkedListGetEntryData(entry);
        Window_Draw(window);
        entry = next;
    }

    return 0;
}

int GUI_AfterDrawWindow()
{
    LinkedListEntry *entry = LinkedListHead(window_list);

    while (entry)
    {
        LinkedListEntry *next = LinkedListNext(entry);
        GUI_Window *window = (GUI_Window *)LinkedListGetEntryData(entry);
        Window_AfterDraw(window);
        entry = next;
    }

    return 0;
}

int GUI_CtrlWindow()
{
    return Window_Ctrl(GUI_GetCurrentWindow());
}

int GUI_EventWindow()
{
    return Window_Event(GUI_GetCurrentWindow());
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
    return LinkedListGetLength(window_list);
}

GUI_Window *GUI_GetCurrentWindow()
{
    LinkedListEntry *tail = LinkedListTail(window_list);

    return (GUI_Window *)LinkedListGetEntryData(tail);
}

GUI_Window *GUI_GetPrevWindow(GUI_Window *window)
{
    LinkedListEntry *entry = LinkedListFindByData(window_list, window);
    LinkedListEntry *prev = LinkedListPrev(entry);

    return (GUI_Window *)LinkedListGetEntryData(prev);
}

GUI_Window *GUI_GetNextWindow(GUI_Window *window)
{
    LinkedListEntry *entry = LinkedListFindByData(window_list, window);
    LinkedListEntry *next = LinkedListNext(entry);

    return (GUI_Window *)LinkedListGetEntryData(next);
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
