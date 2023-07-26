#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  
  kbd->keydown = 48;
  kbd->keycode = AM_KEY_1;
}
