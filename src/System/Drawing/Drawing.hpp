/// @file Drawing.hpp
/// @brief System.Drawing namespace - Graphics primitives, images, fonts, and rendering.
///
/// This file is the master include for the System.Drawing namespace, providing:
/// - Color: 32-bit ARGB color representation
/// - Point, Size, Rectangle: Geometric primitives
/// - Image: 32-bit ARGB bitmap images with file loading support
/// - Font: Bitmap (FON) and TrueType (TTF) font rendering
/// - Graphics: Drawing context for rendering primitives, text, and images
/// - GraphicsBuffer: Framebuffer management for VGA and VBE modes
/// - HatchStyle: Fill patterns for hatched brushes
/// - SystemIcons: Named constants for system icon library
///
/// @note All colors and images use 32-bit ARGB format internally. For low-color
/// display modes (4bpp/8bpp VGA), content is dithered at render time.

#ifndef SYSTEM_DRAWING_HPP
#define SYSTEM_DRAWING_HPP

// Core types required by Drawing classes
#include "../Types.hpp"
#include "../Array.hpp"
#include "../Exception.hpp"

// Drawing enums (BufferMode, BorderStyle, FontStyle, StringAlignment)
#include "Enums.hpp"

// Geometric primitives
#include "Color.hpp"
#include "Point.hpp"
#include "Size.hpp"
#include "Rectangle.hpp"

// Fill patterns
#include "HatchStyle.hpp"

// Image class (includes BitmapFileHeader, BitmapInfoHeader)
#include "Image.hpp"

// System icon constants
#include "SystemIcons.hpp"

// Font rendering
#include "Font.hpp"

// Graphics buffer and context
#include "GraphicsBuffer.hpp"
#include "Graphics.hpp"

#endif // SYSTEM_DRAWING_HPP
