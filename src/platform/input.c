#include "input.h"

typedef struct {
    int state;
    int prev;
} ObeliskInputState;

static ObeliskInputState keyStates[GLFW_KEY_LAST + 1];
static ObeliskInputState mouseStates[GLFW_MOUSE_BUTTON_LAST + 1];

void obeliskUpdateInput(GLFWwindow* context) {
    for (int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; ++i) {
        keyStates[i].prev = keyStates[i].state;
        keyStates[i].state = (glfwGetKey(context, i) == GLFW_PRESS);
    }

    for (int i = GLFW_MOUSE_BUTTON_1; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
        mouseStates[i].prev = mouseStates[i].state;
        mouseStates[i].state = (glfwGetMouseButton(context, i) == GLFW_PRESS);
    }
}

int obeliskKeyPressed(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode == GLFW_KEY_UNKNOWN) return 0;

    int state = glfwGetKey(glfwGetCurrentContext(), keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

int obeliskKeyReleased(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode == GLFW_KEY_UNKNOWN) return 0;
    return glfwGetKey(glfwGetCurrentContext(), keycode) == GLFW_RELEASE;
}

int obeliskKeyHit(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode <= GLFW_KEY_UNKNOWN) return 0;
    return keyStates[keycode].state && !keyStates[keycode].prev;
}

int obeliskKeyDown(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode <= GLFW_KEY_UNKNOWN) return 0;
    return keyStates[keycode].state;
}

int obeliskKeyUp(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode <= GLFW_KEY_UNKNOWN) return 0;
    return keyStates[keycode].prev && !keyStates[keycode].state;
}

int obeliskMousePressed(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return glfwGetMouseButton(glfwGetCurrentContext(), button) == GLFW_PRESS;
}

int obeliskMouseReleased(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return glfwGetMouseButton(glfwGetCurrentContext(), button) == GLFW_RELEASE;
}

int obeliskMouseHit(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return mouseStates[button].state && !mouseStates[button].prev;
}

int obeliskMouseDown(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return mouseStates[button].state;
}

int obeliskMouseUp(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return mouseStates[button].prev && !mouseStates[button].state;
}

void obeliskCursorPos(float* x, float* y) {
    double xpos, ypos;
    glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);

    if (x) *x = (float)xpos;
    if (y) *y = (float)ypos;
}

float obeliskCursorX() {
    float x;
    obeliskCursorPos(&x, NULL);
    return x;
}

float obeliskCursorY() {
    float y;
    obeliskCursorPos(NULL, &y);
    return y;
}
