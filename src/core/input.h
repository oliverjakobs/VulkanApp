#ifndef OBELISK_INPUT_H
#define OBELISK_INPUT_H

#include "../common.h"

/* The unknown key */
#define OBELISK_KEY_UNKNOWN            -1

/* Printable keys */
#define OBELISK_KEY_SPACE              32
#define OBELISK_KEY_APOSTROPHE         39  /* ' */
#define OBELISK_KEY_COMMA              44  /* , */
#define OBELISK_KEY_MINUS              45  /* - */
#define OBELISK_KEY_PERIOD             46  /* . */
#define OBELISK_KEY_SLASH              47  /* / */
#define OBELISK_KEY_0                  48
#define OBELISK_KEY_1                  49
#define OBELISK_KEY_2                  50
#define OBELISK_KEY_3                  51
#define OBELISK_KEY_4                  52
#define OBELISK_KEY_5                  53
#define OBELISK_KEY_6                  54
#define OBELISK_KEY_7                  55
#define OBELISK_KEY_8                  56
#define OBELISK_KEY_9                  57
#define OBELISK_KEY_SEMICOLON          59  /* ; */
#define OBELISK_KEY_EQUAL              61  /* = */
#define OBELISK_KEY_A                  65
#define OBELISK_KEY_B                  66
#define OBELISK_KEY_C                  67
#define OBELISK_KEY_D                  68
#define OBELISK_KEY_E                  69
#define OBELISK_KEY_F                  70
#define OBELISK_KEY_G                  71
#define OBELISK_KEY_H                  72
#define OBELISK_KEY_I                  73
#define OBELISK_KEY_J                  74
#define OBELISK_KEY_K                  75
#define OBELISK_KEY_L                  76
#define OBELISK_KEY_M                  77
#define OBELISK_KEY_N                  78
#define OBELISK_KEY_O                  79
#define OBELISK_KEY_P                  80
#define OBELISK_KEY_Q                  81
#define OBELISK_KEY_R                  82
#define OBELISK_KEY_S                  83
#define OBELISK_KEY_T                  84
#define OBELISK_KEY_U                  85
#define OBELISK_KEY_V                  86
#define OBELISK_KEY_W                  87
#define OBELISK_KEY_X                  88
#define OBELISK_KEY_Y                  89
#define OBELISK_KEY_Z                  90
#define OBELISK_KEY_LEFT_BRACKET       91  /* [ */
#define OBELISK_KEY_BACKSLASH          92  /* \ */
#define OBELISK_KEY_RIGHT_BRACKET      93  /* ] */
#define OBELISK_KEY_GRAVE_ACCENT       96  /* ` */
#define OBELISK_KEY_WORLD_1            161 /* non-US #1 */
#define OBELISK_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define OBELISK_KEY_ESCAPE             256
#define OBELISK_KEY_ENTER              257
#define OBELISK_KEY_TAB                258
#define OBELISK_KEY_BACKSPACE          259
#define OBELISK_KEY_INSERT             260
#define OBELISK_KEY_DELETE             261
#define OBELISK_KEY_RIGHT              262
#define OBELISK_KEY_LEFT               263
#define OBELISK_KEY_DOWN               264
#define OBELISK_KEY_UP                 265
#define OBELISK_KEY_PAGE_UP            266
#define OBELISK_KEY_PAGE_DOWN          267
#define OBELISK_KEY_HOME               268
#define OBELISK_KEY_END                269
#define OBELISK_KEY_CAPS_LOCK          280
#define OBELISK_KEY_SCROLL_LOCK        281
#define OBELISK_KEY_NUM_LOCK           282
#define OBELISK_KEY_PRINT_SCREEN       283
#define OBELISK_KEY_PAUSE              284
#define OBELISK_KEY_F1                 290
#define OBELISK_KEY_F2                 291
#define OBELISK_KEY_F3                 292
#define OBELISK_KEY_F4                 293
#define OBELISK_KEY_F5                 294
#define OBELISK_KEY_F6                 295
#define OBELISK_KEY_F7                 296
#define OBELISK_KEY_F8                 297
#define OBELISK_KEY_F9                 298
#define OBELISK_KEY_F10                299
#define OBELISK_KEY_F11                300
#define OBELISK_KEY_F12                301
#define OBELISK_KEY_F13                302
#define OBELISK_KEY_F14                303
#define OBELISK_KEY_F15                304
#define OBELISK_KEY_F16                305
#define OBELISK_KEY_F17                306
#define OBELISK_KEY_F18                307
#define OBELISK_KEY_F19                308
#define OBELISK_KEY_F20                309
#define OBELISK_KEY_F21                310
#define OBELISK_KEY_F22                311
#define OBELISK_KEY_F23                312
#define OBELISK_KEY_F24                313
#define OBELISK_KEY_F25                314
#define OBELISK_KEY_KP_0               320
#define OBELISK_KEY_KP_1               321
#define OBELISK_KEY_KP_2               322
#define OBELISK_KEY_KP_3               323
#define OBELISK_KEY_KP_4               324
#define OBELISK_KEY_KP_5               325
#define OBELISK_KEY_KP_6               326
#define OBELISK_KEY_KP_7               327
#define OBELISK_KEY_KP_8               328
#define OBELISK_KEY_KP_9               329
#define OBELISK_KEY_KP_DECIMAL         330
#define OBELISK_KEY_KP_DIVIDE          331
#define OBELISK_KEY_KP_MULTIPLY        332
#define OBELISK_KEY_KP_SUBTRACT        333
#define OBELISK_KEY_KP_ADD             334
#define OBELISK_KEY_KP_ENTER           335
#define OBELISK_KEY_KP_EQUAL           336
#define OBELISK_KEY_LEFT_SHIFT         340
#define OBELISK_KEY_LEFT_CONTROL       341
#define OBELISK_KEY_LEFT_ALT           342
#define OBELISK_KEY_LEFT_SUPER         343
#define OBELISK_KEY_RIGHT_SHIFT        344
#define OBELISK_KEY_RIGHT_CONTROL      345
#define OBELISK_KEY_RIGHT_ALT          346
#define OBELISK_KEY_RIGHT_SUPER        347
#define OBELISK_KEY_MENU               348

