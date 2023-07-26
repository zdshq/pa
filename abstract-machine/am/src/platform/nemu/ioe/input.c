#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  // kbd->keydown = 0;
  // kbd->keycode = AM_KEY_NONE;
  if (inl(KBD_ADDR)) {
    int code = inl(KBD_ADDR);
    kbd->keydown = code & KEYDOWN_MASK;
    kbd->keycode = code & ~(KEYDOWN_MASK);
  } else {
    kbd->keydown = false;
    kbd->keycode = AM_KEY_NONE;
  }
}
