#include "input.h"

#include "platform.h"

typedef struct
{
    u8 keys[MINIMAL_KEY_LAST + 1];
    u8 prev_keys[MINIMAL_KEY_LAST + 1];

    u8 buttons[MINIMAL_MOUSE_BUTTON_LAST + 1];
    u8 prev_buttons[MINIMAL_MOUSE_BUTTON_LAST + 1];

    f32 cursorX, cursorY;
} MinimalInputState;

static MinimalInputState state = { 0 };

void minimalUpdateInput()
{
    minimalMemCopy(&state.prev_keys, &state.keys, MINIMAL_KEY_LAST + 1);

    minimalMemCopy(&state.prev_buttons, &state.buttons, MINIMAL_MOUSE_BUTTON_LAST + 1);
}

u8 minimalProcessKey(MinimalKeycode keycode, u8 action)
{
    if (minimalKeycodeValid(keycode) && state.keys[keycode] != action)
    {
        state.keys[keycode] = action;
        return MINIMAL_OK;
    }

    return MINIMAL_FAIL;
}

u8 minimalProcessMouseButton(MinimalMouseButton button, u8 action)
{
    if (minimalMouseButtonValid(button) && state.buttons[button] != action)
    {
        state.buttons[button] = action;
        return MINIMAL_OK;
    }

    return MINIMAL_FAIL;
}

u8 minimalProcessMouseMove(f32 x, f32 y)
{
    state.cursorX = x;
    state.cursorY = y;
    return MINIMAL_OK;
}

u8 minimalKeycodeValid(MinimalKeycode keycode)
{
    return keycode >= MINIMAL_KEY_FIRST && keycode <= MINIMAL_KEY_LAST;
}

u8 minimalMouseButtonValid(MinimalMouseButton button)
{
    return button >= MINIMAL_MOUSE_BUTTON_1 && button <= MINIMAL_MOUSE_BUTTON_LAST;
}

u8 minimalKeyPressed(MinimalKeycode keycode)
{
    if (!minimalKeycodeValid(keycode)) return 0;
    return state.keys[keycode] && !state.prev_keys[keycode];
}

u8 minimalKeyReleased(MinimalKeycode keycode)
{
    if (!minimalKeycodeValid(keycode)) return 0;
    return state.prev_keys[keycode] && !state.keys[keycode];
}

u8 minimalKeyDown(MinimalKeycode keycode)
{
    if (!minimalKeycodeValid(keycode)) return 0;
    return state.keys[keycode];
}

u8 minimalMousePressed(MinimalMouseButton button)
{
    if (!minimalMouseButtonValid(button)) return 0;
    return state.buttons[button] && !state.prev_buttons[button];
}

u8 minimalMouseReleased(MinimalMouseButton button)
{
    if (!minimalMouseButtonValid(button)) return 0;
    return state.prev_buttons[button] && !state.buttons[button];
}

u8 minimalMouseDown(MinimalMouseButton button)
{
    if (!minimalMouseButtonValid(button)) return 0;
    return state.buttons[button];
}

void minimalCursorPos(f32* x, f32* y)
{
    if (x) *x = state.cursorX;
    if (y) *y = state.cursorY;
}

f32 minimalCursorX() { return state.cursorX; }
f32 minimalCursorY() { return state.cursorY; }