#define OBELISK_KEY_LAST               OBELISK_KEY_MENU

/* key mods */
#define OBELISK_MOD_SHIFT       0x0001
#define OBELISK_MOD_CONTROL     0x0002
#define OBELISK_MOD_ALT         0x0004
#define OBELISK_MOD_SUPER       0x0008
#define OBELISK_MOD_CAPS_LOCK   0x0010
#define OBELISK_MOD_NUM_LOCK    0x0020

/* mouse buttons */
#define OBELISK_MOUSE_BUTTON_1      0
#define OBELISK_MOUSE_BUTTON_2      1
#define OBELISK_MOUSE_BUTTON_3      2
#define OBELISK_MOUSE_BUTTON_LAST   OBELISK_MOUSE_BUTTON_3
#define OBELISK_MOUSE_BUTTON_LEFT   OBELISK_MOUSE_BUTTON_1
#define OBELISK_MOUSE_BUTTON_RIGHT  OBELISK_MOUSE_BUTTON_2
#define OBELISK_MOUSE_BUTTON_MIDDLE OBELISK_MOUSE_BUTTON_3

/* input action */
#define OBELISK_RELEASE 0
#define OBELISK_PRESS   1

/* define types for key and mouse button */
typedef int8_t ObeliskMouseButton;
typedef int16_t ObeliskKey;

void obeliskUpdateInput();
int obeliskProcessKey(ObeliskKey key, uint8_t action);
int obeliskProcessMouseButton(ObeliskMouseButton button, uint8_t action);
int obeliskProcessMouseMovement(int32_t x, int32_t y);

/* key is pressed down */
int obeliskKeyPressed(ObeliskKey keycode);
/* key was just pressed */
int obeliskKeyHit(ObeliskKey keycode);
/* key was just released */
int obeliskKeyUp(ObeliskKey keycode);

int obeliskMousePressed(ObeliskMouseButton button);
int obeliskMouseHit(ObeliskMouseButton button);
int obeliskMouseUp(ObeliskMouseButton button);

void obeliskMousePos(int32_t* x, int32_t* y);
int32_t obeliskMouseX();
int32_t obeliskMouseY();

#endif // !OBELISK_INPUT_H
