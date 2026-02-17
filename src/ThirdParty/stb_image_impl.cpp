// stb_image implementation file
// This file should be compiled once and linked with the project

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO  // We'll use our own file loading
#define STBI_NO_HDR    // Don't need HDR support
#define STBI_NO_LINEAR // Don't need linear light
#include "stb_image.h"
