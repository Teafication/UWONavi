// This checks if IDC_STATIC is not already defined, and if not, defines it with the value -1
#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif

// The following are resource identifiers for various resources in the program, like icons, dialogs, and menus

// ID for the main frame (likely the main window or frame of the app)
#define IDR_MAINFRAME                           128

// ID for a small icon used in the application
#define IDI_SMALL                               129

// ID for the ShipRouteManageView dialog (likely a user interface for managing ship routes)
#define IDD_SHIPROUTEMANAGEVIEW                 131

// ID for the AboutBox dialog (likely a dialog showing about information)
#define IDD_ABOUTBOX                            132

// ID for a popup menu related to managing ship routes
#define IDR_SHIPROUTEMANAGEPOPUPMENU            133

// ID for an icon representing a star
#define IDI_STAR                                136

// ID for a general popup menu
#define IDR_POPUPMENU                           137

// ID for the ship route list control (likely a UI element displaying ship routes)
#define IDC_SHIPROUTELIST                       138

// ID for a blank icon (probably a placeholder or default icon)
#define IDI_BLANK                               145

// ID for the copyright label control in the UI
#define IDC_COPYRIGHT_LABEL                     146

// ID for the version label control in the UI
#define IDC_VERSION_LABEL                       147

// Another ID for a version label (possibly for a different version display)
#define IDC_VERSION_LABEL2                      148

// The following are menu item identifiers for various actions in the app (often used to handle user interactions)

#define IDM_DELETE_SHIP_ROUTE                   40000  // Menu option to delete a ship route
#define IDM_ROUTE_MANAGE_OPTION                 40001  // Menu option for route management options
#define IDM_DEBUG_INTERVAL_NORMAL               40002  // Menu option to set debug interval to normal
#define IDM_DEBUG_INTERVAL_HIGH                 40003  // Menu option to set debug interval to high
#define IDM_JOINT_SHIP_ROUTE                    40006  // Menu option to join a ship route
#define IDM_SHOW_SHIPROUTEMANAGEVIEW            40007  // Menu option to show the ship route management view
#define IDM_ABOUT                               40008  // Menu option to show the About dialog
#define IDM_EXIT                                40009  // Menu option to exit the application
#define IDM_TOGGLE_TRACE_SHIP                   40010  // Menu option to toggle tracing the ship
#define IDM_ERASE_SHIP_ROUTE                    40011  // Menu option to erase a ship route
#define IDM_TOGGLE_KEEP_FOREGROUND              40012  // Menu option to toggle keeping the application in the foreground
#define IDM_TOGGLE_SPEED_METER                  40013  // Menu option to toggle the speed meter display
#define IDM_TOGGLE_VECTOR_LINE                  40014  // Menu option to toggle the vector line display
#define IDM_SAME_SCALE                          40015  // Menu option to set the scale to the same value
#define IDM_ZOOM_IN                             40016  // Menu option to zoom in
#define IDM_ZOOM_OUT                            40017  // Menu option to zoom out
#define IDM_TOGGLE_DEBUG_AUTO_CRUISE            40018  // Menu option to toggle automatic cruise during debugging
#define IDM_DESELECT_ROUTE                      40019  // Menu option to deselect a ship route
#define IDM_DEBUG_CLOSE_ROUTE                   40020  // Menu option to close a route during debugging
#define IDM_TOGGLE_FAVORITE                     40021  // Menu option to toggle the route as a favorite
#define IDM_JOINT_LATEST_ROUTE                  40022  // Menu option to join the latest ship route
