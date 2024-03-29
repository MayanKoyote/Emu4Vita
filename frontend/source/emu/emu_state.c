#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/io/fcntl.h>

#include "activity/activity.h"
#include "setting/setting.h"
#include "gui/gui.h"
#include "emu/emu.h"
#include "utils.h"
#include "file.h"
#include "config.h"

GUI_Texture *Emu_GetStateScreenshotTexture(int num)
{
    char path[MAX_PATH_LENGTH];
    GUI_Texture *tex = NULL;

    MakeSavestatePath(path, num);
    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0)
        return NULL;

    EmuStateHeader header;
    memset(&header, 0, sizeof(EmuStateHeader));
    sceIoRead(fd, &header, sizeof(EmuStateHeader));
    if (header.version != STATES_VERSION)
    {
        sceIoClose(fd);
        return false;
    }
    tex = GUI_CreateTextureFormat(header.screenshot_width, header.screenshot_height, SCEEENSHOT_PIXEL_FORMAT);
    if (tex)
    {
        sceIoLseek(fd, header.screenshot_offset, SCE_SEEK_SET);
        sceIoRead(fd, vita2d_texture_get_datap(tex), header.screenshot_size);
    }
    sceIoClose(fd);

    return tex;
}

GUI_Texture *Emu_GetStatePreviewTexture(int num)
{
    char path[MAX_PATH_LENGTH];
    GUI_Texture *tex = NULL;

    MakeSavestatePath(path, num);
    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0)
        return NULL;

    EmuStateHeader header;
    memset(&header, 0, sizeof(EmuStateHeader));
    sceIoRead(fd, &header, sizeof(EmuStateHeader));
    if (header.version != STATES_VERSION)
    {
        sceIoClose(fd);
        return false;
    }
    tex = GUI_CreateTextureFormat(header.preview_width, header.preview_height, SCEEENSHOT_PIXEL_FORMAT);
    if (tex)
    {
        sceIoLseek(fd, header.preview_offset, SCE_SEEK_SET);
        sceIoRead(fd, vita2d_texture_get_datap(tex), header.preview_size);
    }
    sceIoClose(fd);

    return tex;
}

int Emu_LoadState(int num)
{
    int ret = 0;
    void *state_buf = NULL;
    int fd = -1;
    char path[MAX_PATH_LENGTH];

    // if (!Emu_IsGameLoaded())
    //     return -1;

    MakeSavestatePath(path, num);

    fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0)
    {
        ret = -1;
        goto END;
    }

    EmuStateHeader header;
    memset(&header, 0, sizeof(EmuStateHeader));

    // Read header
    sceIoLseek(fd, 0, SCE_SEEK_SET);
    sceIoRead(fd, &header, sizeof(EmuStateHeader));

    if (header.version != STATES_VERSION)
    {
        APP_LOG("[SAVESTATE] Load state: state version is different from internal version\n");
        ret = -1;
        goto END;
    }

    size_t serialize_size = retro_serialize_size();
    if (header.state_size > serialize_size)
    {
        APP_LOG("[SAVESTATE] Load state: state size is larger than implementation expects\n");
        ret = -1;
        goto END;
    }

    state_buf = malloc(header.state_size);
    if (!state_buf)
    {
        APP_LOG("[SAVESTATE] Load state: alloc state buf failed\n");
        ret = -1;
        goto END;
    }

    char *buf = (char *)state_buf;
    int64_t remaining = header.state_size;
    int64_t transfer = TRANSFER_SIZE;
    sceIoLseek(fd, header.state_offset, SCE_SEEK_SET);

    while (remaining > 0)
    {
        if (remaining < TRANSFER_SIZE)
            transfer = remaining;
        else
            transfer = TRANSFER_SIZE;

        int read = sceIoRead(fd, buf, transfer);
        if (read < 0)
        {
            APP_LOG("[SAVESTATE] Load state: read file failed\n");
            ret = -1;
            goto END;
        }
        if (read == 0)
            break;

        buf += read;
        remaining -= read;
    }

    if (header.disk_index != Emu_DiskGetImageIndex())
        Emu_DiskChangeImageIndex(header.disk_index);

    if (!retro_unserialize(state_buf, header.state_size))
    {
        APP_LOG("[SAVESTATE] Load state: retro_unserialize failed\n");
        ret = -1;
        goto END;
    }

    Emu_CleanAudio();

END:
    if (state_buf)
        free(state_buf);
    if (fd >= 0)
        sceIoClose(fd);

    return ret;
}

