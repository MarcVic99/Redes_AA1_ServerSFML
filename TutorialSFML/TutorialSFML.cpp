//// TutorialSFML.cpp : This file contains the 'main' function. Program execution begins and ends there.
////
//#include <iostream>
//#include <SFML/Graphics.hpp>
//#include <SFML/Audio.hpp>
//#include "Source.cpp"
//
//#define WIDTH 1280
//#define HEIGHT 720
//
//#define FRAME_WIDTH 96
//#define FRAME_HEIGHT 101
//#define NUM_FRAMES 10
//#define ANIMATION_SPEED 0.1f
//
//void HandleEvent(const sf::Event& event, sf::RenderWindow& window)
//{
//	if (event.is<sf::Event::Closed>())
//	{
//		window.close();
//	}
//
//	if (const sf::Event::KeyPressed* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
//		switch (keyPressed->code)
//		{
//			case sf::Keyboard::Key::Escape:
//				window.close();
//				break;
//		}
//	}
//
//	if (const sf::Event::MouseButtonPressed* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
//		switch (mousePressed->button)
//		{
//		case sf::Mouse::Button::Left:
//			std::cout << "Left button pressed at: " << mousePressed->position.x << ", " << mousePressed->position.y << std::endl;
//			break;
//		}
//	}
//}
//
//void Render(sf::RenderWindow& window, /*sf::RectangleShape& square*/ sf::Sprite& animatedSprite) {
//	window.clear(sf::Color(0x000000FF));
//	//window.draw(square);
//	window.draw(animatedSprite);
//	window.display();
//}
//
//sf::Texture LoadSpriteSheet(const std::string& filePath)
//{
//	sf::Texture texture;
//	if (!texture.loadFromFile(filePath)) {
//		std::cerr << "Error loading sprite sheet!" << std::endl;
//		return texture;
//	}
//	return texture;
//}
//
//void UpdateSprite(sf::Sprite& animatedSprite, int& currentFrame, float& deltaTimeAnimation) {
//	if (deltaTimeAnimation >= ANIMATION_SPEED)
//	{
//		currentFrame = (currentFrame + 1) % NUM_FRAMES;
//		animatedSprite.setTextureRect(sf::IntRect({ currentFrame * FRAME_WIDTH, 0 }, { FRAME_WIDTH, FRAME_HEIGHT }));
//		deltaTimeAnimation = 0.f;
//	}
//}
//
//int main()
//{
//	Source();
//	//sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode({ WIDTH, HEIGHT }), "Tutorial SFML");
//
//	//sf::Clock deltaTimeClock;
//	//float deltaTimeAnimation = 0.f;
//	//int currentFrame = 0;
//
//	//sf::RectangleShape square;
//	//square.setSize({ 100.f, 100.f });
//	//square.setFillColor(sf::Color::Green);
//	//square.setPosition({ WIDTH / 2.f - square.getSize().x / 2, HEIGHT / 2.f - square.getSize().y / 2 });
//
//	//sf::Texture spriteSheetTexture = LoadSpriteSheet("Assets/Spritesheets/S_Link.png");
//	//sf::Sprite animatedSprite = sf::Sprite(spriteSheetTexture);
//
//	//sf::SoundBuffer buffer("Assets/Sounds/FX_Quack.wav");
//	//sf::Sound sound(buffer);
//	//sound.play();
//
//	//while (window->isOpen())
//	//{
//	//	float deltaTime = deltaTimeClock.restart().asSeconds();
//	//	deltaTimeAnimation += deltaTime;
//
//	//	while (const std::optional event = window->pollEvent())
//	//	{
//	//		HandleEvent(*event, *window);
//	//	}
//	//	UpdateSprite(animatedSprite, currentFrame, deltaTimeAnimation);
//	//	Render(*window, /*square*/ animatedSprite);
//	//}	
//
//	//delete window;
//}
//
//// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
//// Debug program: F5 or Debug > Start Debugging menu
//
//// Tips for Getting Started: 
////   1. Use the Solution Explorer window to add/manage files
////   2. Use the Team Explorer window to connect to source control
////   3. Use the Output window to see build output and other messages
////   4. Use the Error List window to view errors
////   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
////   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
