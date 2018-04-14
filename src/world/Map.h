#pragma once

#include <random>
#include "math/voronoi/voronoi.h"

const float VORONOI_EPSILON = 1e-2f;

// Structured as a voronoi graph.
class Map {
public:
    struct Site;
    struct Edge {
        Vector<Vec2> points;

        // Delunay Triangulation. d0.centre -> d1.centre
        Site* d0;
        Site* d1;

        Vec2& v0();
        const Vec2& v0() const;
        Vec2& v1();
        const Vec2& v1() const;
    };

    struct GraphEdge {
        SharedPtr<Edge> shared_edge;
        Vector<Vec2> points;
        Site* neighbour;

        Vec2& v0();
        const Vec2& v0() const;
        Vec2& v1();
        const Vec2& v1() const;
    };

    struct Site {
        Vec2 centre;
        Vector<GraphEdge> edges;
        bool usable;

        const static int EDGE_DETAIL = 0; // Total number of points in an edge = 1 << EDGE_DETAIL + 2

        double edgeAngle(const Edge& e) const;
        double vertexAngle(const Vec2& v) const;
    };

    explicit Map(int num_points, const Vec2& min, const Vec2& max, std::mt19937& rng);

    Vector<Site>& sites();
    const Vector<Site>& sites() const;

private:
    Vector<Site> sites_;
};
