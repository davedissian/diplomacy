#include "Common.h"
#include "world/State.h"
#include "world/Map.h"
#include "math/Noise.h"

namespace {
void subdivide(Vector<Vec2>& points, std::mt19937& rng, const Vec2& A, const Vec2& B, const Vec2& C, const Vec2& D, float min_length) {
    //std::cout << "A " << A << " B " << B << " C " << C << " D " << D << std::endl;
    //std::cout << "Distance AC: " << vecDistance(A, C) << " Distance BD: " << vecDistance(B, D) << "Min: " << min_length << std::endl;
    if (vecDistance(A, C) < min_length || vecDistance(B, D) < min_length) {
        return;
    }

    // Subdivide the quadrilateral
    std::uniform_real_distribution<float> subdivide_range{0.3f, 0.7f};
    float p = subdivide_range(rng);  // vertical (along A-D and B-C)
    float q = subdivide_range(rng);  // horizontal (along A-B and D-C)

    // Midpoints
    Vec2 E = lerp(A, D, p);
    Vec2 F = lerp(B, C, p);
    Vec2 G = lerp(A, B, q);
    Vec2 I = lerp(D, C, q);

    // Central point
    Vec2 H = lerp(E, F, q);

    // Divide the quad into subquads, but meet at H
    std::uniform_real_distribution<float> subquad_range{-0.4f, 0.4f};
    float s = 1.0f - subquad_range(rng);
    float t = 1.0f - subquad_range(rng);

    subdivide(points, rng, A, G, H, E, min_length);
    //subdivide(points, rng, A, lerp(G, B, s), H, lerp(E, D, t), min_length);
    points.push_back(H);
    subdivide(points, rng, H, F, C, I, min_length);
    //subdivide(points, rng, H, lerp(F, C, s), C, lerp(I, D, t), min_length);
}

Vector<Vec2> buildNoisyLineSegments(std::mt19937& rng, const Vec2& A, const Vec2& B, const Vec2& C, const Vec2& D, float min_length) {
    Vector<Vec2> points;
    points.push_back(A);
    //subdivide(points, rng, A, B, C, D, min_length);
    points.push_back(C);
    return points;
}
}

Vec2 &Map::Edge::v0() {
    return points.front();
}

const Vec2 &Map::Edge::v0() const {
    return points.front();
}

Vec2 &Map::Edge::v1() {
    return points.back();
}

const Vec2 &Map::Edge::v1() const {
    return points.back();
}

Vec2 &Map::GraphEdge::v0() {
    return points.front();
}

const Vec2 &Map::GraphEdge::v0() const {
    return points.front();
}

Vec2 &Map::GraphEdge::v1() {
    return points.back();
}

const Vec2 &Map::GraphEdge::v1() const {
    return points.back();
}

double Map::Site::edgeAngle(const Edge &e) const {
    Vec2 centre_offset = (e.v0() + e.v1()) * 0.5f - centre;
    return atan2(centre_offset.y, centre_offset.x);
}

double Map::Site::vertexAngle(const Vec2 &v) const {
    return atan2(v.y - centre.y, v.x - centre.x);
}

