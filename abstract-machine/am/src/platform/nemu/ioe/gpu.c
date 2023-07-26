#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  int i;
  int w = 0;  // TODO: get the correct width
  int h = 0;  // TODO: get the correct height
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = inl(VGACTL_ADDR) >> 16, .height = inl(VGACTL_ADDR) & 0xffff,
    .vmemsz = inl(VGACTL_ADDR) >> 16 * inl(VGACTL_ADDR) & 0xffff
  };
  printf("width:%d height:%d\r\n",cfg->width,cfg->height);
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t *p = ctl->pixels;
  if(ctl->w != 0 && ctl->h != 0){
    for(int i = ctl->x ; i < (ctl->x+1) && i < (inl(VGACTL_ADDR) >> 16); i++){
      for(int j = ctl->y; j < (ctl->y+1) && j < (uint16_t)(inl(VGACTL_ADDR)); j++){
        outl(FB_ADDR + (j * (uint16_t)(inl(VGACTL_ADDR)) + i)*sizeof(uint32_t), p[j*ctl->w+i]);
      }
    }    
    printf("break\r\n");
  }
  else{
    if (ctl->sync) {      
      printf("hhhaaaaaaaaaaaaaaa\r\n");
      outl(SYNC_ADDR, 1);
    }    
  }


  // int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  // if (w == 0 || h == 0) return;
  // feclearexcept(-1);
  // SDL_Surface *s = SDL_CreateRGBSurfaceFrom(ctl->pixels, w, h, 32, w * sizeof(uint32_t),
  //     RMASK, GMASK, BMASK, AMASK);
  // SDL_Rect rect = { .x = x, .y = y };
  // SDL_BlitSurface(s, NULL, surface, &rect);
  // SDL_FreeSurface(s);
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
