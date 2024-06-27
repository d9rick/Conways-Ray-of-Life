#include "../_deps/raylib-src/src/raylib.h"
#include "button.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (850)
#define TOOLBAR_HEIGHT (50)
#define SIM_HEIGHT (SCREEN_HEIGHT - TOOLBAR_HEIGHT)

#define WINDOW_TITLE "Conway's Ray of Life"

typedef struct {
    int x;
    int y;
    bool alive;
}  Cell;

typedef struct {
    int height;
    int width;
    Cell** universe;
} Board;

// frees the memory allocated for the board
void freeBoard(Board *board) {
    for (int i = 0; i < board->height; i++) {
        free(board->universe[i]);
    }
    free(board->universe);
    free(board);
}

// places an alive cell at the given coordinates
void placeCell(Board* board, int x, int y) {
    board->universe[y][x].alive = true;
}

// creates a blank board with all dead cells
Board* createBoard(int height, int width) {
    // create the board
    Board *board = (Board*)malloc(sizeof(Board));
    if(!board) {
        fprintf(stderr, "failed to allocate the board");
        exit(EXIT_FAILURE);
    }

    // set the members
    board->height = height;
    board->width = width;

    // create the row pointers
    board->universe = (Cell**)malloc(height * sizeof(Cell*));
    if (!board->universe) {
        fprintf(stderr, "failed to allocate the board");
        exit(EXIT_FAILURE);
    }

    // fill each row 
    for(int i = 0; i < height; i++) {
        board->universe[i] = (Cell*)malloc(width * sizeof(Cell));

        // handle failure
        if (!board->universe[i]) {
            fprintf(stderr, "Failed to allocate memory for a row\n");
            for (int j = 0; j < i; j++) {
                free(board->universe[j]);
            }
            free(board->universe);
            free(board);
            exit(EXIT_FAILURE);
        }
    }

    // create all cells (DEAD BY DEFAULT)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            board->universe[i][j] = (Cell){
                i,
                j,
                0
            };
        }
    }

    return board;
}

// draws the given board to the window
void drawBoard(Board* board) {
    const int squareSize = SIM_HEIGHT / board->height;

    // draw each cell
    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width; x++) {
            Color cellColor = board->universe[y][x].alive ? WHITE : BLACK;
            DrawRectangle(x * squareSize, y * squareSize, squareSize, squareSize, cellColor);
        }
    }

}

// creates a board for testing based on input
Board* testBoard(char* startPosition) {
    // create blank board
    Board* board;

    if (strcmp(startPosition, "checkerboard") == 0) {
        const int height = 10;
        const int width  = 10;
        board = createBoard(height, width);
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                board->universe[i][j] = (Cell){
                    i,
                    j,
                    (i + j) % 2
                };
            }
        }

        return board;
    }
    else if (strcmp(startPosition, "blinker") == 0) {
        const int height = 5;
        const int width  = 5;
        board = createBoard(height, width);

        placeCell(board, 2, 1);
        placeCell(board, 2, 2);
        placeCell(board, 2, 3);

        return board;
    }
    else if (strcmp(startPosition, "toad") == 0) {
        const int height = 6;
        const int width  = 6;
        board = createBoard(height, width);

        placeCell(board, 2, 2);
        placeCell(board, 2, 3);
        placeCell(board, 2, 4);
        placeCell(board, 3, 1);
        placeCell(board, 3, 2);
        placeCell(board, 3, 3);

        return board;
    }
    else if (strcmp(startPosition, "beacon") == 0) {
        const int height = 6;
        const int width  = 6;
        board = createBoard(height, width);

        placeCell(board, 1, 1);
        placeCell(board, 1, 2);
        placeCell(board, 2, 1);
        placeCell(board, 2, 2);
        placeCell(board, 3, 3);
        placeCell(board, 3, 4);
        placeCell(board, 4, 3);
        placeCell(board, 4, 4);

        return board;
    }

    return NULL;
}

// evolves a given board's universe by one generation
void evolve(Board* board) {
    // constants used to evaluate neighbor counts more quickly
    const int neighborRows[8] = {0,  0,  1, 1, 1, -1, -1, -1};
    const int neighborCols[8] = {1, -1, -1, 0, 1, -1,  0,  1};

    // create temporary array for next generation
    bool** nextGeneration = (bool**)malloc(board->height * sizeof(bool*));
    for (int i = 0; i < board->height; i++) {
        nextGeneration[i] = (bool*)malloc(board->width * sizeof(bool));
    }

    // calculate the "aliveness" of every given cell (https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life#Rules)
    for (int i = 0; i < board->height; i++) {
        for (int j = 0; j < board->width; j++) {
            // calculate the number of (living) neighbors for each cell
            int numNeighbors = 0;
            for(int k = 0; k < 8; k++) {
                int neighborCol = j + neighborCols[k];
                int neighborRow = i + neighborRows[k];

                // check that coords are in bounds
                if (neighborCol < 0 || neighborCol >= board->width || neighborRow < 0 || neighborRow >= board->height) {
                    continue;
                }

                // count each living neighbor
                numNeighbors += board->universe[neighborRow][neighborCol].alive;
            }

            // Any live cell with fewer than two live neighbours dies, as if by underpopulation.
            // Any live cell with more than three live neighbours dies, as if by overpopulation.
            if (board->universe[i][j].alive) {
                nextGeneration[i][j] = (numNeighbors == 2 || numNeighbors == 3);
            }
            // Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
            else {
                nextGeneration[i][j] = (numNeighbors == 3);
            }
        }
    }

    // update the universe with the next generation
    for(int i = 0; i < board->height; i++) {
        for(int j = 0; j < board->width; j++) {
            board->universe[i][j].alive = nextGeneration[i][j];
        }
    }

    // free the nextGeneration
    for(int i = 0; i < board->height; i++) {
        free(nextGeneration[i]);
    }
    free(nextGeneration);
}

