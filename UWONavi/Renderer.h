#pragma once

// Include necessary headers
#include "Noncopyable.h"  // For preventing copying of the renderer
#include "Vector.h"       // For vector operations, like ship direction
#include "Image.h"        // For image manipulation (loading textures)
#include "ShipRoute.h"    // For ship routes and related operations

class Config;            // Forward declaration for Config class
class WorldMap;         // Forward declaration for WorldMap class
class Texture;          // Forward declaration for Texture class
class ShipRouteList;    // Forward declaration for ShipRouteList class

// Renderer is responsible for rendering the world map, ship position, routes, and overlays.
class Renderer : private Noncopyable {
private:
    // Private member variables for rendering and map management
    const WorldMap* m_worldMap;            //!< World map object
    Texture* m_worldMapTexture;            //!< Texture for the world map
    HDC m_hdcPrimary;                         //!< Handle to the primary device context
    HGLRC m_hglrc;                            //!< Handle to the OpenGL rendering context
    SIZE m_viewSize;                          //!< Size of the rendering window
    double m_viewScale;                       //!< Current zoom level of the map
    POINT m_focusPointInWorldCoord;           //!< World coordinates of the center of the view
    POINT m_shipPointInWorld;                 //!< Position of the ship in world coordinates
    bool m_shipVectorLineEnabled;             //!< Flag to control ship vector line rendering
    bool m_speedMeterEnabled;                 //!< Flag to control speedometer rendering
    bool m_traceShipEnabled;                  //!< Flag to control ship position tracking

public:
    // Constructor: Initializes all member variables with default values
    Renderer() :
        m_hdcPrimary(),
        m_viewSize(),
        m_viewScale(1.0),
        m_focusPointInWorldCoord(),
        m_shipPointInWorld(),
        m_shipVectorLineEnabled(true),
        m_speedMeterEnabled(true),
        m_traceShipEnabled(true)
    {
    }

    // Destructor: Currently empty, but can be extended for cleanup
    ~Renderer() {}

    // Setup the renderer with config, primary device context, and world map
    void setup(const Config* config, HDC hdcPrimary, const WorldMap* worldMap);

    // Clean up resources, including OpenGL context and textures
    void teardown();

    // Set the view size (rendering window size)
    void setViewSize(const SIZE& viewSize);

    // Zoom in and zoom out methods, return true if the scale changed
    bool zoomIn();
    bool zoomOut();

    // Reset the zoom level to the default (1.0)
    void resetViewScale();

    // Getter for the current view scale
    inline double viewScale() const { return m_viewScale; }

    // Offset the focus point in the view (for drag functionality)
    void offsetFocusInViewCoord(const POINT& offset);

    // Set the position of the ship in world coordinates
    void setShipPositionInWorld(const POINT& shipPositionInWorld);

    // Enable or disable ship position tracking (ship trace)
    void enableTraceShip(bool enabled) { m_traceShipEnabled = enabled; }

    // Enable or disable the ship vector line (ship's heading)
    void setVisibleShipRoute(bool visible) { m_shipVectorLineEnabled = visible; }

    // Render the scene: map, ship vector, speed meter, and ship routes
    void render(const Vector& shipVector, double shipVelocity, Texture* shipTexture, const ShipRouteList* shipRouteList);

    // Enable or disable the speedometer display
    void enableSpeedMeter(bool enabled) { m_speedMeterEnabled = enabled; }

    // Create a texture from an image (for rendering)
    Texture* createTextureFromImage(const Image& image);

private:
    // Initialize configuration settings (like initial survey coordinates)
    void setConfig(const Config* config);

    // Initialize OpenGL context and settings
    void setupGL();

    // Set the world map for rendering (apply texture)
    void setWorldMap(const WorldMap* worldMap);

    // Get the size of the scaled map based on the current view scale
    SIZE scaledMapSize() const;

    // Get the origin (top-left corner) of the map in view coordinates
    POINT mapOriginInView() const;

    // Calculate the drawing offset in view coordinates based on world coordinates
    POINT drawOffsetFromWorldCoord(const POINT& worldCoord) const;

    // Render the world map and associated elements
    void renderMap(const Vector& shipVector, Texture* shipTexture, const ShipRouteList* shipRouteList);

    // Render the list of ship routes
    void renderShipRouteList(int width, int height, const ShipRouteList* shipRouteList);

    // Render the speedometer with the ship's velocity
    void renderSpeedMeter(double shipVelocity);

    // Render the individual lines of a ship route
    void renderLines(const ShipRoutePtr shipRoute, float mapWidth, float mapHeight);

    // Render a texture on the screen
    void renderTexture(Texture& texture, float w, float h);
    void renderTexture(Texture& texture, float x, float y, float w, float h);

    // Helper function to get the center point of the view
    inline POINT viewCenterPoint() const {
        POINT p = { m_viewSize.cx / 2, m_viewSize.cy / 2 };
        return p;
    }
};
