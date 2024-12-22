#pragma once

// Include necessary headers
#include <cstdint>      // For fixed-width integer types (uint8_t, uint32_t)
#include <memory>       // For memory management (std::unique_ptr, etc.)
#include <vector>       // For using vectors
#include <Windows.h>    // For Windows-specific types (HBITMAP, SIZE, etc.)
#include <string>       // For using std::wstring

#include "Noncopyable.h"  // To prevent copying of Image instances

// Enum to define pixel formats
//! @brief Defines different pixel formats supported by the image
enum PixelFormat {
    k_PixelFormat_Unknown,  //!< Unknown pixel format
    k_PixelFormat_RGB,      //!< 24-bit RGB format (R8G8B8)
    k_PixelFormat_RGBA,     //!< 32-bit RGBA format (R8G8B8A8)
};

// The Image class is responsible for image manipulation and loading.
class Image : private Noncopyable {
private:
    HBITMAP m_hbmp;              // Handle to the bitmap object
    SIZE m_size;                 // Size of the image (width and height)
    PixelFormat m_pixelFormat; // Pixel format (RGB or RGBA)
    uint8_t* m_bits;             // Pointer to the image pixel data
    uint32_t m_stride;           // The number of bytes per row (stride)

public:
    // Constructor: Initializes an empty image with default values
    Image() :
        m_hbmp(nullptr),
        m_size(),
        m_bits(nullptr),
        m_stride(0),
        m_pixelFormat(k_PixelFormat_Unknown)
    {
    }

    // Destructor: Cleans up and resets the image
    ~Image()
    {
        reset();
    }

    // Resets the image: Deletes the bitmap and clears all member variables
    void reset()
    {
        if (m_hbmp) {
            ::DeleteObject(m_hbmp);  // Delete the bitmap object
            m_hbmp = nullptr;
        }
        m_size = SIZE();  // Reset the size to default (0, 0)
        m_stride = 0;     // Reset the stride
        m_pixelFormat = k_PixelFormat_Unknown;  // Reset pixel format
    }

    // Copies the contents of another image (src) to this one
    void copy(const Image& src)
    {
        createImage(src.m_size, src.m_pixelFormat);  // Create a new image with the same size and format
        ::memcpy(m_bits, src.m_bits, src.m_size.cy * src.m_stride);  // Copy the pixel data
    }

    // Stretch-copies the source image into this image, resizing it to the new size
    bool stretchCopy(const Image& src, const SIZE& size)
    {
        return stretchCopy(src, size.cx, size.cy);  // Calls the other overload with the width and height
    }

    // Stretch-copies the source image into this image, resizing it to the specified width and height
    bool stretchCopy(const Image& src, uint32_t width, uint32_t height);

    // Checks if this image is compatible with the given size (width and height)
    bool isCompatible(const SIZE& size) const
    {
        if (!m_hbmp) {
            return false;  // If no bitmap is assigned, it's not compatible
        }
        if (m_size.cx != size.cx || m_size.cy != size.cy) {
            return false;  // If the size doesn't match, it's not compatible
        }
        return true;  // If everything matches, it's compatible
    }

    // Gets the handle to the bitmap object (HBITMAP)
    HBITMAP bitmapHandle() const
    {
        return m_hbmp;
    }

    // Returns the size of the image (width and height)
    const SIZE& size() const
    {
        return m_size;
    }

    // Returns the width of the image
    LONG width() const
    {
        return m_size.cx;
    }

    // Returns the height of the image
    LONG height() const
    {
        return m_size.cy;
    }

    // Returns the pixel format of the image (RGB or RGBA)
    PixelFormat pixelFormat() const
    {
        return m_pixelFormat;
    }

    // Returns the stride of the image (number of bytes per row)
    uint32_t stride() const
    {
        return m_stride;
    }

    // Returns a pointer to the image's pixel data (constant access)
    const uint8_t* imageBits() const
    {
        return m_bits;
    }

    // Returns a pointer to the image's pixel data (mutable access)
    uint8_t* mutableImageBits()
    {
        return m_bits;
    }

    // Creates a new image with the specified width, height, and pixel format
    bool createImage(int width, int height, PixelFormat pixelFormat = k_PixelFormat_RGB);

    // Overload of createImage that accepts a SIZE object
    bool createImage(const SIZE& size, PixelFormat pixelFormat = k_PixelFormat_RGB)
    {
        return createImage(size.cx, size.cy, pixelFormat);  // Call the other overload
    }

    // Loads an image from a file, given the file name
    bool loadFromFile(const std::wstring& fileName);
};
