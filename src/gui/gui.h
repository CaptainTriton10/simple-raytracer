#ifndef GUI_H
#define GUI_H

#include "../renderer/renderer.h"
#include "../objects/objects.h"

typedef struct ValueFloat {
    float value;
    bool editMode;
    char textVal[MAX_BUFFER_SIZE];
} ValueFloat;

typedef struct ObjectEntry {
    char *name;
    int index;
    Hittable object;
} ObjectEntry;

typedef struct ValueVec3 {
    ValueFloat v[3];
} ValueVec3;

typedef struct Properties {
    ValueVec3 position;
    ObjectEntry selected;
    Vector2 scroll;
} Properties;

typedef struct Outliner {
    ObjectEntry objects[32];
    Vector2 scroll;
} Outliner;

typedef struct Sidebar {
    Properties properties;
    Outliner outliner;
} Sidebar;

typedef struct GUI {
    Sidebar sidebar;
} GUI;

void MainGUI(RenderSettings *settings, GUI *gui, Scene scene);

#endif