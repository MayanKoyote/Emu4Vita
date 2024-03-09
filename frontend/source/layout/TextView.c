#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gui/gui.h"
#include "Layout.h"
#include "utils_string.h"

struct TextView
{
    LayoutParams params;
    uint32_t bg_color;
    char *text_ori;
    char *text_buf;
    int text_ori_w;
    int text_ori_h;
    int text_x;
    int text_y;
    uint32_t text_color;
    int single_line;
    int text_scoll_enabled;
    int text_scoll_count;
    int text_x_off;
    void *userdata;
};

static void TextViewDestroy(void *view)
{
    TextView *textView = (TextView *)view;
    LayoutParams *params = LayoutParamsGetParams(textView);

    if (!textView || params->dont_free)
        return;

    if (textView->text_ori)
        free(textView->text_ori);
    if (textView->text_buf)
        free(textView->text_buf);
    free(textView);
}

static int TextViewUpdate(void *view)
{
    if (!view)
        return -1;

    TextView *textView = (TextView *)view;
    LayoutParams *params = LayoutParamsGetParams(textView);

    if (params->available_w <= 0 || params->available_h <= 0)
    {
        params->measured_w = 0;
        params->measured_h = 0;
        return -1;
    }

    int view_max_w = params->available_w - params->margin_left - params->margin_right;
    int view_max_h = params->available_h - params->margin_top - params->margin_bottom;
    int view_wrap_w = textView->text_ori_w + params->padding_left + params->padding_right;
    int view_wrap_h = textView->text_ori_h + params->padding_top + params->padding_bottom;

    if (textView->text_buf)
        free(textView->text_buf);
    textView->text_buf = NULL;

    if (textView->text_ori)
    {
        int text_max_w = view_max_w - params->padding_left - params->padding_right;

        if (textView->text_ori_w > text_max_w)
        {
            if (textView->single_line)
                textView->text_buf = StringMakeShortByWidth(textView->text_ori, text_max_w);
            else
                textView->text_buf = StringBreakLineByWidth(textView->text_ori, text_max_w);
            view_wrap_h = GUI_GetTextHeight(textView->text_buf) + params->padding_top + params->padding_bottom;
        }
    }

    params->wrap_w = view_wrap_w;
    params->wrap_h = view_wrap_h;

    if (params->layout_w == TYPE_LAYOUT_PARAMS_MATH_PARENT)
        params->measured_w = view_max_w;
    else if (params->layout_w == TYPE_LAYOUT_PARAMS_WRAP_CONTENT)
        params->measured_w = view_wrap_w;
    else
        params->measured_w = params->layout_w;
    if (params->measured_w > view_max_w)
        params->measured_w = view_max_w;
    if (params->measured_w < 0)
        params->measured_w = 0;

    if (params->layout_h == TYPE_LAYOUT_PARAMS_MATH_PARENT)
        params->measured_h = view_max_h;
    else if (params->layout_h == TYPE_LAYOUT_PARAMS_WRAP_CONTENT)
        params->measured_h = view_wrap_h;
    else
        params->measured_h = params->layout_h;
    if (params->measured_h > view_max_h)
        params->measured_h = view_max_h;
    if (params->measured_h < 0)
        params->measured_h = 0;

    return 0;
}

