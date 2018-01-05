#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "glm/vec2.hpp"

#include <iostream>

#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>

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

using std::make_unique;

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