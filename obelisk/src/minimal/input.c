#include "input.h"

#include "platform.h"

typedef struct
{
    uint8_t keys[MINIMAL_KEY_LAST + 1];
    uint8_t prev_keys[MINIMAL_KEY_LAST + 1];

    uint8_t buttons[MINIMAL_MOUSE_BUTTON_LAST + 1];
    uint8_t prev_buttons[MINIMAL_MOUSE_BUTTON_LAST + 1];

    float cursorX, cursorY;
} MinimalInputState;

static MinimalInputState state = { 0 };

void minimalUpdateInput()
{
    MINIMAL_MEMCPY(&state.prev_keys, &state.keys, MINIMAL_KEY_LAST + 1);
    MINIMAL_MEMCPY(&state.prev_buttons, &state.buttons, MINIMAL_MOUSE_BUTTON_LAST + 1);
}

uint8_t minimalProcessKey(MinimalKeycode keycode, uint8_t action)
{
    if (minimalKeycodeValid(keycode) && state.keys[keycode] != action)
    {
        state.keys[keycode] = action;
        return MINIMAL_OK;
    }

    return MINIMAL_FAIL;
}

uint8_t minimalProcessMouseButton(MinimalMouseButton button, uint8_t action)
{
    if (minimalMouseButtonValid(button) && state.buttons[button] != action)
    {
        state.buttons[button] = action;
        return MINIMAL_OK;
    }

    return MINIMAL_FAIL;
}

uint8_t minimalProcessMouseMove(float x, float y)
{
    state.cursorX = x;
    state.cursorY = y;
    return MINIMAL_OK;
}

uint8_t minimalKeycodeValid(MinimalKeycode keycode)
{
    return keycode >= MINIMAL_KEY_FIRST && keycode <= MINIMAL_KEY_LAST;
}

uint8_t minimalMouseButtonValid(MinimalMouseButton button)
{
    return button >= MINIMAL_MOUSE_BUTTON_1 && button <= MINIMAL_MOUSE_BUTTON_LAST;
}

uint8_t minimalKeyPressed(MinimalKeycode keycode)
{
    if (!minimalKeycodeValid(keycode)) return 0;
    return state.keys[keycode] && !state.prev_keys[keycode];
}

uint8_t minimalKeyReleased(MinimalKeycode keycode)
{
    if (!minimalKeycodeValid(keycode)) return 0;
    return state.prev_keys[keycode] && !state.keys[keycode];
}

uint8_t minimalKeyDown(MinimalKeycode keycode)
{
    if (!minimalKeycodeValid(keycode)) return 0;
    return state.keys[keycode];
}

uint8_t minimalMousePressed(MinimalMouseButton button)
{
    if (!minimalMouseButtonValid(button)) return 0;
    return state.buttons[button] && !state.prev_buttons[button];
}

uint8_t minimalMouseReleased(MinimalMouseButton button)
{
    if (!minimalMouseButtonValid(button)) return 0;
    return state.prev_buttons[button] && !state.buttons[button];
}

uint8_t minimalMouseDown(MinimalMouseButton button)
{
    if (!minimalMouseButtonValid(button)) return 0;
    return state.buttons[button];
}

void minimalCursorPos(float* x, float* y)
{
    if (x) *x = state.cursorX;
    if (y) *y = state.cursorY;
}

float minimalCursorX() { return state.cursorX; }
float minimalCursorY() { return state.cursorY; }
