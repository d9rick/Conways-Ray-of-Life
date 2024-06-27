#include "./button.h"

// returns true if a button is pressed
bool isButtonClicked(Button button, Vector2 mousepoint) {
    return CheckCollisionPointRec(mousepoint, button.bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// draws a button
void drawButton(Button button) {
    // Draw button
    DrawRectangleRec(button.bounds, button.color);
    DrawText(button.text, button.bounds.x + 20, button.bounds.y + 10, 20, BLACK);
}