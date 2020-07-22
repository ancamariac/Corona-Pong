#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <math.h>
#include <sstream>

#define PI   3.141592

using namespace sf;

//TODO : add cough sounds

// collision between 2 rectangles
bool intersects(const RectangleShape& rect1, const RectangleShape& rect2)
{
    FloatRect r1 = rect1.getGlobalBounds();
    FloatRect r2 = rect2.getGlobalBounds();
    return r1.intersects(r2);
}

// collision between a rectangle and a sprite
bool collides(const RectangleShape& rect, const Sprite& ball)
{
    FloatRect r = rect.getGlobalBounds();
    FloatRect b = ball.getGlobalBounds();
    return r.intersects(b);
}

float clamp(const float x, const float a, const float b)
{
    return std::min(std::max(a, x), b);
}

class Game
{
private:
    static const int FRAMES_PER_SECOND = 60;
    static const int MAX_FRAMESKIP = 10;
    static const int width = 640;
    static const int height = 480;
    static const int borderSize = 12;
    static const int margin = 50;
    static const int moveDistance = 20;


    RenderWindow window;
    Font font;

    RectangleShape top;
    RectangleShape left;
    RectangleShape right;
    RectangleShape bottom;

    Texture texture;
    Sprite ball;

    Vector2f ballSpeed;

    RectangleShape player1;
    RectangleShape player2;

    RectangleShape middleLine;

    Text title;
    Text start;
    Text won1;
    Text won2;
    Text lost1;
    Text lost2;
    Text score;
    Text fps;

    Time time;
    Int32 updateTime;

    unsigned int p1Score, p2Score;

    enum states { INTRO, SINGLEPLAYER, MULTIPLAYER, P1WON, P1LOST };

    int gameState;

public:
    Game()
    {
    }

    bool init()
    {
        VideoMode videoMode(width, height);
        window.create(videoMode, "Corona Pong");
        window.setVerticalSyncEnabled(true);
        window.setFramerateLimit(FRAMES_PER_SECOND);

        if (!font.loadFromFile("assets/tomb.otf"))
            return false;

        setup();
        return true;
    }

    int exec()
    {
        Clock renderClock, updateClock;
        while (window.isOpen())
        {
            time = renderClock.getElapsedTime();
            float fFps = 1000000 / time.asMicroseconds();
            std::stringstream s;
            s << fFps << " fps";
            fps.setString(s.str());
            renderClock.restart();

            const Int64 frameTime = 1000000 / FRAMES_PER_SECOND;
            Clock c;
            Time t = c.getElapsedTime();
            Int64 nextFrameTime = t.asMicroseconds() + frameTime;

            int loops = 0;
            while (t.asMicroseconds() < nextFrameTime && loops < MAX_FRAMESKIP)
            {
                processEvents();
                updateTime = updateClock.restart().asMilliseconds();
                update();
                t = c.getElapsedTime();
                loops++;
            }

            display();
        }

        return EXIT_SUCCESS;
    }

private:

    void processEvents()
    {
        Event event;
        while (window.pollEvent(event))
        {
            if ((event.type == Event::Closed) ||
                ((event.type == Event::KeyPressed) && (event.key.code == Keyboard::Escape)))
                window.close();
            else
                if ((event.type == Event::KeyPressed) && (event.key.code == Keyboard::S) && (gameState == INTRO))
                    gameState = SINGLEPLAYER;
                else
                    if ((event.type == Event::KeyPressed) && (event.key.code == Keyboard::M) && (gameState == INTRO))
                        gameState = MULTIPLAYER;
        }
    }

    void update()
    {
        if (gameState != SINGLEPLAYER && gameState != MULTIPLAYER)
            return;

        if (gameState == SINGLEPLAYER)
        {
            updatePlayer1();
            updateBotPlayer();
            checkCollisions();
            updateBall();
        }
        else
            if (gameState == MULTIPLAYER)
            {
                updatePlayer1();
                updatePlayer2();
                checkCollisions();
                updateBall();
            }

        // detect if game is over
        if (p1Score >= 10 && p1Score >= p2Score + 2)
            gameState = P1WON;
        if (p2Score >= 10 && p2Score >= p1Score + 2)
            gameState = P1LOST;
    }

