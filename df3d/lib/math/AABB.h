#pragma once

#include "BoundingVolume.h"

namespace df3d {

class BoundingSphere;

class AABB : public BoundingVolume
{
    glm::vec3 m_min;
    glm::vec3 m_max;
public:
    AABB();
    ~AABB();

    void reset() override;
    void updateBounds(const glm::vec3 &point) override;

    const glm::vec3 &minPoint() const;
    const glm::vec3 &maxPoint() const;

    bool isValid() const override;

    bool contains(const glm::vec3 &point) const;
    bool intersects(const AABB &other) const;
    bool intersects(const BoundingSphere &sphere) const;

    glm::vec3 getCenter() const;
    void getCorners(glm::vec4 output[8]) const;
};

}
