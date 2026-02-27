#ifndef TEXTURES_H
#define TEXTURES_H

#include <raylib.h>
#include <stddef.h>

#define DEBUG_ATLAS 0
#define MAX_LOOP_DEPTH 256
#define MAX_TEXTURE_PATH 64

typedef struct Atlas {
    Image *textures;
    char **filepaths;
    size_t textureCount;
    size_t atlasSize;
    size_t chunkSize;
} Atlas;

Image CreateAtlas(Atlas atlas);
Atlas GetTextures(const char *texturesPath, size_t chunkSize);
int GetTextureIndex(Atlas atlas, const char *name);

#endif
