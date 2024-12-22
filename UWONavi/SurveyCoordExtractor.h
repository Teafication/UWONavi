#pragma once

#include <cinttypes>     // For fixed-width integer types (uint8_t, uint32_t)
#include <vector>        // For using vectors
#include <map>           // For using std::map

#include "Noncopyable.h"  // Prevent copying of the class
#include "Image.h"        // Image handling class, provides access to the image data

//! @brief This class is responsible for extracting survey coordinates from an image.
//! It processes the image by binarizing it and comparing the resulting patterns against predefined templates.
class SurveyCoordExtractor : private Noncopyable {
private:
    // Type definition for a dictionary that maps sample bit patterns to corresponding numbers
    typedef std::map<const std::string*, int> BitsDictionary;

private:
    const Image& m_image;           //!< The image from which coordinates will be extracted
    const uint32_t m_width;            //!< The width of the image (in pixels)
    const uint32_t m_height;           //!< The height of the image (in pixels)
    uint32_t m_extractOffset;          //!< The current offset for extraction (where in the image to start looking)

    std::vector<uint8_t> m_binalizedImage;  //!< The binarized (black and white) version of the image

public:
    //! @brief Constructor for SurveyCoordExtractor
    //! @param image The Image object that holds the image data
    SurveyCoordExtractor(const Image& image);

    //! @brief Destructor for SurveyCoordExtractor
    virtual ~SurveyCoordExtractor();

    //! @brief Extracts numbers from the image (coordinates)
    //! @return A vector of integers representing the extracted numbers
    std::vector<int> extractNumbers();

private:
    // Extracts numbers from the image when the height is 11 pixels (specific use case)
    std::vector<int> extractNumbersForHeight11();

    // Extracts a single number from the image (when height is 11 pixels)
    int extractOneNumbersForHeight11();

    // Resets the extraction state, i.e., resets the offset to 0
    void resetExtractState();

    // Resets the candidates dictionary, clearing any previously matched bit patterns
    void resetCandidates(BitsDictionary& bitsDictionary);

    // Converts the image to a binary (black and white) format for easier processing
    std::vector<uint8_t> binalizeImage();
};
