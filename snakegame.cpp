#include <iostream>
#include <deque>
#include <algorithm>
#include "include/raylib.h"
#include "include/raymath.h"
using namespace std;

double lastUpdatedTime = 0.0;
bool IntervalPassed(double interval) {
    double currentTime = GetTime(); //time elapsed since InitWindow
    if(currentTime - lastUpdatedTime >= interval) {
        lastUpdatedTime = currentTime;
        return true;
    } return false;
}
bool OverlappedWithSnake(const Vector2& cell, const deque<Vector2>& snakeBody) {
    /* find_if() example syntax
        auto it = std::find_if(
            numbers.begin(), 
            numbers.end(), 
            [](int n) { // Lambda function
                    return n % 2 != 0; // “Should the alg continue searching?”
            }
        );

        if (it != numbers.end()) {
            std::cout << "The first odd number found is: " << *it << std::endl;
        } else {
            std::cout << "No odd numbers found." << std::endl;
        }
    */
    auto pos = find_if( 
        snakeBody.begin(),
        snakeBody.end(),
        [&] (Vector2 itCell) { //[&] takes cell by reference , itCell = iterating cell
            return (cell == itCell); //if yes, find a different position
        }
    );

    if(pos == snakeBody.end()) return false;
    else return true;
}

enum directionAngle {Right = 270, Down = 0, Left = 90, Up = 180};

#pragma region Color Palette
Color BG1 = {216, 239, 230, 255}; //window background
Color BG2 = {191, 226, 225, 255}; //for checkerboarding
Color SNAKE = {30, 38, 75, 255}; //snake's body
Color FRAME = {68, 75, 110, 255}; //frame around playground
Color TEXT = {130, 51, 204, 255}; //title,score text color
#pragma endregion

#pragma region Screen

const int cellSize = 30;
const int cellCount = 20;
const int playGround = cellSize * cellCount; //screen = 20 cells of sidelength 30 = 600px
const float frameOffset = 2 * cellSize;
const float frameWidth = 10.0f;

#pragma endregion

#pragma region Game Classes

class Snake {
public:
    deque<Vector2> body;
    Vector2 direction;
    bool isGrowing = false;

    void Spawn() {
        body = {Vector2{13, 5}, Vector2{14, 5}, Vector2{15, 5}};
        direction = {-1, 0}; // -X
    }
    
    void Move() {
        body.push_front(body[0] + direction);

        if(!isGrowing) body.pop_back();
        else isGrowing = false;
    }

    void Draw(int headAngle) {
        for(auto &cell : body) {
            Rectangle fullSegment = {cell.x * cellSize + frameOffset, cell.y * cellSize + frameOffset, cellSize, cellSize};
            DrawRectangleRounded(fullSegment, 0.3f, 4, SNAKE);

            /** @todo
            float segmentX = cell.x * (cellSize - direction.x)/2;
            float segmentY = cell.y * (cellSize - direction.y)/2;
            Rectangle halfSegment = {segmentX, segmentY, (cellSize - direction.x)/2, (cellSize - direction.y)/2};

            if(cell == body[0]) {
                //DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color)
                DrawCircleSector(
                    {cell.x * cellSize + cellSize/2 + frameOffset, cell.y * cellSize + cellSize/2 + frameOffset},
                    cellSize/2, (float)headAngle, (float)headAngle+180, 10, SNAKE
                );

                DrawRectangleRounded(halfSegment, 0.3f, 4, SNAKE);
            } 
            else if(cell != body[body.size()-1]) DrawRectangleRec(fullSegment, SNAKE);
            else {
                DrawRectangleRounded(fullSegment, 0.3f, 4, SNAKE);
                DrawRectangleRec(halfSegment, SNAKE);
            }
            */
        }
    }
};
class Food {
public:
    Vector2 position;

    #pragma region food sprite
    Texture2D apple;
    Food() {
        Image apple = LoadImage("Sprites/apple.png");
        ImageResize(&apple, cellSize, cellSize);

        this->apple = LoadTextureFromImage(apple);

        UnloadImage(apple);
    }
    ~Food() {UnloadTexture(apple);}
    #pragma endregion

    void Spawn(deque<Vector2> snakeBody) {
        int attempts = 0; //for safety and avoiding infinite loops
        do {
            position = {
                (float)GetRandomValue(0, cellCount-1), 
                (float)GetRandomValue(0, cellCount-1)
            }; attempts++; //random position generator 
        }while(OverlappedWithSnake(position, snakeBody) && attempts < 1000000);
    }

    void Draw() {
        DrawTexture(apple, position.x * cellSize + frameOffset, position.y * cellSize + frameOffset, WHITE);
    }
};

class Game {
public:
    Snake snake;
    Food food;
    bool paused = true;
    int headAngle = directionAngle::Left;

    void Spawn() { snake.Spawn(); food.Spawn(snake.body); }
    
    Game() {Spawn();}
    
    void CheckCollisions() {
        if(snake.body[0].x < 0 || snake.body[0].y < 0 || snake.body[0].x >= cellCount || snake.body[0].y >= cellCount) GameOver(); //wall collision
        
        deque<Vector2> headless = snake.body;
        headless.pop_front();
        if(OverlappedWithSnake(snake.body[0], headless)) GameOver(); //body collision
        
    }

    void ChangeDirection() {
        if((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && snake.direction.y != 1) {
            snake.direction = {0, -1};
            paused = false;
            headAngle = directionAngle::Up;
        } 
        
        if((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && snake.direction.y != -1) {
            snake.direction = {0, 1};
            paused = false;
            headAngle = directionAngle::Down;
        }
        
        if((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && snake.direction.x != 1) {
            snake.direction = {-1, 0};
            paused = false;
            headAngle = directionAngle::Left;
        }
        
        if((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && snake.direction.x != -1) {
            snake.direction = {1, 0};
            paused = false;
            headAngle = directionAngle::Right;
        }
    }
    
    void Update() {
        snake.Move();
        
        //Eat
        if(food.position == snake.body[0]) {
            food.Spawn(snake.body); 
            snake.isGrowing = true;
        }
    }
    
    void GameOver() {
        Spawn();
        paused = true;
    }
    
    void Draw() { 
        for(int row = 0; row < cellCount; row ++) {
            for(int col = 0; col < cellCount; col++) {
                if((row + col) % 2 == 0) DrawRectangle(col * cellSize + frameOffset, row * cellSize + frameOffset, cellSize, cellSize, BG2);
            }
        }

        Rectangle frame = {frameOffset - frameWidth, frameOffset - frameWidth, (float)playGround + 2*frameWidth, (float)playGround + 2*frameWidth};
        DrawRectangleLinesEx(frame, frameWidth, FRAME);

        snake.Draw(headAngle); food.Draw(); 
    }
};

#pragma endregion

int main() {
    InitWindow(playGround + 2*frameOffset, playGround + 2*frameOffset, "Hungry Snake");
    SetTargetFPS(100);

    Game game;

    /* Game Loop */
    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BG1);

        //Event Handling
        game.CheckCollisions();
        game.ChangeDirection(); //checks in every frame

        //Updating
        if(IntervalPassed(0.13) && !game.paused) game.Update(); //updates every 150ms

        //Drawing
        game.Draw(); 

        EndDrawing();
    }

    CloseWindow();
    return 0;
}