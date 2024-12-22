//***********************************************************
//                Header and Dependency Includes
//***********************************************************
#include "stdafx.h"
#include "ShipRouteManageView.h"
#include "UWONavi.h"
#include "Resource.h"
#include "ShipRouteList.h"

//***********************************************************
//                 Constants and Helper Functions
//***********************************************************
namespace {
    // This constant holds the window class name used for creation
    static const wchar_t* const k_windowClassName = L"{8DFEDF35-DD1F-4907-8BC1-12C0CD7080B5}";

    // Helper function to transform the normalized point into a string
    // It multiplies x and y by some world width/height, rounds them, and
    // combines them into a formatted string like "123,456".
    inline std::wstring s_makePointString(const NormalizedPoint& point)
    {
        std::wstring str;
        str = std::to_wstring(static_cast<int>(::round(point.x() * k_worldWidth)))
            + L","
            + std::to_wstring(static_cast<int>(::round(point.y() * k_worldHeight)));
        return std::move(str);
    }
}

//***********************************************************
//          ShipRouteManageView Destructor
//***********************************************************
ShipRouteManageView::~ShipRouteManageView()
{
    teardown();
}

//***********************************************************
//          Setup and Teardown Methods
//***********************************************************
bool ShipRouteManageView::setup(ShipRouteList& shipRouteList)
{
    // Store the provided route list pointer and create the dialog.
    m_routeList = &shipRouteList;
    m_hwnd = ::CreateDialogParam(
        g_hinst,
        MAKEINTRESOURCE(IDD_SHIPROUTEMANAGEVIEW),
        g_hwndMain,
        dlgProcThunk,
        reinterpret_cast<LPARAM>(this)
    );

    // If creation fails, reset routeList pointer to null and return false.
    if (!m_hwnd) {
        m_routeList = NULL;
        return false;
    }

    // If creation succeeds, set this object to be the observer for route list changes.
    m_routeList->setObserver(this);
    return true;
}

