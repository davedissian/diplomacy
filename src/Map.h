#pragma once

#include <random>
#include <voronoi/voronoi.h>

const float VORONOI_EPSILON = 1e-2f;

// Structured as a voronoi graph.
class Map {
public:
    struct Tile;
    struct Edge {
        Vector<Vec2> points;

        // Delunay Triangulation. d0.centre -> d1.centre
        Tile* d0;
        Tile* d1;

        Vec2& v0();
        const Vec2& v0() const;
        Vec2& v1();
        const Vec2& v1() const;
    };

    struct TileEdge {
        SharedPtr<Edge> shared_edge;
        Vector<Vec2> points;
        Tile* neighbour;

        Vec2& v0();
        const Vec2& v0() const;
        Vec2& v1();
        const Vec2& v1() const;
    };

    struct Tile {
        Vec2 centre;
        Vector<TileEdge> edges;
        bool usable;

        const static int EDGE_DETAIL = 0; // Total number of points in an edge = 1 << EDGE_DETAIL + 2

        double edgeAngle(const Edge& e);
        double vertexAngle(const Vec2& v);
    };

    explicit Map(const voronoi::Voronoi& voronoi, const Vec2& min, const Vec2& max, std::mt19937& rng);

    Vector<Tile>& tiles();
    const Vector<Tile>& tiles() const;

private:
    Vector<Tile> tiles_;
};
