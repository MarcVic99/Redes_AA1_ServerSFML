#pragma once
#include <SFML/System.hpp>

class TimeManager {
public:
	void startFrame() {
		float frame = clock.restart().asSeconds();
		if (frame > maxFrame) frame = maxFrame;
		deltaTime = frame;
		accumulator += frame;
	}
	float dt() const { return deltaTime; }
	float fixedStep() const { return fixedDt; }
	bool shouldRunFixed() const { return accumulator >= fixedDt; }
	void consumeFixedStep() { accumulator -= fixedDt; }
private:
	sf::Clock clock;
	float deltaTime = 0.f;
	float accumulator = 0.f;
	const float fixedDt = 1.f / 60.f;
	const float maxFrame = 0.1f;
};
