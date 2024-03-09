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
#include "init.h"

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

static int ImagesThreadFunc(SceSize args, void *argp)
{
    GUI_SetDefaultWallpaper(LoadImageTextureByName(WALLPAPER_PNG_NAME));
    GUI_SetDefaultSplash(LoadImageTextureByName(SPLASH_PNG_NAME));

    GUI_SetCheckBoxTexture(LoadImageTextureByName(CHECKBOX_ON_PNG_NAME), LoadImageTextureByName(CHECKBOX_OFF_PNG_NAME));
    GUI_SetRadioButtonTexture(LoadImageTextureByName(RADIOBUTTON_ON_PNG_NAME), LoadImageTextureByName(RADIOBUTTON_OFF_PNG_NAME));

    sceKernelExitDeleteThread(0);
    return 0;
}

static void GUI_InitImages()
{
    gui_images_thid = sceKernelCreateThread("gui_images_thread", ImagesThreadFunc, 0x10000100, 0x10000, 0, 0, NULL);
    if (gui_images_thid >= 0)
        sceKernelStartThread(gui_images_thid, 0, NULL);
}

static void GUI_DeinitImages()
{
    if (gui_images_thid >= 0)
    {
        sceKernelWaitThreadEnd(gui_images_thid, NULL, NULL);
        sceKernelDeleteThread(gui_images_thid);
        gui_images_thid = -1;
    }
    GUI_SetDefaultWallpaper(NULL);
    GUI_SetDefaultSplash(NULL);
}

void GUI_WaitInitEnd()
{
    if (gui_images_thid >= 0)
    {
        sceKernelWaitThreadEnd(gui_images_thid, NULL, NULL);
        sceKernelDeleteThread(gui_images_thid);
        gui_images_thid = -1;
    }
}

void GUI_Init()
{
    GUI_InitLib();
    GUI_InitImages();
    GUI_InitShaders();
    GUI_InitFont();
}

void GUI_Deinit()
{
    GUI_DeinitImages();
    GUI_DeinitShaders();
    GUI_DeinitFont();
    GUI_DeinitLib();
}
