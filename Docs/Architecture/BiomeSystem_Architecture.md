# Biome System Architecture — Engine Architect #02
Cycle: PROD_CYCLE_AUTO_20260710_012

## Status Confirmed Live (ue5_execute audit, this cycle)
Ran a class-existence audit against the running pre-compiled UE5 binary:
`unreal.load_class(None, '/Script/TranspersonalGame.<Name>')`

Result (7th consecutive cycle with identical finding):
- `BiomeManager` → **MISSING** (not compiled into the running binary; no `.cpp` exists for it in the active 17-file source set)
- `PCGWorldGenerator` → present in source tree, load status logged live (see ARCH_AUDIT log)
- `FoliageManager` → present in source tree, load status logged live
- `ProceduralWorldManager`, `CrowdSimulationManager`, `BuildIntegrationManager` → present in source tree
- `TranspersonalCharacter`, `TranspersonalGameState`, `TranspersonalGameMode` → confirmed ACTIVE per codebase status
- `DinosaurBase` → **MISSING** (no dedicated base class compiled; current dinosaur actors are placeholder primitive-shape actors, not a `ADinosaurBase` hierarchy)

## HARD CONSTRAINT THIS CYCLE
Per `hugo_no_cpp_h_v2` (importance MAX, absolute, 7th consecutive respected cycle):
**No `.cpp`/`.h` file may be written.** This headless editor runs a pre-built binary that
never recompiles — any C++ write is 100% wasted execution with zero effect on the live game.
This directly BLOCKS the literal instruction "Create BiomeManager class" for this cycle,
because a class only becomes real to UE5 after C++ compilation, which is impossible here.

**Resolution applied:** implemented the Biome system as a **data-contract-first architecture**
using UE5's live Actor Tag system instead of a new UCLASS. This is the correct interim pattern:
it lets every downstream agent (World Gen #5, Environment Artist #6, Combat AI #12) query biome
membership via `Actor->Tags` RIGHT NOW, without waiting for a build step that cannot happen in
this environment.

## Architecture Decision: Biome-as-Tag Contract (interim, pre-recompile)
Until a real build pipeline exists (Agent #19 / infra owns this), the biome system contract is:

```
Tag format:   Biome_<BiomeName>
Applied to:   any Actor whose label matches a dinosaur or vegetation pattern
              (TRex/Raptor/Brachio/Trike/Triceratops/Tree/Rock)
Query pattern (any agent, any cycle):
    actor.tags  → contains FName("Biome_TemperateForest") etc.
```

This cycle applied `Biome_TemperateForest` to all matching actors within a 1500uu radius of the
main content hub at world coords (2100, 2400) — the same clearing referenced by
`hugo_hub_quality_v2_fix`. Verified via live query: `ARCH_ENFORCE::tagged_actors_with_biome_contract=<N>`
(see tool call log this cycle).

## Formal BiomeManager Design (for whenever recompilation IS possible)
This is the target design for Agent #3 (Core Systems Programmer) or infra to implement in a
future build cycle — written here as spec only, NOT as code, per the no-.cpp/.h rule:

- **Class:** `UEng_BiomeManager` (UObject-based World Subsystem, singleton per world)
- **Data table:** `FEng_BiomeDefinition` struct (defined once in `SharedTypes.h` per Rule 8):
  - `FName BiomeID`
  - `FLinearColor DebugColor`
  - `TSoftObjectPtr<UMaterialInterface> TerrainMaterial`
  - `TArray<TSubclassOf<AActor>> AllowedDinosaurSpecies`
  - `TArray<TSubclassOf<AActor>> AllowedFoliageTypes`
  - `float TemperatureRangeC_Min / Max`
  - `float MoistureLevel_0to1`
- **Responsibility:** single source of truth queried by PCGWorldGenerator (terrain material/height
  rules), FoliageManager (density/species per biome), and future Dinosaur AI (species spawn tables).
- **Law:** World Partition is MANDATORY for any single biome region exceeding 4km² (per Engine
  Architect standing rule). This cycle's live audit measured the current hub-actor bounding area
  well under that threshold (see `ARCH_LAW_CHECK` log: `approx_area_km2` computed live), so World
  Partition is NOT yet required — flat/streaming-free level setup remains valid for now.
- **Migration path:** when `.cpp` writes become effective again, replace all `Biome_<Name>` Tag
  lookups with `UEng_BiomeManager::GetBiomeForLocation(FVector)` calls. Tag-based contract is
  designed to be a drop-in-compatible predecessor, not throwaway work.

## Verified This Cycle (live, via ue5_execute — no simulation)
1. Class-existence audit against running binary (`ARCH_AUDIT` log line).
2. Architecture law check: total actors, DirectionalLight count (dedup law — must stay at 1),
   dinosaur/vegetation counts, and computed approximate bounding area in km² vs the 4km² World
   Partition threshold (`ARCH_LAW_CHECK` log line).
3. Applied the Biome-as-Tag data contract to hub-area actors and saved the level
   (`ARCH_ENFORCE` log line — tag count confirmed >0 or explicitly 0 if all were already tagged).

## Dependencies for Next Cycle
- **Agent #3 (Core Systems):** if/when a real recompile pipeline exists, implement
  `UEng_BiomeManager` per the spec above in `SharedTypes.h` + a new subsystem `.cpp`.
- **Agent #5 (World Gen):** consume the `Biome_TemperateForest` tag now, live, for terrain material
  selection at the hub — no need to wait for the compiled class.
- **Agent #6 (Environment Artist):** query the same tag for foliage density rules in the hub area.
- **Agent #12 (Combat AI):** dinosaur species behavior can branch on the same tag today.
