# Vegetation System Technical Specifications v1 — Engine Architect
**Date**: 17 Jun 2026  
**Cycle**: PROD_CYCLE_AUTO_20260617_003  
**Agent**: #2 Engine Architect

---

## OVERVIEW

Technical specifications for vegetation distribution across biome zones. Defines density levels, asset types, and placement rules for Agent #6 (Environment Artist) implementation.

---

## VEGETATION DENSITY LEVELS

### Low Density (Savanna)
- **Target Asset Count**: 15
- **Spacing**: 200-400 units between assets
- **Types**: Grass clumps, scattered acacia-style trees
- **Distribution**: Clustered around water sources, sparse elsewhere

### Medium Density (Wetland, River Valley)
- **Target Asset Count**: 25-30 per zone
- **Spacing**: 100-200 units between assets
- **Types**: Reeds, palms, river plants, bushes, moss patches
- **Distribution**: Dense near water, gradient to sparse at zone edges

### High Density (Forest)
- **Target Asset Count**: 50
- **Spacing**: 50-150 units between assets
- **Types**: Tall trees (conifers, cycads), ferns, undergrowth, fallen logs
- **Distribution**: Uniform coverage with natural variation

### Very Low Density (Rocky)
- **Target Asset Count**: 10
- **Spacing**: 300-600 units between assets
- **Types**: Shrubs, hardy bushes, rock formations
- **Distribution**: Concentrated in crevices and sheltered areas

---

## ASSET TYPE SPECIFICATIONS

### Trees
- **Mesh Requirements**: LOD chain (3 levels minimum)
- **Collision**: Simple capsule or box collision for trunks
- **Scale Variation**: 0.8x to 1.2x random scale
- **Rotation**: Random yaw (0-360°), zero pitch/roll

### Ferns & Undergrowth
- **Mesh Requirements**: Billboard or simple geometry
- **Collision**: None (decorative only)
- **Scale Variation**: 0.7x to 1.3x random scale
- **Clustering**: Groups of 3-7 assets

### Rocks
- **Mesh Requirements**: Static mesh with LOD
- **Collision**: Complex collision for large rocks, simple for small
- **Scale Variation**: 0.5x to 2.0x random scale
- **Rotation**: Random rotation on all axes

### Grass & Ground Cover
- **Mesh Requirements**: Instanced static mesh or foliage type
- **Collision**: None
- **Scale Variation**: 0.8x to 1.2x random scale
- **Density**: 5-10 instances per square meter in applicable zones

---

## PLACEMENT RULES

### Elevation-Based Distribution
- **Flat Terrain (slope < 15°)**: All vegetation types allowed
- **Moderate Slope (15-30°)**: Trees and rocks only
- **Steep Slope (> 30°)**: Rocks only, no vegetation

### Biome-Specific Rules
- **Savanna**: Avoid dense tree clusters, maintain sight lines
- **Forest**: Prioritize vertical layering (canopy + undergrowth)
- **Wetland**: Concentrate vegetation near low elevation areas
- **Rocky**: Sparse distribution, favor crevices and sheltered spots
- **River Valley**: Linear distribution following terrain valleys

### Collision Avoidance
- **Minimum Distance to Dinosaur Territories**: 100 units (prevent spawn blocking)
- **Minimum Distance to Player Start**: 500 units (clear spawn area)
- **Minimum Distance Between Large Trees**: 150 units (prevent overlap)

---

## PERFORMANCE OPTIMIZATION

### LOD Configuration
- **LOD 0**: 0-1000 units (full detail)
- **LOD 1**: 1000-3000 units (medium detail)
- **LOD 2**: 3000-6000 units (low detail)
- **Cull Distance**: 6000 units

### Instancing
- **Grass/Ground Cover**: Use Instanced Static Mesh Component
- **Repeated Assets**: Use HISM (Hierarchical Instanced Static Mesh)
- **Unique Assets**: Standard Static Mesh Actor

### Draw Call Budget
- **Maximum Unique Meshes**: 20 (to minimize material switches)
- **Maximum Total Instances**: 130 (within CAP limit)

---

## TECHNICAL IMPLEMENTATION

### UE5 Python Workflow (Agent #6)
```python
import unreal
import random

# Load vegetation assets
tree_mesh = unreal.load_asset('/Game/Environment/Vegetation/SM_Tree_01')
fern_mesh = unreal.load_asset('/Game/Environment/Vegetation/SM_Fern_01')
rock_mesh = unreal.load_asset('/Game/Environment/Rocks/SM_Rock_01')

# Get biome zone markers
actors = unreal.EditorLevelLibrary.get_all_level_actors()
veg_markers = [a for a in actors if 'VegDensity' in a.get_actor_label()]

for marker in veg_markers:
    zone_loc = marker.get_actor_location()
    zone_name = marker.get_actor_label()
    
    # Parse density from marker name
    if 'Savanna' in zone_name:
        count = 15
        spacing = 300
    elif 'Forest' in zone_name:
        count = 50
        spacing = 100
    # ... (other biomes)
    
    # Spawn vegetation in circular pattern around marker
    for i in range(count):
        angle = random.uniform(0, 360)
        distance = random.uniform(0, 1500)
        offset_x = distance * unreal.MathLibrary.cos(angle)
        offset_y = distance * unreal.MathLibrary.sin(angle)
        
        spawn_loc = unreal.Vector(
            zone_loc.x + offset_x,
            zone_loc.y + offset_y,
            zone_loc.z
        )
        
        # Spawn static mesh actor
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            spawn_loc,
            unreal.Rotator(0, random.uniform(0, 360), 0)
        )
        actor.static_mesh_component.set_static_mesh(tree_mesh)
        actor.set_actor_label(f"Tree_{zone_name}_{i:03d}")
```

---

## VALIDATION CRITERIA

✅ **Asset Count**: Total vegetation assets = 130 ± 10  
✅ **Distribution**: Each biome has target asset count ± 20%  
✅ **Collision**: No vegetation blocking PlayerStart or dinosaur territories  
✅ **Performance**: Draw calls < 100, frame time < 16ms  
✅ **Visual Quality**: Viewport shows green vegetation visible from multiple angles

---

## CURRENT STATUS

- ✅ Vegetation density markers deployed (5 zones)
- ✅ Technical specifications documented
- ⏳ Asset population pending (Agent #6)
- ⏳ LOD configuration pending (Agent #6)
- ⏳ Performance validation pending (Agent #18)

---

**Specifications Status**: COMPLETE  
**Implementation Status**: PENDING (Agent #6)  
**Next Agent**: #6 Environment Artist (implement vegetation population using these specs)
