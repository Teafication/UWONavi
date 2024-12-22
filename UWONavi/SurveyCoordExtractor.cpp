#include "stdafx.h"
#include "UWONavi.h"
#include "SurveyCoordExtractor.h"

namespace {
    // Constants and sample bit patterns for detecting numbers in the image
    const int k_numberWidth = 5;  // Width of a number in pixels (in terms of bit patterns)
    const std::vector<std::string> k_sampleBits = {
        "00111111100"
        "01000000010"
        "01000000010"
        "00111111100"
        "00000000000",    // 0
        "00100000000"
        "01111111110"
        "00000000000"
        "00000000000"
        "00000000000",    // 1
        "00110000110"
        "01000011010"
        "01000100010"
        "00111000010"
        "00000000000",    // 2
        "00110001100"
        "01000100010"
        "01000100010"
        "00111011100"
        "00000000000",    // 3
        "00000011000"
        "00001101000"
        "00110001000"
        "01111111110"
        "00000001000",    // 4
        "01111101100"
        "01001000010"
        "01001000010"
        "01000111100"
        "00000000000",    // 5
        "00111111100"
        "01000100010"
        "01000100010"
        "00110011100"
        "00000000000",    // 6
        "01000000000"
        "01000001110"
        "01001110000"
        "01110000000"
        "00000000000",    // 7
        "00111011100"
        "01000100010"
        "01000100010"
        "00111011100"
        "00000000000",    // 8
        "00111001100"
        "01000100010"
        "01000100010"
        "00111111100"
        "00000000000",    // 9
    };
};

// Constructor: Initializes the extractor with an image
SurveyCoordExtractor::SurveyCoordExtractor(const Image& image)
    : m_image(image),
    m_width(image.size().cx),
    m_height(image.size().cy),
    m_extractOffset()
{
}

// Destructor: Clean up any resources (if any) used by the extractor
SurveyCoordExtractor::~SurveyCoordExtractor()
{
}

// Main function for extracting numbers from the image
std::vector<int> SurveyCoordExtractor::extractNumbers()
{
    std::vector<int> values;

    // If the image height is 11 pixels, proceed with extraction for height 11
    if (m_height == 11) {
        resetExtractState();  // Reset extraction state (offset)
        values = extractNumbersForHeight11();  // Extract numbers from the image
    }

#ifndef NDEBUG
    // Debugging: Copy the binarized image back into the image for visualization
    {
        const uint8_t* s = &m_binalizedImage[0];
        uint8_t* d = const_cast<Image&>(m_image).mutableImageBits();
        for (uint32_t i = 0; i < m_binalizedImage.size(); ++i) {
            const uint8_t v = *s++;
            *d++ = v;
            *d++ = v;
            *d++ = v;
        }
    }
#endif
    return values;  // Return the extracted values (numbers)
}

// Extract numbers from the image when the height is 11 pixels
std::vector<int> SurveyCoordExtractor::extractNumbersForHeight11()
{
    const int dxThreshold = int(k_numberWidth + 4);  // Threshold for horizontal distance between numbers
    std::vector<int> values;  // Vector to store the extracted numbers
    std::string number;  // String to accumulate digits for each number

    // Process the image and extract numbers
    while (m_extractOffset < m_width) {
        const int prevOffset = m_extractOffset;
        const int v = extractOneNumbersForHeight11();  // Extract one number at a time
        const int dx = m_extractOffset - prevOffset;

        // If the horizontal distance between numbers is too large, save the current number and reset
        if (dxThreshold < dx) {
            if (0 < number.length()) {
                values.push_back(std::stoi(number));  // Convert string to number and add to the values
            }
            number = "";  // Reset number string
        }

        // If a valid digit (0-9) was found, append it to the current number string
        if (0 <= v && v <= 9) {
            number += std::to_string(v);
        }
    }

    // If any remaining number exists, add it to the list
    if (0 < number.length()) {
        values.push_back(std::stoi(number));  // Convert the remaining number string to integer and add
    }
    return values;
}

