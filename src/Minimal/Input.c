#include "Input.h"

typedef struct {
    int state;
    int prev;
} InputState;

static InputState key_states[GLFW_KEY_LAST + 1];
static InputState mouse_states[GLFW_MOUSE_BUTTON_LAST + 1];

void MinimalUpdateInput(GLFWwindow* context) {
    for (int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; ++i) {
        key_states[i].prev = key_states[i].state;
        key_states[i].state = (glfwGetKey(context, i) == GLFW_PRESS);
    }

    for (int i = GLFW_MOUSE_BUTTON_1; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
        mouse_states[i].prev = key_states[i].state;
        mouse_states[i].state = (glfwGetMouseButton(context, i) == GLFW_PRESS);
    }
}

int MinimalKeyPressed(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode == GLFW_KEY_UNKNOWN) return 0;

    int state = glfwGetKey(glfwGetCurrentContext(), keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

int MinimalKeyReleased(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode == GLFW_KEY_UNKNOWN) return 0;
    return glfwGetKey(glfwGetCurrentContext(), keycode) == GLFW_RELEASE;
}

int MinimalKeyHit(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode <= GLFW_KEY_UNKNOWN) return 0;
    return key_states[keycode].state && !key_states[keycode].prev;
}

int MinimalKeyDown(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode <= GLFW_KEY_UNKNOWN) return 0;
    return key_states[keycode].state;
}

int MinimalKeyUp(Key keycode) {
    if (keycode > GLFW_KEY_LAST || keycode <= GLFW_KEY_UNKNOWN) return 0;
    return key_states[keycode].prev && !key_states[keycode].state;
}

int MinimalMousePressed(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return glfwGetMouseButton(glfwGetCurrentContext(), button) == GLFW_PRESS;
}

int MinimalMouseReleased(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return glfwGetMouseButton(glfwGetCurrentContext(), button) == GLFW_RELEASE;
}

int MinimalMouseHit(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return mouse_states[button].state && !mouse_states[button].prev;
}

int MinimalMouseDown(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return mouse_states[button].state;
}

int MinimalMouseUp(MouseButton button) {
    if (button > GLFW_MOUSE_BUTTON_LAST || button < GLFW_MOUSE_BUTTON_1) return 0;
    return mouse_states[button].prev && !mouse_states[button].state;
}

void MinimalCursorPos(float* x, float* y) {
    double xpos, ypos;
    glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);

    if (x) *x = (float)xpos;
    if (y) *y = (float)ypos;
}

float MinimalCursorX() {
    float x;
    MinimalCursorPos(&x, NULL);
    return x;
}

float MinimalCursorY() {
    float y;
    MinimalCursorPos(NULL, &y);
    return y;
}