#include "stdafx.h"

// Include necessary libraries for image handling
#include <objbase.h>         // Required for COM initialization (GDI+)
#include <gdiplus.h>         // GDI+ for image handling
#include <gdiplusbitmap.h>   // GDI+ bitmap manipulation

#include "Image.h"        // Image class header

namespace {
    // Helper function to calculate the stride (the number of bytes per row) of an image
    inline uint32_t s_strideFromWidthAndBitsPerPixel(const uint32_t width, const uint32_t bpp)
    {
        uint32_t stride = width * bpp / 8;  // Calculate initial stride
        stride = stride + (4 - stride % 4) % 4;  // Ensure that the stride is a multiple of 4
        return stride;
    }

    // Helper function to copy image data from 32-bit to 24-bit format
    inline void s_copyImage24From32(uint8_t* const dst, const uint8_t* const src, const uint32_t width, const uint32_t height)
    {
        const uint32_t srcStride = s_strideFromWidthAndBitsPerPixel(width, 32);  // Calculate source stride (32-bit)
        const uint32_t dstStride = s_strideFromWidthAndBitsPerPixel(width, 24);  // Calculate destination stride (24-bit)

        const uint8_t* s = src;  // Pointer to the source image data
        uint8_t* d = dst;        // Pointer to the destination image data

        // Loop through each row of the image
        for (uint32_t y = 0; y < height; ++y) {
            s = src + (y * srcStride);  // Move to the start of the current row in the source image
            d = dst + (y * dstStride);  // Move to the start of the current row in the destination image

            // Loop through each pixel in the row
            for (uint32_t x = 0; x < width; ++x) {
                // Copy the RGB channels (32-bit to 24-bit)
                *d++ = *s++;  // Blue channel
                *d++ = *s++;  // Green channel
                *d++ = *s++;  // Red channel
                ++s;          // Skip the alpha channel
            }
        }
    }
}

bool Image::stretchCopy(const Image& src, uint32_t width, uint32_t height)
{
    // Ensure the destination image can be created with the given size
    if (!createImage(width, height)) {
        return false;
    }

    HDC hdc = ::GetDC(NULL);  // Get the device context for the screen
    HDC hdcSrc = ::CreateCompatibleDC(hdc);  // Create a compatible DC for the source image
    HDC hdcDst = ::CreateCompatibleDC(hdc);  // Create a compatible DC for the destination image

    // Save the current state of the device contexts
    ::SaveDC(hdcSrc);
    ::SaveDC(hdcDst);
    ::SelectObject(hdcDst, m_hbmp);  // Select the destination bitmap
    ::SelectObject(hdcSrc, src.m_hbmp);  // Select the source bitmap

    // If the source and destination sizes are different, stretch the image to fit
    if (m_size.cx != src.m_size.cx || m_size.cy != src.m_size.cy) {
        POINT org;
        ::GetBrushOrgEx(hdcDst, &org);  // Save the current brush origin
        ::SetStretchBltMode(hdcDst, HALFTONE);  // Set the stretch mode to halftone for better quality
        ::SetBrushOrgEx(hdcDst, org.x, org.y, NULL);  // Restore the brush origin

        // Stretch the source image to fit the destination
        ::StretchBlt(hdcDst, 0, 0, m_size.cx, m_size.cy,
            hdcSrc, 0, 0, src.m_size.cx, src.m_size.cy,
            SRCCOPY);
    }
    else {
        // If the sizes are the same, just copy the image directly
        ::BitBlt(hdcDst, 0, 0, m_size.cx, m_size.cy,
            hdcSrc, 0, 0, SRCCOPY);
    }

    m_pixelFormat = src.m_pixelFormat;  // Set the pixel format to the source format

    // Restore the device contexts
    ::RestoreDC(hdcSrc, -1);
    ::DeleteDC(hdcSrc);
    ::RestoreDC(hdcDst, -1);
    ::DeleteDC(hdcDst);
    ::ReleaseDC(NULL, hdc);  // Release the screen device context

    return true;
}

