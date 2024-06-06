#include <input.h>
#include <stdio.h>

bool getKeyDown(uint8_t scancode) {
    return keyboardStateBuffer[scancode] && !keyboardStateBacklogBuffer[scancode];
}

bool getKey(uint8_t scancode) {
    return keyboardStateBuffer[scancode] && keyboardStateBacklogBuffer[scancode];
}

bool getKeyUp(uint8_t scancode) {
    return !keyboardStateBuffer[scancode] && keyboardStateBacklogBuffer[scancode];
}

bool getMouseButtonDown(uint8_t button) {
    return mouseStateBuffer[button] && !mouseStateBacklogBuffer[button];
}

bool getMouseButton(uint8_t button) {
    return mouseStateBuffer[button] && mouseStateBacklogBuffer[button];
}

bool getMouseButtonUp(uint8_t button) {
    return !mouseStateBuffer[button] && mouseStateBacklogBuffer[button];
}

void flushInput() {
    memcpy(keyboardStateBacklogBuffer, keyboardStateBuffer, SDL_NUM_SCANCODES);

    memcpy(mouseStateBacklogBuffer, mouseStateBuffer, MAX_MOUSE_BUTTONS);
}