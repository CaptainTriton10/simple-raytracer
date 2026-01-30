# Simple Raytracer
### _Based on the **Ray Tracing in One Weekend** blog_

![Raytraced Example](./images/example01.png)

This is a basic raytracer I'm working on for the HackClub flavourtown event, built in C using the Raylib library.

This project is currently still in development and I haven't created releases for all platforms and architectures yet **(only Windows arm64)**, however I will release it to more platforms in the future, when I can get a proper pipeline for building the project.

## Controls

### Camera

- **Movement** - WASD

- **Zoom** - Scroll Wheel

### Render Settings

- **Anti-Aliasing Toggle** - '1' key

[![starline](https://starlines.qoo.monster/assets/CaptainTriton10/simple-raytracer)](https://github.com/qoomon/starline)

## Scene Configs

In order to load and specify objects for a scene, you need to create a scene.toml file in `./configs`. 

### `[data]`
`objects`: a string array with the name of all the objects in the scene

Usage:
```toml
[data]
objects = ["obj1", "obj2", "obj3"]
```

### `[{object name}]`
`position`: a float array with three values for x, y and z

`radius`: a float for the radius of the sphere

`material`: a string with the name of the material for the sphere

Usage:
```toml
[obj1]
position = [1.0, 4.6, -12.34]
radius = 0.5
material = "mat1"
```

### `[{material name}]`
`type`: an integer for the type of the material, where:

- 0 = diffuse
- 1 = metallic
- 2 = glass

`albedo`: a float array with three **normalised** values for the base colour of the material

`roughness`: a float value for the roughness of metallic materials

`ior`: a float value for the index of refraction for glass materials

Usage:

```toml
[mat1]
type = 0
albedo = [0.7, 0.3, 0.9]
roughness = 0.0
ior = 0.0
```

### Full Example

```toml
[data]
objects = ["s1", "s2"]

[s1]
position = [0.0, 0.0, 0.0]
radius = 0.5
material = "mat2"

[s2]
position = [0.0, -10.5, 0.0]
radius = 10.0
material = "mat1"

[mat1]
type = 0
albedo = [0.1, 0.1, 0.15]
roughness = 0.0
ior = 0.0

[mat2]
type = 2
albedo = [0.0, 0.0, 0.0]
roughness = 0.0
ior = 0.78
```
