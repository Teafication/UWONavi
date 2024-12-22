#pragma once
#include "Noncopyable.h"
#include "ShipRouteList.h"

//! @brief A view that manages and interacts with a list of ship routes.
class ShipRouteManageView : private Noncopyable, public IShipRouteListObserver {
private:
    // Column indices for the list view control, representing columns in the UI
    enum ColumnIndex {
        k_ColumnIndex_StartPoint,   //!< Start point column
        k_ColumnIndex_EndPoint,     //!< End point column
        k_ColumnIndex_Length,       //!< Length of the route column
    };

    // Icon indices used for display icons in the view
    enum IconIndex {
        k_IconIndex_Blank,   //!< Blank icon (used for empty or default states)
        k_IconIndex_Star,    //!< Star icon (used for favorite routes)
    };

    HWND m_hwnd = nullptr;                  //!< Handle to the window (for GUI interactions)
    ShipRouteList* m_routeList = nullptr;  //!< Pointer to the list of ship routes

    HWND m_listViewCtrl = nullptr;         //!< Handle to the list view control (UI element)
    int m_selectionIndex = -1;             //!< Index of the currently selected route (if any)
    ShipRouteWeakPtr m_selectedRoute;   //!< Weak pointer to the currently selected route

    size_t m_visibleCount = 50;            //!< Maximum number of visible routes to display in the view

public:
    ShipRouteManageView() = default;  //!< Default constructor
    ~ShipRouteManageView();           //!< Destructor

    //! @brief Initializes the route view with the provided route list.
    //! @param shipRouteList The list of ship routes to be managed
    bool setup(ShipRouteList& shipRouteList);

    //! @brief Cleans up resources and terminates the route view.
    void teardown();

    //! @brief Makes the window visible and brings it to the foreground.
    void activate()
    {
        ::ShowWindow(m_hwnd, SW_SHOWNORMAL);  // Show the window
        ::SetForegroundWindow(m_hwnd);        // Bring the window to the front
    }

    // Observer methods (called when the ship route list is updated)

    //! @brief Called when a new ship route is added to the list.
    //! @param shipRoute The route that was added
    virtual void onShipRouteListAddRoute(ShipRoutePtr shipRoute) override;

    //! @brief Called when an existing ship route is updated.
    //! @param shipRoute The route that was updated
    virtual void onShipRouteListUpdateRoute(ShipRoutePtr shipRoute) override;

    //! @brief Called when a ship route is removed from the list.
    //! @param shipRoute The route that was removed
    virtual void onShipRouteListRemoveItem(ShipRoutePtr shipRoute) override;

    //! @brief Called when all ship routes are removed from the list.
    virtual void onShipRouteListRemoveAllItems() override;

private:
    // Static function for handling dialog messages
    static INT_PTR CALLBACK dlgProcThunk(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    // Callback function for handling dialog messages
    BOOL CALLBACK dlgProc(UINT msg, WPARAM wp, LPARAM lp);

    //! @brief Handles command events (such as button presses).
    //! @param eventCode The event code of the command
    //! @param cmdId The ID of the command
    //! @param ctrl Handle to the control that triggered the event
    void onCommand(WORD eventCode, WORD cmdId, HANDLE ctrl);

    //! @brief Handles notifications from UI elements.
    //! @param nmh The notification header
    void onNotify(LPNMHDR nmh);

    //! @brief Sets up the list view control to display the routes.
    void setupRouteList();

    //! @brief Updates the count of visible list items in the route view.
    void updateVisibleListItemCount();

    //! @brief Selects a row in the list view.
    //! @param index The index of the row to select
    //! @param isSelection Whether the row should be selected or unselected
    void selectRow(int index, bool isSelection);
};
