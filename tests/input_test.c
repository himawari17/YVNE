#include "../src/platform/input.h"

#include <assert.h>

static void test_keyboard(YVNE_Input *input)
{
    const YVNE_InputState *state = YVNE_InputGetState(input);
    const YVNE_InputButton *button = &state->events[YVNE_INPUT_LEFT];
    SDL_Event event = {.type = SDL_EVENT_KEY_DOWN};
    event.key.scancode = SDL_SCANCODE_SPACE;
    YVNE_InputHandle(input, &event);
    assert(button->down && button->pressed && !button->released);
    YVNE_InputOnFrame(input);
    assert(button->down && !button->pressed && !button->released);

    event.key.repeat = true;
    YVNE_InputHandle(input, &event);
    assert(button->down && !button->pressed && !button->released);
    event.type = SDL_EVENT_KEY_UP;
    YVNE_InputHandle(input, &event);
    assert(!button->down && !button->pressed && button->released);
    YVNE_InputOnFrame(input);
    YVNE_InputHandle(input, &event);
    assert(!button->down && !button->pressed && !button->released);

    event.type = SDL_EVENT_KEY_DOWN;
    event.key.repeat = false;
    YVNE_InputHandle(input, &event);
    event.type = SDL_EVENT_KEY_UP;
    YVNE_InputHandle(input, &event);
    assert(!button->down && button->pressed && button->released);
    YVNE_InputOnFrame(input);
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.scancode = SDL_SCANCODE_A;
    YVNE_InputHandle(input, &event);
    for (size_t i = 0; i < YVNE_INPUT_COUNT; ++i) {
        assert(!state->events[i].down);
        assert(!state->events[i].pressed && !state->events[i].released);
    }
}

static void test_mouse(YVNE_Input *input)
{
    const YVNE_MouseState *mouse = &YVNE_InputGetState(input)->mouse;
    SDL_Event event = {.type = SDL_EVENT_MOUSE_MOTION};
    event.motion.x = 10.5f;
    event.motion.y = 20.0f;
    event.motion.xrel = 2.0f;
    event.motion.yrel = -3.0f;
    YVNE_InputHandle(input, &event);
    event.motion.x = 12.5f;
    event.motion.y = 16.0f;
    event.motion.xrel = -1.5f;
    event.motion.yrel = 4.0f;
    YVNE_InputHandle(input, &event);
    assert(mouse->mouse_x == 12.5f && mouse->mouse_y == 16.0f);
    assert(mouse->delta_x == 0.5f && mouse->delta_y == 1.0f);
    YVNE_InputOnFrame(input);
    assert(mouse->delta_x == 0.0f && mouse->delta_y == 0.0f);
    assert(mouse->mouse_x == 12.5f && mouse->mouse_y == 16.0f);

    const Uint8 buttons[] = {SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT};
    const YVNE_InputButton *states[] = {&mouse->left, &mouse->right};
    for (size_t i = 0; i < 2; ++i) {
        event = (SDL_Event){.type = SDL_EVENT_MOUSE_BUTTON_DOWN};
        event.button.button = buttons[i];
        event.button.x = 40.5f + (float)i;
        event.button.y = 60.0f;
        YVNE_InputHandle(input, &event);
        assert(mouse->mouse_x == event.button.x && mouse->mouse_y == event.button.y);
        assert(states[i]->down && states[i]->pressed && !states[i]->released);
        YVNE_InputOnFrame(input);
        assert(states[i]->down && !states[i]->pressed && !states[i]->released);
        YVNE_InputHandle(input, &event);
        assert(!states[i]->pressed);
        event.type = SDL_EVENT_MOUSE_BUTTON_UP;
        YVNE_InputHandle(input, &event);
        assert(!states[i]->down && states[i]->released);
        YVNE_InputOnFrame(input);
        YVNE_InputHandle(input, &event);
        assert(!states[i]->down && !states[i]->pressed && !states[i]->released);
        event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        YVNE_InputHandle(input, &event);
        event.type = SDL_EVENT_MOUSE_BUTTON_UP;
        YVNE_InputHandle(input, &event);
        assert(!states[i]->down && states[i]->pressed && states[i]->released);
        YVNE_InputOnFrame(input);
    }

    event = (SDL_Event){.type = SDL_EVENT_MOUSE_BUTTON_DOWN};
    event.button.button = SDL_BUTTON_MIDDLE;
    YVNE_InputHandle(input, &event);
    event.type = SDL_EVENT_QUIT;
    YVNE_InputHandle(input, &event);
    assert(!mouse->left.down && !mouse->right.down);
    assert(!mouse->left.pressed && !mouse->right.pressed);
}

int main(void)
{
    YVNE_Input *input = YVNE_InputInit();
    assert(input != NULL);
    const YVNE_InputState *state = YVNE_InputGetState(input);
    assert(state != NULL);
    for (size_t i = 0; i < YVNE_INPUT_COUNT; ++i) {
        assert(!state->events[i].down && !state->events[i].pressed);
        assert(!state->events[i].released);
    }
    assert(!state->mouse.left.down && !state->mouse.right.down);
    assert(state->mouse.mouse_x == 0.0f && state->mouse.mouse_y == 0.0f);
    test_keyboard(input);
    test_mouse(input);

    YVNE_InputHandle(input, NULL);
    SDL_Event event = {.type = SDL_EVENT_KEY_DOWN};
    YVNE_InputHandle(NULL, &event);
    YVNE_InputOnFrame(NULL);
    assert(YVNE_InputGetState(NULL) == NULL);
    YVNE_InputDestroy(input);
    YVNE_InputDestroy(NULL);
    return 0;
}
