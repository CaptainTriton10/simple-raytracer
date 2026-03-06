#include "gui.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "../../include/raygui.h"

#define MARGIN 10
#define PADDING 10
#define HEADER_SIZE 23
#define BLOCK_SIZE 20
#define SCROLLBAR_MAGIC_OFFSET 5

static void Style() {
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
}

static void FloatParam(int startPos[2], int width, int index, char *text, ValueFloat *value) {
    int position[2] = {startPos[0], startPos[1] + index * (BLOCK_SIZE + PADDING)};
    int size[2] = {width, BLOCK_SIZE};

    if(GuiValueBoxFloat((Rectangle){
        position[0], position[1],
        size[0], size[1]
    }, text, value->textVal, &value->value, value->editMode)) value->editMode = !value->editMode;
}

static bool ObjectEntryGUI(int startPos[2], int width, ObjectEntry object) {
    int position[2] = {startPos[0], startPos[1] + object.index * (BLOCK_SIZE + PADDING)};
    int size[2] = {width, BLOCK_SIZE};

    if (GuiButton((Rectangle){
        position[0], position[1],
        size[0], size[1]
    }, object.name)) {
        return true;
    }

    return false;
}

static void ObjectData(Rectangle view, int size[2], Properties *properties, int type) {
    float y = view.y + properties->scroll.y;

    int paramPosition[2] = {
        view.x + view.width / 2 - 2 * PADDING + SCROLLBAR_MAGIC_OFFSET,
        y + PADDING};

    switch (type) {
        case SPHERE:
            FloatParam(paramPosition, size[0] / 2, 0, "Position X ",
                &properties->position.v[0]
            );
            FloatParam(paramPosition, size[0] / 2, 1, "Y ",
                &properties->position.v[1]
            );
            FloatParam(paramPosition, size[0] / 2, 2, "Z ",
                &properties->position.v[2]
            );

            FloatParam(paramPosition, size[0] / 2, 4, "Radius ",
                &properties->position.v[2]
            );
    }

}

static void PropertiesGUI(RenderSettings *settings, int width, Properties *properties, int selected, Scene *scene) {
    int position[2] = {settings->width - MARGIN - width, MARGIN};
    int size[2] = {width, settings->height / 2 - MARGIN};

    Rectangle panel = {
        position[0], position[1],
        size[0], size[1]
    };

    Rectangle content = {
        0, 0,
        width - 2 * PADDING, 1000
    };

    Rectangle view = {0};

    Hittable object = scene->objects[selected];

    char title[MAX_BUFFER_SIZE];
    sprintf(title, "Properties - %s", object.name);

    GuiScrollPanel(panel, title, content, &properties->scroll, &view);

    BeginScissorMode(view.x, view.y, view.width, view.height);

    ObjectData(view, size, properties, object.type);

    EndScissorMode();
}

static void OutlinerGUI(RenderSettings *settings, int width, Outliner *outliner, int *selected, Hittable *objects, size_t objCount) {
    int position[2] = {settings->width - MARGIN - width, MARGIN + settings->height / 2};
    int size[2] = {width, settings->height / 2 - 2 * MARGIN};

    Rectangle panel = {
        position[0], position[1],
        size[0], size[1]
    };

    Rectangle content = {
        0, 0,
        width - 2 * PADDING, 1000
    };

    Rectangle view = {0};

    GuiScrollPanel(panel, "Outliner", content, &outliner->scroll, &view);

    BeginScissorMode(view.x, view.y, view.width, view.height);

    float y = view.y + outliner->scroll.y;

    int paramPosition[2] = {
        view.x + PADDING,
        y + PADDING};

    ObjectEntry objectEntries[objCount];

    for (int i = 0; i < objCount; i++) {
        objectEntries[i] = (ObjectEntry){
            .object = objects[i],
            .index = i
        };
        objectEntries[i].name = malloc(MAX_BUFFER_SIZE);
        sprintf(objectEntries[i].name, "%s", objects[i].name);

        if (ObjectEntryGUI(paramPosition, view.width - 2 * PADDING + SCROLLBAR_MAGIC_OFFSET, objectEntries[i]))
            *selected = i;
    }

    EndScissorMode();
}

int Compare(void *_context, const void *a, const void *b) {
    const Hittable *objA = a;
    const Hittable *objB = b;

    return objA->index - objB->index;
}

void MainGUI(RenderSettings *settings, GUI *gui, Scene *scene) {
    Style();

    size_t size = scene->objCount * sizeof(Hittable);

    Hittable *orderedObjects = malloc(size);
    memcpy(orderedObjects, scene->objects, size);

    qsort_s(orderedObjects, scene->objCount, sizeof(Hittable), Compare, 0);

    PropertiesGUI(settings, 350, &gui->sidebar.properties, gui->sidebar.selected, scene);
    OutlinerGUI(settings, 350, &gui->sidebar.outliner, &gui->sidebar.selected, orderedObjects, scene->objCount);
}