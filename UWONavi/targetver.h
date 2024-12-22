#pragma once  // Ensures that this header file is only included once in a single compilation unit.

/////////////////////////////////////////////////////////////////////////////
// The purpose of this file is to define the target Windows version for your project.
// It includes headers that configure the Windows API to be used in your application.

/////////////////////////////////////////////////////////////////////////////
// The comment here explains that the SDKDDKVer.h header file is used to define the version
// of the Windows SDK that you want to target in your project. This allows the code to be compatible
// with a specific version of Windows, ensuring that your project only uses APIs that exist in that version.

/////////////////////////////////////////////////////////////////////////////
// By defining _WIN32_WINNT, you're specifying the minimum Windows version your application 
// will support. The value `0x0501` corresponds to Windows XP.

#define _WIN32_WINNT 0x0501  // Windows XP (the value 0x0501 corresponds to Windows XP)
#include <SDKDDKVer.h>  // This header file is part of the Windows SDK and is used to set the Windows version.

