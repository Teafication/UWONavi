#include "stdafx.h"
#include "ShipRouteList.h"
#include <fstream>

namespace {
    // Structure to define the file header for route data serialization
    struct FileHeaderV1 {
        enum {
            k_FileVersion = 1,  // Version of the file format
        };
        const uint32_t version = k_FileVersion;  // File version
        uint32_t favoritsCount = 0;  // Number of favorite routes in the file
    };

    typedef FileHeaderV1 FileHeader;  // Alias for the file header structure
}


// Serialization of the ShipRouteList to an output stream
std::ostream& operator <<(std::ostream& os, const ShipRouteList& shipRouteList)
{
    _ASSERT(os.good());  // Ensure the output stream is in a good state
    if (!os.good()) {
        throw std::runtime_error("output stream error.");
    }

    FileHeader fileHeader;  // Create a file header

    const auto headPos = os.tellp();  // Get the current position in the stream
    os.seekp(sizeof(fileHeader), std::ios::cur);  // Move the write position forward to leave space for the header

    // Write each favorite route to the stream
    for (const auto& shipRoute : shipRouteList.m_shipRouteList) {
        if (shipRoute->isFavorite()) {
            ++fileHeader.favoritsCount;  // Increment the favorite count
            os << *shipRoute;  // Serialize the route if it's a favorite
        }
    }

    const auto tailPos = os.tellp();  // Get the position after writing all the routes

    os.seekp(headPos, std::ios::beg);  // Go back to the position of the header
    os.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));  // Write the header
    os.seekp(tailPos, std::ios::beg);  // Move the write position back to where it was

    _ASSERT(os.good());  // Ensure the output stream is still good after writing
    return os;  // Return the output stream
}


// Deserialization of the ShipRouteList from an input stream
std::istream& operator >>(std::istream& is, ShipRouteList& shipRouteList)
{
    _ASSERT(is.good());  // Ensure the input stream is in a good state
    if (!is.good()) {
        throw std::runtime_error("input stream error.");
    }

    FileHeader fileHeader;  // Create a file header
    is.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));  // Read the header from the stream

    if (fileHeader.version != FileHeader::k_FileVersion) {
        throw std::runtime_error("unknown file version.");
    }

    ShipRouteList::RouteList workRouteList;

    // Read each favorite route from the stream
    for (uint32_t i = 0; i < fileHeader.favoritsCount; ++i) {
        ShipRoutePtr shipRoute(new ShipRoute());
        is >> *shipRoute;  // Deserialize the route from the stream
        workRouteList.push_back(std::move(shipRoute));  // Add the route to the list
    }

    shipRouteList.m_shipRouteList.swap(workRouteList);  // Swap the new list with the current one

    _ASSERT(is.good());  // Ensure the input stream is still good after reading
    return is;  // Return the input stream
}


// Close the current route by marking it as fixed (non-editable)
void ShipRouteList::closeRoute()
{
    if (!m_shipRouteList.empty()) {
        m_shipRouteList.back()->setFix(true);  // Mark the last route as fixed
    }
}


// Add a new route point to the current route. If the last route is fixed, start a new route.
void ShipRouteList::addRoutePoint(const NormalizedPoint point)
{
    if (m_shipRouteList.empty() || m_shipRouteList.back()->isFixed()) {
        addRoute();  // If the last route is fixed or there are no routes, start a new route
    }
    m_shipRouteList.back()->addRoutePoint(point);  // Add the point to the last route

    if (m_observer) {
        m_observer->onShipRouteListUpdateRoute(m_shipRouteList.back());  // Notify the observer about the route update
    }
}


// Remove a specific ship route from the list
void ShipRouteList::removeShipRoute(ShipRoutePtr shipRoute)
{
    _ASSERT(shipRoute != nullptr);  // Ensure the route to remove is valid
    auto it = std::find(m_shipRouteList.begin(), m_shipRouteList.end(), shipRoute);
    if (it == m_shipRouteList.end()) {
        return;  // If the route is not found, do nothing
    }
    ShipRoutePtr removeTarget = shipRoute;
    m_shipRouteList.erase(it);  // Erase the route from the list

    if (m_observer) {
        m_observer->onShipRouteListRemoveItem(removeTarget);  // Notify the observer about the route removal
    }
}


// Clear all ship routes from the list
void ShipRouteList::clearAllItems()
{
    m_shipRouteList.clear();  // Clear the list of routes
    if (m_observer) {
        m_observer->onShipRouteListRemoveAllItems();  // Notify the observer that all routes have been removed
    }
}


// Add a new route to the list
void ShipRouteList::addRoute()
{
    // Add a new empty route to the list
    m_shipRouteList.push_back(ShipRoutePtr(new ShipRoute()));
    if (m_observer) {
        m_observer->onShipRouteListAddRoute(m_shipRouteList.back());  // Notify the observer about the new route
    }

    // Remove excess routes if there are too many routes without favorites
    int favorits = 0;
    for (auto route : m_shipRouteList) {
        if (route->isFavorite()) {
            ++favorits;
        }
    }

    const int overCount = m_shipRouteList.size() - (m_maxRouteCountWithoutFavorits + favorits);
    if (0 < overCount) {
        int removeCount = 0;
        auto it = m_shipRouteList.begin();
        while (it != m_shipRouteList.end() && removeCount < overCount) {
            if ((*it)->isFavorite()) {
                ++it;
                continue;  // Skip removing favorite routes
            }
            auto itNext = std::next(it, 1);
            m_shipRouteList.erase(it);  // Remove non-favorite routes
            ++removeCount;
            it = itNext;
        }
    }
}


// Join the current route with a previous route by reverse index
void ShipRouteList::joinPreviousRouteAtReverseIndex(int reverseIndex)
{
    _ASSERT(0 <= reverseIndex);
    _ASSERT(reverseIndex < (int)m_shipRouteList.size());  // Ensure the index is valid

    RouteList::iterator itBase = m_shipRouteList.begin();
    std::advance(itBase, indexFromReverseIndex(reverseIndex));  // Get the base route at the reverse index
    RouteList::iterator itPrev = std::prev(itBase);  // Get the previous route
    _ASSERT(itPrev != m_shipRouteList.end());  // Ensure the previous route exists
    ShipRoutePtr baseRoute = *itBase;
    ShipRoutePtr prevRoute = *itPrev;

    m_shipRouteList.erase(itPrev);  // Remove the previous route from the list

    // Handle world wrapping when joining the two routes
    bool isHilight = prevRoute->isHilight() | baseRoute->isHilight();  // Combine the highlight status of both routes
    if (m_observer) {
        m_observer->onShipRouteListRemoveItem(prevRoute);  // Notify the observer about the removal
    }

    baseRoute->jointPreviousLinesWithRoute(*prevRoute);  // Join the previous route's lines with the base route
    baseRoute->setHilight(isHilight);  // Set the highlight status of the base route

    if (m_observer) {
        m_observer->onShipRouteListUpdateRoute(baseRoute);  // Notify the observer about the update
    }
}
