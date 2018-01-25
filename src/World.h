#pragma once

#include "Map.h"
#include "State.h"
#include "gameplay/Unit.h"

class World {
public:
    World(int num_points, const Vec2& min, const Vec2& max);

    // Drawing.
    void draw(sf::RenderWindow* window);
    void drawTile(sf::RenderWindow* window, const Map::Tile& tile, sf::Color colour);
    void drawTileEdge(sf::RenderWindow* window, const Map::Tile& tile, sf::Color colour);
    void drawJoinedRibbon(sf::RenderWindow* window, const Vector<Vec2>& points, float inner_thickness, float outer_thickness, const sf::Color& colour);

    // Tiles
    const Vector<Map::Tile>& mapTiles() const;

private:
    HashMap<int, UniquePtr<State>> states_;

    typedef std::mt19937 RngType;
    RngType rng_;
    UniquePtr<Map> map_;
    HashSet<Map::Tile*> unclaimed_tiles_;

private:
    void generateStates(int count, int max_size);
    void fillStates(int count);
    bool growState(State* state);
};
