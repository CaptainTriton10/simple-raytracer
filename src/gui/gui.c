#include "gui.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
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

static void PropertiesGUI(RenderSettings *settings, int width, Properties *properties) {
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

    GuiScrollPanel(panel, "Properties", content, &properties->scroll, &view);

    BeginScissorMode(view.x, view.y, view.width, view.height);

    float y = view.y + properties->scroll.y;

    int paramPosition[2] = {
        view.x + view.width / 2 - 2 * PADDING + SCROLLBAR_MAGIC_OFFSET,
        y + PADDING};

    FloatParam(paramPosition, size[0] / 2, 0, "Position X ",
        &properties->position.v[0]
    );
    FloatParam(paramPosition, size[0] / 2, 1, "Y ",
        &properties->position.v[1]
    );
    FloatParam(paramPosition, size[0] / 2, 2, "Z ",
        &properties->position.v[2]
    );

    EndScissorMode();

}

static void OutlinerGUI(RenderSettings *settings, int width, Outliner *outliner, int *selected, Scene scene) {
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

    ObjectEntry objects[scene.objCount];

    for (int i = 0; i < scene.objCount; i++) {
        objects[i] = (ObjectEntry){
            .object = scene.objects[i],
            .index = i
        };
        objects[i].name = malloc(MAX_BUFFER_SIZE);
        sprintf(objects[i].name, "%s", scene.names[i]);

        ObjectEntryGUI(paramPosition, view.width - 2 * PADDING + SCROLLBAR_MAGIC_OFFSET, objects[i]);
    }

    EndScissorMode();
}

void MainGUI(RenderSettings *settings, GUI *gui, Scene scene) {
    Style();
    int *selected = 0;

    PropertiesGUI(settings, 350, &gui->sidebar.properties);
    OutlinerGUI(settings, 350, &gui->sidebar.outliner, selected, scene);
}