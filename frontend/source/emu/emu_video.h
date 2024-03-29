#ifndef __M_EMU_VIDEO_H__
#define __M_EMU_VIDEO_H__

#include <stdint.h>
#include "gui/gui.h"

#define SCEEENSHOT_PIXEL_FORMAT GUI_PIXEL_FORMAT_U8U8U8U8_ABGR

void Emu_PauseVideo();
void Emu_ResumeVideo();

void Emu_RequestUpdateVideoDisplay();
void Emu_ShowControllerPortToast();

void Emu_BeforeDrawVideo();
void Emu_DrawVideo();
void Emu_DrawVideoWidgets();

int Emu_InitVideo();
int Emu_DeinitVideo();

GUI_Texture *Emu_GetCurrentVideoTexture();
GUI_Texture *Emu_GetNextVideoTexture();
int Emu_GetVideoDisplayRotate();
void Emu_GetVideoBaseWH(uint32_t *width, uint32_t *height);
void Emu_GetVideoDisplayWH(uint32_t *width, uint32_t *height);

uint32_t *Emu_GetVideoScreenshotData(uint32_t *width, uint32_t *height, uint64_t *size, int rotate, int use_shader);
int Emu_SaveVideoScreenshot(char *path);

int Emu_SignalVideoSema();
int Emu_WaitVideoSema();

void Emu_SetMicrosPerFrame(uint64_t micros);
uint64_t Emu_GetMicrosPerFrame();

#endif