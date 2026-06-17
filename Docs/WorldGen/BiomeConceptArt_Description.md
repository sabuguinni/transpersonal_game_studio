# Biome Concept Art — World Generator Agent #5
## Cycle: PROD_CYCLE_AUTO_20260617_011

### Biome Visual Identity Reference

#### 1. Forest Biome (Upper-Left quadrant)
- **Palette**: Deep greens, dark browns, dappled light through canopy
- **Key features**: Towering cycads (8-15m), dense fern undergrowth, mossy boulders
- **Terrain**: Elevated (Z=110-160), cliff edges dropping to Plains
- **Atmosphere**: Cool, shadowed, mysterious — 70% canopy cover
- **Actors placed**: Boulder_Forest_001-004, CliffEdge_Forest_001-004, AmbLight_Forest_001 (green tint)

#### 2. Plains Biome (Center)
- **Palette**: Golden yellows, amber, warm browns
- **Key features**: Open grassland, scattered rock outcrops, river crossing
- **Terrain**: Mid-elevation (Z=90-120), gentle rolling hills
- **Atmosphere**: Warm, exposed, high visibility — prime hunting ground
- **Actors placed**: River segments (Forest→Plains transition), WaterSource_River_001

#### 3. Savana Biome (Right/East)
- **Palette**: Burnt orange, ochre, dusty red
- **Key features**: Rocky outcrops (RockOutcrop_Savana_001-004), sparse vegetation
- **Terrain**: Variable elevation (Z=95-130), rocky ground
- **Atmosphere**: Hot, arid, dramatic rock formations — T-Rex territory
- **Actors placed**: RockOutcrop_Savana_001-004, AmbLight_Savana_001 (warm orange)

#### 4. Pantano/Swamp Biome (Lower/South)
- **Palette**: Murky greens, dark blues, grey fog
- **Key features**: Mudflats, standing water, dead trees, dense fog
- **Terrain**: Low elevation (Z=75-90), waterlogged ground
- **Atmosphere**: Oppressive, foggy, dangerous — Raptor ambush zone
- **Actors placed**: Mudflat_Pantano_001-003, River_Delta_Pantano_001-002, AmbLight_Pantano_001 (blue tint)

### River System
- **Source**: Forest biome at (800, -400, 118)
- **Path**: Forest → Plains → Pantano delta
- **Segments**: 6 river planes + 2 delta expansions
- **Z-gradient**: 118 → 85 (natural downhill flow)

### Cliff Boundary
- **Location**: Forest/Plains boundary (diagonal NW-SE)
- **Height drop**: ~40-60 units
- **Markers**: CliffEdge_Forest_001-004 (cube meshes, rotated 45°)
- **Purpose**: Natural barrier, creates elevation drama, limits dinosaur pathfinding

### Next Steps for Environment Artist (Agent #6)
1. Replace cube/sphere placeholders with proper rock/boulder meshes from Dinosaur_Pack
2. Add foliage density (HISM trees) around river banks
3. Apply mud/wet material to Pantano mudflat planes
4. Add particle fog emitters in Pantano biome
5. Rock ring around campfire in Plains biome (as noted by Agent #4)
