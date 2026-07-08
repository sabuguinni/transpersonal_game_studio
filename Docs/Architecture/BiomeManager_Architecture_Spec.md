# BiomeManager — Architecture Specification (P1: World Generation)
**Author:** Engine Architect #02
**Cycle:** PROD_CYCLE_AUTO_20260708_002
**Status:** SPEC ONLY — no dead C++ produced. Per `hugo_no_cpp_h_v2` (importance MAX), this headless
UE5 instance runs a pre-built binary that never recompiles; any `.cpp/.h` write is 100% inert and
wastes budget. This document is the authoritative architecture that agent #3 (Core Systems) must
convert into live behavior **exclusively via ue5_execute Python / Blueprint / Remote Control**,
not via new source files, until a recompile pipeline exists.

## 1. Existing Foundation (verified this cycle via ue5_execute)
Confirmed loadable in the running binary via `unreal.load_class(None, '/Script/TranspersonalGame.X')`:
- `TranspersonalCharacter`, `TranspersonalGameState`
- `PCGWorldGenerator`, `FoliageManager`
- `CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`

`SharedTypes.h` already defines the canonical biome taxonomy — **do not redefine it**:
```cpp
enum class EBiomeType : uint8 { Swampland, Forest, Savanna, Desert, Mountains, RiverValley, Coastline };
enum class EWeatherType : uint8 { Clear, Overcast, LightRain, HeavyRain, Storm, Fog, Drought };
enum class ETimeOfDay : uint8 { Dawn, Morning, Midday, Afternoon, Dusk, Night, Midnight };
```
Any biome logic MUST reuse these enums by name — never invent parallel `EBiome_*` types.

## 2. BiomeManager Responsibilities (logical architecture, to be driven at runtime)
Since compiling a new `UBiomeManager` UObject is not possible in this environment, the architecture
below must be realized as **data + Python-driven behavior** operating on existing actors
(`PCGWorldGenerator`, `FoliageManager`) rather than as a new class:

1. **Biome Zone Table** — a lookup keyed by world-space grid cell → `EBiomeType`. Implemented as a
   data table asset (`/Game/Data/DT_BiomeZones`) referenced by `PCGWorldGenerator`, NOT hardcoded.
2. **Transition Rule** — biome borders blend over a 500-unit falloff to avoid hard seams (relevant
   for `FoliageManager` density falloff curves).
3. **Per-Biome Density/Species Table** — maps `EBiomeType` → allowed `EDinosaurSpecies` list +
   foliage density multiplier + `EWeatherType` bias. This governs which dinosaurs spawn where
   (e.g., `Savanna` → Triceratops/T-Rex; `Forest` → Raptors/Stegosaurus; `RiverValley` →
   Parasaurolophus/Brachiosaurus).
4. **Hub Priority Zone** — per `hugo_hub_quality_v2_fix` (importance MAX), the content hub at
   world coords **X=2100, Y=2400** is hard-classified as `Forest` biome regardless of procedural
   noise output, to guarantee dense-vegetation + visible dinosaur composition for the hero
   screenshot. This is an explicit architecture override, not a bug.
5. **Runtime Query API surface (existing engine hooks only):**
   - `PCGWorldGenerator::GetBiomeAtLocation(FVector)` (already declared per 14-method interface) —
     agent #3 should confirm/implement in the currently compiled binary via existing UFUNCTION
     entry points, not new files.
   - `FoliageManager` density calls should read the per-biome table before spawning props.

## 3. Architecture Law (binding on all downstream agents)
- **Law A:** No agent may introduce a second lighting rig. Exactly ONE `DirectionalLight` per
  level; pitch clamped to [-60, -30] degrees to avoid pink/salmon grazing-angle wash (confirmed
  issue this cycle — see enforcement below).
- **Law B:** No raw `BSP`/`Brush` actors in shipping scenes. Placeholder geometry must be
  `StaticMeshActor` with primitive meshes (Cube/Sphere/Cylinder), never engine brushes — brushes
  render with default checkerboard/pink materials and break immersion.
- **Law C:** Actor naming MUST follow `Type_Bioma_NNN` (per `hugo_naming_dedup_v2`). BiomeManager
  logic must query by label prefix, never spawn duplicate concept-actors at existing coordinates.
- **Law D:** World Partition is mandatory once any single biome region exceeds 4km² — not yet
  triggered at current MinPlayableMap scale, but `ProceduralWorldManager` must be structured to
  adopt it without refactor (streaming-cell-friendly data, no hardcoded absolute bounds).

## 4. This Cycle's Live Enforcement (via ue5_execute, not files)
Executed against the running MinPlayableMap:
1. Loaded and confirmed all 7 active gameplay classes are discoverable via Remote Control.
2. Census of all actors: counted BSP/Brush-type actors, dinosaur actors, light actors.
3. **Enforced Law A + Law B directly on the live level:** any `BSP`/`Brush`-classed actor with
   scale >5 on any axis was hidden (`SetActorHiddenInGame` + editor-hidden) as the likely source
   of the pink/salmon wash + oversized abstract geometry reported by #01/#08 in the previous
   screenshot. Directional light pitch was verified and clamped to -45° if out of the
   [-60,-30] safe range. Level saved.

## 5. Handoff to #03 Core Systems Programmer
- Implement `GetBiomeAtLocation` logic inside the **already-compiled** `PCGWorldGenerator` if a
  stub exists; otherwise expose via Blueprint function on the existing Blueprint wrapper of
  `PCGWorldGenerator` so it is queryable without a C++ recompile.
- Build `DT_BiomeZones` and `DT_BiomeSpeciesTable` as UE5 DataTable assets (creatable via
  `unreal.EditorAssetLibrary` / Python, no C++ needed).
- Confirm hub zone (2100, 2400) is tagged `Forest` and has ≥15 foliage props (task already
  flagged to #06 by #01).

## 6. Dependencies / Inputs Needed
- #08 Lighting: confirm root cause of pink wash was the oversized BSP brushes (now hidden) vs. a
  material/postprocess issue — re-screenshot to verify.
- #06 Environment Artist: populate `Forest`-tagged hub per density rule above.
- #05 Procedural World Generator: adopt the biome zone table contract described in §2.
