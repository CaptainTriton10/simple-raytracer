#ifndef GUI_H
#define GUI_H

#include "../renderer/renderer.h"

#define MAX_BUFFER_SIZE 32

typedef struct ValueFloat {
    float value;
    bool editMode;
    char textVal[MAX_BUFFER_SIZE];
} ValueFloat;

typedef struct ValueVec3 {
    ValueFloat v[3];
} ValueVec3;

typedef struct Sidebar {
    ValueVec3 position;
} Sidebar;

typedef struct GUI {
    Sidebar sidebar;
} GUI;

static void Style();
static void SidebarGUI(RenderSettings *settings, int width, GUI *gui);
void MainGUI(RenderSettings *settings, GUI *gui);

#endif