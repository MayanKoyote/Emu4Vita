#ifndef __M_GUI_WINDOW_H__
#define __M_GUI_WINDOW_H__

typedef struct GUI_Window GUI_Window;

typedef struct GUI_WindowCallbacks
{
    int (*onOpen)(GUI_Window *window);
    int (*onClose)(GUI_Window *window);
    int (*onBeforeDraw)(GUI_Window *window);
    int (*onDraw)(GUI_Window *window);
    int (*onAfterDraw)(GUI_Window *window);
    int (*onCtrl)(GUI_Window *window);
    int (*onEvent)(GUI_Window *window);
} GUI_WindowCallbacks;

int GUI_InitWindow();
int GUI_DeinitWindow();

GUI_Window *GUI_CreateWindow();
void GUI_DestroyWindow(GUI_Window *window); // 如果window已打开，请使用GUI_CloseWindow

int GUI_OpenWindow(GUI_Window *window);
int GUI_CloseWindow(GUI_Window *window);
int GUI_CloseAllWindows();
int GUI_CloseOtherWindows();

int GUI_SetWindowAutoFree(GUI_Window *window, int auto_free);
int GUI_SetWindowData(GUI_Window *window, void *data);
int GUI_SetWindowCallbacks(GUI_Window *window, GUI_WindowCallbacks *callbacks);

int GUI_GetWindowCount();
void *GUI_GetWindowData(GUI_Window *window);

int GUI_GetWindowLayoutPosition(int *x, int *y);
int GUI_GetWindowAvailableSize(int *w, int *h);

#endif
