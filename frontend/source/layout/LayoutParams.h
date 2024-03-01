#ifndef __M_LAYOUT_PARAMS_H__
#define __M_LAYOUT_PARAMS_H__

//-----------------------------------------------------------------------------------
// 通用的布局参数，控件结构体第一个参数必须为LayoutParams以供LayoutParams强制转换使用
//-----------------------------------------------------------------------------------

#define MAX_AVAILABLE_WIDTH 4096
#define MAX_AVAILABLE_HEIGHT 4096

enum TypeLayoutSize
{
    TYPE_LAYOUT_WRAP_CONTENT = -0x01,
    TYPE_LAYOUT_MATH_PARENT = -0x02,
};

enum TypeLayoutOrientation
{
    TYPE_LAYOUT_ORIENTATION_FRAME,
    TYPE_LAYOUT_ORIENTATION_HORIZONTAL,
    TYPE_LAYOUT_ORIENTATION_VERTICAL,
};

typedef struct
{
    int dont_free;
    int orientation;
    int scroll_enabled;

    int available_w;
    int available_h;
    int measured_w;
    int measured_h;
    int wrap_w;
    int wrap_h;
    int layout_x;
    int layout_y;
    int layout_w;
    int layout_h;
    int padding_left;
    int padding_right;
    int padding_top;
    int padding_bottom;
    int margin_left;
    int margin_right;
    int margin_top;
    int margin_bottom;

    void (*destroy)(void *view);
    int (*update)(void *view);
    void (*draw)(void *view);
} LayoutParams;

LayoutParams *NewLayoutParams();
LayoutParams *LayoutParamsGetParams(void *view);

// 设置参数
int LayoutParamsSetAutoFree(void *view, int auto_free);
int LayoutParamsSetOrientation(void *view, int orientation);
int LayoutParamsSetScrollEnabled(void *view, int enabled);
int LayoutParamsSetMargin(void *view, int left, int right, int top, int bottom);
int LayoutParamsSetPadding(void *view, int left, int right, int top, int bottom);
int LayoutParamsSetLayoutPosition(void *view, int layout_x, int layout_y);
int LayoutParamsSetAvailableSize(void *view, int available_w, int available_h);
int LayoutParamsSetLayoutSize(void *view, int layout_w, int layout_h);

// 获取参数
int LayoutParamsGetLayoutPosition(void *view, int *layout_x, int *layout_y);
int LayoutParamsGetAvailableSize(void *view, int *available_w, int *available_h);
int LayoutParamsGetLayoutSize(void *view, int *layout_w, int *layout_h);

// 调用Update后才能获取正确的值
int LayoutParamsGetMeasuredSize(void *view, int *measured_w, int *measured_h);
int LayoutParamsGetWrapSize(void *view, int *wrap_w, int *wrap_h);

void LayoutParamsDestroy(void *view);
int LayoutParamsUpdate(void *view);
void LayoutParamsDraw(void *view);

#endif