bool Image::createImage(int width, int height, PixelFormat pixelFormat)
{
    reset();  // Reset any previous image data

    uint32_t bitCount = 0;

    // Create a bitmap based on the specified pixel format (RGB or RGBA)
    if (pixelFormat == k_PixelFormat_RGB) {
        BITMAPINFOHEADER bmih = { sizeof(bmih) };
        bmih.biWidth = width;
        bmih.biHeight = -height;  // Negative height indicates top-down bitmap
        bmih.biPlanes = 1;
        bmih.biBitCount = 24;
        bmih.biSizeImage = width * height * 3;  // 3 bytes per pixel (RGB)
        m_hbmp = ::CreateDIBSection(NULL, (LPBITMAPINFO)&bmih, DIB_RGB_COLORS, (void**)&m_bits, NULL, 0);
        bitCount = bmih.biBitCount;
    }
    else if (pixelFormat == k_PixelFormat_RGBA) {
        BITMAPV5HEADER bmih = { sizeof(bmih) };
        bmih.bV5Compression = BI_BITFIELDS;
        bmih.bV5BlueMask = 0xFF;
        bmih.bV5GreenMask = 0xFF << 8;
        bmih.bV5RedMask = 0xFF << 16;
        bmih.bV5AlphaMask = 0xFF << 24;
        bmih.bV5Width = width;
        bmih.bV5Height = -height;  // Negative height for top-down bitmap
        bmih.bV5Planes = 1;
        bmih.bV5BitCount = 32;
        bmih.bV5SizeImage = width * height * 4;  // 4 bytes per pixel (RGBA)
        m_hbmp = ::CreateDIBSection(NULL, (LPBITMAPINFO)&bmih, DIB_RGB_COLORS, (void**)&m_bits, NULL, 0);
        bitCount = bmih.bV5BitCount;
    }
    else {
        abort();  // Invalid pixel format
        return false;
    }

    // If the bitmap was created successfully, initialize its properties
    if (!m_hbmp) {
        return false;
    }

    m_size.cx = width;
    m_size.cy = height;
    m_pixelFormat = pixelFormat;
    m_stride = s_strideFromWidthAndBitsPerPixel(width, bitCount);  // Calculate stride (row size)
    return true;
}

bool Image::loadFromFile(const std::wstring& fileName)
{
    reset();  // Reset any previous image data

    std::auto_ptr<Gdiplus::Bitmap> image;  // Use GDI+ to load the image
    HBITMAP hbmp = NULL;
    image.reset(Gdiplus::Bitmap::FromFile(fileName.c_str()));  // Load image from file
    image->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hbmp);  // Convert to HBITMAP format

    // Determine the pixel format based on the source image format
    Gdiplus::PixelFormat srcPixelFormat = image->GetPixelFormat();
    image.reset();  // Release the image object
    if (!hbmp) {
        return false;  // Failed to load image
    }

    PixelFormat pixelFormat;

    // Set the pixel format based on the source image's format
    if (srcPixelFormat & PixelFormatGDI) {
        pixelFormat = k_PixelFormat_RGB;
    }
    else if (srcPixelFormat & (PixelFormatGDI | PixelFormatAlpha)) {
        pixelFormat = k_PixelFormat_RGBA;
    }
    else {
        abort();  // Unsupported format
        return false;
    }

    // Retrieve the image's width and height
    BITMAP bmp = { 0 };
    ::GetObject(hbmp, sizeof(bmp), &bmp);
    if (!createImage(bmp.bmWidth, bmp.bmHeight, pixelFormat)) {
        ::DeleteObject(hbmp);
        return false;  // Failed to create image
    }

    std::vector<uint8_t> buffer;
    buffer.resize(::GetBitmapBits(hbmp, 0, NULL));  // Get the image bits into a buffer
    ::GetBitmapBits(hbmp, buffer.size(), &buffer[0]);  // Copy the image bits into the buffer
    ::DeleteObject(hbmp);

    // Copy the image data into the image object based on the pixel format
    if (pixelFormat == k_PixelFormat_RGB) {
        switch (bmp.bmBitsPixel) {
        case 24:
            ::memcpy(m_bits, &buffer[0], m_stride);  // Direct copy for 24-bit RGB
            break;
        case 32:
            s_copyImage24From32(m_bits, &buffer[0], m_size.cx, m_size.cy);  // Convert 32-bit to 24-bit
            break;
        default:
            return false;  // Unsupported bit count
        }
    }
    else if (pixelFormat == k_PixelFormat_RGBA) {
        ::memcpy(m_bits, &buffer[0], m_stride);  // Direct copy for RGBA
    }
    else {
        abort();  // Invalid format
        return false;
    }
    return true;
}
