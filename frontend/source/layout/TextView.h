#ifndef __M_TEXT_VIEW_H__
#define __M_TEXT_VIEW_H__

#include "Layout.h"

typedef struct TextView TextView;

int TextViewSetBgColor(TextView *textView, uint32_t color);
int TextViewSetText(TextView *textView, const char *text);
int TextViewSetTextColor(TextView *textView, uint32_t color);
int TextViewSetSingleLine(TextView *textView, int single_line);
int TextViewSetTextScollEnabled(TextView *textView, int enabled);

const char *TextViewGetText(TextView *textView);

int TextViewInit(TextView *textView);
TextView *NewTextView();

#endif