// saves the current board to a file named "universe.gol"
void exportBoard(Board* board) {
    FILE* file = fopen("universe.gol", "w");
    if (!file) {
        fprintf(stderr, "Failed to open file for writing\n");
        return;
    }

    // write the board dimensions
    fprintf(file, "%d %d\n", board->height, board->width);

    // write the board state
    for (int i = 0; i < board->height; i++) {
        for (int j = 0; j < board->width; j++) {
            fprintf(file, "%d ", board->universe[i][j].alive);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// imports the universe.gol file and sets the board to the imported state
void importBoard(Board* board) {
    FILE* file = fopen("universe.gol", "r");
    if (!file) {
        fprintf(stderr, "Failed to open file for reading\n");
        return;
    }

    // read the board dimensions
    int height, width;
    fscanf(file, "%d %d", &height, &width);

    // read the board state
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int alive;
            fscanf(file, "%d", &alive);
            board->universe[i][j].alive = alive;
        }
    }

    fclose(file);
}

int main(void)
{
    // window setup
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    // create board
    Board* board = createBoard(10, 10);
    if (!board) {
        fprintf(stderr, "Failed to create test board! Aborting.");
        exit(EXIT_FAILURE);
    }

    // button declarations
    Button pauseButton = {
        .bounds = { SCREEN_WIDTH - BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT },
        .color = GRAY,
        .text = "Play"
    };
    Button evolveButton = {
        .bounds = { SCREEN_WIDTH - BUTTON_WIDTH * 2, SCREEN_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT },
        .color = GREEN,
        .text = "Evolve"
    };
    Button exportButton = {
        .bounds = { SCREEN_WIDTH - BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT },
        .color = RED,
        .text = "Export"
    };
    Button importButton = {
        .bounds = { SCREEN_WIDTH - BUTTON_WIDTH * 2, SCREEN_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT },
        .color = BLUE,
        .text = "Import"
    };
    Button settingsButton = {
        .bounds = { SCREEN_WIDTH - BUTTON_WIDTH * 4, SCREEN_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT },
        .color = PURPLE,
        .text = "Settings"
    };
    Button editButton = {
        .bounds = { BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT },
        .color = ORANGE,
        .text = "Edit"
    };

    // game state
    bool paused = true;
    bool showSettings = false;
    bool editing = false;
    const float updateInterval = 0.5f; // Update every 0.5 seconds
    float elapsedTime = 0.0f;

    // main loop
    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition();

        // buttons
        if (isButtonClicked(pauseButton, mousePos) && !showSettings) { // pause
            paused = !paused;
            pauseButton.text = paused ? "Play" : "Pause";
        }
        if (isButtonClicked(evolveButton, mousePos) && !showSettings && paused) { // evolve
            evolve(board);
        }
        if (isButtonClicked(importButton, mousePos) && showSettings && paused) { // import
            importBoard(board);
        }
        if (isButtonClicked(exportButton, mousePos) && showSettings && paused) { // export
            exportBoard(board);
        }
        if (isButtonClicked(settingsButton, mousePos)) { // settings
            showSettings = !showSettings;
        }
        if (isButtonClicked(editButton, mousePos) && paused) { // edit
            editing = !editing;
            editButton.text = editing ? "Done" : "Edit";
        }

        // evolve the board if game not paused and not editing
        if (!paused && !editing) {
            elapsedTime += GetFrameTime(); // Add time since last frame
            if (elapsedTime >= updateInterval) {
                evolve(board);
                elapsedTime = 0.0f; // Reset elapsed time
            }
        }

        if (editing) {
            // place cells if the mouse is in bounds
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                int x = mousePos.x / (SIM_HEIGHT / board->height);
                int y = mousePos.y / (SIM_HEIGHT / board->height);

                bool outOfBounds = (x < 0 || x >= board->width || y < 0 || y >= board->height);

                if (!outOfBounds) {
                    placeCell(board, x, y);
                }
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            // draw the board
            drawBoard(board);

            drawButton(settingsButton);
            if (showSettings) {
                drawButton(importButton);
                drawButton(exportButton);
            } else {
                drawButton(pauseButton);
                drawButton(evolveButton);
                drawButton(editButton);
            }
            
            
        EndDrawing();
    }

    // free resources
    freeBoard(board);

    CloseWindow();

    return 0;
}