    void setup()
    {
        top.setPosition(-2 * borderSize, 0);
        top.setSize(Vector2f(width + 4 * borderSize, borderSize));

        left.setPosition(-borderSize * 3, 0);
        left.setSize(Vector2f(borderSize, height));

        right.setPosition(width + 2 * borderSize, 0);
        right.setSize(Vector2f(borderSize, height));

        bottom.setPosition(-2 * borderSize, height - borderSize);
        bottom.setSize(Vector2f(width + 4 * borderSize, borderSize));

        top.setFillColor(Color(100, 100, 100));
        top.setOutlineColor(Color::Blue);
        top.setOutlineThickness(3);

        left.setFillColor(Color(100, 100, 100));
        left.setOutlineColor(Color::Blue);
        left.setOutlineThickness(3);

        right.setFillColor(Color(100, 100, 100));
        right.setOutlineColor(Color::Blue);
        right.setOutlineThickness(3);

        bottom.setFillColor(Color(100, 100, 100));
        bottom.setOutlineColor(Color::Blue);
        bottom.setOutlineThickness(3);

        texture.loadFromFile("assets/ball.png");
        ball.setTexture(texture);
        FloatRect bounds = ball.getLocalBounds();
        ball.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        ball.setPosition(width / 2, height / 2);

        player1.setSize(Vector2f(borderSize, 90));
        player1.setPosition(margin - borderSize, height / 2 - 25);
        player1.setFillColor(Color(0, 122, 245));
        player1.setOutlineColor(Color::Red);
        player1.setOutlineThickness(3);

        player2.setSize(Vector2f(borderSize, 90));
        player2.setPosition(width - margin, height / 2 - 25);
        player2.setFillColor(Color(0, 122, 245));
        player2.setOutlineColor(Color::Red);
        player2.setOutlineThickness(3);

        middleLine.setFillColor(Color(100, 100, 100, 10));
        middleLine.setOutlineColor(Color(100, 100, 100, 30));
        middleLine.setOutlineThickness(2);
        middleLine.setPosition(width / 2, 0);
        middleLine.setSize(Vector2f(0, height));

        title.setString("Corona Pong");
        title.setFont(font);
        title.setCharacterSize(50);
        title.setPosition(width / 2 - title.getGlobalBounds().width / 2, 100);
        title.setFillColor(Color::Blue);

        start.setString("Press S for singleplayer or M for multiplayer");
        start.setFont(font);
        start.setCharacterSize(30);
        start.setPosition(width / 2 - start.getGlobalBounds().width / 2, 400);
        start.setFillColor(Color::Red);

        won1.setString("Player1 has won this game.\n\n");
        won1.setFont(font);
        won1.setCharacterSize(20);
        won1.setPosition(width / 2 - won1.getGlobalBounds().width / 2, height / 2 - won1.getGlobalBounds().height / 2);
        won1.setFillColor(Color::Green);

        won2.setString("Player2 has won this game.\n\n");
        won2.setFont(font);
        won2.setCharacterSize(20);
        won2.setPosition(width / 2 - won2.getGlobalBounds().width / 2, height / 2 - won2.getGlobalBounds().height / 2);
        won2.setFillColor(Color::Green);

        lost1.setString("We wish Player1 a speedy recovery!");
        lost1.setFont(font);
        lost1.setCharacterSize(20);
        lost1.setPosition(width / 2 - lost1.getGlobalBounds().width / 2, height / 2 - lost1.getGlobalBounds().height / 2);
        lost1.setFillColor(Color::Red);

        lost2.setString("We wish Player2 a speedy recovery!");
        lost2.setFont(font);
        lost2.setCharacterSize(20);
        lost2.setPosition(width / 2 - lost2.getGlobalBounds().width / 2, height / 2 - lost2.getGlobalBounds().height / 2);
        lost2.setFillColor(Color::Red);

        score.setString("0   0");
        score.setFont(font);
        score.setCharacterSize(40);
        score.setPosition(width / 2 - score.getGlobalBounds().width / 2, 40);
        score.setFillColor(Color::Blue);

        fps.setString("0");
        fps.setFont(font);
        fps.setCharacterSize(30);
        fps.setPosition(fps.getGlobalBounds().width / 2, 40);
        fps.setFillColor(Color(52, 0, 100, 50));

        resetGame1();
        p1Score = 0;
        p2Score = 0;
        gameState = INTRO;
    }

