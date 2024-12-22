#include "stdafx.h"
#include "Texture.h"
#include "Image.h"

// Constructor for Texture class
// Initializes the texture ID and generates a new texture in OpenGL.
Texture::Texture() :
    m_texID(),
    m_width(),
    m_height()
{
    // Generate a texture ID using OpenGL's glGenTextures function
    ::glGenTextures(1, &m_texID);
}

// Destructor for Texture class
// Deletes the texture from OpenGL when the Texture object is destroyed.
Texture::~Texture()
{
    // Check if the texture ID is valid (non-zero)
    if (m_texID) {
        unbind();  // Unbind the texture before deleting
        ::glDeleteTextures(1, &m_texID);  // Delete the texture using OpenGL
    }
}

// Set the image data for this texture
// Binds the texture, uploads image data, and sets appropriate OpenGL texture parameters.
void Texture::setImage(const Image& image)
{
    bind();  // Bind the texture so we can modify it

    // Check the pixel format of the image
    if (image.pixelFormat() == k_PixelFormat_RGB) {
        // If the image is in RGB format (24-bit), set up OpenGL for RGB texture
        ::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // Set unpack alignment to 1 byte
        ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  // Specify the 2D texture
            image.width(), image.height(),
            0, GL_BGR_EXT,                       // Use BGR instead of RGB for OpenGL compatibility
            GL_UNSIGNED_BYTE, image.imageBits()); // Provide the image data as unsigned bytes
    }
    else if (image.pixelFormat() == k_PixelFormat_RGBA) {
        // If the image is in RGBA format (32-bit), set up OpenGL for RGBA texture
        ::glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // Set unpack alignment to 4 bytes
        ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  // Specify the 2D texture
            image.width(), image.height(),
            0, GL_BGRA_EXT,                       // Use BGRA instead of RGBA for OpenGL compatibility
            GL_UNSIGNED_BYTE, image.imageBits()); // Provide the image data as unsigned bytes
    }
    else {
        // If the image format is unsupported, abort the operation
        abort();
    }

    // Store the image dimensions (width and height) for later use
    m_width = image.width();
    m_height = image.height();

    unbind();  // Unbind the texture after the operation is complete
}

// Bind the texture to OpenGL
// This makes the texture the active texture for subsequent rendering operations.
void Texture::bind()
{
    ::glEnable(GL_TEXTURE_2D);            // Enable 2D texturing in OpenGL
    ::glBindTexture(GL_TEXTURE_2D, m_texID);  // Bind the texture to the GL_TEXTURE_2D target

    // Set the texture environment to replace the texture color with the texture itself
    ::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Set texture filtering parameters
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Nearest neighbor filtering for magnification
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Nearest neighbor filtering for minification
}

// Unbind the texture from OpenGL
// This is done after using the texture to ensure that other operations can be done without modifying the texture.
void Texture::unbind()
{
    ::glBindTexture(GL_TEXTURE_2D, 0);  // Unbind the texture (set the current texture to 0)
    ::glDisable(GL_TEXTURE_2D);         // Disable 2D texturing in OpenGL
}
