#ifndef MINIMAL_INPUT_H
#define MINIMAL_INPUT_H

#include <GLFW/glfw3.h>

/* define types for key and mouse button */
typedef int8_t MouseButton;
typedef int16_t Key;

void MinimalUpdateInput(GLFWwindow* context);

int MinimalKeyPressed(Key keycode);
int MinimalKeyReleased(Key keycode);

int MinimalKeyHit(Key keycode);
int MinimalKeyDown(Key keycode);
int MinimalKeyUp(Key keycode);

int MinimalMousePressed(MouseButton button);
int MinimalMouseReleased(MouseButton button);

int MinimalMouseHit(MouseButton button);
int MinimalMouseDown(MouseButton button);
int MinimalMouseUp(MouseButton button);

void MinimalCursorPos(float* x, float* y);
float MinimalCursorX();
float MinimalCursorY();

#endif // !MINIMAL_INPUT_H
