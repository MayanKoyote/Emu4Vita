#ifndef __M_SETTING_STATES_H__
#define __M_SETTING_STATES_H__

int Setting_InitState();
int Setting_DeinitState();

void Setting_DrawState();
void Setting_CtrlState();

void Setting_SetStateSelectId(int id);
int Setting_GetStateSelectId();

int Setting_GetStatePreviewSize(int *width, int *height);

#endif
