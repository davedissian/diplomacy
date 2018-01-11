#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"

// GUI
#include "gui/imgui.h"
#include "gui/imgui-SFML.h"

// STL
#include <iostream>

#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <utility>

using String = std::string;

template <typename T>
using Vector = std::vector<T>;

template <typename T>
using Queue = std::queue<T>;

template <typename T>
using HashSet = std::unordered_set<T>;

template <typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template <typename T>
using UniquePtr = std::unique_ptr<T>;

template <typename A, typename B>
using Pair = std::pair<A, B>;

template <typename T>
using SharedPtr = std::shared_ptr<T>;

using std::make_unique;
using std::make_shared;

using Vec2i = glm::ivec2;
using Vec2 = glm::vec2;
using Vec3i = glm::ivec3;
using Vec3 = glm::vec3;

using i16 = std::int16_t;
using i32 = std::int32_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using f32 = float;
using f64 = double;

namespace std {
    template <> struct hash<Vec2i>
    {
        size_t operator()(const Vec2i& x) const
        {
            return hash<i32>()((x.x & 0xFFFF) | ((x.y & 0xFFFF) << 16));
        }
    };
}

inline sf::Vector2f toSFML(const Vec2& v) {
    return {v.x, v.y};
}

inline sf::Vector2f toSFML(const Vec2i& v) {
    return {float(v.x), float(v.y)};
}

inline Vec2 fromSFML(const sf::Vector2f& v) {
    return {v.x, v.y};
}

inline Vec2i fromSFML(const sf::Vector2i& v) {
    return {v.x, v.y};
}

inline float clamp(float x, float min_value, float max_value) {
    return std::max(min_value, std::min(x, max_value));
}

inline float lerp(float a, float b, float t) {
    return a * (1.0f - t) + b * t;
}

inline Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
    return a * (1.0f - t) + b * t;
}

// Lerp between a and b by a factor of t over 'time' seconds.
inline float damp(float a, float b, float t, float time, float dt) {
    return lerp(a, b, 1.0f - pow(t, dt / time));
}

// Lerp between a and b by a factor of t over 'time' seconds.
inline Vec2 damp(const Vec2& a, const Vec2& b, float t, float time, float dt) {
    return lerp(a, b, 1.0f - pow(t, dt / time));
}

// Intersection between two lines defined as p1->p2 and p3->p4.
inline Vec2 intersection(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4) {
    float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
    float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;

    float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    // If d is zero, there is no intersection.
    if (d == 0) {
        return {0, 0};
    }

    // Get the x and y.
    float pre = (x1 * y2 - y1 * x2), post = (x3 * y4 - y3 * x4);
    float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
    float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

    // Return the point of intersection.
    return {x, y};
}