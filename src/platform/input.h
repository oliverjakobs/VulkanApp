#ifndef OBELISK_INPUT_H
#define OBELISK_INPUT_H

#include "platform.h"

/* define types for key and mouse button */
typedef int8_t MouseButton;
typedef int16_t Key;

void obeliskUpdateInput(GLFWwindow* context);

int obeliskKeyPressed(Key keycode);
int obeliskKeyReleased(Key keycode);

int obeliskKeyHit(Key keycode);
int obeliskKeyDown(Key keycode);
int obeliskKeyUp(Key keycode);

int obeliskMousePressed(MouseButton button);
int obeliskMouseReleased(MouseButton button);

int obeliskMouseHit(MouseButton button);
int obeliskMouseDown(MouseButton button);
int obeliskMouseUp(MouseButton button);

void obeliskCursorPos(float* x, float* y);
float obeliskCursorX();
float obeliskCursorY();

#endif // !OBELISK_INPUT_H
