#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define MAX_MOUSE_BUTTONS 8

bool keyboardStateBacklogBuffer[SDL_NUM_SCANCODES];
bool keyboardStateBuffer[SDL_NUM_SCANCODES];

bool mouseStateBacklogBuffer[MAX_MOUSE_BUTTONS];
bool mouseStateBuffer[MAX_MOUSE_BUTTONS];

bool getKeyDown(uint8_t scancode);
bool getKey(uint8_t scancode);
bool getKeyUp(uint8_t scancode);

bool getMouseButtonDown(uint8_t button);
bool getMouseButton(uint8_t button);
bool getMouseButtonUp(uint8_t button);

void flushInput();