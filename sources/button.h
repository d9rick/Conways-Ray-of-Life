#include "../_deps/raylib-src/src/raylib.h"

#define BUTTON_WIDTH (200)
#define BUTTON_HEIGHT (50)

typedef struct {
    Rectangle bounds;
    Color color;
    const char* text;
} Button;

bool isButtonClicked(Button button, Vector2 mousepoint);
void drawButton(Button button);