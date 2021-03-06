#pragma once

namespace df3d {

class MathUtils
{
public:
    static const glm::vec4 XAxis;
    static const glm::vec4 YAxis;
    static const glm::vec4 ZAxis;
    static const glm::vec3 UnitVec3;

    // Creates rotation which rotates from v1 to v2.
    static glm::quat fromToRotation(const glm::vec3 &v1, const glm::vec3 &v2);

    static glm::vec2 toPolar(const glm::vec2 &cartesian);
    static glm::vec2 fromPolar(float angle, float dist);

    static float signedDistanceToPlane(const glm::vec4 &plane, const glm::vec3 &point);

    static glm::vec3 safeNormalize(const glm::vec3 &v);

    static float gaussian(float x, float mean, float stddev);

    template<typename T>
    static T lerp(const T &a, const T &b, float t)
    {
        return a * (1.0f - t) + b * t;
    }
};

struct Spherical
{
    float r;
    float yaw;
    float pitch;

    Spherical(float r, float yaw, float pitch);
    Spherical(const glm::vec3 &v);
    ~Spherical();

    glm::vec3 toCartesian();
};

struct Ray
{
    glm::vec3 origin;
    glm::vec3 dir;
};

}
