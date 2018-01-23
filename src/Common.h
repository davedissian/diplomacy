#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtx/vector_query.hpp"

// GUI
#include "gui/imgui.h"
#include "gui/imgui-SFML.h"

// STL
#include <iostream>
#include <iomanip>
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

inline std::ostream& operator<<(std::ostream& stream, const Vec2& v)
{
    stream << v.x << ", " << v.y;
    return stream;
}


namespace std {
    template <> struct hash<Vec2i>
    {
        size_t operator()(const Vec2i& x) const
        {
            return hash<i32>()((x.x & 0xFFFF) | ((x.y & 0xFFFF) << 16));
        }
    };
}

// HSV colour.
class RGBColour {
public:
    float r;
    float g;
    float b;
    float a;

    RGBColour(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    RGBColour(const sf::Color& c) {
        r = float(c.r) / 255.0f;
        g = float(c.g) / 255.0f;
        b = float(c.b) / 255.0f;
        a = float(c.a) / 255.0f;
    }

    operator sf::Color() {
        return sf::Color(sf::Uint8(r * 255.0f), sf::Uint8(g * 255.0f), sf::Uint8(b * 255.0f), sf::Uint8(a * 255.0f));
    }
};
class HSVColour {
public:
    float h; // 0 - 360.
    float s; // 0 - 1
    float v; // 0 - 1
    float a; // 0 - 1

    HSVColour(float h, float s, float v, float a = 1.0f) : h(h), s(s), v(v), a(a) {}

    HSVColour(const sf::Color& sfml_colour) {
        RGBColour colour(sfml_colour);

        a = colour.a;

        float min = colour.r < colour.g ? colour.r : colour.g;
        min = min < colour.b ? min : colour.b;

        float max = colour.r > colour.g ? colour.r : colour.g;
        max = max > colour.b ? max : colour.b;

        v = max;
        float delta = max - min;
        if (delta < 0.00001f) {
            s = 0.0f;
            h = 0.0f;
        }
        if (max > 0.0f) {
            s = (delta / max);
        } else {
            // if max is 0, then r = g = b = 0
            // s = 0, h is undefined
            s = 0.0f;
            h = NAN;
            return;
        }

        if (colour.r >= max) {
            // between yellow & magenta.
            h = (colour.g - sfml_colour.b) / delta;
        } else if (colour.g >= max) {
            // between cyan & yellow
            h = 2.0f + (colour.b - colour.r) / delta;
        } else {
            // between magenta & cyan
            h = 4.0f + (colour.r - colour.g) / delta;
        }

        h *= 60.0;

        if (h < 0.0f) {
            h += 360.0f;
        }
    }

    operator sf::Color() {
        if(s <= 0.0) {
            return RGBColour{v, v, v, a};
        }

        float hh = h;
        if (hh >= 360.0) {
            hh = 0.0;
        }
        hh /= 60.0;
        long i = (long)hh;
        float ff = hh - i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - (s * ff));
        float t = v * (1.0f - (s * (1.0f - ff)));

        switch (i) {
            case 0:
                return RGBColour{v, t, p, a};
            case 1:
                return RGBColour{q, v, p, a};
            case 2:
                return RGBColour{p, v, t, a};
            case 3:
                return RGBColour{p, q, v, a};
            case 4:
                return RGBColour{t, p, v, a};
            case 5:
            default:
                return RGBColour{v, p, q, a};
        }
    }
};

// SFML conversion functions.
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

// Math.
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

// Vec2.
inline float vecLength(const Vec2& a) {
    return glm::length(a);
}

inline float vecDistance(const Vec2& a, const Vec2& b) {
    return vecLength(a - b);
}

inline bool vecEqual(const Vec2& a, const Vec2& b, const float eps = std::numeric_limits<float>::epsilon()) {
    return glm::isNull(a - b, eps);
}