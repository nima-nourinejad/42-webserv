#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
	window.setMouseCursorVisible(false);
    sf::RectangleShape player(sf::Vector2f(20, 20));
	sf::RectangleShape ball(sf::Vector2f(10, 10));
	sf::RectangleShape goal(sf::Vector2f(20, 20));
    player.setFillColor(sf::Color::Green);
	ball.setFillColor(sf::Color::Red);
	goal.setFillColor(sf::Color::Blue);
	ball.setPosition(110, 110);
	player.setPosition(90, 180);
	goal.setPosition(85, 0);
	sf::Font font;
	font.loadFromFile("FromCartoonBlocks.ttf");
	sf::Text text;
	text.setFont(font);
	text.setString("Score : 0");
	int score = 0;
	text.setCharacterSize(48);
	text.setPosition(0, 100);
	bool goalCounted = false;
	float xVelocity = 1;
	float yVelocity = 1;
	float goalVelocity = 0.65f;
    while (window.isOpen())
    {
		window.setFramerateLimit(60);
		float xMouse = sf::Mouse::getPosition(window).x;
		float yMouse = sf::Mouse::getPosition(window).y;
		
		if (xMouse > player.getPosition().x && player.getPosition().x < 180)
			player.move(1, 0);
		if (xMouse < player.getPosition().x && player.getPosition().x > 0)
			player.move(-1, 0);
		if (yMouse > player.getPosition().y && player.getPosition().y < 180)
			player.move(0, 1);
		if (yMouse < player.getPosition().y && player.getPosition().y > 0)
			player.move(0, -1);
		if (player.getGlobalBounds().intersects(ball.getGlobalBounds()))
		{
			if (player.getPosition().x < ball.getPosition().x)
				xVelocity = 1.5f;
			if (player.getPosition().x > ball.getPosition().x)
				xVelocity = -1.5f;
			if (player.getPosition().y < ball.getPosition().y)
				yVelocity = 1.5f;
			if (player.getPosition().y > ball.getPosition().y)
				yVelocity = -1.5f;
		}
		if (ball.getPosition().x + xVelocity > 170 || ball.getPosition().x + xVelocity < 20)
			xVelocity = -xVelocity;
		ball.move(xVelocity, 0);
		if (goal.getPosition().x + xVelocity > 170 || goal.getPosition().x + xVelocity < 20)
			goalVelocity = -goalVelocity;
		goal.move(goalVelocity, 0);
		if (ball.getPosition().y + yVelocity > 190|| ball.getPosition().y + yVelocity< 0)
			yVelocity = -yVelocity;
		ball.move(0, yVelocity);
		if (ball.getGlobalBounds().intersects(goal.getGlobalBounds()) && !goalCounted)
		{
			goalCounted = true;
			score++;
			text.setString("Score : " + std::to_string(score));
		}
		if (!ball.getGlobalBounds().intersects(goal.getGlobalBounds()) && goalCounted)
			goalCounted = false;
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
			if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
				window.close();
        }

        window.clear();
		window.draw(goal);
        window.draw(player);
		window.draw(ball);
		window.draw(text);
        window.display();
    }

    return 0;
}
-----------------------------264134442740479456584250737924--