void ShipRouteManageView::teardown()
{
    // Destroy the window if it's still around.
    if (m_hwnd) {
        ::DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

//***********************************************************
//          Observer Callbacks for Route List Changes
//***********************************************************
void ShipRouteManageView::onShipRouteListAddRoute(ShipRoutePtr shipRoute)
{
    // Update the list item count and reselect the proper row if needed.
    updateVisibleListItemCount();
    if (ShipRoutePtr selectedRoute = m_selectedRoute.lock()) {
        int reverseIndex = m_routeList->reverseIndexFromShipRoute(selectedRoute);
        selectRow(m_selectionIndex, false);
        selectRow(reverseIndex, true);
    }
}

void ShipRouteManageView::onShipRouteListUpdateRoute(ShipRoutePtr shipRoute)
{
    // Check if the route index is valid, update selection and redraw if necessary.
    int reverseIndex = m_routeList->reverseIndexFromShipRoute(shipRoute);
    if (0 <= reverseIndex) {
        if (shipRoute->isHilight()) {
            selectRow(m_selectionIndex, false);
            selectRow(reverseIndex, true);
        }
        ListView_RedrawItems(m_listViewCtrl, reverseIndex, reverseIndex);
    }
}

void ShipRouteManageView::onShipRouteListRemoveItem(ShipRoutePtr shipRoute)
{
    // If the removed item was the selected route, deselect it.
    if (m_selectedRoute.lock() == shipRoute) {
        if (0 <= m_selectionIndex) {
            selectRow(m_selectionIndex, false);
        }
    }
    updateVisibleListItemCount();
}

void ShipRouteManageView::onShipRouteListRemoveAllItems()
{
    // Clear the entire list view when all routes are removed.
    ListView_DeleteAllItems(m_hwnd);
}

//***********************************************************
//     Dialog Procedure Thunk (Static -> Instance)
//***********************************************************
INT_PTR CALLBACK ShipRouteManageView::dlgProcThunk(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    // Convert the user data to a ShipRouteManageView instance on init.
    ShipRouteManageView* instance = reinterpret_cast<ShipRouteManageView*>(::GetWindowLong(hwnd, GWLP_USERDATA));

    if (msg == WM_INITDIALOG) {
        instance = reinterpret_cast<ShipRouteManageView*>(lp);
        instance->m_hwnd = hwnd;
        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(instance));
    }
    if (!instance) {
        return FALSE;
    }
    return instance->dlgProc(msg, wp, lp);
}

//***********************************************************
//    Dialog Procedure (Main Window Message Handling)
//***********************************************************
BOOL CALLBACK ShipRouteManageView::dlgProc(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG:
        // Initialize the list control for routes and set position on creation.
        setupRouteList();
        {
            RECT rc = { 0 };
            rc = s_screenRectFromClientRect(g_hwndMain, rc);
            ::SetWindowPos(m_hwnd, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        break;

    case WM_COMMAND:
        onCommand(HIWORD(wp), LOWORD(wp), (HWND)lp);
        break;

    case WM_NOTIFY:
        onNotify(reinterpret_cast<LPNMHDR>(lp));
        break;

    case WM_SIZE:
    {
        // Handle resizing of the list control within the dialog.
        RECT rcClient = s_clientRect(m_hwnd);
        RECT rcList = s_clientRectFromScreenRect(m_hwnd, s_windowRect(m_listViewCtrl));
        rcList.right = rcClient.right;
        rcList.bottom = rcClient.bottom;
        int width = rcList.right - rcList.left;
        int height = rcList.bottom - rcList.top;

        ::SetWindowPos(
            m_listViewCtrl,
            NULL,
            0,
            0,
            rcList.right - rcList.left,
            rcList.bottom - rcList.top,
            SWP_NOMOVE | SWP_NOZORDER
        );
    }
    return FALSE;

    default:
        return FALSE;
    }
    return TRUE;
}

//***********************************************************
//    Command Handler (Menu, Buttons, etc.)
//***********************************************************
void ShipRouteManageView::onCommand(WORD eventCode, WORD cmdId, HANDLE ctrl)
{
    switch (cmdId) {
    case IDOK:
    case IDCANCEL:
        // Hides the dialog on OK or Cancel.
        ::ShowWindow(m_hwnd, SW_HIDE);
        break;

    case IDM_DESELECT_ROUTE:
        // Clears the currently selected route.
        if (ShipRoutePtr selectedRoute = m_selectedRoute.lock()) {
            selectRow(m_selectionIndex, false);
        }
        break;

    case IDM_DELETE_SHIP_ROUTE:
        // Removes the currently selected route from the list.
        if (ShipRoutePtr selectedRoute = m_selectedRoute.lock()) {
            selectRow(m_selectionIndex, false);
            m_routeList->removeShipRoute(selectedRoute);
        }
        break;

    case IDM_JOINT_SHIP_ROUTE:
        // Joins the current route with the previous one if applicable.
        if (ShipRoutePtr selectedRoute = m_selectedRoute.lock()) {
            m_routeList->joinPreviousRouteAtReverseIndex(m_selectionIndex);
        }
        break;

    case IDM_TOGGLE_FAVORITE:
        // Toggles the favorite status (represented as a star icon).
        if (ShipRoutePtr selectedRoute = m_selectedRoute.lock()) {
            selectedRoute->setFavorite(!selectedRoute->isFavorite());
            ListView_RedrawItems(m_listViewCtrl, m_selectionIndex, m_selectionIndex);
        }
        break;

    case IDM_JOINT_LATEST_ROUTE:
        // Joins the currently active route with the latest one if more than one exists.
        if (1 < m_routeList->getList().size()) {
            m_routeList->joinPreviousRouteAtReverseIndex(0);
        }
        break;

    default:
        break;
    }
}

//***********************************************************
//        Notification Handler (List View Events)
//***********************************************************
void ShipRouteManageView::onNotify(LPNMHDR nmh)
{
    switch (nmh->idFrom) {
    case IDC_SHIPROUTELIST:
        switch (nmh->code) {
        case LVN_GETDISPINFO:
        {
            // Populates the list view items with appropriate data
            LV_DISPINFO* dispInfo = reinterpret_cast<LV_DISPINFO*>(nmh);
            LVITEM& item = dispInfo->item;
            ShipRoutePtr route = m_routeList->getRouteAtReverseIndex(item.iItem);

            // If route is invalid (maybe removed), refresh list count.
            if (!route) {
                updateVisibleListItemCount();
                break;
            }

            // Fill in text for different columns based on route data
            if (item.mask & LVIF_TEXT) {
                std::wstring str;
                switch (item.iSubItem) {
                case k_ColumnIndex_StartPoint:
                    if (route->getLines().empty()) {
                        str = L"-";
                    }
                    else {
                        if (!route->getLines().front().empty()) {
                            str = s_makePointString(route->getLines().front().front());
                        }
                    }
                    break;
                case k_ColumnIndex_EndPoint:
                    if (route->getLines().empty()) {
                        str = L"-";
                    }
                    else {
                        if (!route->getLines().back().empty()) {
                            str = s_makePointString(route->getLines().back().back());
                        }
                    }
                    break;
                case k_ColumnIndex_Length:
                    if (route->getLines().empty()) {
                        str = L"-";
                    }
                    else {
                        // This example just shows a numeric representation (rounded distance).
                        str = std::to_wstring(static_cast<int>(::round(route->length())));
                    }
                    break;
                default:
                    break;
                }
                ::lstrcpy(item.pszText, str.c_str());
            }

            // Assign an image index for the favorite (star) or blank icon
            if (item.mask & LVIF_IMAGE) {
                if (route->isFavorite()) {
                    item.iImage = k_IconIndex_Star;
                }
                else {
                    item.iImage = k_IconIndex_Blank;
                }
            }
        }
        break;

        case NM_RCLICK:
            // On right-click, display a context menu for the selected route if it’s valid.
            if (ShipRoutePtr selectedRoute = m_selectedRoute.lock()) {
                POINT point = { 0 };
                ::GetCursorPos(&point);
                HMENU menu = ::LoadMenu(g_hinst, MAKEINTRESOURCE(IDR_SHIPROUTEMANAGEPOPUPMENU));
                menu = ::GetSubMenu(menu, 0);

                // If this is the very last route, disable the "join" option.
                size_t count = m_routeList->getList().size();
                if (count == (m_selectionIndex + 1)) {
                    ::EnableMenuItem(menu, IDM_JOINT_SHIP_ROUTE, MF_BYCOMMAND | MF_DISABLED);
                }

                // If it's already a favorite, check that box in the popup.
                if (selectedRoute->isFavorite()) {
                    ::CheckMenuItem(menu, IDM_TOGGLE_FAVORITE, MF_BYCOMMAND | MF_CHECKED);
                }

                // Show the popup menu at the cursor position.
                ::TrackPopupMenu(
                    menu,
                    TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NOANIMATION,
                    point.x,
                    point.y,
                    0,
                    m_hwnd,
                    NULL
                );
            }
            break;

        case LVN_ITEMCHANGED:
        {
            // Detects when an item is selected or deselected, updating highlight accordingly.
            LPNMLISTVIEW nmlv = reinterpret_cast<LPNMLISTVIEW>(nmh);

            // If newly selected, highlight the item and store it as the selected route.
            if (((nmlv->uOldState & LVIS_SELECTED) == 0) && (nmlv->uNewState & LVIS_SELECTED)) {
                m_selectionIndex = nmlv->iItem;
                ShipRoutePtr selectedRoute = m_routeList->getRouteAtReverseIndex(m_selectionIndex);
                selectedRoute->setHilight(true);
                m_selectedRoute = selectedRoute;
                ::InvalidateRect(g_hwndMain, NULL, FALSE);
            }
            // If deselected, remove highlight and reset the selection index.
            else if ((nmlv->uOldState & LVIS_SELECTED) && ((nmlv->uNewState & LVIS_SELECTED) == 0)) {
                if (ShipRoutePtr selectedRoute = m_selectedRoute.lock()) {
                    selectedRoute->setHilight(false);
                    m_selectedRoute.reset();
                }
                m_selectionIndex = -1;
                ::InvalidateRect(g_hwndMain, NULL, FALSE);
            }
        }
        break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

//***********************************************************
//       Setup the Route List Control (ListView)
//***********************************************************
void ShipRouteManageView::setupRouteList()
{
    // Retrieve the handle for our ListView control
    m_listViewCtrl = ::GetDlgItem(m_hwnd, IDC_SHIPROUTELIST);

    // Configure extended styles for the ListView
    DWORD exStyle = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyle(m_listViewCtrl, exStyle);

    // Create an image list and add two icons (blank and star).
    HIMAGELIST imageList;
    imageList = ::ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 0);
    ImageList_AddIcon(imageList, ::LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_BLANK)));
    ImageList_AddIcon(imageList, ::LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_STAR)));
    ListView_SetImageList(m_listViewCtrl, imageList, LVSIL_SMALL);

    // Insert columns for Departure, Arrival, and Distance
    int index = 0;
    LV_COLUMN column = { 0 };
    column.mask = LVCF_TEXT;

    column.pszText = L"Departure";
    ListView_InsertColumn(m_listViewCtrl, index, &column);
    ++index;

    column.pszText = L"Arrival";
    ListView_InsertColumn(m_listViewCtrl, index, &column);
    ++index;

    column.pszText = L"Distance";
    ListView_InsertColumn(m_listViewCtrl, index, &column);
    ++index;

    // Set the total number of items and resize columns
    updateVisibleListItemCount();
    for (int i = 0; i < index; i++) {
        ListView_SetColumnWidth(m_listViewCtrl, i, LVSCW_AUTOSIZE_USEHEADER);
    }
}

//***********************************************************
//   Update the List Control’s Visible Item Count
//***********************************************************
void ShipRouteManageView::updateVisibleListItemCount()
{
    // Tells the ListView how many items to expect
    ListView_SetItemCountEx(m_listViewCtrl, m_routeList->getList().size(), LVSICF_NOSCROLL);
}

//***********************************************************
//        Select or Deselect a Row Programmatically
//***********************************************************
void ShipRouteManageView::selectRow(int index, bool isSelection)
{
    // Sets the selection state for the provided row index
    if (index < 0) {
        return;
    }
    ListView_SetItemState(
        m_listViewCtrl,
        index,
        isSelection ? LVIS_SELECTED : 0,
        LVIS_SELECTED
    );
}
