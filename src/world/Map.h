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
        Site* d[2];

        // Delunay Triangulation can be made from edges:
        //  sites[0].centre -> sites[1].centre

        // First and last points.
        Vec2& v0();
        const Vec2& v0() const;
        Vec2& v1();
        const Vec2& v1() const;
    };

    struct GraphEdge {
        SharedPtr<Edge> edge;
        Vector<Vec2> points;
        Site* neighbour;
        GraphEdge* next;
        float angle;

        // First and last points.
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
