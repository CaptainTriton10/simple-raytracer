#include "gui.h"
#include "../utils/utils.h"
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
            FloatParam(paramPosition, size[0] / 2, 0, "Position X ", &properties->position.v[0]);
            FloatParam(paramPosition, size[0] / 2, 1, "Y ", &properties->position.v[1]);
            FloatParam(paramPosition, size[0] / 2, 2, "Z ", &properties->position.v[2] );

            FloatParam(paramPosition, size[0] / 2, 4, "Radius ", &properties->radius);
            break;

        case QUAD:
            FloatParam(paramPosition, size[0] / 2, 0, "Position X ", &properties->position.v[0]);
            FloatParam(paramPosition, size[0] / 2, 1, "Y ", &properties->position.v[1]);
            FloatParam(paramPosition, size[0] / 2, 2, "Z ", &properties->position.v[2]);

            FloatParam(paramPosition, size[0] / 2, 4, "Corner 1 X ", &properties->u.v[0]);
            FloatParam(paramPosition, size[0] / 2, 5, "Y ", &properties->u.v[1]);
            FloatParam(paramPosition, size[0] / 2, 6, "Z ", &properties->u.v[2]);

            FloatParam(paramPosition, size[0] / 2, 8, "Corner 2 X ", &properties->v.v[0]);
            FloatParam(paramPosition, size[0] / 2, 9, "Y ", &properties->v.v[1]);
            FloatParam(paramPosition, size[0] / 2, 10, "Z ", &properties->v.v[2]);
            break;

        default:
            GuiLabel((Rectangle){
                view.x + PADDING, paramPosition[1],
                size[0], BLOCK_SIZE
            }, "Unrecognised type. ");
    }
}

static void PropertiesGUI(RenderSettings *settings, int width, Properties *properties, int selected, Hittable *objects) {
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

    Hittable object = objects[selected];

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

static int GetTrueIndex(Hittable object, Scene scene) {
    for (int i = 0; i < scene.objCount; i++) {
        if (scene.objects[i].index == object.index) {
            return i;
        }
    }

    return -1;
}

static void UpdateVec3(Vector3 *vec1, ValueFloat *vec2) {
    vec1->x = vec2[0].value;
    vec1->y = vec2[1].value;
    vec1->z = vec2[2].value;
}

static void UpdateParamFloat(ValueFloat *param, float new) {
    char textVal[MAX_BUFFER_SIZE];

    sprintf(textVal, "%.3f", new);
    *param = (ValueFloat){
        .value = new,
        .editMode = false
    };
    strcpy(param->textVal, textVal);
}

static void UpdateParamVec3(ValueFloat *vec1, Vector3 vec2) {
    char textVal[MAX_BUFFER_SIZE];

    sprintf(textVal, "%.3f", vec2.x);
    vec1[0] = (ValueFloat){
        .value = vec2.x,
        .editMode = false
    };
    strcpy(vec1[0].textVal, textVal);

    sprintf(textVal, "%.3f", vec2.y);
    vec1[1] = (ValueFloat){
        .value = vec2.y,
        .editMode = false
    };
    strcpy(vec1[1].textVal, textVal);

    sprintf(textVal, "%.3f", vec2.z);
    vec1[2] = (ValueFloat){
        .value = vec2.z,
        .editMode = false
    };
    strcpy(vec1[2].textVal, textVal);
}


static void UpdateObject(Hittable *object, GUI *gui) {
    switch (object->type) {
        case SPHERE:
            Sphere s = HittableToSphere(*object);

            UpdateVec3(&s.pos, gui->sidebar.properties.position.v);

            s.radius = gui->sidebar.properties.radius.value;

            Hittable temp = TranslateSphereData(s);
            memcpy(object->data, temp.data, DATA_WIDTH * sizeof(float) * 4);

            break;
        case QUAD:
            Quad q = HittableToQuad(*object);

            UpdateVec3(&q.Q, gui->sidebar.properties.position.v);
            UpdateVec3(&q.u, gui->sidebar.properties.u.v);
            UpdateVec3(&q.v, gui->sidebar.properties.v.v);

            temp = TranslateQuadData(q);
            memcpy(object->data, temp.data, DATA_WIDTH * sizeof(float) * 4);

            break;

        default:
            error("Unrecognised object type");
            break;
    }
}

void InitSelectedObject(Hittable object, GUI *gui) {
    switch (object.type) {
        case SPHERE:
            Sphere s = HittableToSphere(object);

            UpdateParamVec3(gui->sidebar.properties.position.v, s.pos);
            UpdateParamFloat(&gui->sidebar.properties.radius, s.radius);

            break;

        case QUAD:
            Quad q = HittableToQuad(object);

            UpdateParamVec3(gui->sidebar.properties.position.v, q.Q);
            UpdateParamVec3(gui->sidebar.properties.u.v, q.u);
            UpdateParamVec3(gui->sidebar.properties.v.v, q.v);

            break;
    }
}

void MainGUI(RenderSettings *settings, GUI *gui, Scene *scene) {
    Style();
    GUI prevGui = *gui;
    int prevSelected = gui->sidebar.selected;

    size_t size = scene->objCount * sizeof(Hittable);

    Hittable *orderedObjects = malloc(size);
    memcpy(orderedObjects, scene->objects, size);
    qsort_s(orderedObjects, scene->objCount, sizeof(Hittable), Compare, 0);

    OutlinerGUI(settings, 350, &gui->sidebar.outliner, &gui->sidebar.selected, orderedObjects, scene->objCount);
    if (prevSelected != gui->sidebar.selected)
        InitSelectedObject(orderedObjects[gui->sidebar.selected], gui);

    PropertiesGUI(settings, 350, &gui->sidebar.properties, gui->sidebar.selected, orderedObjects);

    int trueIndex = GetTrueIndex(orderedObjects[gui->sidebar.selected], *scene);

    UpdateObject(&scene->objects[trueIndex], gui);
}