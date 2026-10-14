// Single translation unit that compiles the header-only third-party mesh
// parsers. Keep this file free of anything else: it exists only so that
// cgltf's implementation is emitted exactly once in the whole program.
//
// ufbx is not here because it ships a real ufbx.c, which CMake adds to the
// target directly.

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
