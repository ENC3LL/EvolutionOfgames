#include <SFML/Graphics.hpp>
#include <vector>

using namespace std;
using namespace sf;

//  ОТСКОК ОТ РАКЕТОК
// Проверка столкновения
bool checkCollision(const RectangleShape& paddle, const CircleShape& ball, float ballRadius)
{
    // Получаем границы ракетки и позицию шарика
    FloatRect paddleBounds = paddle.getGlobalBounds();
    Vector2f ballCenter = ball.getPosition();

    // Добавляем радиус, чтобы получить центр шарика
    ballCenter.x += ballRadius;
    ballCenter.y += ballRadius;

    float paddleLeft = paddleBounds.position.x;
    float paddleTop = paddleBounds.position.y;
    float paddleWidth = paddleBounds.size.x;
    float paddleHeight = paddleBounds.size.y;

    // Находим ближайшую точку на прямоугольнике к центру шара
    float closestX = max(paddleLeft, min(ballCenter.x, paddleLeft + paddleWidth));
    float closestY = max(paddleTop, min(ballCenter.y, paddleTop + paddleHeight));

    // Вычисляем расстояние
    float distanceX = ballCenter.x - closestX;
    float distanceY = ballCenter.y - closestY;
    float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);

    // Сравниваем с квадратом радиуса
    return distanceSquared < (ballRadius * ballRadius);
}

// Функция для создания пунктирной линии
vector<RectangleShape> createDashedLine(float startX, float startY, float endY, float dashSize = 10.0f, float gapSize = 10.0f)
{
    vector<RectangleShape> dashes;
    float currentY = startY;
    bool isDash = true; // Начинаем с отрисовки пунктира

    while (currentY < endY)
    {
        if (isDash)
        {
            RectangleShape dash(Vector2f(4.0f, dashSize)); // Тонкий прямоугольник
            dash.setFillColor(Color::White);
            dash.setPosition(Vector2f(startX - dash.getSize().x / 2.0f, currentY));
            dashes.push_back(dash);
        }
        currentY += isDash ? dashSize : gapSize;
        isDash = !isDash;
    }

    return dashes;
}

