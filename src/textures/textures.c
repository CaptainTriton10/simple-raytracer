#include "textures.h"
#include "../utils/utils.h"
#include <iso646.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Image CreateAtlas(Atlas atlas) {
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

    if (DEBUG_ATLAS) {
        ExportImage(output, "atlas_debug.jpg");
    }

    printf("Created atlas. \n");
    free(scaledTextures);

    return output;
}

Atlas GetTextures(const char *texturesPath, size_t chunkSize) {
    const FilePathList list = LoadDirectoryFiles(texturesPath);

    Image *textures = malloc(list.count * sizeof(Image));
    char **filepaths = malloc(list.count * MAX_TEXTURE_PATH);

    for (int i = 0; i < list.count; i++) {
        filepaths[i] = malloc(MAX_TEXTURE_PATH);
        strcpy(filepaths[i], list.paths[i]);
    }

    Atlas atlas;

    for (int i = 0; i < list.count; i++) {
        textures[i] = LoadImage(list.paths[i]);
    }

    int atlasDivisions;

    int i = 1;
    bool shouldBreak = false;
    while (!shouldBreak || i > MAX_LOOP_DEPTH) {
        if (i * i >= list.count) {
            atlasDivisions = i;
            shouldBreak = true;
        }

        i++;
    }

    atlas = (Atlas){
        .chunkSize = chunkSize,
        .atlasSize = atlasDivisions * chunkSize,
        .textureCount = list.count,
        .textures = textures,
        .filepaths = filepaths
    };

    return atlas;
}

int GetTextureIndex(Atlas atlas, const char *name) {
    for (int i = 0; i < atlas.textureCount; i++) {
        printf("%s vs %s\n", atlas.filepaths[i], name);
        if (strcmp(atlas.filepaths[i], name) == 0) {
            return i;
        }
    }


    return -1;
}
