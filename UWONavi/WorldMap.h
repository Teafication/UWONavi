#pragma once
#include "Noncopyable.h"
#include "Image.h"
#include "Config.h"
#include "Vector.h"
#include "NormalizedPoint.h"

/**
 * WorldMap
 *
 * - Inherits from Noncopyable to ensure we cannot copy
 *   or assign a WorldMap object, preventing duplicate
 *   references to underlying resources like images.
 *
 * - Holds an Image representing the world map (m_mapImage).
 *
 * - Provides methods to load the map image from file,
 *   fetch the map image reference, convert world coordinates
 *   to image coordinates, and retrieve a normalized point.
 */
class WorldMap : private Noncopyable {
    friend class Renderer;

private:
    Image m_mapImage; // The actual image of the world map.

public:
    /**
     * Constructor and destructor are trivial here,
     * but explicitly declared for clarity.
     */
    WorldMap() {}
    virtual ~WorldMap() {}

    /**
     * Loads the world map from the specified file name.
     * Returns true if loading is successful, otherwise false.
     */
    bool loadFromFile(const std::wstring& fileName);

    /**
     * Returns a constant reference to the internally stored Image.
     * Useful for rendering or other read-only operations.
     */
    const Image& image() const {
        return m_mapImage;
    }

    /**
     * Converts a world coordinate (e.g. position in the game world)
     * into a corresponding coordinate in the map image space.
     * This helps render or track points on the map’s image.
     */
    POINT imageCoordFromWorldCoord(const POINT& worldCoord) const;

    /**
     * Creates a NormalizedPoint from the given world coordinate.
     * A NormalizedPoint is scaled to a range [0..1] for both
     * x and y, based on the k_worldWidth and k_worldHeight constants.
     */
    NormalizedPoint normalizedPoint(const POINT worldCoord) const {
        float nx = worldCoord.x / static_cast<float>(k_worldWidth);
        float ny = worldCoord.y / static_cast<float>(k_worldHeight);
        return NormalizedPoint(nx, ny);
    }
};