// Процедурная генерация цифр (семисегментный индикатор)
vector<RectangleShape> createDigit(float x, float y, float size, int digit)
{
    vector<RectangleShape> segments;

    // Нормализуем digit (0-9)
    digit = abs(digit % 10);

    // Параметры сегментов
    float segmentThickness = size * 0.15f; // Толщина сегмента
    float segmentLength = size * 0.6f;     // Длина горизонтальных сегментов

    // Позиции сегментов относительно (x, y)
    Vector2f positions[7] = {
        {x + segmentThickness, y},                              // a (верхний горизонтальный)
        {x + segmentThickness + segmentLength, y + segmentThickness}, // b (верхний правый вертикальный)
        {x + segmentThickness + segmentLength, y + segmentThickness + segmentLength}, // c (нижний правый вертикальный)
        {x + segmentThickness, y + 2 * segmentThickness + segmentLength}, // d (нижний горизонтальный)
        {x, y + segmentThickness + segmentLength},              // e (нижний левый вертикальный)
        {x, y + segmentThickness},                              // f (верхний левый вертикальный)
        {x + segmentThickness, y + segmentThickness + segmentLength}  // g (средний горизонтальный)
    };

    // Маски для цифр 0-9: какие сегменты должны быть активны
    // Порядок сегментов: a, b, c, d, e, f, g
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
            if (i == 0 || i == 3 || i == 6) // Горизонтальные сегменты (a, d, g)
            {
                segment = RectangleShape(Vector2f(segmentLength, segmentThickness));
            }
            else // Вертикальные сегменты (b, c, e, f)
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

// Функция для создания отображения счета (без ведущих нулей)
vector<RectangleShape> createScoreDisplay(float x, float y, float size, int score)
{
    vector<RectangleShape> allSegments;

    // Обрабатываем отрицательные числа (хотя в Pong они не нужны)
    bool negative = score < 0;
    score = abs(score);

    // Определяем, сколько цифр нужно отобразить
    int numDigits = 1;
    if (score >= 10) numDigits = 2;
    if (score >= 100) numDigits = 3; // На всякий случай

    // Позиция для первой цифры
    float digitSpacing = size * 0.8f;
    float startX = x;

    // Если число меньше 10, показываем только одну цифру
    if (score < 10)
    {
        startX += digitSpacing; // Сдвигаем вправо для центрирования
        numDigits = 1;
    }

    // Создаем цифры справа налево
    int currentScore = score;
    for (int i = numDigits - 1; i >= 0; i--)
    {
        int digit = currentScore % 10;
        currentScore /= 10;

        float digitX = startX + i * digitSpacing;
        auto digitSegments = createDigit(digitX, y, size, digit);

        // Добавляем все сегменты этой цифры
        allSegments.insert(allSegments.end(), digitSegments.begin(), digitSegments.end());
    }

    return allSegments;
}


int main()
{
    // Создание окна
    RenderWindow window(VideoMode({ 800, 600 }), "Pong Game");

    // Размеры ракеток
    Vector2f paddleSize(20.0f, 100.0f);
    float paddleSpeed = 200.0f; // Скорость

    // Левая ракетка 
    RectangleShape leftPaddle(paddleSize);
    leftPaddle.setFillColor(Color::White);
    leftPaddle.setPosition(Vector2f(20.0f /**X**/, 250.0f /**Y**/));

    // Правая ракетка
    RectangleShape rightPaddle(paddleSize);
    rightPaddle.setFillColor(Color::White);
    rightPaddle.setPosition(Vector2f(760.0f, 250.0f));

    //ШАРИК Создание и настройка
    float ballRadius = 10.0f;
    CircleShape ball(ballRadius);
    ball.setFillColor(Color::White);
    ball.setPosition(Vector2f(400.0f, 300.0f)); // Центр экрана

    // Скорость шарика (пикселей в секунду)
    Vector2f ballVelocity(200.0f, 150.0f);

    // Счет игроков
    int leftScore = 0;
    int rightScore = 0;

    // Создание пунктирной линии по центру
    vector<RectangleShape> centerDashes = createDashedLine(400.0f, 20.0f, 580.0f, 15.0f, 10.0f);

    // Создание счета из прямоугольников (без шрифта)
    vector<RectangleShape> leftScoreDisplay;
    vector<RectangleShape> rightScoreDisplay;

    // Обновление отображения счета
    auto updateScoreDisplay = [&]() {
        leftScoreDisplay = createScoreDisplay(320.0f, 30.0f, 35.0f, leftScore);
        rightScoreDisplay = createScoreDisplay(430.0f, 30.0f, 35.0f, rightScore);
        };

    updateScoreDisplay();

    // Часы для delta time
    Clock clock;
    window.setFramerateLimit(60); // fps

    // Главный цикл
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // Обработка событий
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

        // Управление левой ракеткой (W/S)
        if (Keyboard::isKeyPressed(Keyboard::Key::W) &&
            leftPaddle.getPosition().y > 0)
            leftPaddle.move(Vector2f(0.0f, -paddleSpeed * deltaTime));

        if (Keyboard::isKeyPressed(Keyboard::Key::S) &&
            leftPaddle.getPosition().y < 600 - paddleSize.y)
            leftPaddle.move(Vector2f(0.0f, paddleSpeed * deltaTime));

        // Управление правой ракеткой (Up/Down)
        if (Keyboard::isKeyPressed(Keyboard::Key::Up) &&
            rightPaddle.getPosition().y > 0)
            rightPaddle.move(Vector2f(0.0f, -paddleSpeed * deltaTime));

        if (Keyboard::isKeyPressed(Keyboard::Key::Down) &&
            rightPaddle.getPosition().y < 600 - paddleSize.y)
            rightPaddle.move(Vector2f(0.0f, paddleSpeed * deltaTime));

        // ДВИЖЕНИЕ ШАРИКА
        ball.move(Vector2f(ballVelocity.x * deltaTime, ballVelocity.y * deltaTime));

        // ОТСКОК ОТ ВЕРХНЕЙ И НИЖНЕЙ ГРАНИЦ
        if (ball.getPosition().y <= 0) // Верхняя граница
        {
            ball.setPosition(Vector2f(ball.getPosition().x, 0.0f));
            ballVelocity.y = abs(ballVelocity.y); // Уходим вниз
        }
        else if (ball.getPosition().y >= 600 - ballRadius * 2) // Нижняя граница
        {
            ball.setPosition(Vector2f(ball.getPosition().x, 600 - ballRadius * 2));
            ballVelocity.y = -abs(ballVelocity.y); // Уходим вверх
        }

        // Используем нашу функцию checkCollision
        // Проверка столкновения с левой ракеткой
        if (checkCollision(leftPaddle, ball, ballRadius))
        {
            ball.setPosition(Vector2f(
                leftPaddle.getPosition().x + paddleSize.x,
                ball.getPosition().y
            ));

            ballVelocity.x = abs(ballVelocity.x);
            float hitPosition = (ball.getPosition().y + ballRadius - leftPaddle.getPosition().y) / paddleSize.y;
            ballVelocity.y = (hitPosition - 0.5f) * 300.0f; // Увеличил для большей динамики
        }

        // Проверка столкновения с правой ракеткой
        if (checkCollision(rightPaddle, ball, ballRadius))
        {
            ball.setPosition(Vector2f(
                rightPaddle.getPosition().x - ballRadius * 2,
                ball.getPosition().y
            ));

            ballVelocity.x = -abs(ballVelocity.x);
            float hitPosition = (ball.getPosition().y + ballRadius - rightPaddle.getPosition().y) / paddleSize.y;
            ballVelocity.y = (hitPosition - 0.5f) * 300.0f; // Увеличил для большей динамики
        }

        // СБРОС ШАРИКА ПРИ ВЫЛЕТЕ ЗА ГРАНИЦЫ И НАЧИСЛЕНИЕ ОЧКОВ
        if (ball.getPosition().x < -50) // Шарик улетел влево - очко правого игрока
        {
            rightScore++;
            updateScoreDisplay();

            // Возвращаем шарик в центр
            ball.setPosition(Vector2f(400.0f, 300.0f));
            // Случайное направление вправо
            ballVelocity = Vector2f(
                250.0f, // Всегда вправо
                (rand() % 201 - 100) // Случайная вертикальная скорость от -100 до 100
            );
        }
        else if (ball.getPosition().x > 850) // Шарик улетел вправо - очко левого игрока
        {
            leftScore++;
            updateScoreDisplay();

            // Возвращаем шарик в центр
            ball.setPosition(Vector2f(400.0f, 300.0f));
            // Случайное направление влево
            ballVelocity = Vector2f(
                -250.0f, // Всегда влево
                (rand() % 201 - 100) // Случайная вертикальная скорость от -100 до 100
            );
        }

        // Отрисовка
        window.clear(Color::Black);

        // Отрисовка пунктирной линии
        for (const auto& dash : centerDashes)
        {
            window.draw(dash);
        }

        // Отрисовка счета (цифры из прямоугольников)
        for (const auto& segment : leftScoreDisplay)
        {
            window.draw(segment);
        }
        for (const auto& segment : rightScoreDisplay)
        {
            window.draw(segment);
        }

        // Отрисовка ракеток и шарика
        window.draw(leftPaddle);
        window.draw(rightPaddle);
        window.draw(ball);

        window.display();
    }

    return 0;
}
