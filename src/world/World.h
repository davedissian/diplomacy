#pragma once

#include "world/Map.h"
#include "world/State.h"
#include "gameplay/Unit.h"

class World {
public:
    World(int num_points, const Vec2& min, const Vec2& max);

    // Map generation.
    void generateStates(int count, int max_size);
    void fillStates(int count);

    // Drawing.
    void draw(sf::RenderWindow* window);
    void drawTile(sf::RenderWindow* window, const Map::Site& tile, sf::Color colour);
    void drawTileEdge(sf::RenderWindow* window, const Map::Site& tile, sf::Color colour);
    void drawJoinedRibbon(sf::RenderWindow* window, const Vector<Vec2>& points, float inner_thickness, float outer_thickness, const sf::Color& colour);

    // States.
    const HashMap<int, SharedPtr<State>>& states() const;
    WeakPtr<State> getStateById(int id) const;

    // Tiles.
    const Vector<Map::Site>& mapTiles() const;

private:
    HashMap<int, SharedPtr<State>> states_;

    std::mt19937 rng_;
    UniquePtr<Map> map_;
    HashSet<Map::Site*> unclaimed_tiles_;

private:
    bool growState(State* state);
};
