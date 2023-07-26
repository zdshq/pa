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
  if (ctl->sync) {
    printf("hhh");
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