    void display()
    {
        window.clear(Color::Black);

        switch (gameState)
        {
        case INTRO:
            window.draw(title);
            window.draw(start);
            break;
        case SINGLEPLAYER:
            window.draw(middleLine);
            window.draw(left);
            window.draw(right);
            window.draw(player1);
            window.draw(player2);
            window.draw(ball);
            window.draw(score);
            window.draw(top);
            window.draw(bottom);
            window.draw(fps);
            break;
        case MULTIPLAYER:
            window.draw(middleLine);
            window.draw(left);
            window.draw(right);
            window.draw(player1);
            window.draw(player2);
            window.draw(ball);
            window.draw(score);
            window.draw(top);
            window.draw(bottom);
            window.draw(fps);
            break;
        case P1WON:
            window.draw(won1);
            window.draw(lost2);
            break;
        case P1LOST:
            window.draw(won2);
            window.draw(lost1);
            break;
        }

        window.display();
    }

    void updatePlayer1()
    {
        // move player 1 pad
        if (Keyboard::isKeyPressed(Keyboard::W))
        {
            player1.move(0, -moveDistance * updateTime / 50.0);
        }
        else
            if (Keyboard::isKeyPressed(Keyboard::S))
            {
                player1.move(0, moveDistance * updateTime / 50.0);
            }
    }

