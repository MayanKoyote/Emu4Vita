#ifndef __M_ALERT_DIALOG_H__
#define __M_ALERT_DIALOG_H__

typedef struct AlertDialog AlertDialog;

AlertDialog *AlertDialog_Create();
void AlertDialog_Destroy(AlertDialog *dialog);

int AlertDialog_Show(AlertDialog *dialog);
int AlertDialog_Dismiss(AlertDialog *dialog);
int AlertDialog_Open(AlertDialog *dialog);  // 和AlertDialog_Show不同的地方在于没有打开动画，直接打开
int AlertDialog_Close(AlertDialog *dialog); // 和AlertDialog_Dismiss不同的地方在于没有关闭动画，直接关闭

int AlertDialog_OnClickDismiss(AlertDialog *dialog, int which);           // 方便关闭弹窗的onClickListener函数，不做任何事，仅关闭窗口，一般可用于AlertDialog_SetNegativeButton
int AlertDialog_ShowSimpleDialog(const char *title, const char *message); // 方便显示信息的弹窗，仅显示信息，不做任何事

int AlertDialog_SetAutoFree(AlertDialog *dialog, int auto_free);
int AlertDialog_SetData(AlertDialog *dialog, void *data);
int AlertDialog_SetTitle(AlertDialog *dialog, const char *title);
int AlertDialog_SetMessage(AlertDialog *dialog, const char *message); // 暂时不支持同时显示message和items，设置时会清除另外一个
int AlertDialog_SetItems(AlertDialog *dialog, char *const *items, int n_items);
int AlertDialog_SetPositiveButton(AlertDialog *dialog, const char *name, int (*onClickListener)(AlertDialog *dialog, int which));
int AlertDialog_SetNegativeButton(AlertDialog *dialog, const char *name, int (*onClickListener)(AlertDialog *dialog, int which));
int AlertDialog_SetNeutralButton(AlertDialog *dialog, const char *name, int (*onClickListener)(AlertDialog *dialog, int which));
int AlertDialog_SetFreeDataCallback(AlertDialog *dialog, void (*freeData)(void *data)); // 用于关闭窗口时自动清掉userdata的回调函数

void *AlertDialog_GetData(AlertDialog *dialog);

#endif