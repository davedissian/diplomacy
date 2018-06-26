#pragma once

class World;

struct RenderContext
{
	sf::RenderWindow* window;
	World* world;
	sf::Font font;
};