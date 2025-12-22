#include <SFML/Graphics.hpp>
#include <vector>

using namespace std;
using namespace sf;

// BOUNCING OFF PADDLES
// Collision check
bool checkCollision(const RectangleShape& paddle, const CircleShape& ball, float ballRadius)
{
    // Get paddle bounds and ball position
    FloatRect paddleBounds = paddle.getGlobalBounds();
    Vector2f ballCenter = ball.getPosition();

    // Add radius to get ball's center
    ballCenter.x += ballRadius;
    ballCenter.y += ballRadius;

    float paddleLeft = paddleBounds.position.x;
    float paddleTop = paddleBounds.position.y;
    float paddleWidth = paddleBounds.size.x;
    float paddleHeight = paddleBounds.size.y;

    // Find closest point on rectangle to ball's center
    float closestX = max(paddleLeft, min(ballCenter.x, paddleLeft + paddleWidth));
    float closestY = max(paddleTop, min(ballCenter.y, paddleTop + paddleHeight));

    // Calculate distance
    float distanceX = ballCenter.x - closestX;
    float distanceY = ballCenter.y - closestY;
    float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);

    // Compare with squared radius
    return distanceSquared < (ballRadius * ballRadius);
}

// Function to create dashed line
vector<RectangleShape> createDashedLine(float startX, float startY, float endY, float dashSize = 10.0f, float gapSize = 10.0f)
{
    vector<RectangleShape> dashes;
    float currentY = startY;
    bool isDash = true; // Start with dash segment

    while (currentY < endY)
    {
        if (isDash)
        {
            RectangleShape dash(Vector2f(4.0f, dashSize)); // Thin rectangle
            dash.setFillColor(Color::White);
            dash.setPosition(Vector2f(startX - dash.getSize().x / 2.0f, currentY));
            dashes.push_back(dash);
        }
        currentY += isDash ? dashSize : gapSize;
        isDash = !isDash;
    }

    return dashes;
}

// Procedural digit generation (seven-segment display)
vector<RectangleShape> createDigit(float x, float y, float size, int digit)
{
    vector<RectangleShape> segments;

    // Normalize digit (0-9)
    digit = abs(digit % 10);

    // Segment parameters
    float segmentThickness = size * 0.15f; // Segment thickness
    float segmentLength = size * 0.6f;     // Horizontal segment length

    // Segment positions relative to (x, y)
    Vector2f positions[7] = {
        {x + segmentThickness, y},                              // a (top horizontal)
        {x + segmentThickness + segmentLength, y + segmentThickness}, // b (top-right vertical)
        {x + segmentThickness + segmentLength, y + segmentThickness + segmentLength}, // c (bottom-right vertical)
        {x + segmentThickness, y + 2 * segmentThickness + segmentLength}, // d (bottom horizontal)
        {x, y + segmentThickness + segmentLength},              // e (bottom-left vertical)
        {x, y + segmentThickness},                              // f (top-left vertical)
        {x + segmentThickness, y + segmentThickness + segmentLength}  // g (middle horizontal)
    };

    // Digit masks (0-9): which segments should be active
    // Segment order: a, b, c, d, e, f, g
    bool digitMasks[10][7] = {
        {true,  true,  true,  true,  true,  true,  false}, // 0
        {false, true,  true,  false, false, false, false}, // 1
        {true,  true,  false, true,  true,  false, true},  // 2
        {true,  true,  true,  true,  false, false, true},  // 3
        {false, true,  true,  false, false, true,  true},  // 4
        {true,  false, true,  true,  false, true,  true},  // 5
        {true,  false, true,  true,  true,  true,  true},  // 6
        {true,  true,  true,  false, false, false, false}, // 7
        {true,  true,  true,  true,  true,  true,  true},  // 8
        {true,  true,  true,  true,  false, true,  true}   // 9
    };

    for (int i = 0; i < 7; i++)
    {
        if (digitMasks[digit][i])
        {
            RectangleShape segment;
            if (i == 0 || i == 3 || i == 6) // Horizontal segments (a, d, g)
            {
                segment = RectangleShape(Vector2f(segmentLength, segmentThickness));
            }
            else // Vertical segments (b, c, e, f)
            {
                segment = RectangleShape(Vector2f(segmentThickness, segmentLength));
            }
            segment.setFillColor(Color::White);
            segment.setPosition(positions[i]);
            segments.push_back(segment);
        }
    }

    return segments;
}

