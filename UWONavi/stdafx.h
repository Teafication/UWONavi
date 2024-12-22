// stdafx.h : Precompiled header file for the project
// This file includes common headers and settings that are used across multiple parts of the project.
// The goal of precompiled headers is to improve compilation time by reducing the need to re-compile frequently used headers.

#pragma once  // Ensures the file is included only once during compilation.

#include "targetver.h"  // Includes a file that defines the target version for the compiler.

#define _CRT_SECURE_NO_WARNINGS  // This macro disables warnings about unsafe functions (like strcpy) from the C runtime library. 
// It’s commonly used to avoid compiler warnings when using older functions that could potentially lead to buffer overflows. However, be cautious when using it.

#ifdef NDEBUG
#define _SECURE_SCL 0  // Disables certain security checks for Standard C++ Library containers when compiling in release mode (NDEBUG).
#endif

#define WIN32_LEAN_AND_MEAN  // This reduces the size of the Windows headers by excluding certain things that are rarely used. It helps reduce compilation time.

#define NOMINMAX  // Prevents the Windows headers from defining the macros min and max, which can cause conflicts with the standard C++ library versions.

#include <algorithm>  // This includes algorithms like min and max, which are used to perform common operations (sorting, finding min/max, etc.).
using std::min;  // Allows direct use of min() without prefixing it with std::.
using std::max;  // Allows direct use of max() without prefixing it with std::.

////////////////////////////////////////////////////////////////////////////
// Windows-specific headers
// These headers are required for interacting with the Windows operating system and performing platform-specific operations.

#include <windows.h>  // This is the main Windows API header that allows access to core Windows functionality (files, memory, input/output).
#include <objbase.h>  // Provides the Object Linking and Embedding (OLE) functionality, which helps interact with COM (Component Object Model) objects.
#include <MMSystem.h>  // This header is for multimedia APIs like sound and video. 

#pragma comment(lib, "winmm.lib")  // This links against the winmm.lib library, which is necessary for multimedia functions like sound.

#include <gl/GL.h>  // OpenGL header for graphics rendering. It allows interaction with the OpenGL graphics library.
#pragma comment(lib, "opengl32.lib")  // Links OpenGL 3D graphics library (needed for OpenGL functionality).

#include <gl/GLU.h>  // OpenGL Utility Library header. Provides higher-level functions to make OpenGL programming easier.
#pragma comment(lib, "glu32.lib")  // Links the OpenGL Utility Library (glu32.lib) to provide math and transformation helpers for OpenGL.

#include <CommCtrl.h>  // Includes Windows common controls library (such as buttons, edit boxes, etc.).

////////////////////////////////////////////////////////////////////////////
// C standard library headers
// These headers are standard C library headers that are needed for general-purpose operations.

#include <stdlib.h>  // Provides standard functions for memory allocation, process control, conversions, etc.
#include <malloc.h>  // Provides functions for dynamic memory allocation.
#include <memory.h>  // Provides functions for manipulating memory blocks (e.g., copying, setting).
#include <tchar.h>  // Defines functions for working with both ANSI and Unicode strings (used for compatibility).

////////////////////////////////////////////////////////////////////////////
// Math headers
// These headers are for mathematical constants and functions.

#define _USE_MATH_DEFINES  // This macro allows access to math constants like M_PI and M_E when including <math.h>.
#include <math.h>  // Includes the standard math functions (trigonometry, logarithms, etc.).

////////////////////////////////////////////////////////////////////////////
// C++ standard library headers
// These headers are for modern C++ standard library features.

#include <cstdint>  // Defines fixed-width integer types like int32_t, uint64_t, etc.
#include <numeric>  // Provides numeric operations like accumulate (for summing values).
#include <memory>  // Defines smart pointers (e.g., std::unique_ptr, std::shared_ptr) and memory management utilities.
#include <vector>  // Defines the vector container (a dynamic array-like structure).
#include <map>  // Defines the map container (a sorted associative container that stores key-value pairs).
#include <set>  // Defines the set container (stores unique elements, automatically sorted).
#include <deque>  // Defines the deque container (a double-ended queue, which allows efficient insertion/removal at both ends).