int Emu_SaveState(int num)
{
    int ret = 0;
    uint32_t *screenshot_buf = NULL;
    uint32_t *preview_buf = NULL;
    void *state_buf = NULL;
    SceUID fd = -1;
    uint32_t screenshot_width;
    uint32_t screenshot_height;
    uint64_t screenshot_size;
    uint64_t screenshot_offset;
    uint32_t preview_width;
    uint32_t preview_height;
    uint64_t preview_size;
    uint64_t preview_offset;
    uint64_t state_size;
    uint64_t state_offset;
    char path[MAX_PATH_LENGTH];

    // if (!Emu_IsGameLoaded())
    //     return -1;

    MakeSavestateDir(path);
    CreateFolder(path);
    MakeSavestatePath(path, num);

    uint32_t base_width, base_height;
    Emu_GetVideoBaseWH(&base_width, &base_height);

    int rotate = Emu_GetVideoDisplayRotate();
    if (rotate == TYPE_DISPLAY_ROTATE_CW_90 || rotate == TYPE_DISPLAY_ROTATE_CW_270)
    {
        screenshot_width = base_height;
        screenshot_height = base_width;
    }
    else
    {
        screenshot_width = base_width;
        screenshot_height = base_height;
    }

    screenshot_size = 0;
    screenshot_offset = sizeof(EmuStateHeader);
    screenshot_buf = Emu_GetVideoScreenshotData(&screenshot_width, &screenshot_height, &screenshot_size, rotate, 0);
    if (!screenshot_buf)
    {
        APP_LOG("[SAVESTATE] Save state: make screenshot failed!\n");
        ret = -1;
        goto END;
    }

    preview_offset = screenshot_offset + screenshot_size;
    preview_width = 0;
    preview_height = 0;
    Setting_GetStatePreviewSize((int *)&preview_width, (int *)&preview_height);
    preview_size = 0;
    preview_buf = Emu_GetVideoScreenshotData(&preview_width, &preview_height, &preview_size, rotate, 0);
    if (!preview_buf)
    {
        APP_LOG("[SAVESTATE] Save state: make preview failed!\n");
        ret = -1;
        goto END;
    }

    state_offset = preview_offset + preview_size;
    state_size = retro_serialize_size();
    if (state_size == 0)
    {
        APP_LOG("[SAVESTATE] Save state: retro_serialize_size failed!\n");
        ret = -1;
        goto END;
    }
    state_buf = malloc(state_size);
    if (!state_buf)
    {
        APP_LOG("[SAVESTATE] Save state: alloc state buf failed!\n");
        ret = -1;
        goto END;
    }
    if (!retro_serialize(state_buf, state_size))
    {
        APP_LOG("[SAVESTATE] Save state: retro_serialize failed!\n");
        ret = -1;
        goto END;
    }

    fd = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0)
    {
        APP_LOG("[SAVESTATE] Save state: open %s failed!\n", path);
        ret = -1;
        goto END;
    }

    EmuStateHeader header;
    memset(&header, 0, sizeof(EmuStateHeader));
    header.version = STATES_VERSION;
    header.screenshot_offset = screenshot_offset;
    header.screenshot_width = screenshot_width;
    header.screenshot_height = screenshot_height;
    header.screenshot_size = screenshot_size;
    header.preview_offset = preview_offset;
    header.preview_width = preview_width;
    header.preview_height = preview_height;
    header.preview_size = preview_size;
    header.state_offset = state_offset;
    header.state_size = state_size;
    header.disk_index = Emu_DiskGetImageIndex();

    sceIoLseek(fd, 0, SCE_SEEK_SET);
    sceIoWrite(fd, &header, sizeof(EmuStateHeader));

    sceIoLseek(fd, screenshot_offset, SCE_SEEK_SET);
    sceIoWrite(fd, screenshot_buf, screenshot_size);

    sceIoLseek(fd, preview_offset, SCE_SEEK_SET);
    sceIoWrite(fd, preview_buf, preview_size);

    sceIoLseek(fd, state_offset, SCE_SEEK_SET);
    char *buf = (char *)state_buf;
    int64_t remaining = state_size;
    int64_t transfer = TRANSFER_SIZE;

    while (remaining > 0)
    {
        if (remaining < TRANSFER_SIZE)
            transfer = remaining;
        else
            transfer = TRANSFER_SIZE;

        int written = sceIoWrite(fd, buf, transfer);
        if (written < 0)
        {
            APP_LOG("[SAVESTATE] Save state: write file failed\n");
            ret = -1;
            goto END;
        }

        buf += written;
        remaining -= written;
    }

END:
    if (screenshot_buf)
        free(screenshot_buf);
    if (preview_buf)
        free(preview_buf);
    if (state_buf)
        free(state_buf);
    if (fd >= 0)
        sceIoClose(fd);

    return ret;
}

int Emu_DeleteState(int num)
{
    char path[MAX_PATH_LENGTH];
    MakeSavestatePath(path, num);
    sceIoRemove(path);
    return 0;
}