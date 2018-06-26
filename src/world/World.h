#pragma once

#include "world/Map.h"
#include "world/State.h"
#include "gameplay/Unit.h"

#include "RenderContext.h"

class World {
public:
    World(int num_points, const Vec2& min, const Vec2& max);

    // Map generation.
    void generateStates(int count, int max_size);
    void fillStates(int count);

    // Drawing.
    void draw(RenderContext& ctx);
    void drawTile(RenderContext& ctx, const Map::Site& tile, sf::Color colour);
    void drawTileEdge(RenderContext& ctx, const Map::Site& tile, sf::Color colour);
	void drawLineList(RenderContext& ctx, const Vector<Vec2>& points, const sf::Color& colour);
	void drawJoinedRibbon(RenderContext& ctx, const Vector<Vec2>& points, float inner_thickness, float outer_thickness, const sf::Color& colour);

	void drawBorder(RenderContext& ctx, Vector<Vector<Map::GraphEdge*>> list_of_boundaries, sf::Color colour);

    // States.
    const HashMap<int, SharedPtr<State>>& states() const;
    WeakPtr<State> getStateById(int id) const;

    // Tiles.
    Vector<Map::Site>& mapSites();
    const Vector<Map::Site>& mapSites() const;

private:
    HashMap<int, SharedPtr<State>> states_;

    std::mt19937 rng_;
    UniquePtr<Map> map_;
    HashSet<Map::Site*> unclaimed_tiles_;

private:
    bool growState(State* state);
};
