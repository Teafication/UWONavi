#pragma once

#include "Noncopyable.h"  // Include for preventing copy operations on this class
#include "Image.h"        // Include the image handling class to interact with image data

//! @brief The Texture class manages OpenGL texture objects.
//! It is responsible for creating, binding, unbinding, and setting texture data in OpenGL.
class Texture : private Noncopyable {
private:
    GLuint m_texID;  //!< Texture ID used by OpenGL to identify this texture
    int m_width;     //!< Width of the texture
    int m_height;    //!< Height of the texture

public:
    //! @brief Default constructor
    Texture();

    //! @brief Destructor
    //! Frees the resources used by the texture.
    ~Texture();

    //! @brief Returns the width of the texture
    int width() const
    {
        return m_width;
    }

    //! @brief Returns the height of the texture
    int height() const
    {
        return m_height;
    }

    //! @brief Sets the image data for the texture
    //! @param image The Image object containing image data to upload as the texture
    void setImage(const Image& image);

    //! @brief Binds the texture to OpenGL so it can be used for rendering
    void bind();

    //! @brief Unbinds the texture, making it inactive for rendering
    void unbind();
};
