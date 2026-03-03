gcc src/*.c src/aabb/*.c src/bvh/*.c src/configs/*.c src/controls/*.c src/objects/*.c src/renderer/*.c src/textures/*.c src/utils/*.c src/gui/*.c -o build/main.exe -I./include -L./lib -lraylib -lopengl32 -lgdi32 -lwinmm -g
./build/main.exe
