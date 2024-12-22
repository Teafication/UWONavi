#include "stdafx.h"
#include <vector>
#include "UWONavi.h"
#include "WorldMap.h"

/**
 * WorldMap is responsible for storing and handling a visual map of the game world.
 * It uses an internal Image object (m_mapImage) to represent the map data.
 */
bool WorldMap::loadFromFile(const std::wstring& fileName) {
    /**
     * 1. Creates an Image object (workImage).
     * 2. Uses an external helper function g_makeFullPath to form a complete file path.
     * 3. Tries to load the image file into workImage.
     * 4. If successful, copies workImage into m_mapImage and resets workImage.
     * 5. Returns true if the load was successful, or false otherwise.
     */
    Image workImage;
    std::wstring filePath = g_makeFullPath(fileName);

    if (!workImage.loadFromFile(filePath.c_str())) {
        return false;
    }

    m_mapImage.copy(workImage);
    workImage.reset();
    return true;
}

/**
 * Converts a point in world coordinates into a point within the map image.
 * - Normalizes the given worldCoord by dividing by k_worldWidth and k_worldHeight.
 * - Scales those normalized coordinates by the dimensions of the m_mapImage.
 */
POINT WorldMap::imageCoordFromWorldCoord(const POINT& worldCoord) const {
    double xNormPos = worldCoord.x / static_cast<double>(k_worldWidth);
    double yNormPos = worldCoord.y / static_cast<double>(k_worldHeight);

    POINT worldPosInImage = {
        static_cast<LONG>(m_mapImage.width() * xNormPos),
        static_cast<LONG>(m_mapImage.height() * yNormPos)
    };
    return worldPosInImage;
}
