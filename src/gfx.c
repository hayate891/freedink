/**
 * Graphics

 * Copyright (C) 2007, 2008  Sylvain Beucler

 * This file is part of GNU FreeDink

 * GNU FreeDink is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.

 * GNU FreeDink is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"
#include "SDL_image.h"

#include <string.h>

#include "freedink_xpm.h"
#include "io_util.h"
#include "gfx.h"
#include "gfx_fonts.h"
#include "gfx_tiles.h"
#include "gfx_sprites.h"
#include "gfx_utils.h"
#include "init.h"

// // DELETEME
// LPDIRECTDRAW            lpDD = NULL;           // DirectDraw object
// //LPDIxRECTDRAWSURFACE     lpDDSOne;       // Offscreen surface 1

// LPDIRECTDRAWSURFACE     lpDDSPrimary = NULL;   // DirectDraw primary surface
// LPDIRECTDRAWSURFACE     lpDDSBack = NULL;      // DirectDraw back surface

// LPDIRECTDRAWSURFACE     lpDDSTwo = NULL;       // Offscreen surface 2
// LPDIRECTDRAWSURFACE     lpDDSTrick = NULL;       // Offscreen surface 2
// LPDIRECTDRAWSURFACE     lpDDSTrick2 = NULL;       // Offscreen surface 2


SDL_Surface *GFX_lpDDSBack = NULL; /* Backbuffer and link to physical
				      screen*/

/* GFX_lpDDSTwo: holds the base scene */
/* Rationale attempt :*/
/* lpDDSTwo contains the background, which is reused for each new
   frame. It is overwritten when switching to another screen. However,
   it can change during a screen: 1) animated tiles (water & fire) 2)
   when a sprite is written on the background (eg when an enemy dies)
   3) with various hacks such as fill_screen() (and maybe
   copy_bmp_to_screen()). */
/* Those changes may conflict with each other (eg: an animated tile
   overwrites half the carcass of a dead enemy). I think that some
   alternate versions of Dink considered those conflicts as bugs (not
   features) and fixed them. Maybe we should do so as well. */
/* After the background is done, all the other operations are applied
   on lpDDSBack, the double buffer which is directly used by the
   physical screen. */
SDL_Surface *GFX_lpDDSTwo = NULL;

/* Beuc: apparently used for the scrolling screen transition and more
   generaly as temporary buffers. Only used by the game, not the
   editor. */
/* Used in freedink.cpp only + as a local/independent temporary buffer
   in show_bmp&copy_bmp&process_show_bmp&load_sprite* */
SDL_Surface *GFX_lpDDSTrick = NULL;
/* Used in freedink.cpp and update_frame.cpp */
SDL_Surface *GFX_lpDDSTrick2 = NULL;


/* Reference palette: this is the canonical Dink palette, loaded from
   TS01.bmp (for freedink) and esplash.bmp (for freedinkedit). The
   physical screen may be changed (e.g. show_bmp()), but this
   canonical palette will stay constant. */
/* PALETTEENTRY  real_pal[256]; */
SDL_Color GFX_real_pal[256];

/** Game-specific **/
/* Palette change: with SDL, SDL_SetColors (aka
   SDL_SetPalette(SDL_PHYSPAL)) apparently triggers a Flip, which
   displays weird colors on the screen for a brief but displeasing
   moment. Besides, SDL_Flip() does not refresh the hardware palette,
   so update the physical palette needs to be done manually - but only
   when the surface is already in its final form. The palette may need
   to be changed before the screen content is ready, so we'll make the
   engine know when he needs to refresh the physical palette: */
/* Tell flip_it* to install the new palette */
int trigger_palette_change = 0;
SDL_Color cur_screen_palette[256];


/**
 * Check if the graphics system is initialized, so we know if we can
 * use it to display error messages to the user
 */
static enum gfx_init_state init_state = GFX_NOT_INITIALIZED;
enum gfx_init_state gfx_get_init_state()
{
  return init_state;
}


/**
 * Graphics subsystem initalization
 */
