#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>

#include "raylib.h"
#include "raymath.h"

int main ()
{
    std::srand(std::time(nullptr));

    /*
    * ----- Init window -----
    */
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

    constexpr const char* c_WindowTitle = "Pong Game";
    constexpr int c_WindowWidth = 1280;
    constexpr int c_WindowHeight = 720;

    InitWindow(c_WindowWidth, c_WindowHeight, c_WindowTitle);

    /*
    * ----- Init paddles -----
    */
    constexpr float c_PaddleWidth = 16.f;
    constexpr float c_PaddleHeight = 120.f;
    constexpr float c_PaddleMarginFromWall = 40.f;
    constexpr float c_PaddleSpeed = 250.f;

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
    constexpr float c_BallRadius = 10.f;
    constexpr float c_BallSpeed = 500.f;

    Vector2 ballCenter{
        .x = c_WindowWidth / 2.f,
        .y = c_WindowHeight / 2.f
    };
    Vector2 ballDirection{ .x = 1.f, .y = 0.f };

    constexpr int c_MaxInitialAngleDeg = 15;
    // Get random angle value from -15 deg up to +30 deg for ball direction.
    const float angleDeg = (-c_MaxInitialAngleDeg + std::rand() % (c_MaxInitialAngleDeg * 2 + 1));
    const float angleRad = angleDeg * DEG2RAD;
    ballDirection = Vector2Rotate(ballDirection, angleRad);

    /*
    * ----- Init score counters -----
    */
    int playerScore = 0;
    std::string playerScoreText{"0"};
    int enemyScore = 0;
    std::string enemyScoreText{"0"};

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
        // TODO(siqek): set max speed for enemy paddle

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
                    // TODO(siqek): reflect and rotate from -x to x angle

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

        // TODO(siqek): check if the ball hits top or the bottom of the paddle

        // Finally move the ball after collision checks.
        ballCenter = ballDest;

        /*
        * ----- Ball crosses vertical wall  -----
        */
        const auto resolvePaddleVerticalWallCollision = [&]() -> void
        {
            ballCenter.x = c_WindowWidth / 2.f;
            ballCenter.y = c_WindowHeight / 2.f;

            // Choose a random direction for the ball.
            ballDirection = {
                .x = -1.f + ((std::rand() % 2) * 2.f),
                .y = 0.f
            };

            // Get random angle value from -15 deg up to +30 deg for ball direction.
            const float angleDeg = (-c_MaxInitialAngleDeg + std::rand() % (c_MaxInitialAngleDeg * 2 + 1));
            const float angleRad = angleDeg * DEG2RAD;
            ballDirection = Vector2Rotate(ballDirection, angleRad);
        };

        if (ballCenter.x < 0.f)
        {
            resolvePaddleVerticalWallCollision();

            ++enemyScore;
            enemyScoreText = std::to_string(enemyScore);
        }
        else if (ballCenter.x > c_WindowWidth)
        {
            resolvePaddleVerticalWallCollision();

            ++playerScore;
            playerScoreText = std::to_string(playerScore);
        }

        /*
        * ----- Render -----
        */
        BeginDrawing();

            ClearBackground(BLACK);

            // Draw dashed line across the window middle
            constexpr Vector2 c_DashedLineStart{ .x = c_WindowWidth / 2.f, .y = 0.f };
            constexpr Vector2 c_DashedLineEnd{ .x = c_WindowWidth / 2.f, .y = c_WindowHeight };
            DrawLineDashed(c_DashedLineStart, c_DashedLineEnd, 10.f, 10.f, RAYWHITE);

            // Draw ball.
            DrawCircleV(ballCenter, c_BallRadius, RAYWHITE);

            // Draw paddles.
            DrawRectangle(playerPaddle.x, playerPaddle.y, c_PaddleWidth, c_PaddleHeight, RAYWHITE);
            DrawRectangle(enemyPaddle.x, enemyPaddle.y, c_PaddleWidth, c_PaddleHeight, RAYWHITE);

            // Draw score texts.
            constexpr int c_TextPosY = c_WindowHeight / 10;
            constexpr int c_FontSize = 32;
            DrawText(enemyScoreText.c_str(), c_WindowWidth / 4 * 1, c_TextPosY, c_FontSize, RAYWHITE);
            DrawText(playerScoreText.c_str(), c_WindowWidth / 4 * 3, c_TextPosY, c_FontSize, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
