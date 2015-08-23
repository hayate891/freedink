#ifndef IOGFXSURFACESW_H
#define IOGFXSURFACESW_H

#include "IOGfxSurface.h"
#include "SDL.h"

class IOGfxSurfaceSW: public IOGfxSurface {
public:
	SDL_Surface* surf;
	IOGfxSurfaceSW(SDL_Surface* s);
	virtual ~IOGfxSurfaceSW();
	virtual void fill(int num, SDL_Color* palette);
	virtual void vlineRGB(Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b);
	virtual void hlineRGB(Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b);
	virtual void drawBox(rect box, int color);
	virtual int blit(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect);
	virtual int blitStretch(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect);
	virtual int blitNoColorKey(IOGfxSurface *src, const SDL_Rect *srcrect, SDL_Rect *dstrect);
	virtual unsigned int getMemUsage();
};

#endif
