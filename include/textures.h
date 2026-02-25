#ifndef TEXTURES_H
#define TEXTURES_H

#include "helpers.h"
#include "raylib.h"
#include <iso646.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct AtlasBase {
    Image *textures;
    size_t textureCount;
    size_t atlasSize;
    size_t chunkSize;
} AtlasBase;



Image CreateAtlas(AtlasBase atlas) {
    if (atlas.atlasSize % atlas.chunkSize != 0) {
        char errMsg[128];
        sprintf(
            errMsg, "Chunk size is not a multiple of the atlas size [%lld -> %lld]",
            atlas.atlasSize, atlas.chunkSize
        );

        error(errMsg);
    }

    Image *scaledTextures = malloc(atlas.textureCount * sizeof(Image));

    for (int i = 0; i < atlas.textureCount; i++) {
        scaledTextures[i] = ImageCopy(atlas.textures[i]);
        ImageResize(&scaledTextures[i], atlas.chunkSize, atlas.chunkSize);
    }

    int divisions = (atlas.atlasSize / atlas.chunkSize);
    int sectors = divisions * divisions;

    Image output = GenImageColor(atlas.atlasSize, atlas.atlasSize, BLANK);

    for (int i = 0; i < atlas.textureCount; i++) {
        int row = i / divisions;
        int column = i % divisions;

        ImageDraw(&output, scaledTextures[i],
            (Rectangle){0, 0, atlas.chunkSize, atlas.chunkSize},
            (Rectangle){
                column * atlas.chunkSize,
                row * atlas.chunkSize,
                atlas.chunkSize,
                atlas.chunkSize
            },
            WHITE
        );
    }

    ExportImage(output, "atlas.jpg");

    printf("Created atlas. \n");
    free(scaledTextures);

    return output;
}

#endif
