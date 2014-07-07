#include "df3d_pch.h"
#include "Viewport.h"

namespace df3d { namespace render {

void Viewport::setDimensions(int x, int y, int w, int h)
{
    if (x < 0 || y < 0 || x >= w || y >= h)
    {
        base::glog << "Trying to set invalid viewport dimensions" << base::logwarn;
        return;
    }

    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;
}

int Viewport::x() const
{
    return m_x;
}

int Viewport::y() const
{
    return m_y;
}

int Viewport::width() const
{
    return m_w;
}

int Viewport::height() const
{
    return m_h;
}

} }