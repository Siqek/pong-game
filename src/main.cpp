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

    Vector2 playerPaddle{
        .x = c_WindowWidth - c_PaddleMarginFromWall - c_PaddleWidth,
        .y = c_WindowHeight / 2.f - c_PaddleHeight / 2.f,
    };
    Vector2 enemyPaddle{
        .x = c_PaddleMarginFromWall,
        .y = c_WindowHeight / 2.f - c_PaddleHeight / 2.f,
    };

    /*
    * ----- Init ball -----
    */
    static constexpr float c_BallRadius = 10.f;
    static constexpr float c_BallSpeed = 500.f;

    Vector2 ballCenter{
        .x = c_WindowWidth / 2.f,
        .y = c_WindowHeight / 2.f
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
        enemyPaddle.y = Clamp(ballCenter.y - c_PaddleHeight / 2.f, 0.f, c_WindowHeight - c_PaddleHeight);

        Vector2 ballDest{
            .x = ballCenter.x + ballDirection.x * dt * c_BallSpeed,
            .y = ballCenter.y + ballDirection.y * dt * c_BallSpeed
        };


        /*
        * ----- Paddle collision with ball -----
        */
        const auto resolvePaddleBallCollision = [&]() -> void
        {
            const bool isBallHeadingRight = ballDirection.x > 0.f;
            const Vector2& paddle = isBallHeadingRight ? playerPaddle : enemyPaddle;

            const float paddleCollisionBound = (isBallHeadingRight
                ? playerPaddle.x - c_BallRadius
                : enemyPaddle.x + c_PaddleWidth + c_BallRadius);

            // Check if the ball crosses the paddle's collision bound.
            const bool isCrossingPaddleFace = (isBallHeadingRight
                ? ballDest.x > paddleCollisionBound && ballCenter.x < paddleCollisionBound
                : ballDest.x < paddleCollisionBound && ballCenter.x > paddleCollisionBound);
            if (isCrossingPaddleFace)
            {
                // Calculate the ball's Y coordinate at the paddle's collision bound.
                const float a = (ballCenter.y - ballDest.y) / (ballCenter.x - ballDest.x);
                const float x = paddleCollisionBound;
                const float x0 = ballCenter.x;
                const float y0 = ballCenter.y;
                const float y = a * (x - x0) + y0;

                // Check if the ball intersects the paddle.
                const bool isHittingPaddle = y >= paddle.y - c_BallRadius && y <= paddle.y + c_PaddleHeight + c_BallRadius;
                if (isHittingPaddle)
                {
                    // Reflect the ball's horizontal direction.
                    ballDirection.x *= -1;

                    // Reflect the destination across the paddle's face.
                    ballDest.x = 2.f * paddleCollisionBound - ballDest.x;
                }
            }
        };
        resolvePaddleBallCollision();


        /*
        * ----- Ball collision with horizontal walls -----
        */
        const bool isBallMovingUp = ballDirection.y < 0.f;
        if (isBallMovingUp)
        {
            if (ballDest.y <= c_BallRadius)
            {
                ballDirection.y *= -1;
                ballDest.y = ballDest.y + 2.f * (c_BallRadius - ballDest.y);
            }
        }
        else
        {
            if (ballDest.y >= c_WindowHeight - c_BallRadius)
            {
                ballDirection.y *= -1;
                ballDest.y = 2.f * (c_WindowHeight - c_BallRadius) - ballDest.y;
            }
        }

        /*
        * ----- Paddle second collision check with ball preventing the ball from entering the paddle after wall bounce -----
        */
        resolvePaddleBallCollision();

        // TODO(siqek): handle ball collision with vertical walls
        // TODO(siqek): count points
        // TODO(siqek): respawn ball after it hits vertical wall

        // Finally move the ball after collision checks.
        ballCenter = ballDest;

        /*
        * ----- Render -----
        */
        BeginDrawing();

            ClearBackground(BLACK);

            static constexpr Vector2 c_DashedLineStart{ .x = c_WindowWidth / 2.f, .y = 0.f };
            static constexpr Vector2 c_DashedLineEnd{ .x = c_WindowWidth / 2.f, .y = c_WindowHeight };
            DrawLineDashed(c_DashedLineStart, c_DashedLineEnd, 10.f, 10.f, RAYWHITE);

            DrawCircleV(ballCenter, c_BallRadius, RAYWHITE);

            DrawRectangle(playerPaddle.x, playerPaddle.y, c_PaddleWidth, c_PaddleHeight, RAYWHITE);
            DrawRectangle(enemyPaddle.x, enemyPaddle.y, c_PaddleWidth, c_PaddleHeight, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
