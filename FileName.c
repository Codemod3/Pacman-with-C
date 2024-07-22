#include <SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_SCALE_FACTOR 1.4
#define ORIGINAL_SCREEN_WIDTH 800
#define ORIGINAL_SCREEN_HEIGHT 600
#define SCREEN_WIDTH (int)(ORIGINAL_SCREEN_WIDTH * SCREEN_SCALE_FACTOR)
#define SCREEN_HEIGHT (int)(ORIGINAL_SCREEN_HEIGHT * SCREEN_SCALE_FACTOR)
#define PACMAN_RADIUS 20
#define GHOST_SIZE 40
#define PACMAN_SPEED 3
#define GHOST_SPEED 5
#define PELLET_RADIUS 5
#define MAZE_WIDTH 18
#define MAZE_HEIGHT 15
#define CELL_SIZE_X (SCREEN_WIDTH / MAZE_WIDTH)
#define CELL_SIZE_Y (SCREEN_HEIGHT / MAZE_HEIGHT)

SDL_Window* window;
SDL_Renderer* renderer;

typedef struct {
    int x, y;
} Position;


typedef struct {
    Position position;
    Position direction;
    int moveCounter;
} GameObject;

typedef struct {
    Position position;
    bool eaten;
} Pellet;


int maze[MAZE_WIDTH][MAZE_HEIGHT] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,0,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,1,0,1},
    {1,0,0,1,0,1,1,1,1,0,0,1,0,1},
    {1,0,0,1,0,1,0,0,1,0,0,1,0,1},
    {1,0,0,1,0,0,0,0,1,0,0,1,0,1},
    {1,0,0,1,0,0,0,0,1,0,0,1,0,1},
    {1,0,0,1,0,1,0,0,1,0,0,1,0,1},
    {1,0,0,1,0,1,1,1,1,0,0,1,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,1,0,1},
    {1,0,0,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

GameObject pacman;
GameObject ghosts[3];
Pellet pellets[MAZE_WIDTH][MAZE_HEIGHT];
bool quit = false;

void initializeGame() {
    int gridX, gridY;

    // Initialize Pac-Man
    do {
        pacman.position.x = rand() % SCREEN_WIDTH;
        pacman.position.y = rand() % SCREEN_HEIGHT;
        // Calculate the grid position of Pac-Man
        gridX = pacman.position.x / CELL_SIZE_X;
        gridY = pacman.position.y / CELL_SIZE_Y;
    } while (maze[gridX][gridY] == 1);

    pacman.direction.x = 0;
    pacman.direction.y = 0;

    // Initialize Ghost 0 at the center
    ghosts[0].position.x = SCREEN_WIDTH / 2;
    ghosts[0].position.y = SCREEN_HEIGHT / 2;

    // Initialize Ghost 1
    do {
        ghosts[1].position.x = rand() % SCREEN_WIDTH;
        ghosts[1].position.y = rand() % SCREEN_HEIGHT;
    } while (maze[ghosts[1].position.x / CELL_SIZE_X][ghosts[1].position.y / CELL_SIZE_Y] == 1);

    ghosts[1].direction.x = 0;
    ghosts[1].direction.y = 0;

    // Initialize Ghost 2 at the right-middle corner
    ghosts[2].position.x = SCREEN_WIDTH - GHOST_SIZE / 2;
    ghosts[2].position.y = SCREEN_HEIGHT / 2;

    // Initialize pellets
    for (int i = 0; i < MAZE_WIDTH; ++i) {
        for (int j = 0; j < MAZE_HEIGHT; ++j) {
            pellets[i][j].position.x = i * CELL_SIZE_X + CELL_SIZE_X / 2;
            pellets[i][j].position.y = j * CELL_SIZE_Y + CELL_SIZE_Y / 2;
            pellets[i][j].eaten = false;
        }
    }
}
void updateGhostDirection(int ghostIndex)
{
    // Generate a random direction (up, down, left, or right)
    int randomDirection = rand() % 4; // 0: up, 1: down, 2: left, 3: right

    // Update the ghost's direction based on the random direction
    switch (randomDirection)
    {
    case 0:
        ghosts[ghostIndex].direction.x = 0;
        ghosts[ghostIndex].direction.y = -1;
        break;
    case 1:
        ghosts[ghostIndex].direction.x = 0;
        ghosts[ghostIndex].direction.y = 1;
        break;
    case 2:
        ghosts[ghostIndex].direction.x = -1;
        ghosts[ghostIndex].direction.y = 0;
        break;
    case 3:
        ghosts[ghostIndex].direction.x = 1;
        ghosts[ghostIndex].direction.y = 0;
        break;
    }
}

bool checkCollision(Position a, Position b, int radiusA, int radiusB)
{
    // Simple circle collision check
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    int distance = dx * dx + dy * dy;
    int radiusSum = radiusA + radiusB;

    return distance <= radiusSum * radiusSum;
}
void moveGhosts() {
    for (int i = 0; i < 3; ++i) {
        // Check if the ghost needs a new direction
        if (ghosts[i].direction.x == 0 && ghosts[i].direction.y == 0) {
            updateGhostDirection(i);
        }

        // Move the ghost
        ghosts[i].position.x += GHOST_SPEED * ghosts[i].direction.x;
        ghosts[i].position.y += GHOST_SPEED * ghosts[i].direction.y;

        // Check if the ghost has moved in the current direction for 10 moves
        if (++ghosts[i].moveCounter >= 40) {
            // Reset the move counter
            ghosts[i].moveCounter = 0;

            // Change direction randomly
            updateGhostDirection(i);
        }

        // Check if the next position is within the maze boundaries
        int gridX = ghosts[i].position.x / CELL_SIZE_X;
        int gridY = ghosts[i].position.y / CELL_SIZE_Y;

        if (gridX < 0 || gridX >= MAZE_WIDTH || gridY < 0 || gridY >= MAZE_HEIGHT || maze[gridX][gridY] == 1) {
            // If the next position is outside the maze or a wall, teleport if it touches the outermost wall
            if (gridX < 0 || gridX >= MAZE_WIDTH || gridY < 0 || gridY >= MAZE_HEIGHT) {
                ghosts[i].position.x = SCREEN_WIDTH / 2;
                ghosts[i].position.y = SCREEN_HEIGHT / 2;
                updateGhostDirection(i); // Change direction randomly after teleporting
            }
            else {
                // Change direction randomly if the next position is against a wall
                updateGhostDirection(i);
            }
        }

        // Check if the ghost is outside the screen boundary
        if (ghosts[i].position.x < 0 || ghosts[i].position.x >= SCREEN_WIDTH ||
            ghosts[i].position.y < 0 || ghosts[i].position.y >= SCREEN_HEIGHT) {
            // Teleport the ghost to the center
            ghosts[i].position.x = SCREEN_WIDTH / 2;
            ghosts[i].position.y = SCREEN_HEIGHT / 2;
            updateGhostDirection(i); // Change direction randomly after teleporting
        }
    }
}

void handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        if (event.type == SDL_QUIT)
        {
            quit = true;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_w:
                pacman.direction.x = 0;
                pacman.direction.y = -1;
                break;
            case SDLK_s:
                pacman.direction.x = 0;
                pacman.direction.y = 1;
                break;
            case SDLK_a:
                pacman.direction.x = -1;
                pacman.direction.y = 0;
                break;
            case SDLK_d:
                pacman.direction.x = 1;
                pacman.direction.y = 0;
                break;
            }
        }
    }
}

