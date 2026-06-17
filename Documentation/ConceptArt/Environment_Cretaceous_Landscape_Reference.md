# Cretaceous Landscape Environment Reference

## Visual Target Description

**Scene Composition**: Wide-angle Cretaceous landscape for survival game environment

### Terrain Features
- **Elevation**: Rolling hills with 200-500m height variation
- **Geological Features**: Large weathered boulders (2-5m diameter)
- **Terrain Texture**: Mix of grassland, exposed rock, volcanic soil

### Vegetation
- **Primary**: Scattered coniferous trees (Araucaria-style prehistoric conifers)
- **Distribution**: Clustered groups of 5-15 trees, not uniform
- **Density**: Medium coverage (30-40% of visible terrain)

### Atmospheric Elements
- **Sky**: Dramatic sunset with orange and purple cloud formations
- **Lighting**: Golden hour warm lighting, long shadows
- **Fog**: Volumetric atmospheric fog in valleys and low areas
- **Visibility**: 2-5km draw distance with atmospheric fade

### Background Elements
- **Mountains**: Distant volcanic peaks (active/dormant)
- **Horizon**: Layered atmospheric perspective with color gradation

### Technical Specifications
- **Rendering**: Photorealistic style, Unreal Engine 5 quality
- **Lighting Model**: Physically-based rendering (PBR)
- **Post-Processing**: Cinematic color grading, subtle bloom
- **Composition**: Rule of thirds, leading lines from terrain contours

## Implementation Notes for UE5

### Landscape Settings
```
- World Partition: Enabled
- Landscape Size: 8x8 km
- Component Size: 127x127 quads
- LOD: 4 levels
- Heightmap Resolution: 2017x2017
```

### Material Layers
1. **Grass/Vegetation** (40%)
2. **Rocky Terrain** (30%)
3. **Volcanic Soil** (20%)
4. **Wetland/Mud** (10%)

### Lighting Setup
- **Directional Light**: Intensity 3.0, Angle -45° pitch, 30° yaw (sunset direction)
- **Sky Light**: Cubemap-based, Intensity 1.0
- **Sky Atmosphere**: Default Cretaceous settings
- **Exponential Height Fog**: Density 0.02, Start Distance 1000m

### Vegetation Distribution
- **Trees**: Procedural foliage spawner, 150-200 instances per km²
- **Rocks**: Manual hero placement + procedural scatter
- **Ground Cover**: Grass/fern clusters via PCG

## Reference for Agents

**Agent #5 (World Generator)**: Use this spec for landscape generation parameters
**Agent #6 (Environment Artist)**: Use this for vegetation/prop placement density
**Agent #8 (Lighting & Atmosphere)**: Use this for time-of-day and fog configuration

---

**Status**: Reference document created as fallback for failed generate_image API call.
**Next Action**: Agents should implement visual targets described above using UE5 tools.
