#ifndef __M_GUI_WINDOW_H__
#define __M_GUI_WINDOW_H__

typedef enum GUI_WindowCloseType
{
    TYPE_WINDOW_CLOSE_SELF = 0,
    TYPE_WINDOW_CLOSE_PREV,
    TYPE_WINDOW_CLOSE_NEXT,
    TYPE_WINDOW_CLOSE_ALL,
    TYPE_WINDOW_CLOSE_ALL_PREV,
    TYPE_WINDOW_CLOSE_ALL_NEXT,
} GUI_WindowCloseType;

typedef struct GUI_Window GUI_Window;

typedef struct GUI_WindowCallbacks
{
    int (*onOpen)(GUI_Window *window);
    int (*onClose)(GUI_Window *window);
    int (*onDraw)(GUI_Window *window);
    int (*onCtrl)(GUI_Window *window);
    int (*onEvent)(GUI_Window *window); // onEvent任何时候都会执行，可以在此更新一些操作，onCtrl只会调用最后一个window的onCtrl，onDraw是在帧执行中的，onEvent是在帧执行后，onEvent比较安全
} GUI_WindowCallbacks;

GUI_Window *GUI_CreateWindow();
void GUI_DestroyWindow(GUI_Window *window); // 如果window已打开，请使用GUI_CloseWindow

int GUI_OpenWindow(GUI_Window *window);
int GUI_CloseWindow(GUI_Window *window);
int GUI_CloseWindowEx(GUI_WindowCloseType type, GUI_Window *window);

int GUI_SetWindowAutoFree(GUI_Window *window, int auto_free);
int GUI_SetWindowData(GUI_Window *window, void *data);
int GUI_SetWindowCallbacks(GUI_Window *window, GUI_WindowCallbacks *callbacks);

int GUI_GetWindowCount();
GUI_Window *GUI_GetCurrentWindow();
GUI_Window *GUI_GetPrevWindow(GUI_Window *window);
GUI_Window *GUI_GetNextWindow(GUI_Window *window);
void *GUI_GetWindowData(GUI_Window *window);

int GUI_GetWindowLayoutPosition(int *x, int *y);
int GUI_GetWindowAvailableSize(int *w, int *h);

#endif
