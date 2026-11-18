#include <algorithm>
#include <cstdlib>
#include <ctime>

#include "raylib.h"
#include "raymath.h"

int main ()
{
    std::srand(std::time(nullptr));

    /*
    * ----- Init window -----
    */
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

    static constexpr const char* c_WindowTitle = "Pong Game";
    static constexpr int c_WindowWidth = 1280;
    static constexpr int c_WindowHeight = 720;

    InitWindow(c_WindowWidth, c_WindowHeight, c_WindowTitle);

    /*
    * ----- Init paddles -----
    */
    static constexpr float c_PaddleWidth = 16.f;
    static constexpr float c_PaddleHeight = 120.f;
    static constexpr float c_PaddleMarginFromWall = 40.f;
    static constexpr float c_PaddleSpeed = 400.f;

    Rectangle playerPaddle{
        .x = c_WindowWidth - c_PaddleMarginFromWall - c_PaddleWidth,
        .y = c_WindowHeight / 2.f - c_PaddleHeight / 2.f,
        .width = c_PaddleWidth,
        .height = c_PaddleHeight
    };
    Rectangle enemyPaddle{
        .x = c_PaddleMarginFromWall,
        .y = c_WindowHeight / 2.f - c_PaddleHeight / 2.f,
        .width = c_PaddleWidth,
        .height = c_PaddleHeight
    };

    /*
    * ----- Init ball -----
    */
    struct Circle {
        Vector2 center{ .x = 0.f, .y = 0.f }; // position of center
        float radius = 0.f;
    };

    static constexpr float c_BallRadius = 10.f;
    static constexpr float c_BallSpeed = 200.f;

    Circle ball{
        .center{ .x = c_WindowWidth / 2.f, .y = c_WindowHeight / 2.f },
        .radius = c_BallRadius
    };
    Vector2 ballDirection{ .x = 1.f, .y = 0.f };

    const float angleRad = (-15 + std::rand() % 31) * DEG2RAD;
    ballDirection = Vector2Rotate(ballDirection, angleRad);

    /*
    * ----- Game loop -----
    */
    while (!WindowShouldClose())
    {
        /*
        * ----- Update -----
        */
        const float dt = std::min(GetFrameTime(), 1.f / 30.f);

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
        {
            playerPaddle.y -= c_PaddleSpeed * dt;
        }
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
        {
            playerPaddle.y += c_PaddleSpeed * dt;
        }

        playerPaddle.y = Clamp(playerPaddle.y, 0.f, c_WindowHeight - c_PaddleHeight);

        Vector2 ballDest{
            .x = ball.center.x + ballDirection.x * dt * c_BallSpeed,
            .y = ball.center.y + ballDirection.y * dt * c_BallSpeed
        };

        // TODO(siqek): determine which paddle check

        // Check if the ball crosses the paddle's face.
        const bool isCrossingPaddleFace = playerPaddle.x > ball.center.x && playerPaddle.x < ballDest.x;
        if (isCrossingPaddleFace)
        {
            // Calculate the ball's Y coordinate at the paddle's face.
            const float a = (ball.center.y - ballDest.y) / (ball.center.x - ballDest.x);
            const float x = playerPaddle.x;
            const float x0 = ball.center.x;
            const float y0 = ball.center.y;
            const float y = a * (x - x0) + y0;

            // Check if the ball intersects the paddle, including its radius.
            const bool isHittingPaddle = y >= playerPaddle.y - ball.radius && y <= playerPaddle.y + playerPaddle.height + ball.radius;
            if (isHittingPaddle)
            {
                // Reflect the ball's horizontal direction.
                ballDirection.x *= -1;

                // Reflect the destination across the paddle's face.
                ballDest.x = 2.f * playerPaddle.x - ballDest.x;
                ballDest.y = y;
            }
        }

        // TODO(siqek): handle ball collision with horizontal walls
        // TODO(siqek): disallow ball to bounce from the wall into inside of a paddle

        // TODO(siqek): handle ball collision with vertical walls
        // TODO(siqek): count points
        // TODO(siqek): respawn ball after it hits vertical wall

        // Finally move the ball after collision checks.
        ball.center = ballDest;

        /*
        * ----- Render -----
        */
        BeginDrawing();

            ClearBackground(BLACK);

            static constexpr Vector2 c_DashedLineStart{ .x = c_WindowWidth / 2.f, .y = 0.f };
            static constexpr Vector2 c_DashedLineEnd{ .x = c_WindowWidth / 2.f, .y = c_WindowHeight };
            DrawLineDashed(c_DashedLineStart, c_DashedLineEnd, 10.f, 10.f, RAYWHITE);

            DrawCircleV(ball.center, ball.radius, RAYWHITE);

            DrawRectangleRec(playerPaddle, RAYWHITE);
            DrawRectangleRec(enemyPaddle, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