int gfx_init(enum gfx_windowed_state windowed)
{
  /* Initialization in progress */
  init_state = GFX_INITIALIZING_VIDEO;

  /* Init graphics subsystem */
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
    {
      init_set_error_msg("Video initialization error: %s", SDL_GetError());
      return -1;
    }

  {
    SDL_Surface *icon = NULL;
    SDL_WM_SetCaption(PACKAGE_STRING, NULL);

    if ((icon = IMG_ReadXPMFromArray(freedink_xpm)) == NULL)
      {
	fprintf(stderr, "Error loading icon: %s\n", IMG_GetError());
      }
    else
      {
	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
      }
  }

  SDL_putenv("SDL_VIDEO_CENTERED=1");

  /* SDL_HWSURFACE is supposed to give direct memory access */
  /* SDL_HWPALETTE makes sure we can use all the colors we need
     (override system palette reserved colors?) */
  /* SDL_DOUBLEBUF is supposed to enable hardware double-buffering
     and is a pre-requisite for SDL_Flip to use hardware, see
     http://www.libsdl.org/cgi/docwiki.cgi/FAQ_20Hardware_20Surfaces_20Flickering */
  if (windowed == GFX_WINDOWED)
    GFX_lpDDSBack = SDL_SetVideoMode(640, 480, 8, SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF);
  else
    GFX_lpDDSBack = SDL_SetVideoMode(640, 480, 8, SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
  if (GFX_lpDDSBack == NULL)
    {
      init_set_error_msg("Unable to set 640x480 video: %s\n", SDL_GetError());
      return -1;
    }
  if (GFX_lpDDSBack->flags & SDL_HWSURFACE)
    printf("INFO: Using hardware video mode.\n");
  else
    printf("INFO: Not using a hardware video mode.\n");

  // GFX
  /* GFX_lpDDSBack = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 8, */
  /* 				       0, 0, 0, 0); */

  // lpDDSTwo/Trick/Trick2 are initialized by loading SPLASH.BMP in
  // doInit()

  /* Hide mouse */
  SDL_ShowCursor(SDL_DISABLE);

  /* Disable Alt-Tab and any other window-manager shortcuts */
  /* SDL_WM_GrabInput(SDL_GRAB_ON); */


  /* Default palette (may be used by early init error messages */
  setup_palette(cur_screen_palette);
  setup_palette(GFX_real_pal);
  SDL_SetPalette(GFX_lpDDSBack, SDL_LOGPAL|SDL_PHYSPAL, cur_screen_palette, 0, 256);


  /* Fonts system, default fonts */
  init_state = GFX_INITIALIZING_FONTS;
  if (gfx_fonts_init() < 0)
    return -1; /* error message set in gfx_fonts_init */


  /* Mouse */
  /* Center mouse and reset relative positionning */
  SDL_WarpMouse(320, 240);
  SDL_PumpEvents();
  SDL_GetRelativeMouseState(NULL, NULL);


  /* We'll handle those events manually */
  SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
  SDL_EventState(SDL_VIDEOEXPOSE, SDL_IGNORE);
  SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);
  SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
  SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
  SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
  SDL_EventState(SDL_KEYUP, SDL_IGNORE);
  SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
  SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
  /* We still process through a SDL_PollEvent() loop: */
  /* - SDL_QUIT: quit on window close and Ctrl+C */
  /* - SDL_MOUSEBUTTONDOWN: don't miss quick clicks */
  /* - Joystick: apparently we need to keep them, otherwise joystick
       doesn't work at all */

  /* SDL_MouseMotionEvent: If the cursor is hidden (SDL_ShowCursor(0))
     and the input is grabbed (SDL_WM_GrabInput(SDL_GRAB_ON)), then
     the mouse will give relative motion events even when the cursor
     reaches the edge of the screen. This is currently only
     implemented on Windows and Linux/Unix-alikes. */
  /* So it's not portable and it blocks Alt+Tab, so let's try
     something else - maybe enable it as a command line option. */
  /* SDL_WM_GrabInput(SDL_GRAB_ON); */
  
  init_state = GFX_INITIALIZED;
  return 0;
}

/**
 * Failsafe graphics mode to display initialization error messages
 */
int gfx_init_failsafe()
{
  /* Init graphics subsystem */
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
    return -1;

  SDL_putenv("SDL_VIDEO_CENTERED=1");
  SDL_WM_SetCaption(PACKAGE_STRING " - Initialization error", NULL);
  SDL_Surface *icon = IMG_ReadXPMFromArray(freedink_xpm);
  if (icon != NULL)
    {
      SDL_WM_SetIcon(icon, NULL);
      SDL_FreeSurface(icon);
    }

  GFX_lpDDSBack = SDL_SetVideoMode(640, 480, 8, SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF);
  if (GFX_lpDDSBack == NULL)
    return -1;

  setup_palette(cur_screen_palette);
  setup_palette(GFX_real_pal);
  SDL_SetPalette(GFX_lpDDSBack, SDL_LOGPAL|SDL_PHYSPAL, cur_screen_palette, 0, 256);

  return gfx_fonts_init_failsafe();
}

/**
 * Unload graphics subsystem
 */
void gfx_quit()
{
  init_state = GFX_QUITTING;

  gfx_fonts_quit();

  tiles_unload();
  sprites_unload();
  
  if (GFX_lpDDSBack   != NULL) SDL_FreeSurface(GFX_lpDDSBack);
  if (GFX_lpDDSTwo    != NULL) SDL_FreeSurface(GFX_lpDDSTwo);
  if (GFX_lpDDSTrick  != NULL) SDL_FreeSurface(GFX_lpDDSTrick);
  if (GFX_lpDDSTrick2 != NULL) SDL_FreeSurface(GFX_lpDDSTrick2);

  init_state = GFX_NOT_INITIALIZED;
}


