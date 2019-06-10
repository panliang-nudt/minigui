/*
 *   This file is part of MiniGUI, a mature cross-platform windowing
 *   and Graphics User Interface (GUI) support system for embedded systems
 *   and smart IoT devices.
 *
 *   Copyright (C) 2019, Beijing FMSoft Technologies Co., Ltd.
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Or,
 *
 *   As this program is a library, any link to this program must follow
 *   GNU General Public License version 3 (GPLv3). If you cannot accept
 *   GPLv3, you need to be licensed from FMSoft.
 *
 *   If you have got a commercial license of this program, please use it
 *   under the terms and conditions of the commercial license.
 *
 *   For more information about the commercial license, please refer to
 *   <http://www.minigui.com/en/about/licensing-policy/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "newgal.h"
#include "sysvideo.h"
#include "pixels_c.h"

#ifdef _MGGAL_DRM

#include "drmvideo.h"

#define DRMVID_DRIVER_NAME "drm"

/* Initialization/Query functions */
static int DRM_VideoInit(_THIS, GAL_PixelFormat *vformat);
static GAL_Rect **DRM_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags);
static GAL_Surface *DRM_SetVideoMode(_THIS, GAL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int DRM_SetColors(_THIS, int firstcolor, int ncolors, GAL_Color *colors);
static void DRM_VideoQuit(_THIS);

/* Hardware surface functions */
static int DRM_AllocHWSurface(_THIS, GAL_Surface *surface);
static void DRM_FreeHWSurface(_THIS, GAL_Surface *surface);

/* DRM driver bootstrap functions */

static int DRM_Available(void)
{
    return(1);
}

static void DRM_DeleteDevice(GAL_VideoDevice *device)
{
    free(device->hidden);
    free(device);
}

static GAL_VideoDevice *DRM_CreateDevice(int devindex)
{
    GAL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (GAL_VideoDevice *)malloc(sizeof(GAL_VideoDevice));
    if ( device ) {
        memset(device, 0, (sizeof *device));
        device->hidden = (struct GAL_PrivateVideoData *)
                malloc((sizeof *device->hidden));
    }
    if ( (device == NULL) || (device->hidden == NULL) ) {
        GAL_OutOfMemory();
        if ( device ) {
            free(device);
        }
        return(0);
    }
    memset(device->hidden, 0, (sizeof *device->hidden));

    /* Set the function pointers */
    device->VideoInit = DRM_VideoInit;
    device->ListModes = DRM_ListModes;
    device->SetVideoMode = DRM_SetVideoMode;
    device->SetColors = DRM_SetColors;
    device->VideoQuit = DRM_VideoQuit;
#ifndef _MGRM_THREADS
    device->RequestHWSurface = NULL;
#endif
    device->AllocHWSurface = DRM_AllocHWSurface;
    device->CheckHWBlit = NULL;
    device->FillHWRect = NULL;
    device->SetHWColorKey = NULL;
    device->SetHWAlpha = NULL;
    device->FreeHWSurface = DRM_FreeHWSurface;

    device->free = DRM_DeleteDevice;

    return device;
}

VideoBootStrap DRM_bootstrap = {
    DRMVID_DRIVER_NAME, "Linux DRM video driver",
    DRM_Available, DRM_CreateDevice
};


static int DRM_VideoInit(_THIS, GAL_PixelFormat *vformat)
{
    _MG_PRINTF("NEWGAL>DRM: Calling init method!\n");

    /* Determine the screen depth (use default 8-bit depth) */
    /* we change this during the GAL_SetVideoMode implementation... */
    vformat->BitsPerPixel = 8;
    vformat->BytesPerPixel = 1;

    /* We're done! */
    return(0);
}

static GAL_Rect **DRM_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags)
{
    if (format->BitsPerPixel < 8) {
        return NULL;
    }

    return (GAL_Rect**) -1;
}

static GAL_Surface *DRM_SetVideoMode(_THIS, GAL_Surface *current,
                int width, int height, int bpp, Uint32 flags)
{
    int pitch;

    if (this->hidden->buffer) {
        free (this->hidden->buffer);
    }

    pitch = width * ((bpp + 7) / 8);
    pitch = (pitch + 3) & ~3;

    this->hidden->buffer = malloc (pitch * height);
    if (!this->hidden->buffer) {
        _ERR_PRINTF ("NEWGAL>DRM: "
                "Couldn't allocate buffer for requested mode\n");
        return NULL;
    }

    memset (this->hidden->buffer, 0, pitch * height);

    /* Allocate the new pixel format for the screen */
    if (!GAL_ReallocFormat (current, bpp, 0, 0, 0, 0)) {
        free(this->hidden->buffer);
        this->hidden->buffer = NULL;
        _ERR_PRINTF ("NEWGAL>DRM: "
                "Couldn't allocate new pixel format for requested mode\n");
        return(NULL);
    }

    /* Set up the new mode framebuffer */
    current->flags = flags & GAL_FULLSCREEN;
    this->hidden->w = current->w = width;
    this->hidden->h = current->h = height;
    current->pitch = pitch;
    current->pixels = this->hidden->buffer;

    /* We're done */
    return(current);
}

static int DRM_AllocHWSurface(_THIS, GAL_Surface *surface)
{
    return(-1);
}

static void DRM_FreeHWSurface(_THIS, GAL_Surface *surface)
{
    surface->pixels = NULL;
}

static int DRM_SetColors(_THIS, int firstcolor, int ncolors, GAL_Color *colors)
{
    /* do nothing of note. */
    return(1);
}

/* Note:  If we are terminated, this could be called in the middle of
   another video routine -- notably UpdateRects.
*/
static void DRM_VideoQuit(_THIS)
{
    if (this->screen->pixels != NULL) {
        free(this->screen->pixels);
        this->screen->pixels = NULL;
    }
}

#endif /* _MGGAL_DRM */
