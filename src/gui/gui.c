#include "gui.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "../../include/raygui.h"

#define MARGIN 10
#define PADDING 10
#define HEADER_SIZE 23
#define BLOCK_SIZE 20

static void Style() {
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
}

static void FloatParam(int startPos[2], int width, int index, char *text, char *textVal, float *value, bool *editMode) {
    int position[2] = {startPos[0], startPos[1] + index * (BLOCK_SIZE + PADDING)};
    int size[2] = {width, BLOCK_SIZE};

    if(GuiValueBoxFloat((Rectangle){
        position[0], position[1],
        size[0], size[1]
    }, text, textVal, value, *editMode)) *editMode = !*editMode;
}

static void SidebarGUI(RenderSettings *settings, int width, GUI *gui) {
    int position[2] = {settings->width - MARGIN - width, MARGIN};
    int size[2] = {width, settings->height - 2 * MARGIN};

    GuiPanel((Rectangle){
        position[0], position[1],
        size[0], size[1]
    }, "Properties");

    int paramPosition[2] = {position[0] + size[0] / 2 - PADDING, position[1] + HEADER_SIZE + PADDING};

    FloatParam(paramPosition, size[0] / 2, 0, "Position X ",
        gui->sidebar.position.v[0].textVal,
        &gui->sidebar.position.v[0].value,
        &gui->sidebar.position.v[0].editMode
    );

    FloatParam(paramPosition, size[0] / 2, 1, "Y ",
        gui->sidebar.position.v[1].textVal,
        &gui->sidebar.position.v[1].value,
        &gui->sidebar.position.v[1].editMode
    );

    FloatParam(paramPosition, size[0] / 2, 2, "Z ",
        gui->sidebar.position.v[2].textVal,
        &gui->sidebar.position.v[2].value,
        &gui->sidebar.position.v[2].editMode
    );
}

void MainGUI(RenderSettings *settings, GUI *gui) {
    Style();
    SidebarGUI(settings, 350, gui);
}