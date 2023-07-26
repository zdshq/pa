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
  // uint32_t* fb = (uint32_t*)(uintptr_t)FB_ADDR;
  // uint32_t* pixels = (uint32_t*)(ctl->pixels);
  // int x = ctl->x, y = ctl->y;
  // int w = ctl->w, h = ctl->h;
  // for (int j = 0; j < h; j++) {
  //   for (int i = 0; i < w; i++) {
  //     fb[(y + j) * 400 + (x + i)] = *(pixels + j * w + i);
  //   }
  // }

  // if (ctl->sync) {
  //   outl(SYNC_ADDR, 1);
  // }
  uint32_t *p = ctl->pixels;
  // uint64_t a;
  if(ctl->w != 0 && ctl->h != 0){
    // printf()
    for(uint32_t i = ctl->x ; i < (ctl->x+ctl->w) && i < (inl(VGACTL_ADDR) >> 16); i++){
      for(uint32_t j = ctl->y; j < (ctl->y+ctl->h) && j < (uint16_t)(inl(VGACTL_ADDR)); j++){
        // a = (uint64_t)(FB_ADDR + (j * (uint16_t)(inl(VGACTL_ADDR) >> 16) + i)*sizeof(uint32_t));
        // printf("i:%d, j:%d h:%d\r\n", i, j, a);
        outl(FB_ADDR + ((j * 400) + i)*sizeof(uint32_t), p[j*ctl->w+i]);
      }
    }    
    // printf("break\r\n");
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