    void updatePlayer2()
    {
        // move player 2 pad
        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            player2.move(0, -moveDistance * updateTime / 50.0);
        }
        else
            if (Keyboard::isKeyPressed(Keyboard::Down))
            {
                player2.move(0, moveDistance * updateTime / 50.0);
            }

    }

    void updateBotPlayer()
    {
        // auto move player2 pad
        const float sizeOfBallY = ball.getTexture()->getSize().y * ball.getScale().y;
        if (ball.getPosition().y < player2.getPosition().y)
        {
            player2.move(0, -moveDistance * updateTime / 40.0);
        }
        else if (ball.getPosition().y + sizeOfBallY > player2.getPosition().y + player2.getSize().y)
        {
            player2.move(0, moveDistance * updateTime / 40.0);
        }
    }

    void updateBall()
    {
        ball.move(ballSpeed.x * updateTime, ballSpeed.y * updateTime);
    }

    void checkCollisions()
    {

        //block players pad inside the play area
        if (intersects(player1, bottom) || intersects(player1, top))
        {
            FloatRect t = top.getGlobalBounds();
            FloatRect b = bottom.getGlobalBounds();
            Vector2f p = player1.getPosition();
            p.y = clamp(p.y, t.top + t.height + 5, b.top - player1.getSize().y - 5);
            player1.setPosition(p);
        }
        if (intersects(player2, bottom) || intersects(player2, top))
        {
            FloatRect t = top.getGlobalBounds();
            FloatRect b = bottom.getGlobalBounds();
            Vector2f p = player2.getPosition();
            p.y = clamp(p.y, t.top + t.height + 5, b.top - player2.getSize().y - 5);
            player2.setPosition(p);
        }

        // ball collides with top and bottom
        if (collides(top, ball))
        {
            FloatRect t = top.getGlobalBounds();
            FloatRect b = ball.getGlobalBounds();
            ballSpeed.y *= -1;
            int u = t.top + t.height - b.top;
            ball.move(0, 2 * u);
        }
        if (collides(bottom, ball))
        {
            FloatRect bot = bottom.getGlobalBounds();
            FloatRect b = ball.getGlobalBounds();
            ballSpeed.y *= -1;
            int u = bot.top - b.height - b.top;
            ball.move(0, 2 * u);
        }
        // ball collides with player1 and player2
        if (collides(player1, ball))
        {
            FloatRect p = player1.getGlobalBounds();
            FloatRect b = ball.getGlobalBounds();

            //let o be the center of p
            Vector2f o = Vector2f(p.left + p.width / 2, p.top + p.height / 2);
            //om: vector from o to the center of the ball
            Vector2f om = Vector2f(b.left + b.width / 2 - o.x, b.top + b.height / 2 - o.y);
            // let's scale om to square dimensions and act as if p is a square
            om.x /= p.width;
            om.y /= p.height;
            // reflect the ball according to the angle of om
            float angle = atan2(om.y, om.x);
            if (abs(angle) < PI / 2) //right - PI/4 is restrictive
            {
                ballSpeed.x = abs(ballSpeed.x);//in case of double contact
                ballSpeed.y = (b.top + b.height / 2 - p.top - p.height / 2) / 100;
                int u = p.left + p.width - b.left;
                b.left = p.left + p.width + u;
                ball.setPosition(b.left, b.top);
                //increase ball speed by 2%
                ballSpeed.x *= 1.02f;
                ballSpeed.y *= 1.02f;
            }
        }
        if (collides(player2, ball))
        {
            FloatRect p = player2.getGlobalBounds();
            FloatRect b = ball.getGlobalBounds();

            //let o be the center of p
            Vector2f o = Vector2f(p.left + p.width / 2, p.top + p.height / 2);
            //om: vector from o to the center of the ball
            Vector2f om = Vector2f(b.left + b.width / 2 - o.x, b.top + b.height / 2 - o.y);
            // let's scale om to square dimensions and act as if p is a square
            om.x /= p.width;
            om.y /= p.height;
            // reflect the ball according to the angle of om
            float angle = atan2(om.y, om.x);
            if (abs(angle) > PI / 2) //left - 3 * PI/4 is restrictive
            {
                ballSpeed.x = -abs(ballSpeed.x);//in case of double contact
                ballSpeed.y = (b.top + b.height / 2 - p.top - p.height / 2) / 100;
                int u = b.left + b.width - p.left;
                b.left = p.left - b.width - u;
                ball.setPosition(b.left, b.top);
                //increase ball speed by 2%
                ballSpeed.x *= 1.02f;
                ballSpeed.y *= 1.02f;
            }
        }

        // check for scoring
        if (collides(left, ball))
        {
            p2Score++;
            std::stringstream str;
            str << p1Score << "   " << p2Score;
            score.setString(str.str());
            score.setPosition(width / 2 - score.getGlobalBounds().width / 2, 40);
            resetGame2();
        }
        if (collides(right, ball))
        {
            p1Score++;
            std::stringstream str;
            str << p1Score << "   " << p2Score;
            score.setString(str.str());
            score.setPosition(width / 2 - score.getGlobalBounds().width / 2, 40);
            resetGame1();
        }

    }

    void resetGame1()
    {
        FloatRect p = player1.getGlobalBounds();
        FloatRect b = ball.getGlobalBounds();
        ball.setPosition(p.left + p.width + 5, height / 2);
        ballSpeed.x = 0.3f;
        ballSpeed.y = 0.3f;
    }

    void resetGame2()
    {
        FloatRect p = player2.getGlobalBounds();
        FloatRect b = ball.getGlobalBounds();
        ball.setPosition(p.left - b.width - 5, height / 2);
        ballSpeed.x = -0.3f;
        ballSpeed.y = 0.3f;
    }
};


int main()
{
    Game game;
    if (!game.init())
        return EXIT_FAILURE;
    return game.exec();
}