Map::Map(int num_points, const Vec2& min, const Vec2& max, std::mt19937& rng) {
    const int relax_count = 100;

    // Seed RNG.
    std::uniform_real_distribution<float> dist(0, 1);
    std::mt19937::result_type const seedval = 0xDEADBEEF; // TODO: get this from somewhere
    rng.seed(seedval);

    // Generate points for voronoi diagram.
    Vector<jcv_point> points;
    for (int i = 0; i < num_points; ++i) {
        points.emplace_back(jcv_point{
            dist(rng) * (max.x - min.x) + min.x,
            dist(rng) * (max.y - min.y) + min.y
        });
    }

    // Build voronoi diagram and relax points using Lloyds Algorithm.
    jcv_rect rect = {
        {min.x, min.y},
        {max.x, max.y}
    };
    jcv_diagram diagram = {};
    jcv_diagram_generate(num_points, points.data(), &rect, &diagram);
    for (int i = 0; i < relax_count; ++i) {
        points.clear();
        const jcv_site* sites = jcv_diagram_get_sites(&diagram);
        for (int j = 0; j < diagram.numsites; ++j) {
            jcv_point p = {0.0f, 0.0f};
            float edge_count = 0.0f;
            for (jcv_graphedge* e = sites[j].edges; e; e = e->next) {
                p.x += e->pos[0].x + e->pos[1].x;
                p.y += e->pos[0].y + e->pos[1].y;
                edge_count++;
            }
            p.x /= edge_count * 2;
            p.y /= edge_count * 2;
            points.emplace_back(p);
        }
        jcv_diagram_generate(num_points, points.data(), &rect, &diagram);
    }

    // Build voronoi data structure from jcv_diagram.
    HashMap<const jcv_edge*, SharedPtr<Edge>> edge_map;
    sites_.resize(static_cast<size_t>(diagram.numsites));
    const jcv_site* diagram_sites = jcv_diagram_get_sites(&diagram);
    for (auto e = jcv_diagram_get_edges(&diagram); e; e = e->next) {
        auto new_edge = make_shared<Edge>();
        new_edge->points = {{e->pos[0].x, e->pos[0].y}, {e->pos[1].x, e->pos[1].y}};
        new_edge->d[0] = &sites_[e->sites[0]->index];
        new_edge->d[1] = &sites_[e->sites[1]->index];
        edge_map.emplace(e, new_edge);
    }
    for (int i = 0; i < diagram.numsites; ++i) {
        sites_[i].usable = true;
        sites_[i].centre = {diagram_sites[i].p.x, diagram_sites[i].p.y};
        for (auto e = diagram_sites[i].edges; e; e = e->next) {
            sites_[i].edges.emplace_back();
            auto& new_edge = sites_[i].edges.back();
            new_edge.edge = edge_map.at(e->edge);
            new_edge.points = {{e->pos[0].x, e->pos[0].y}, {e->pos[1].x, e->pos[1].y}};
            new_edge.next = nullptr;
            new_edge.angle = e->angle;
            if (e->neighbor) {
                new_edge.neighbour = &sites_[e->neighbor->index];
            } else {
                // An edge not having a neighbour site indicates that this is a site on
                // the edges of the map. Therefore, it's not usable.
                new_edge.neighbour = nullptr;
                sites_[i].usable = false;
            }
        }
        sites_[i].owning_state = nullptr;
        for (int edge_index = 0; edge_index < (sites_[i].edges.size() - 1); ++edge_index) {
            sites_[i].edges[edge_index].next = &sites_[i].edges[edge_index + 1];
        }
    }
    edge_map.clear();
    jcv_diagram_free(&diagram);


#if 0
    // Build a list of edges.
    Vector<Edge> initial_edges;
    initial_edges.reserve(static_cast<size_t>(diagram.numsites * 5)); // A reasonable guess to avoid reallocations.
    sites_.reserve(static_cast<size_t>(diagram.numsites));
    const jcv_site* sites = jcv_diagram_get_sites(&diagram);
    for (int i = 0; i < diagram.numsites; ++i) {
        sites_.emplace_back();
        Site& tile = sites_.back();
        tile.centre = {sites[i].p.x, sites[i].p.y};
        tile.usable = true;

        // Setup initial edges.
        for (jcv_graphedge* e = sites[i].edges; e; e = e->next) {
            auto& edge = *e->edge;
            initial_edges.push_back({{Vec2(edge.pos[0].x, edge.pos[0].y), Vec2(edge.pos[1].x, edge.pos[1].y)}, &tile, nullptr});
        }
    }
    jcv_diagram_free(&diagram);

    // Merge identical edges in worst case O(N sqrt(N)) time.
    //
    // Let an edge E consist of two points (p,q), and let x~y be a relation defined as |x-y| < e (e = epsilon value):
    //   Two edges A and B are identical, when A.p ~ B.p and A.q ~ B.q, or A.p ~ B.q and A.q ~ B.p.
    //
    // This algorithm first sorts the array of edges (p, q) by p.x. Then, for each edge, look for the other edge which
    // can be merged into this one. This is guaranteed to deduplicate the edges and reduce the size of the array by half.
    //
    // If the array is sorted, checking for the edge which duplicates an edge E will involve a search along the x axis
    // from x = E.p.x. The first vertex of duplicated edge D is guaranteed to lie on the x axis in the range
    // 'E.p.x <= D.p.x < e'. As we have to deal with potentially flipped edges, we may have to search until the
    // x value of the 2nd vertex of D, which makes our range 'E.p.x <= D.p.x < L+2e' (where L is the maximum edge length).
    //
    // Let N be the number of sites (which have an average number of edges independent of N). Sorting the array costs
    // O(NlogN). Iterating through the array costs O(N) for the outer loop. The inner loop will traverse a distance of
    // L+2e (L being the length of each axis divided by the number of sites per axis - A/sqrt(N) - and e being constant).
    // Traversing a distance of L is going to iterate through A/L edges, which is O(sqrt(N)). This gives a worst case
    // complexity of O(N sqrt(N)).
    std::cout << "Voronoi: Merging identical edges." << std::endl;
    std::sort(initial_edges.begin(), initial_edges.end(), [](const Edge& a, const Edge& b) -> bool {
        return a.v0().x < b.v0().x;
    });
    Vector<SharedPtr<Edge>> edges;
    auto ShouldMergeEdges = [](const Edge& a, const Edge& b) -> bool {
        return (vecEqual(a.v0(), b.v0(), VORONOI_EPSILON) && vecEqual(a.v1(), b.v1(), VORONOI_EPSILON)) ||
               (vecEqual(a.v0(), b.v1(), VORONOI_EPSILON) && vecEqual(a.v1(), b.v0(), VORONOI_EPSILON));
    };
    for (int i = 0; i < initial_edges.size(); ++i) {
        for (int j = i + 1; j < initial_edges.size(); ++j) {
            if (ShouldMergeEdges(initial_edges[i], initial_edges[j])) {
                //std::cout << "Merging " << i << " and " << j << " - " << initial_edges[i].d0 << " " << initial_edges[j].d0 << " - " << initial_edges[i].d1 << " " << initial_edges[j].d1 << std::endl;
                SharedPtr<Edge> new_edge = make_shared<Edge>();
                new_edge->points = std::move(initial_edges[i].points);
                new_edge->d[0] = initial_edges[i].d[0];
                new_edge->d[1] = initial_edges[j].d[0];
                edges.emplace_back(new_edge);
                break;
            }
        }
    }

    // Populate edge lists inside each tile.
    std::cout << "Voronoi: Populating edge lists." << std::endl;
    for (SharedPtr<Edge>& edge : edges) {
        edge->d[0]->edges.push_back({edge, {}, nullptr});
        edge->d[1]->edges.push_back({edge, {}, nullptr});
    }

    for (auto &edge : edges) {
        auto &points = edge->points;
        float f = 0.5f;
        Vec2 t = lerp(edge->v0(), edge->d[0]->centre, f);
        Vec2 q = lerp(edge->v0(), edge->d[1]->centre, f);
        Vec2 r = lerp(edge->v1(), edge->d[0]->centre, f);
        Vec2 s = lerp(edge->v1(), edge->d[1]->centre, f);

        Vec2 edge_midpoint = (edge->v0() + edge->v1()) * 0.5f;

        float min_length = 10.0f;
        //auto path0 = buildNoisyLineSegments(rng, edge->v0(), t, edge_midpoint, q, min_length);
        //auto path1 = buildNoisyLineSegments(rng, edge_midpoint, r, edge->v1(), s, min_length);
        //std::reverse(path1.begin(), path1.end());
        //path0.pop_back();
        //path0.insert(path0.end(), path1.begin(), path1.end());
        edge->points = {edge->v0(), edge->v1()};
    }

    /*
    // Apply edge noise.
    //const float noise_range = 0.4f;
    //std::uniform_real_distribution<float> noise_dist(0.5f - noise_range * 0.5f, 0.5f + noise_range * 0.5f);
    PerlinNoise noise(std::uniform_int_distribution<uint>{}(rng));//fBmNoise noise(std::uniform_int_distribution<uint>{}(rng), 1, 2.0f, 1.0f, 2.0f, 0.5f);
    float noise_offset = std::uniform_real_distribution<float>{0.0f, 1000.0f}(rng);
    for (auto &edge : edges) {
        auto& points = edge->points;

        // Generate noise values.
        Vector<float> noise_values;
        float dist_per_noise = 20.0f;
        int num_noise_values = int(glm::distance(points.front(), points.back()) / dist_per_noise);
        if (num_noise_values > 0) {
            num_noise_values -= 1;
        }
        for (int i = 0; i < num_noise_values; ++i) {
            float position_on_edge = float(i + 1) / (num_noise_values + 2);
            float noise_value = (float)noise.noise(noise_offset + position_on_edge * 10.0f, 0.0, 0.0);
            float noise_damp = (position_on_edge < 0.5f) ? (position_on_edge * 2.0f) : (2.0f - 2.0f * position_on_edge);
            noise_value = (noise_value - 0.5f) * noise_damp + 0.5f;
            noise_values.push_back(noise_value);
        }

        // Generate points.
        Vec2 v0 = points.front();
        Vec2 v1 = points.back();
        Vec2 direction = points.back() - points.front();
        float distance = glm::length(direction);
        direction /= distance;
        Vec2 edge_mid = (v0 + v1) * 0.5f;
        Vec2 normal = {-direction.y, direction.x};
        Vec2 min_noise0 = intersection(edge_mid - normal * distance, edge_mid + normal * distance, v0, edge->d0->centre);
        Vec2 min_noise1 = intersection(edge_mid - normal * distance, edge_mid + normal * distance, v1, edge->d0->centre);
        float min_dist = std::min(glm::length(min_noise0 - edge_mid), glm::length(min_noise1 - edge_mid));// ? min_noise0 : min_noise1;
        Vec2 max_noise0 = intersection(edge_mid - normal * distance, edge_mid + normal * distance, v0, edge->d1->centre);
        Vec2 max_noise1 = intersection(edge_mid - normal * distance, edge_mid + normal * distance, v1, edge->d1->centre);
        float max_dist = std::min(glm::length(max_noise0 - edge_mid), glm::length(max_noise1 - edge_mid));// ? max_noise0 : max_noise1;
        for (int i = 0; i < num_noise_values; ++i) {
            float position_on_edge = float(i + 1) / (num_noise_values + 2);
            Vec2 midpoint = lerp(v0, v1, position_on_edge);
            if (noise_values[i] < 0.5f) {
                points.insert(points.begin() + i + 1, lerp(midpoint - normal * min_dist, midpoint, noise_values[i] * 2.0f));
            } else {
                points.insert(points.begin() + i + 1, lerp(midpoint, midpoint + normal * max_dist, noise_values[i] * 2.0f - 1.0f));
            };
        }
    }
     */

    // Reorder edges in each tile by their angle relative to the centre of the tile.
    // atan2(y,x) would return -pi from the left and move clockwise, so edges will be ordered west, north, east, south.
    std::cout << "Voronoi: Sorting edges by angle." << std::endl;
    for (auto it = sites_.begin(); it < sites_.end(); ++it) {
        auto& tile = *it;

        // Sort edges.
        std::sort(tile.edges.begin(), tile.edges.end(), [&tile](const GraphEdge& a, const GraphEdge& b) {
            return tile.edgeAngle(*a.edge) < tile.edgeAngle(*b.edge);
        });

        // Initialise edges
        for (auto& edge : tile.edges) {
            double angle0 = tile.vertexAngle(edge.edge->v0());
            double angle1 = tile.vertexAngle(edge.edge->v1());
            double difference = angle1 - angle0;

            const double pi = 3.14159;
            bool flipped = (difference > 0 && difference > pi) || (difference < 0 && (-difference <= pi));
            edge.points = edge.edge->points;
            if (flipped) {
                std::reverse(edge.points.begin(), edge.points.end());
            }
        }

        // Fill in incomplete pairs. This should only happen on border sites.
        for (int i = 0; i < tile.edges.size(); ++i) {
            Vec2& a = tile.edges[i].v1();
            Vec2& b = tile.edges[(i + 1) % tile.edges.size()].v0();
            if (!vecEqual(a, b, VORONOI_EPSILON)) {
                SharedPtr<Edge> new_base_edge = make_shared<Edge>();
                new_base_edge->points = {a, b};
                new_base_edge->d[0] = &tile;
                new_base_edge->d[1] = nullptr;
                GraphEdge new_edge;
                new_edge.edge = new_base_edge;
                new_edge.points = new_edge.edge->points;
                tile.edges.insert(tile.edges.begin() + i + 1, new_edge);

                // We're definitely a border tile. Set as unusable.
                tile.usable = false;
            }
        }

        // Populate corresponding neighbours. It will be the delunay point which is not this tile.
        for (auto &edge : tile.edges) {
            edge.neighbour = edge.edge->d[0] == &tile ? edge.edge->d[1] : edge.edge->d[0];
        }
    }
#endif
}

Vector<Map::Site> &Map::sites() {
    return sites_;
}

const Vector<Map::Site> &Map::sites() const {
    return sites_;
}

Vector<Vector<Map::Map::GraphEdge*>> Map::unorderedBoundaries(const HashSet<Site*>& sites)
{
	// Build edge list containing site boundaries.
	Vector<Vector<Map::GraphEdge*>> exclave_boundaries;
	exclave_boundaries.emplace_back();
	for (auto &site : sites)
	{
		for (int i = 0; i < site->edges.size(); ++i)
		{
			if (sites.count(site->edges[i].neighbour) == 0)
			{
				exclave_boundaries.back().push_back(&site->edges[i]);
			}
		}
	}
	return exclave_boundaries;
}
