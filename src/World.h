#pragma once

#include <random>
#include <voronoi/voronoi.h>

class State;

const float VORONOI_EPSILON = 1e-2f;

class VoronoiGraph {
public:
    struct Tile;
    struct Edge {
        Vec2 v0;
        Vec2 v1;
        // Delunay Triangulation. d0.centre -> d1.centre
        Tile* d0;
        Tile* d1;
    };

    struct OrderedEdge {
        SharedPtr<Edge> edge;
        bool flipped;

        Vec2& v0();
        const Vec2& v0() const;
        Vec2& v1();
        const Vec2& v1() const;
    };

    struct Tile {
        Vec2 centre;
        // Neighbour i is joined at edge i.
        Vector<OrderedEdge> edges;
        Vector<Tile*> neighbours;

        double edgeAngle(const Edge& e);
        double vertexAngle(const Vec2& v);
        Pair<Vec2, Vec2> orderedEdgeVertices(const OrderedEdge& e);
    };

    explicit VoronoiGraph(const voronoi::Voronoi& voronoi);

    Vector<Tile>& tiles();
    const Vector<Tile>& tiles() const;

private:
    Vector<Tile> tiles_;
};

class World {
public:
    World(int num_points, float size_x, float size_y);

    // Drawing.
    void draw(sf::RenderWindow* window);
    void drawTile(sf::RenderWindow* window, const VoronoiGraph::Tile& tile, sf::Color colour);
    void drawTileEdge(sf::RenderWindow* window, const VoronoiGraph::Tile& tile, sf::Color colour);
    void drawJoinedRibbon(sf::RenderWindow* window, const Vector<Vec2>& points, float inner_thickness, float outer_thickness, const sf::Color& colour);

    // Events.
    void onMouseMove(const Vec2& projected_mouse_position);

private:
    HashMap<int, UniquePtr<State>> states_;

    typedef std::mt19937 RngType;
    RngType rng_;
    UniquePtr<VoronoiGraph> map_;
    HashSet<VoronoiGraph::Tile*> unclaimed_tiles_;
    VoronoiGraph::Tile* selected_tile_;

private:
    void generateStates(int count, int max_size);
    void fillStates(int count);
    bool growState(State* state);
};