// Extract a single number from the image (for height 11 pixels)
int SurveyCoordExtractor::extractOneNumbersForHeight11()
{
    const std::vector<uint8_t>& binalizedImage = binalizeImage();  // Get the binarized image data
    const size_t maskLength = m_height * k_numberWidth;  // The length of the number mask

    bool found = false;  // Flag to track if a number was found
    std::string bitString;  // String to store the bit pattern of the current number

    BitsDictionary candidates;  // Dictionary to store potential matching candidates
    resetCandidates(candidates);  // Reset the candidates for each number

    // Loop through each column of the image, starting from the extract offset
    for (uint32_t x = m_extractOffset; x < m_width; ++x) {
        uint32_t vert = 0;
        std::string vertString;

        // Extract vertical bit pattern for each pixel in the column
        for (uint32_t y = 0; y < m_height; ++y) {
            const uint8_t v = binalizedImage[y * m_width + x] ? 1 : 0;  // Get pixel value (binary)
            vert = (vert << 1) | v;  // Update the vertical bit pattern
            vertString += (v) ? '1' : '0';  // Store the bit as '1' or '0'
        }

        // Skip invalid columns (fully white or fully black)
        if (!found) {
            if (vert == 0 || vert == 0x3FF) {  // Skip all-white or all-black columns
                continue;
            }
            found = true;  // Mark that a number has started to be found
        }

        bitString += vertString;  // Append the vertical bit string

        // If the bit string is shorter than the mask length, continue accumulating bits
        if (bitString.size() < maskLength) {
            for (size_t i = 0; i < k_sampleBits.size(); ++i) {
                const std::string& sample = k_sampleBits[i];
                if (sample.compare(0, bitString.length(), bitString) == 0) {
                    candidates[&sample] = i;  // Add to candidates if the bit string matches the prefix
                }
                else {
                    candidates.erase(&sample);  // Remove from candidates if the bit doesn't match
                }
            }
            if (candidates.empty()) {
                bitString = "";  // Reset if no candidates match
                resetCandidates(candidates);  // Reset the candidate list
            }
            continue;
        }
        else {
            // Once the bit string reaches the mask length, check for a valid match
            for (auto it = candidates.begin(); it != candidates.end(); ++it) {
                const std::string& candidate = *it->first;
                const int number = it->second;
                if (candidate.compare(bitString) == 0) {
                    m_extractOffset = x + 1;  // Update the extract offset for the next number
                    return number;  // Return the matched number
                }
            }
            // Break if no match is found
            break;
        }
    }

    m_extractOffset = m_width;  // If no number is found, set the extract offset to the end of the image
    return -1;  // Return -1 to indicate no number found
}

// Reset the extraction state (e.g., the extraction offset)
void SurveyCoordExtractor::resetExtractState()
{
    m_extractOffset = 0;
}

// Reset the candidates dictionary (used for matching bit patterns)
void SurveyCoordExtractor::resetCandidates(BitsDictionary& bitsDictionary)
{
    bitsDictionary.clear();
    for (size_t i = 0; i < k_sampleBits.size(); ++i) {
        bitsDictionary[&k_sampleBits[i]] = i;  // Initialize the dictionary with sample bit patterns
    }
}

// Binarize the image (convert the image to black and white)
std::vector<uint8_t> SurveyCoordExtractor::binalizeImage()
{
    if (m_binalizedImage.empty()) {
        const uint32_t bytesPerPixel = 3;  // RGB 24bit image format
        const uint32_t stride = m_width * bytesPerPixel;
        const uint8_t* const bits = m_image.imageBits();

        std::vector<uint8_t> binalizedImage;
        binalizedImage.resize(m_width * m_height);

        // Process each pixel and convert to black and white (binary)
        for (size_t i = 0; i < binalizedImage.size(); ++i) {
            const uint32_t offset = i * bytesPerPixel;
            const uint16_t r = bits[offset + 0];
            const uint16_t g = bits[offset + 1];
            const uint16_t b = bits[offset + 2];

            // Calculate the total intensity of the pixel (sum of RGB)
            const uint16_t total = r + g + b;
            binalizedImage[i] = ((240 * 3) <= total) ? 255 : 0;  // Set to white (255) or black (0) based on intensity
        }

        binalizedImage.swap(m_binalizedImage);  // Swap the result with the member variable
    }

    return m_binalizedImage;  // Return the binarized image
}
