#pragma once
#include <iostream>
#include <list>
#include "UWONavi.h"
#include "ShipRoute.h"
#include "NormalizedPoint.h"

class ShipRouteList;
class IShipRouteListObserver;

//! @brief Represents a list of ship routes with the ability to add, remove, and update routes.
class ShipRouteList {
    //! @note Friend declaration for serialization and deserialization
    friend std::ostream& operator <<(std::ostream& os, const ShipRouteList& shipRouteList);
    friend std::istream& operator >>(std::istream& is, ShipRouteList& shipRouteList);

private:
    typedef std::list<ShipRoutePtr> RouteList;  // List of ship route pointers

private:
    RouteList m_shipRouteList;  //!< List of ship routes
    IShipRouteListObserver* m_observer = nullptr;  //!< Observer to notify about route list changes
    size_t m_maxRouteCountWithoutFavorits = 30;  //!< Maximum number of routes allowed without favorites

public:
    ShipRouteList() = default;  // Default constructor
    ~ShipRouteList() = default;  // Default destructor

    //! @brief Set the observer for the route list to notify about changes.
    //! @param observer The observer to be notified about changes
    void setObserver(IShipRouteListObserver* observer)
    {
        m_observer = observer;  // Set the observer
    }

    //! @brief Close the current route by marking it as fixed (non-editable).
    void closeRoute();

    //! @brief Add a new route point to the current route. If the last route is fixed, start a new route.
    //! @param point The point to add to the route
    void addRoutePoint(const NormalizedPoint point);

    //! @brief Get the list of all ship routes.
    //! @return A constant reference to the list of ship routes
    const RouteList& getList() const
    {
        return m_shipRouteList;
    }

    //! @brief Get a route at a specific reverse index (starting from the end).
    //! @param reverseIndex The reverse index (starting from the end of the list)
    //! @return The ship route at the specified reverse index or nullptr if not found
    ShipRoutePtr getRouteAtReverseIndex(int reverseIndex)
    {
        if (m_shipRouteList.size() <= (size_t)reverseIndex) {
            return nullptr;  // If the index is out of bounds, return nullptr
        }
        RouteList::iterator it = m_shipRouteList.begin();
        std::advance(it, indexFromReverseIndex(reverseIndex));  // Move the iterator to the desired index
        _ASSERT(*it != nullptr);  // Ensure the iterator points to a valid route
        return *it;  // Return the ship route at the specified reverse index
    }

    //! @brief Get the reverse index of a given ship route in the list.
    //! @param shipRoute The ship route whose reverse index is to be found
    //! @return The reverse index of the ship route or -1 if the route is not found
    int reverseIndexFromShipRoute(ShipRoutePtr shipRoute) const
    {
        auto it = std::find(m_shipRouteList.crbegin(), m_shipRouteList.crend(), shipRoute);
        if (it == m_shipRouteList.crend()) {
            return -1;  // If the route is not found, return -1
        }
        const int reverseIndex = std::distance(m_shipRouteList.crbegin(), it);
        return reverseIndex;  // Return the reverse index of the ship route
    }

    //! @brief Remove a specific ship route from the list.
    //! @param shipRoute The ship route to remove from the list
    void removeShipRoute(ShipRoutePtr shipRoute);

    //! @brief Clear all ship routes from the list.
    void clearAllItems();

    //! @brief Join the current route with a previous route at a specific reverse index.
    //! @param reverseIndex The reverse index of the route to join with
    void joinPreviousRouteAtReverseIndex(int reverseIndex);

private:
    //! @brief Convert a reverse index to a regular index.
    //! @param reverseIndex The reverse index (starting from the end)
    //! @return The regular index in the list
    int indexFromReverseIndex(int reverseIndex) const
    {
        return m_shipRouteList.size() - reverseIndex - 1;  // Convert reverse index to regular index
    }

    //! @brief Add a new empty route to the list.
    void addRoute();
};


//! @brief Observer interface for receiving notifications about changes in the ship route list.
class IShipRouteListObserver {
public:
    IShipRouteListObserver() = default;  // Default constructor
    virtual ~IShipRouteListObserver() = default;  // Default destructor

    //! @brief Notify when a new ship route is added to the list.
    //! @param shipRoute The ship route that was added
    virtual void onShipRouteListAddRoute(ShipRoutePtr shipRoute) = 0;

    //! @brief Notify when a ship route is updated in the list.
    //! @param shipRoute The ship route that was updated
    virtual void onShipRouteListUpdateRoute(ShipRoutePtr shipRoute) = 0;

    //! @brief Notify when a ship route is removed from the list.
    //! @param shipRoute The ship route that was removed
    virtual void onShipRouteListRemoveItem(ShipRoutePtr shipRoute) = 0;

    //! @brief Notify when all ship routes are removed from the list.
    virtual void onShipRouteListRemoveAllItems() = 0;
};
