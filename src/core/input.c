#include "input.h"

#include <string.h>

typedef ObeliskKey ObeliskKeyboardState[OBELISK_KEY_LAST + 1];

typedef struct {
    int32_t x, y;
    ObeliskMouseButton buttons[OBELISK_MOUSE_BUTTON_LAST + 1];
} ObeliskMouseState;

typedef struct {
    ObeliskKeyboardState keys;
    ObeliskKeyboardState keysPrev;
    ObeliskMouseState mouse;
    ObeliskMouseState mousePrev;
} ObeliskInputState;

static ObeliskInputState inputState = { 0 };

void obeliskUpdateInput() {
    memcpy(&inputState.keysPrev, &inputState.keys, sizeof(ObeliskKeyboardState));
    memcpy(&inputState.mousePrev, &inputState.mouse, sizeof(ObeliskMouseState));
}

int obeliskProcessKey(ObeliskKey key, uint8_t action) {
    if (inputState.keys[key] == action) return 0;

    inputState.keys[key] = action;
    return 1;
}

int obeliskProcessMouseButton(ObeliskMouseButton button, uint8_t action) {
    if (inputState.mouse.buttons[button] == action) return 0;

    inputState.mouse.buttons[button] = action;
    return 1;
}

int obeliskProcessMouseMovement(int32_t x, int32_t y) {
    if (inputState.mouse.x == x && inputState.mouse.y == y) return 0;
    
    inputState.mouse.x = x;
    inputState.mouse.y = y;
    return 1;
}

int obeliskKeyPressed(ObeliskKey keycode) {
    if (keycode > OBELISK_KEY_LAST || keycode <= OBELISK_KEY_UNKNOWN) return 0;
    return inputState.keys[keycode] == OBELISK_PRESS;
}

int obeliskKeyHit(ObeliskKey keycode) {
    if (keycode > OBELISK_KEY_LAST || keycode <= OBELISK_KEY_UNKNOWN) return 0;
    return inputState.keys[keycode] == OBELISK_PRESS 
        && inputState.keysPrev[keycode] == OBELISK_RELEASE;
}

int obeliskKeyUp(ObeliskKey keycode) {
    if (keycode > OBELISK_KEY_LAST || keycode <= OBELISK_KEY_UNKNOWN) return 0;
    return inputState.keys[keycode] == OBELISK_RELEASE 
        && inputState.keysPrev[keycode] == OBELISK_PRESS;
}

int obeliskMousePressed(ObeliskMouseButton button) {
    if (button > OBELISK_MOUSE_BUTTON_LAST || button < OBELISK_MOUSE_BUTTON_1) return 0;
    return inputState.mouse.buttons[button] == OBELISK_PRESS;
}

int obeliskMouseHit(ObeliskMouseButton button) {
    if (button > OBELISK_MOUSE_BUTTON_LAST || button < OBELISK_MOUSE_BUTTON_1) return 0;
    return inputState.mouse.buttons[button] == OBELISK_PRESS 
        && inputState.mousePrev.buttons[button] == OBELISK_RELEASE;
}

int obeliskMouseUp(ObeliskMouseButton button) {
    if (button > OBELISK_MOUSE_BUTTON_LAST || button < OBELISK_MOUSE_BUTTON_1) return 0;
    return inputState.mouse.buttons[button] == OBELISK_RELEASE 
        && inputState.mousePrev.buttons[button] == OBELISK_PRESS;
}

void obeliskMousePos(int32_t* x, int32_t* y) {
    if (x) *x = inputState.mouse.x;
    if (y) *y = inputState.mouse.y;
}

int32_t obeliskMouseX() { return inputState.mouse.x; }
int32_t obeliskMouseY() { return inputState.mouse.y; }
