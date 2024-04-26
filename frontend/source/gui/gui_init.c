#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/threadmgr.h>
#include "psp2/system_param.h"

#include "gui.h"
#include "file.h"
#include "config.h"
#include "lang.h"
#include "app.h"

static SceUID gui_images_thid = -1;

static GUI_Texture *LoadImageTextureByName(const char *name)
{
    GUI_Texture *texture = NULL;
    char path[MAX_PATH_LENGTH];

    if (private_assets_dir)
    {
        snprintf(path, MAX_PATH_LENGTH, "%s/%s", private_assets_dir, name);
        texture = GUI_LoadPNGFile(path);
    }
    if (!texture && public_assets_dir)
    {
        snprintf(path, MAX_PATH_LENGTH, "%s/%s", public_assets_dir, name);
        texture = GUI_LoadPNGFile(path);
    }

    return texture;
}

static int GuiImagesThreadEntry(SceSize args, void *argp)
{
    GUI_Texture *texture;

    texture = LoadImageTextureByName(WALLPAPER_PNG_NAME);
    GUI_SetImage(ID_GUI_IMAGE_WALLPAPER, texture);
    texture = LoadImageTextureByName(SPLASH_PNG_NAME);
    GUI_SetImage(ID_GUI_IMAGE_SPLASH, texture);
    texture = LoadImageTextureByName(CHECKBOX_ON_PNG_NAME);
    GUI_SetImage(ID_GUI_IMAGE_CHECKBOX_ON, texture);
    texture = LoadImageTextureByName(CHECKBOX_OFF_PNG_NAME);
    GUI_SetImage(ID_GUI_IMAGE_CHECKBOX_OFF, texture);
    texture = LoadImageTextureByName(RADIOBUTTON_ON_PNG_NAME);
    GUI_SetImage(ID_GUI_IMAGE_RADIOBUTTON_ON, texture);
    texture = LoadImageTextureByName(RADIOBUTTON_OFF_PNG_NAME);
    GUI_SetImage(ID_GUI_IMAGE_RADIOBUTTON_OFF, texture);

    gui_images_thid = -1;
    sceKernelExitDeleteThread(0);
    return 0;
}

static int GUI_InitImages()
{
    int ret = gui_images_thid = sceKernelCreateThread("gui_images_thread", GuiImagesThreadEntry, 0x10000100, SCE_KERNEL_CPU_MASK_USER_ALL, 0, 0, NULL);
    if (gui_images_thid >= 0)
        ret = sceKernelStartThread(gui_images_thid, 0, NULL);
    return ret;
}

static void GUI_WaitImagesThreadEnd()
{
    if (gui_images_thid >= 0)
    {
        sceKernelWaitThreadEnd(gui_images_thid, NULL, NULL);
        sceKernelDeleteThread(gui_images_thid);
        gui_images_thid = -1;
    }
}

static void GUI_DeinitImages()
{
    GUI_WaitImagesThreadEnd();

    GUI_SetImage(ID_GUI_IMAGE_WALLPAPER, NULL);
    GUI_SetImage(ID_GUI_IMAGE_SPLASH, NULL);
    GUI_SetImage(ID_GUI_IMAGE_CHECKBOX_ON, NULL);
    GUI_SetImage(ID_GUI_IMAGE_CHECKBOX_OFF, NULL);
    GUI_SetImage(ID_GUI_IMAGE_RADIOBUTTON_ON, NULL);
    GUI_SetImage(ID_GUI_IMAGE_RADIOBUTTON_OFF, NULL);
}

void GUI_WaitInitEnd()
{
    GUI_WaitImagesThreadEnd();
}

void GUI_Init()
{
    GUI_InitLib();
    GUI_InitImages();
    GUI_InitShaders();
    GUI_InitFont();
    GUI_InitDraw();
    GUI_InitActivity();
    GUI_InitWindow();
    GUI_InitToast();
}

void GUI_Deinit()
{
    GUI_DeinitActivity();
    GUI_DeinitWindow();
    GUI_DeinitToast();
    GUI_DeinitDraw();
    GUI_DeinitImages();
    GUI_DeinitShaders();
    GUI_DeinitFont();
    GUI_DeinitLib();
}
