# Chozo

## Install & Build

```console
$ git clone https://github.com/BowiEgo/Chozo
$ cd Chozo
$ git submodule update --init --recursive
$ ./tools/vcpkg/bootstrap-vcpkg.sh
$ ./tools/vcpkg/vcpkg install
```

```console
$ mkdir build
$ cmake -S . -B build
$ cmake --build build
```

or just use the shell sript contains above commands.

```console
$ git clone https://github.com/BowiEgo/Chozo
$ cd Chozo
$ chmod +x install.sh
$ ./install.sh
```

## Run

```console
$ ./bin/ChozoEditor
```

## Documentation

## Examples

## Features

### Backends

-   [x] OpenGL 4.1 for Linux, macOS and Windows
-   [ ] Vulkan 1.0 for Android, Linux, macOS, and Windows
-   [ ] WebGPU 1.0 for all platforms

### Rendering

-   [ ] Clustered forward renderer
-   [ ] Cook-Torrance microfacet specular BRDF
-   [ ] Lambertian diffuse BRDF
-   [ ] Custom lighting/surface shading
-   [ ] HDR/linear lighting
-   [ ] Metallic workflow
-   [ ] Clear coat
-   [ ] Anisotropic lighting
-   [ ] Approximated translucent (subsurface) materials
-   [ ] Cloth/fabric/sheen shading
-   [ ] Normal mapping & ambient occlusion mapping
-   [ ] Image-based lighting
-   [ ] Physically-based camera (shutter speed, sensitivity and aperture)
-   [ ] Physical light units
-   [ ] Point lights, spot lights, and directional light
-   [ ] Specular anti-aliasing
-   [ ] Point, spot, and directional light shadows
-   [ ] Cascaded shadows
-   [ ] EVSM, PCSS, DPCF, or PCF shadows
-   [ ] Transparent shadows
-   [ ] Contact shadows
-   [ ] Screen-space ambient occlusion
-   [ ] Screen-space reflections
-   [ ] Screen-space refraction
-   [ ] Global fog
-   [ ] Dynamic resolution (with support for AMD FidelityFX FSR)

### Post processing

-   [ ] HDR bloom
-   [ ] Depth of field bokeh
-   [ ] Multiple tone mappers: generic (customizable), ACES, filmic, etc.
-   [ ] Color and tone management: luminance scaling, gamut mapping
-   [ ] Color grading: exposure, night adaptation, white balance, channel mixer,
        shadows/mid-tones/highlights, ASC CDL, contrast, saturation, etc.
-   [ ] TAA, FXAA, MSAA
-   [ ] Screen-space lens flares

### glTF 2.0

-   Encodings

    -   [ ] Embeded
    -   [ ] Binary

-   Primitive Types

    -   [ ] Points
    -   [ ] Lines
    -   [ ] Line Loop
    -   [ ] Line Strip
    -   [ ] Triangles
    -   [ ] Triangle Strip
    -   [ ] Triangle Fan

-   Animation

    -   [ ] Transform animation
    -   [ ] Linear interpolation
    -   [ ] Morph animation
        -   [ ] Sparse accessor
    -   [ ] Skin animation
    -   [ ] Joint animation

-   Extensions
    -   [ ] KHR_draco_mesh_compression
    -   [ ] KHR_lights_punctual
    -   [ ] KHR_materials_clearcoat
    -   [ ] KHR_materials_emissive_strength
    -   [ ] KHR_materials_ior
    -   [ ] KHR_materials_pbrSpecularGlossiness
    -   [ ] KHR_materials_sheen
    -   [ ] KHR_materials_transmission
    -   [ ] KHR_materials_unlit
    -   [ ] KHR_materials_variants
    -   [ ] KHR_materials_volume
    -   [ ] KHR_materials_specular
    -   [ ] KHR_mesh_quantization
    -   [ ] KHR_texture_basisu
    -   [ ] KHR_texture_transform
    -   [ ] EXT_meshopt_compression
