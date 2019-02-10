#pragma once

#include "Map.h"

class World;
struct RenderContext;

inline Vector<HashSet<Vec2i>> getExclaves(HashSet<Vec2i> points) {
    /*
        Example point set:
        ### ##
        ### ###
        ##   #

        Exclaves are found with the following algorithm:

        1) Remove point A from the input.
        2) Create a new exclave E.
        3) Add A to exclave E.
        4) Do a breadth first search from A, treating immediate neighbours as edges, removing
           from input set when visited.
        5) If more points exist, goto 1.
    */

    // Find exclaves.
    Vector<HashSet<Vec2i>> exclaves;
    while (!points.empty()) {
        // Create exclave.
        exclaves.emplace_back(HashSet<Vec2i>{});
        auto& exclave = exclaves.back();

        // Search for neighbours.
        Queue<Vec2i> bfs_queue;
        bfs_queue.push(*points.begin());
        points.erase(points.begin());
        while (!bfs_queue.empty()) {
            // Dequeue and add to exclave.
            Vec2i p = bfs_queue.front();
            exclave.insert(p);
            bfs_queue.pop();

            // Traverse neighbours.
            /*
            for (int d = 0; d < D_COUNT; ++d) {
                Vec2i neighbour = p + getDirectionOffset((Direction)d);
                auto neighbour_it = points.find(neighbour);
                if (neighbour_it != points.end()) {
                    bfs_queue.push(neighbour);
                    points.erase(neighbour_it);
                }
            }
             */
        }
    }
    return exclaves;
}

class City
{
public:
	City(const String& name, Map::Site* site, Vec2& position);

	void draw(RenderContext& ctx, sf::Shape& shape);

private:
	Map::Site* site_;
	Vec2 position_;
	sf::Text gui_name_;
};

class State {
public:
    State(sf::Color colour, const String& name, const HashSet<Map::Site*>& land);

    void setName(const String& name);

    void addLandTile(Map::Site* tile);
    void removeLandTile(Map::Site *tile);

    void draw(RenderContext& ctx, bool highlighted);
    void drawBorders(RenderContext& ctx);
    void drawOverlays(RenderContext& ctx);

    Vector<Vector<Map::GraphEdge*>> unorderedBoundary() const;
    const HashSet<Map::Site*>& land() const;

    const Vec2 midpoint() const;

	sf::Color colour() const;

private:
    sf::Color colour_;
    String name_;
    HashSet<Map::Site*> land_;
    Vec2 centre_;

    // Rendering data.
    sf::Text gui_name_;
	sf::RectangleShape capital_shape_;
	sf::CircleShape city_shape_;
};