static int TextViewDraw(void *view)
{
    if (!view)
        return -1;

    TextView *textView = (TextView *)view;
    LayoutParams *params = LayoutParamsGetParams(textView);

    if (params->measured_w <= 0 || params->measured_h <= 0)
        return 0;

    int view_x = params->layout_x + params->margin_left;
    int view_y = params->layout_y + params->margin_top;
    int view_max_w = params->measured_w;
    int view_max_h = params->measured_h;

    if (textView->bg_color)
        GUI_DrawFillRectangle(view_x, view_y, view_max_w, view_max_h, textView->bg_color);

    if (textView->text_ori)
    {
        int text_x = view_x + params->padding_left + textView->text_x;
        int text_y = view_y + params->padding_top + textView->text_y;
        int text_max_w = view_max_w - params->padding_left - params->padding_right;
        int text_max_h = view_max_h - params->padding_top; // - params->padding_bottom;

        if (params->wrap_w < params->measured_w)
        {
            if (params->gravity & TYPE_LAYOUT_PARAMS_GRAVITY_RIGHT)
                text_x += (params->measured_w - params->wrap_w);
        }
        if (params->wrap_h < params->measured_h)
        {
            if (params->gravity & TYPE_LAYOUT_PARAMS_GRAVITY_BOTTOM)
                text_y += (params->measured_h - params->wrap_h);
        }

        const char *text = textView->text_buf;
        uint32_t color = textView->text_color;

        GUI_SetClipping(text_x, text_y, text_max_w, text_max_h);

        if (textView->text_scoll_enabled && textView->text_ori_w > text_max_w)
        {
            text = textView->text_ori;

            if (textView->text_scoll_count < 60)
            {
                textView->text_x_off = 0;
            }
            else if (textView->text_scoll_count < textView->text_ori_w + 60)
            {
                textView->text_x_off--;
            }
            else if (textView->text_scoll_count < textView->text_ori_w + 90)
            {
                color = (color & 0x00FFFFFF) | ((((color >> 24) * (textView->text_scoll_count - textView->text_ori_w - 60)) / 30) << 24); // fade-in in 0.5s
                textView->text_x_off = 0;
            }
            else
            {
                textView->text_scoll_count = 0;
            }

            textView->text_scoll_count++;
            text_x += textView->text_x_off;
        }
        else
        {
            textView->text_scoll_count = 0;
            textView->text_x_off = 0;
        }

        if (!text)
            text = textView->text_ori;
        if (text)
            GUI_DrawText(text_x, text_y, color, text);

        GUI_UnsetClipping();
    }

    return 0;
}

int TextViewSetBgColor(TextView *textView, uint32_t color)
{
    if (!textView)
        return -1;

    textView->bg_color = color;
    return 0;
}

int TextViewSetData(TextView *textView, void *data)
{
    if (!textView)
        return -1;

    textView->userdata = data;
    return 0;
}

int TextViewSetText(TextView *textView, const char *text)
{
    if (!textView)
        return -1;

    if (textView->text_ori)
        free(textView->text_ori);
    textView->text_ori = NULL;
    if (textView->text_buf)
        free(textView->text_buf);
    textView->text_buf = NULL;
    textView->text_ori_w = 0;
    textView->text_ori_h = 0;

    if (text)
    {
        textView->text_ori = malloc(strlen(text) + 1);
        if (textView->text_ori)
        {
            strcpy(textView->text_ori, text);
            textView->text_ori_w = GUI_GetTextWidth(textView->text_ori);
            textView->text_ori_h = GUI_GetTextHeight(textView->text_ori);
            if (textView->text_ori_h <= 0)
                textView->text_ori_h = GUI_GetLineHeight();
        }
    }

    return 0;
}

int TextViewSetTextColor(TextView *textView, uint32_t color)
{
    if (!textView)
        return -1;

    textView->text_color = color;

    return 0;
}

int TextViewSetSingleLine(TextView *textView, int single_line)
{
    if (!textView)
        return -1;

    textView->single_line = single_line;

    return 0;
}

int TextViewSetTextScollEnabled(TextView *textView, int enabled)
{
    if (!textView)
        return -1;

    textView->text_scoll_enabled = enabled;

    return 0;
}

void *TextViewGetData(TextView *textView)
{
    return textView ? textView->userdata : NULL;
}

const char *TextViewGetText(TextView *textView)
{
    return textView ? textView->text_ori : NULL;
}

TextView *NewTextView()
{
    TextView *textView = (TextView *)malloc(sizeof(TextView));
    if (!textView)
        return NULL;
    memset(textView, 0, sizeof(TextView));

    LayoutParams *params = LayoutParamsGetParams(textView);
    params->destroy = TextViewDestroy;
    params->update = TextViewUpdate;
    params->draw = TextViewDraw;

    return textView;
}
