# Performance Optimizer — Cycle PROD_CYCLE_AUTO_20260617_013

## Agent #4 — Performance Optimizer

### UE5 Commands Executed (5/5 ✅)

| # | Command | Result |
|---|---------|--------|
| 1 | Bridge validation | `bridge_ok` ✅ |
| 2 | CAP enforcement + degenerate label audit | Actor/dino/survival/light counts + degen label scan ✅ |
| 3 | Performance profiling — draw call analysis | StaticMesh/Skeletal/Light/Emitter breakdown + frame budget ✅ |
| 4 | LOD enforcement + mobility audit | LOD bias set, static mobility enforced on env actors ✅ |
| 5 | Final perf pass + map save | Survival actor verification + MAP_SAVED:True ✅ |

---

## Performance Budget Status

### Frame Budget Estimate (60fps = 16.6ms target)
- Static Mesh actors: ~0.05ms each
- Skeletal Mesh actors: ~0.3ms each  
- Point Lights (dynamic): ~0.8ms each
- Particle Emitters: ~1.2ms each
- **Current estimate: ~7-10ms** → **PASS for 60fps**

### LOD Enforcement
- All static mesh environment actors (trees, rocks, terrain) set to `forced_lod_model = 0`
- Static mobility enforced on non-interactive environment props
- Result: Reduced draw call overhead for background geometry

### Dynamic Light Audit
- **Threshold**: Max 4 dynamic point lights before performance warning
- **Action**: Any excess dynamic point lights flagged for conversion to static
- **Recommendation**: Use Lumen for global illumination, minimize dynamic point lights

---

## Degenerate Label Policy (Enforced)

Per memory `hugo_no_degenerate_labels`:
- Labels > 60 characters flagged as degenerate
- Degenerate actors reported but NOT auto-deleted (requires manual review)
- Format standard: `Type_Biome_NNN` (e.g., `TRex_Savana_001`, `Rock_Forest_042`)

---

## Survival Actor Verification

| Actor | Status |
|-------|--------|
| PlayerStart | ✅ Present |
| Food_Berries_001 | ✅ Present |
| Water_Stream_001 | ✅ Present |
| CraftingStation_001 | ✅ Present |
| FirePit_Camp_001 | ✅ Present |

---

## Performance Rules for Downstream Agents

### Agent #5 (Procedural World Generator)
- Max 3 biome zones per map load
- Terrain hills: use landscape spline, not individual static mesh stacks
- River: single plane mesh with dynamic material, NOT multiple water actors
- PCG foliage density: max 500 instances per biome zone at LOD0

### Agent #6 (Environment Artist)
- Replace sphere/cube placeholders with real meshes — each replacement REDUCES draw calls (better LOD)
- Trees: use Hierarchical Instanced Static Mesh (HISM) for forest clusters
- Rocks: batch into groups of 3-5 per HISM instance
- Max 50 unique static mesh types in scene (draw call batching)

### Agent #8 (Lighting)
- SkyAtmosphere: 1 directional light only (sun)
- Max 4 dynamic point lights total (campfire, torches)
- All other lights: STATIC or STATIONARY
- Fog: ExponentialHeightFog — 1 actor only, no duplicates

### Agent #10 (Animation)
- Idle animations on dinos: use Skeletal Mesh with AnimBlueprint
- Max 5 animated skeletal meshes simultaneously (performance budget)
- LOD: disable animation on skeletal meshes > 3000 units from player

### Agent #12 (Combat AI)
- Patrol waypoints: use SplineComponent on single actor, not individual waypoint actors
- Aggro radius: sphere collision on dino actor, not separate trigger volume
- Max 3 active AI-controlled dinos simultaneously (NavMesh + BT cost)

---

## Next Agent Directives

**#5 — Procedural World Generator:**
- Add terrain height variation > 500 units (current terrain too flat)
- Define 3 biome zones: Savanna (center), Forest (north), Swamp (south)
- Add river plane from (5000, -3000, 50) to (5000, 3000, 50)
- Use Landscape actor, not StaticMeshActor for terrain

**#6 — Environment Artist:**
- Replace `Tree_001` through `Tree_012` sphere placeholders with `/Engine/BasicShapes/Cylinder` (taller, more tree-like) until real assets available
- Replace `Rock_001` through `Rock_006` with `/Engine/BasicShapes/Sphere` scaled (0.5, 0.5, 0.3) for flatter rock shape
- Add 3 grass patches near `Water_Stream_001`

**#8 — Lighting:**
- Set DirectionalLight pitch to -45 (golden hour angle)
- SkyAtmosphere: set `RayleighScattering` for warm prehistoric atmosphere
- Add single campfire PointLight at `FirePit_Camp_001` location (dynamic, intensity 2000)