void moveGameObjects()
{
    // Move Pac-Man
    int newPacmanX = pacman.position.x + PACMAN_SPEED * pacman.direction.x;
    int newPacmanY = pacman.position.y + PACMAN_SPEED * pacman.direction.y;

    // Check for collisions with walls
    int gridX = newPacmanX / CELL_SIZE_X;
    int gridY = newPacmanY / CELL_SIZE_Y;

    // Ensure Pac-Man stays within the boundaries
    if (gridX >= 0 && gridX < MAZE_WIDTH && gridY >= 0 && gridY < MAZE_HEIGHT && maze[gridX][gridY] != 1)
    {
        pacman.position.x = newPacmanX;
        pacman.position.y = newPacmanY;
    }

    // Check for pellet collisions
    int pelletGridX = pacman.position.x / CELL_SIZE_X;
    int pelletGridY = pacman.position.y / CELL_SIZE_Y;
    if (!pellets[pelletGridX][pelletGridY].eaten)
    {
        pellets[pelletGridX][pelletGridY].eaten = true;
    }

    // Check for collisions with ghosts
    for (int i = 0; i < 3; ++i)
    {
        if (checkCollision(pacman.position, ghosts[i].position, PACMAN_RADIUS, GHOST_SIZE / 2))
        {
            quit = true; // Game over on collision with a ghost
        }
    }

    // Move Ghosts
    moveGhosts();
}


void drawGame() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw Connected Maze with Thinner Walls
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < MAZE_WIDTH; ++i) {
        for (int j = 0; j < MAZE_HEIGHT; ++j) {
            if (maze[i][j] == 1) {
                SDL_Rect wallRect = { i * CELL_SIZE_X, j * CELL_SIZE_Y, CELL_SIZE_X, CELL_SIZE_Y };
                SDL_RenderFillRect(renderer, &wallRect);
            }
        }
    }

    // Draw Pellets
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < MAZE_WIDTH; ++i) {
        for (int j = 0; j < MAZE_HEIGHT; ++j) {
            if (!pellets[i][j].eaten) {
                SDL_Rect pelletRect = { pellets[i][j].position.x - PELLET_RADIUS, pellets[i][j].position.y - PELLET_RADIUS, 2 * PELLET_RADIUS, 2 * PELLET_RADIUS };
                SDL_RenderFillRect(renderer, &pelletRect);
            }
        }
    }

    // Draw Pac-Man
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect pacmanRect = { pacman.position.x - PACMAN_RADIUS, pacman.position.y - PACMAN_RADIUS, 2 * PACMAN_RADIUS, 2 * PACMAN_RADIUS };
    SDL_RenderFillRect(renderer, &pacmanRect);

    // Draw Ghosts
    // Ghost 0 (Red)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect ghost0Rect = { ghosts[0].position.x - GHOST_SIZE / 2, ghosts[0].position.y - GHOST_SIZE / 2, GHOST_SIZE, GHOST_SIZE };
    SDL_RenderFillRect(renderer, &ghost0Rect);

    // Ghost 1 (Pink)
    SDL_SetRenderDrawColor(renderer, 255, 182, 193, 255);
    SDL_Rect ghost1Rect = { ghosts[1].position.x - GHOST_SIZE / 2, ghosts[1].position.y - GHOST_SIZE / 2, GHOST_SIZE, GHOST_SIZE };
    SDL_RenderFillRect(renderer, &ghost1Rect);

    // Ghost 2 (Green)
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_Rect ghost2Rect = { ghosts[2].position.x - GHOST_SIZE / 2, ghosts[2].position.y - GHOST_SIZE / 2, GHOST_SIZE, GHOST_SIZE };
    SDL_RenderFillRect(renderer, &ghost2Rect);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }

    window = SDL_CreateWindow("Pac-Man Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        return 1;
    }

    initializeGame();

    while (!quit) {
        handleEvents();
        moveGameObjects();
        drawGame();

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}