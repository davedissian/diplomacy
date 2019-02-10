#include "Common.h"
#include "world/State.h"
#include "world/World.h"

City::City(const String& name, Map::Site* site, Vec2& position) : site_{site}, position_{position}
{
	gui_name_.setString(name);
	gui_name_.setPosition(toSFML(position));
}

void City::draw(RenderContext& ctx, sf::Shape& shape)
{
	gui_name_.setFont(ctx.font);
	ctx.window->draw(gui_name_);

	shape.setPosition(toSFML(position_));
	ctx.window->draw(shape);
}

State::State(sf::Color colour, const String& name, const HashSet<Map::Site*>& land) : colour_(colour), name_(name), land_(land) {
    colour_.a = 100;
    gui_name_.setString(name);

    // Calculate centre.
    centre_ = {0.0f, 0.0f};
    for (auto& tile : land_) {
        centre_ += tile->centre;
    }
    centre_ /= (float)land_.size();

    gui_name_.setPosition(toSFML(centre_));
    gui_name_.setCharacterSize(20);
}

void State::setName(const String &name) {
    name_ = name;
    gui_name_.setString(name);
}

void State::addLandTile(Map::Site *tile) {
    if (tile->owning_state) {
        tile->owning_state->removeLandTile(tile);
    }
    land_.insert(tile);
    tile->owning_state = this;
}

void State::removeLandTile(Map::Site *tile) {
    land_.erase(tile);
    tile->owning_state = nullptr;
}

void State::draw(RenderContext& ctx, bool highlighted) {
    sf::Color colour = colour_;
    if (!highlighted) {
        colour.a = 40;
    }
    for (const auto tile : land_) {
        ctx.world->drawTile(ctx, *tile, colour);
        //world->drawTileEdge(window, *tile, sf::Color(colour_.r, colour_.g, colour_.b, 40));
    }
}

void State::drawBorders(RenderContext& ctx) {
	ctx.world->drawBorder(ctx, Map::unorderedBoundaries(land_), colour_);
}

void State::drawOverlays(RenderContext& ctx) {
	gui_name_.setFont(ctx.font);
    ctx.window->draw(gui_name_);
}

Vector<Vector<Map::GraphEdge*>> State::unorderedBoundary() const {
    // Build edge list containing state boundaries.
    Vector<Vector<Map::GraphEdge*>> exclave_boundaries;
    exclave_boundaries.emplace_back();
    for (auto &state_tile : land_) {
        for (int i = 0; i < state_tile->edges.size(); ++i) {
            if (land_.count(state_tile->edges[i].neighbour) == 0) {
                exclave_boundaries.back().push_back(&state_tile->edges[i]);
            }
        }
    }
    return exclave_boundaries;
}

const HashSet<Map::Site*>& State::land() const {
    return land_;
}

const Vec2 State::midpoint() const {
    return centre_;
}

sf::Color State::colour() const
{
	return colour_;
}