// Create score display (no leading zeros)
vector<RectangleShape> createScoreDisplay(float x, float y, float size, int score)
{
    vector<RectangleShape> allSegments;

    // Handle negative numbers (not needed in Pong but for completeness)
    bool negative = score < 0;
    score = abs(score);

    // Determine number of digits to display
    int numDigits = 1;
    if (score >= 10) numDigits = 2;
    if (score >= 100) numDigits = 3; // Just in case

    // Position for first digit
    float digitSpacing = size * 0.8f;
    float startX = x;

    // For single-digit scores, shift right for centering
    if (score < 10)
    {
        startX += digitSpacing;
        numDigits = 1;
    }

    // Create digits from right to left
    int currentScore = score;
    for (int i = numDigits - 1; i >= 0; i--)
    {
        int digit = currentScore % 10;
        currentScore /= 10;

        float digitX = startX + i * digitSpacing;
        auto digitSegments = createDigit(digitX, y, size, digit);

        // Add all segments of this digit
        allSegments.insert(allSegments.end(), digitSegments.begin(), digitSegments.end());
    }

    return allSegments;
}


int main()
{
    // Create window
    RenderWindow window(VideoMode({ 800, 600 }), "Pong Game");

    // Paddle dimensions
    Vector2f paddleSize(20.0f, 100.0f);
    float paddleSpeed = 200.0f; // Speed in pixels/second

    // Left paddle 
    RectangleShape leftPaddle(paddleSize);
    leftPaddle.setFillColor(Color::White);
    leftPaddle.setPosition(Vector2f(20.0f /**X**/, 250.0f /**Y**/));

    // Right paddle
    RectangleShape rightPaddle(paddleSize);
    rightPaddle.setFillColor(Color::White);
    rightPaddle.setPosition(Vector2f(760.0f, 250.0f));

    // BALL: Creation and setup
    float ballRadius = 10.0f;
    CircleShape ball(ballRadius);
    ball.setFillColor(Color::White);
    ball.setPosition(Vector2f(400.0f, 300.0f)); // Screen center

    // Ball velocity (pixels per second)
    Vector2f ballVelocity(200.0f, 150.0f);

    // Player scores
    int leftScore = 0;
    int rightScore = 0;

    // Create center dashed line
    vector<RectangleShape> centerDashes = createDashedLine(400.0f, 20.0f, 580.0f, 15.0f, 10.0f);

    // Create score displays using rectangles (no fonts)
    vector<RectangleShape> leftScoreDisplay;
    vector<RectangleShape> rightScoreDisplay;

    // Update score display
    auto updateScoreDisplay = [&]() {
        leftScoreDisplay = createScoreDisplay(320.0f, 30.0f, 35.0f, leftScore);
        rightScoreDisplay = createScoreDisplay(430.0f, 30.0f, 35.0f, rightScore);
        };

    updateScoreDisplay();

    // Clock for delta time
    Clock clock;
    window.setFramerateLimit(60); // FPS limit

    // Main loop
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // Event handling
        while (const optional<Event> event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
                window.close();

            if (const auto* keyEvent = event->getIf<Event::KeyPressed>())
            {
                if (keyEvent->code == Keyboard::Key::Escape)
                    window.close();
            }
        }

        // Left paddle controls (W/S)
        if (Keyboard::isKeyPressed(Keyboard::Key::W) &&
            leftPaddle.getPosition().y > 0)
            leftPaddle.move(Vector2f(0.0f, -paddleSpeed * deltaTime));

        if (Keyboard::isKeyPressed(Keyboard::Key::S) &&
            leftPaddle.getPosition().y < 600 - paddleSize.y)
            leftPaddle.move(Vector2f(0.0f, paddleSpeed * deltaTime));

        // Right paddle controls (Up/Down)
        if (Keyboard::isKeyPressed(Keyboard::Key::Up) &&
            rightPaddle.getPosition().y > 0)
            rightPaddle.move(Vector2f(0.0f, -paddleSpeed * deltaTime));

        if (Keyboard::isKeyPressed(Keyboard::Key::Down) &&
            rightPaddle.getPosition().y < 600 - paddleSize.y)
            rightPaddle.move(Vector2f(0.0f, paddleSpeed * deltaTime));

        // BALL MOVEMENT
        ball.move(Vector2f(ballVelocity.x * deltaTime, ballVelocity.y * deltaTime));

        // BOUNCE OFF TOP AND BOTTOM BOUNDARIES
        if (ball.getPosition().y <= 0) // Top boundary
        {
            ball.setPosition(Vector2f(ball.getPosition().x, 0.0f));
            ballVelocity.y = abs(ballVelocity.y); // Move downward
        }
        else if (ball.getPosition().y >= 600 - ballRadius * 2) // Bottom boundary
        {
            ball.setPosition(Vector2f(ball.getPosition().x, 600 - ballRadius * 2));
            ballVelocity.y = -abs(ballVelocity.y); // Move upward
        }

        // Use our collision check function
        // Check collision with left paddle
        if (checkCollision(leftPaddle, ball, ballRadius))
        {
            ball.setPosition(Vector2f(
                leftPaddle.getPosition().x + paddleSize.x,
                ball.getPosition().y
            ));

            ballVelocity.x = abs(ballVelocity.x);
            float hitPosition = (ball.getPosition().y + ballRadius - leftPaddle.getPosition().y) / paddleSize.y;
            ballVelocity.y = (hitPosition - 0.5f) * 300.0f; // Increased for more dynamics
        }

        // Check collision with right paddle
        if (checkCollision(rightPaddle, ball, ballRadius))
        {
            ball.setPosition(Vector2f(
                rightPaddle.getPosition().x - ballRadius * 2,
                ball.getPosition().y
            ));

            ballVelocity.x = -abs(ballVelocity.x);
            float hitPosition = (ball.getPosition().y + ballRadius - rightPaddle.getPosition().y) / paddleSize.y;
            ballVelocity.y = (hitPosition - 0.5f) * 300.0f; // Increased for more dynamics
        }

        // RESET BALL ON OUT-OF-BOUNDS AND UPDATE SCORES
        if (ball.getPosition().x < -50) // Ball went left - point for right player
        {
            rightScore++;
            updateScoreDisplay();

            // Reset ball to center
            ball.setPosition(Vector2f(400.0f, 300.0f));
            // Random direction to the right
            ballVelocity = Vector2f(
                250.0f, // Always right
                (rand() % 201 - 100) // Random vertical speed from -100 to 100
            );
        }
        else if (ball.getPosition().x > 850) // Ball went right - point for left player
        {
            leftScore++;
            updateScoreDisplay();

            // Reset ball to center
            ball.setPosition(Vector2f(400.0f, 300.0f));
            // Random direction to the left
            ballVelocity = Vector2f(
                -250.0f, // Always left
                (rand() % 201 - 100) // Random vertical speed from -100 to 100
            );
        }

        // RENDERING
        window.clear(Color::Black);

        // Draw center dashed line
        for (const auto& dash : centerDashes)
        {
            window.draw(dash);
        }

        // Draw scores (rectangle-based digits)
        for (const auto& segment : leftScoreDisplay)
        {
            window.draw(segment);
        }
        for (const auto& segment : rightScoreDisplay)
        {
            window.draw(segment);
        }

        // Draw paddles and ball
        window.draw(leftPaddle);
        window.draw(rightPaddle);
        window.draw(ball);

        window.display();
    }

    return 0;
}
