#include "df3d_pch.h"
#include "Texture.h"

#include "OpenGLCommon.h"
#include "Image.h"
#include <base/Controller.h>
#include "RenderManager.h"
#include "Renderer.h"

namespace df3d { namespace render {

bool isPot(size_t v)
{
    return v && !(v & (v - 1));
}

size_t getNextPot(size_t v)
{
    if (!isPot(v))
    {
        int n = 0;
        while (v >>= 1)
            ++n;

        v = 1 << (n + 1);
    }
    return v;
}

#if defined(__WIN32__)

GLenum getGLTextureType(Texture::Type type)
{
    switch (type)
    {
    case Texture::TEXTURE_1D:
        return GL_TEXTURE_1D;
    case Texture::TEXTURE_2D:
        return GL_TEXTURE_2D;
    case Texture::TEXTURE_3D:
        return GL_TEXTURE_3D;
    case Texture::TEXTURE_CUBE:
        return GL_TEXTURE_CUBE_MAP;
    default:
        break;
    }

    return GL_INVALID_ENUM;
}

#elif defined(__ANDROID__)

GLenum getGLTextureType(Texture::Type type)
{
    switch (type)
    {
    case Texture::TEXTURE_2D:
        return GL_TEXTURE_2D;
    case Texture::TEXTURE_CUBE:
        return GL_TEXTURE_CUBE_MAP;
    case Texture::TEXTURE_3D:
    case Texture::TEXTURE_1D:
    default:
        break;
    }

    return GL_INVALID_ENUM;
}

#endif

GLint getGLFilteringMode(Texture::Filtering filtering, bool mipmapped)
{
    switch (filtering)
    {
    case Texture::NEAREST:
        return !mipmapped ? GL_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
    case Texture::BILINEAR:
        return !mipmapped ? GL_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
    case Texture::TRILINEAR:
        return !mipmapped ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR;
    default:
        break;
    }

    return -1;
}

GLint getGlWrapMode(Texture::WrapMode mode)
{
    switch (mode)
    {
    case Texture::WM_WRAP:
        return GL_REPEAT;
    case Texture::WM_CLAMP:
        return GL_CLAMP_TO_EDGE;
    default:
        break;
    }

    return -1;
}

bool Texture::createGLTexture()
{
    if (m_glid)
        return true;

    if (m_textureType == Texture::TEXTURE_TYPE_NONE)
    {
        base::glog << "Can not create texture. Specify texture type." << base::logwarn;
        return false;
    }

    m_glType = getGLTextureType(m_textureType);
    if (m_glType == GL_INVALID_ENUM)
    {
        base::glog << "Texture type is not supported." << base::logwarn;
        return false;
    }

    auto actWidth = getNextPot(m_image->width());
    auto actHeight = getNextPot(m_image->height());
    auto maxSize = g_renderManager->getRenderer()->getMaxTextureSize();
    if (actWidth > maxSize || actHeight > maxSize)
    {
        base::glog << "Failed to create texture. Size is too big." << base::logwarn;
        return false;
    }

    m_actualWidth = actWidth;
    m_actualHeight = actHeight;

    if (!(m_actualWidth == m_image->width() && m_actualHeight == m_image->height()))
        base::glog << "Texture with name" << m_image->getGUID() << "is not pot" << base::logdebug;

    glGenTextures(1, &m_glid);
    glBindTexture(m_glType, m_glid);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    auto wrapMode = getGlWrapMode(m_wrapMode);
    glTexParameteri(m_glType, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(m_glType, GL_TEXTURE_WRAP_T, wrapMode);
#if defined(__WIN32__)
    glTexParameteri(m_glType, GL_TEXTURE_WRAP_R, wrapMode);
#endif
    glTexParameteri(m_glType, GL_TEXTURE_MAG_FILTER, m_filteringMode == NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(m_glType, GL_TEXTURE_MIN_FILTER, getGLFilteringMode(m_filteringMode, m_mipmapped));

    unsigned int glPixelFormat = 0;
    switch (m_image->pixelFormat())
    {
    case Image::PF_RGB:
    case Image::PF_BGR:
        glPixelFormat = GL_RGB;
        break;
    case Image::PF_RGBA:
        glPixelFormat = GL_RGBA;
        break;
    case Image::PF_GRAYSCALE:
        glPixelFormat = GL_LUMINANCE;   // FIXME: is it valid on ES?
        break;
    default:
        base::glog << "Invalid GL texture pixel format" << base::logwarn;
        return false;
    }

    if (m_textureType == TEXTURE_1D)
    {
        return false;
    }
    else if (m_textureType == TEXTURE_2D)
    {
        // FIXME:
        // Init empty texture.
        glTexImage2D(m_glType, 0, glPixelFormat, m_actualWidth, m_actualHeight, 0, glPixelFormat, GL_UNSIGNED_BYTE, nullptr);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_image->width(), m_image->height(), glPixelFormat, GL_UNSIGNED_BYTE, m_image->data());
    }
    else if (m_textureType == TEXTURE_3D)
    {
        return false;
        //glTexImage3D(m_glType, 0, glPixelFormat, m_image->width(), m_image->depth(), ..., 0,
        //    glPixelFormat, GL_UNSIGNED_BYTE, )
    }
    else if (m_textureType == TEXTURE_CUBE)
    {
#if defined(__WIN32__)
        return false;
        //GLsizei width = m_width / 6;
        //GLsizei height = m_height;

        //glPixelStorei(GL_UNPACK_ROW_LENGTH, m_width);

        //for (size_t i = 0; i < 6; i++)
        //{
        //    glPixelStorei(GL_UNPACK_SKIP_PIXELS, i * width);
        //    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glPixelFormat, width, height,
        //        0, glPixelFormat, GL_UNSIGNED_BYTE, m_data);
        //}

        //glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        //glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
#else
        return false;
#endif
    }
    else
    {
        glBindTexture(m_glType, 0);
        return false;
    }

    if (m_mipmapped)
        glGenerateMipmap(m_glType);

    glBindTexture(m_glType, 0);

    printOpenGLError();

    return true;
}


Texture::Texture()
    : m_glType(GL_INVALID_ENUM)
{
}

Texture::~Texture()
{
    if (m_glid)
    {
        glBindTexture(m_glType, 0);
        glDeleteTextures(1, &m_glid);
    }
}

void Texture::setType(Texture::Type newType)
{
    m_textureType = newType;

    // TODO:
    // Should recreate texture!!!
}

void Texture::setFilteringMode(Texture::Filtering newFiltering)
{
    m_filteringMode = newFiltering;

    // TODO:
    // Should recreate texture!!!
}

void Texture::setMipmapped(bool hasMipmaps)
{
    m_mipmapped = hasMipmaps;
}

void Texture::setWrapMode(WrapMode mode)
{
    m_wrapMode = mode;
}

void Texture::setImage(shared_ptr<Image> image)
{
    if (!image)
    {
        base::glog << "Trying to set null image to a texture" << base::logwarn;
        return;
    }

    m_image = image;
}

shared_ptr<Image> Texture::getImage() const
{
    return m_image;
}

size_t Texture::getOriginalWidth() const
{
    return m_image ? m_image->width() : 0;
}

size_t Texture::getOriginalHeight() const
{
    return m_image ? m_image->height() : 0;
}

size_t Texture::getActualWidth() const
{
    return m_actualWidth;
}

size_t Texture::getActualHeight() const
{
    return m_actualHeight;
}

bool Texture::bind(size_t unit)
{
    if (!m_image || !m_image->valid())
        return false;

    if (createGLTexture())
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(m_glType, m_glid);
        return true;
    }
    else
        return false;
}

void Texture::unbind()
{
    if (m_glid == 0)
        return;

    glBindTexture(m_glType, 0);
}

void Texture::recreate()
{
    if (!m_image->valid())
        return;
    // TODO:
}

} }