/* Schedule a change to the physical screen's palette for the next
   frame */
void change_screen_palette(SDL_Color* new_palette)
{
  /* Now this one is tricky: DX/Woe has a "feature" where palette
     indexes 0 and 255 are fixed to black and white,
     respectively. This is the opposite of the default Dink palette
     - which is why fill_screen(0) is black and not white as in the
     Dink palette. This also makes "Lyna's Story"'s palette change a
     bit ugly, because pure black and white colors are not reversed
     when you enter "negative" color mode. This does not affect
     other indexes. Technically this happens when you get a palette
     from GetEntries(), and when you CreatePalette() without
     specifying DDPCAPS_ALLOW256 (so respectively, in
     change_screen_palette() and load_palette_from_*()). But well,
     reproducing the bug is important for backward compatibility. */
  
  memcpy(cur_screen_palette, new_palette, sizeof(cur_screen_palette));
  
  cur_screen_palette[0].r = 0;
  cur_screen_palette[0].g = 0;
  cur_screen_palette[0].b = 0;
  cur_screen_palette[255].r = 255;
  cur_screen_palette[255].g = 255;
  cur_screen_palette[255].b = 255;
  
  /* Applying the logical palette to the physical screen may trigger
     a Flip, so don't do it right now */
  trigger_palette_change = 1;
}


/* LoadBMP wrapper. Load a new graphic from file, and apply the
   reference palette so that all subsequent blits are faster (color
   convertion is avoided) - although the initial loading time will be
   somewhat longer. */
static SDL_Surface* load_bmp_internal(char *filename, SDL_RWops *rw, int from_mem, int set_pal) {
  SDL_Surface *image, *copy;

  if (from_mem == 1)
    {
      image = IMG_Load_RW(rw, 1);
    }
  else
    {
      ciconvert(filename);
      image = IMG_Load(filename);
    }

  if (image == NULL)
    {
      /* fprintf(stderr, "load_bmp_internal: %s\n", SDL_GetError()); */
      /* Maybe it's just because we're at the end of a sequence */
      return NULL;
    }

  /* Make an identical copy of the surface */
  /* We use SDL_ConvertSurface(); SDL_DisplayFormat() would be simpler
     but applies a different palette (the screen's) */
  copy = SDL_ConvertSurface(image, image->format, image->flags);

  if (set_pal == 1)
    {
      SDL_Color palette[256];
      load_palette_from_surface(image, palette);
      change_screen_palette(palette);
      /* Pretend that the image uses the current screen and buffers
	 palette, to avoid color conversion to the reference palette
	 (maintain palette indexes). We maintain palette indexes so
	 that they will match the physical screen's palette, which we
	 just change. */
      /* Note: cur_screen_palette is not exactly the same as palette,
	 because DX reserves some indexes, and FreeDink reimplement
	 this limitation for compatibility. So we still need a blit
	 with color convertion to take reserved indexes into
	 account. Typically skipping this step will reverse black and
	 white (with Dink palette indexes: 255 and 0; with DX reserved
	 indexes: 0 and 255). */
      SDL_SetPalette(image, SDL_LOGPAL, cur_screen_palette, 0, 256);
    }
  else
    {
      /* Prepare a color conversion to the reference palette */
      SDL_SetPalette(image, SDL_LOGPAL, GFX_real_pal, 0, 256);
    }

  /* Blit the copy back to the original, with a potentially different
     palette, which triggers color conversion to image's palette. */
  SDL_BlitSurface(copy, NULL, image, NULL);
  SDL_FreeSurface(copy);
  
  /* In the end, the image must use the reference palette: that way no
     mistaken color conversion will occur during blits to other
     surfaces/buffers. Blits should also be faster(?). */
  SDL_SetPalette(image, SDL_LOGPAL, GFX_real_pal, 0, 256);

  return image;
}

/* LoadBMP wrapper, from file */
SDL_Surface* load_bmp(char *filename)
{
  return load_bmp_internal(filename, NULL, 0, 0);
}

/* LoadBMP wrapper, from FILE pointer */
SDL_Surface* load_bmp_from_fp(FILE* in)
{
  if (in == NULL)
    return NULL;
  SDL_RWops *rw = SDL_RWFromFP(in, /*autoclose=*/1);
  return load_bmp_internal(NULL, rw, 1, 0);
}

/* LoadBMP wrapper, from memory */
SDL_Surface* load_bmp_from_mem(SDL_RWops *rw)
{
  return load_bmp_internal(NULL, rw, 1, 0);
}

/* LoadBMP wrapper + use as current palette */
SDL_Surface* load_bmp_setpal(FILE* in)
{
  if (in == NULL)
    return NULL;
  SDL_RWops *rw = SDL_RWFromFP(in, /*autoclose=*/1);
  return load_bmp_internal(NULL, rw, 1, 1);
}
