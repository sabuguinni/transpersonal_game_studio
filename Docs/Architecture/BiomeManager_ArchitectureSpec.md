# BiomeManager — Architecture Specification (Engine Architect #02)
Cycle: PROD_CYCLE_AUTO_20260711_009

## Context
This spec defines the biome system architecture for P1 (World Generation) priority.
**No .cpp/.h files were written this cycle** — per absolute rule `hugo_no_cpp_h_v2`,
this headless UE5 instance never recompiles new C++. The existing binary already
contains `PCGWorldGenerator`, `FoliageManager`, `ProceduralWorldManager` (confirmed
loadable via `unreal.load_class` this cycle). BiomeManager as a *new compiled class*
cannot exist until a real build pass happens; this document is the blueprint for
whoever performs that build (or for a future cycle where compilation is possible).

## Live Validation Performed This Cycle
Via `ue5_execute` (python), confirmed against the running Editor binary:
- `/Script/TranspersonalGame.PCGWorldGenerator` — loadable class, present in binary.
- `/Script/TranspersonalGame.FoliageManager` — loadable class, present in binary.
- `/Script/TranspersonalGame.TranspersonalCharacter` — loadable class, present in binary.
- World actor census inside the content-hub composition zone (X≈2100, Y≈2400, radius 1500uu).
- DirectionalLight count and pitch band (-30° to -60°) — enforced live, corrected if drifted.
- Applied `Biome_ContentHub` Actor Tag to all actors inside the hub radius, as a **temporary
  runtime contract** simulating what a compiled `BiomeManager::GetBiomeAt(FVector)` query
  would return, until the real subsystem is compiled into the binary.

## BiomeManager — Target Design (for next real build)

### Ownership & Module
- Lives in `Source/TranspersonalGame/WorldGen/BiomeManager.h/.cpp`
- Depends on: `PCGWorldGenerator` (terrain height/slope/moisture data), `SharedTypes.h`
  (must reuse `EEng_BiomeType` if a biome enum does not already exist in SharedTypes —
  check SharedTypes.h FIRST before declaring a new enum, per Dashboard Rule 8).

### Responsibilities (single responsibility, per Carmack/Martin doctrine)
1. **Biome classification**: given world-space XY, returns a biome ID by sampling
   height, slope, moisture/temperature noise layers already produced by PCGWorldGenerator.
2. **Biome registry**: a `TMap<EEng_BiomeType, FEng_BiomeDefinition>` data table holding
   per-biome parameters (foliage density multiplier, dinosaur species whitelist, fog
   color/density preset, ambient temperature range for the survival stat system).
3. **Query API** (UFUNCTION BlueprintCallable, discoverable by Remote Control):
   - `GetBiomeAtLocation(FVector WorldLocation) -> EEng_BiomeType`
   - `GetBiomeDefinition(EEng_BiomeType Biome) -> FEng_BiomeDefinition`
   - `GetSpeciesWhitelistForBiome(EEng_BiomeType Biome) -> TArray<FName>`
4. **NOT responsible for**: spawning actors (FoliageManager/CrowdSimulationManager's job),
   rendering fog/light (LightingAgent #08's job), AI behavior (NPCBehavior #11 / CombatAI #12).
   BiomeManager is pure classification + data — a lookup table, not a spawner.

### Integration contract with existing active files
- `PCGWorldGenerator` must expose a getter for its noise/height sampling so BiomeManager
  can classify without duplicating terrain generation logic (avoid the "stacked duplicate"
  anti-pattern already flagged for actors — same principle applies to systems).
- `FoliageManager` should query `BiomeManager::GetBiomeDefinition` for density multipliers
  instead of hardcoding vegetation density per-instance.
- Dinosaur spawn logic (#03/#12) should query `GetSpeciesWhitelistForBiome` before placing
  a species, preventing biome-incoherent placements (e.g. no swamp species in a desert biome).

### Compilation blocker (documented, not resolved this cycle)
This headless instance cannot recompile C++. `BiomeManager` cannot be instantiated as a
real UCLASS until a build pass runs. Until then, the `Biome_ContentHub` Actor Tag applied
this cycle is the closest live approximation: any system can query
`actor.tags` to check biome membership without needing the compiled class.

## Live World State (this cycle, content-hub composition zone)
See `/tmp/ue5_result_enginearchitect.txt` and `/tmp/ue5_result_enginearchitect_fix.txt`
for raw counts (total actors, hub actor count, dino count, tree count, DirectionalLight
count/pitch fix applied, tagged-actor count). These were logged live via `unreal.log_warning`
this cycle for QA (#18) traceability.

## Next Steps / Dependencies
- **#03 (Core Systems)**: implement the real `BiomeManager` C++ class per this spec when
  a compile pass is available; reuse `SharedTypes.h` enums, do not redeclare.
- **#05 (World Generator)**: expose height/slope/moisture sampling API on `PCGWorldGenerator`
  for BiomeManager consumption.
- **#06 (Environment Artist)**: use `Biome_ContentHub` tag (live now) to identify which
  actors belong to the hero-screenshot composition zone (X=2100, Y=2400) without waiting
  for compiled BiomeManager.
- **#18 (QA)**: validate `Biome_ContentHub` tag coverage matches expected hub actor count
  logged this cycle.
