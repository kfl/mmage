
/* SDL stuff */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


/* Mosml stuff */
#include <mosml/mlvalues.h>
#include <mosml/fail.h>
#include <mosml/alloc.h>
#include <mosml/memory.h>
#include <mosml/str.h>

#ifdef WIN32
#define EXTERNML __declspec(dllexport)
#define INLINE
#else
#define EXTERNML
#define INLINE inline
#endif

#define Surface_val(x) ((SDL_Surface *) Field(x, 1))
#define Surface_val_lv(x) (Field(x, 1))

static INLINE value mmage_alloc_final(mlsize_t len, final_fun fun) {
  return alloc_final(len, fun, 0, 1);
}

static void mmage_surface_finalize(value obj) {
  SDL_FreeSurface(Surface_val(obj));
}


static INLINE value mmage_make_surface(SDL_Surface *surface) {
  value res;
  res = mmage_alloc_final(2, &mmage_surface_finalize);
  Surface_val_lv(res) = surface;
  return res;
}

EXTERNML value mmage_get_width(value surface) {
  return Val_long(Surface_val(surface)->w);
}


EXTERNML value mmage_get_height(value surface) {
  return Val_long(Surface_val(surface)->h);
}

EXTERNML value mmage_get_pitch(value surface) {
  return Val_long(Surface_val(surface)->pitch);
}

EXTERNML value mmage_get_bytes_per_pixel(value surface) {
  return Val_long(Surface_val(surface)->format->BytesPerPixel);
}


EXTERNML value mmage_copy_surface(value surface) {
  SDL_Surface *srfc1 = Surface_val(surface);
  SDL_Surface *srfc2 = SDL_ConvertSurface(srfc1, srfc1->format, srfc1->flags);
  return mmage_make_surface(srfc2);
}

EXTERNML value mmage_surface_lock(value surface) {
  SDL_LockSurface(Surface_val(surface));
  return Val_unit;
}

EXTERNML value mmage_surface_unlock(value surface) {
  SDL_UnlockSurface(Surface_val(surface));
  return Val_unit;
}


EXTERNML value mmage_get_rgb(value surface, value idx) {
  // Assumes that surface is locked
  SDL_Surface *surf = Surface_val(surface);

  /* uint32_t pixel = *( (uint8_t *)surf->pixels + Long_val(y) * surf->pitch  */
  /*                                             + Long_val(x) * surf->format->BytesPerPixel) ; */
  uint32_t pixel = *(uint32_t *)( (uint8_t *)surf->pixels + Long_val(idx));

  uint8_t r;
  uint8_t g;
  uint8_t b;

  SDL_GetRGB( pixel, surf->format,  &r, &g, &b );
  //printf("%d, read %d, %d, %d\n", Long_val(idx), r,g,b);


  value res = alloc_tuple(3);
  Field(res, 0) = Val_long(r);
  Field(res, 1) = Val_long(g);
  Field(res, 2) = Val_long(b);
  return res;
}

EXTERNML value mmage_set_rgb(value surface, value idx, value r, value g, value b) {
  // Assumes that surface is locked
  SDL_Surface *surf = Surface_val(surface);

  // In an ideal world we could just write this
  /* ((uint8_t *)surf->pixels)[ Long_val(idx) ] = SDL_MapRGB( surf->format, */
  /*                                                          Long_val(r), */
  /*                                                          Long_val(g), */
  /*                                                          Long_val(b) ); */

  uint32_t pixel = SDL_MapRGB( surf->format,
                               Long_val(r),
                               Long_val(g),
                               Long_val(b) );
  uint8_t *p = ((uint8_t *)surf->pixels) + Long_val(idx);

  switch(surf->format->BytesPerPixel) {
  case 1:
    *p = (uint8_t) pixel;
    break;
  case 2:
    *p = (uint16_t) pixel;
    break;
  case 3:
    /* if(SDL_BYTEORDER == SDL_BIG_ENDIAN) { */
    /*   printf("BIG\n"); */
      p[2] = (uint8_t) pixel & 0x0000ff;
      p[1] = (uint8_t) (pixel & 0x00ff00) >> 8;
      p[0] = (uint8_t) (pixel & 0x0000ff) >> 16;
    /* } else { */
    /*   p[0] = (uint8_t) (pixel & 0x0000ff); */
    /*   p[1] = (uint8_t) ((pixel & 0x00ff00) >> 8); */
    /*   p[2] = (uint8_t) ((pixel & 0x0000ff) >> 16); */
    /* } */
    break;
  case 4:
    *(uint32_t *)p = pixel;
    break;
  default: break;
  }

  return Val_unit;
}

EXTERNML value mmage_create_surface(value width, value height, value r, value g, value b) {
  /* use the default masks for the depth: */
  SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE,Long_val(width),Long_val(height),32,0,0,0,0);
  if (surface == NULL) failwith((char *) SDL_GetError());

  uint32_t colour = SDL_MapRGB( surface->format,
                                Long_val(r),
                                Long_val(g),
                                Long_val(b) );
  //  printf("Colour: %X\n", colour);
  int err = SDL_FillRect(surface, NULL, colour);
  if (err < 0) failwith((char *) SDL_GetError());


  return mmage_make_surface(surface);
}


EXTERNML value mmage_init(value unit) {
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0) {
    failwith((char *) SDL_GetError());
  }
  // load support for the JPG and PNG image formats
  int flags=IMG_INIT_JPG|IMG_INIT_PNG;
  int initted=IMG_Init(flags);
  if((initted & flags) != flags) {
    failwith((char *) IMG_GetError());
  }
  return unit;
}

EXTERNML value mmage_image_load(value filename) {
    SDL_Surface *surface = IMG_Load( String_val(filename) );

    if (surface == NULL) failwith((char *) IMG_GetError());

    return mmage_make_surface(surface);
}


EXTERNML value mmage_image_savebmp(value surf, value filename) {
    SDL_Surface *surface = Surface_val(surf);

    SDL_SaveBMP(surface, String_val(filename));

    return Val_unit;
}

EXTERNML value mmage_show(value surf) {
    SDL_Surface *surface = Surface_val(surf);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    window = SDL_CreateWindow("Mmage Viewer",            //    window title
                              SDL_WINDOWPOS_CENTERED,    //    initial x position
                              SDL_WINDOWPOS_CENTERED,    //    initial y position
                              surface->w,                //    width, in pixels
                              surface->h,                //    height, in pixels
                              SDL_WINDOW_SHOWN           //    flags - see below
                              );
    if (window == NULL) failwith((char *) SDL_GetError());

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) failwith((char *) SDL_GetError());

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) failwith((char *) SDL_GetError());

    while (1) {
      SDL_Event e;
      if (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
          break;
        }
      }

      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, NULL, NULL);
      SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return Val_unit;
}
