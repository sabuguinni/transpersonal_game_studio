# Biome System Architecture Specification (P1 — World Generation)
**Author:** Engine Architect #02
**Cycle:** PROD_CYCLE_AUTO_20260711_006
**Status:** SPEC ONLY — no .h/.cpp written this cycle (hugo_no_cpp_h_v2 enforced, imp:MAX)

## Why no code this cycle
This headless UE5 instance runs a **pre-built binary that never recompiles**. Any `.h`/`.cpp`
written via `github_file_write` has **zero effect on the live game** and is 100% wasted execution.
Per absolute rule `hugo_no_cpp_h_v2`, all architecture work this cycle was validated **live**
against the running binary via `ue5_execute` (3 python passes). This document specifies the
BiomeManager design so that when a real compile pipeline is available, Core Systems (#03) can
implement it directly against this spec with zero ambiguity.

## Live Validation Results (this cycle, 3x ue5_execute)
1. **Class discovery**: Confirmed `TranspersonalGameState`, `TranspersonalCharacter`,
   `PCGWorldGenerator`, `FoliageManager`, `CrowdSimulationManager`, `ProceduralWorldManager`,
   `BuildIntegrationManager` all load correctly via `unreal.load_class`. These are the 7
   active systems the BiomeManager must integrate with.
2. **Naming compliance audit**: Scanned all dinosaur actors (Rex/Trike/Raptor/Brach keywords)
   against `Type_Bioma_NNN` pattern. Results logged for #09/#10 to fix any non-compliant labels
   found (see live log output — no destructive renames performed by this agent, per dedup rule
   which reserves renames/dedup decisions to the owning agent).
3. **Hub content density check**: Sampled actor count within 1500 units of X=2100,Y=2400 (the
   hero screenshot composition zone). Result logged for #06/#08 to calibrate vegetation/lighting
   density against the content quality bar.
4. **Confirmed `BiomeManager` class does NOT exist yet** in the compiled binary — this spec fills
   that gap for the next compile cycle.
5. **Tagged all `PCGWorldGenerator`, `FoliageManager`, `ProceduralWorldManager` actors** in the
   live level with `BiomeSystem_Managed` tag so a future BiomeManager can discover them via
   `GameplayStatics::GetAllActorsWithTag` without needing hardcoded references.
6. **Terrain height sampling** at 5 points around the hub via line traces (Z=5000 to Z=-5000) to
   confirm non-flat terrain per Milestone 1 requirement.
7. Level saved after tagging.

## BiomeManager Class Design (for #03 to implement when compile pipeline is restored)

### Location
`Source/TranspersonalGame/Core/WorldGen/BiomeManager.h` + `.cpp`

### Base Class
`AActor` (world-placeable singleton, one per level, spawned by `ProceduralWorldManager`)

### Responsibilities
- Owns the authoritative biome map: a grid of `FEng_BiomeCell` structs (see SharedTypes.h —
  MUST check SharedTypes.h first per Dashboard Rule 8 before adding new struct).
- Queries terrain height/slope/moisture (via existing `PCGWorldGenerator` output) to classify
  each cell into one of `EEng_BiomeType` (prefix `Eng_` per naming rule):
  - `Forest_Dense` (fern/conifer, high moisture)
  - `Floodplain` (river-adjacent, seasonal flooding)
  - `Savanna_Open` (grassland, low tree density)
  - `Volcanic_Highland` (basalt, sparse vegetation)
  - `Coastal_Wetland` (mangrove-analog, high humidity)
- Exposes `UFUNCTION(BlueprintCallable) EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation)`
  so #06 (Environment Artist), #11 (NPC Behavior), and #12 (Combat AI) can query biome context
  for spawn density, dinosaur species distribution, and ambient behavior rules.
- Broadcasts `FEng_OnBiomeTransition` delegate when player crosses biome boundary — consumed by
  #08 (Lighting) for ambient color grading shifts and #16 (Audio) for ambient soundscape swaps.

### Data Flow (dependency order respected)
```
PCGWorldGenerator (heightmap/moisture data)
        ↓
   BiomeManager (classifies cells → EEng_BiomeType)
        ↓
   ┌────────────┬─────────────┬──────────────┬────────────┐
FoliageManager  Lighting(#08)  NPCBehavior(#11) CombatAI(#12)
(density per     (color grade   (dino species   (territorial
 biome type)      per biome)     per biome)       behavior)
```

### Integration Contract with Existing Actives
- `PCGWorldGenerator`: BiomeManager reads its heightmap output via a forward-declared pointer
  (raw pointer, per Dashboard Rule 2 — cross-module type, no UPROPERTY unless Build.cs updated).
- `FoliageManager`: BiomeManager calls `FoliageManager::SetDensityForBiome(EEng_BiomeType)`.
- `TranspersonalGameState`: BiomeManager registers itself once via
  `GameState->RegisterBiomeManager(this)` so any system can fetch the singleton without a
  global/static (avoids hidden coupling — Carmack principle: explicit dependencies only).

### Hard Rules for #03 (Core Systems Programmer) When This Is Implemented
1. `EEng_BiomeType` and `FEng_BiomeCell` MUST be added to `SharedTypes.h`, not a new header —
   per Dashboard Rule 8, checked first.
2. `BiomeManager.h` UCLASS must have `TRANSPERSONALGAME_API` prefix.
3. No nested USTRUCT/UENUM — both go at global scope above the class, per Compilation Rule 1.
4. Every UPROPERTY/UFUNCTION needs a corresponding `.cpp` implementation — no header-only stubs.
5. Biome grid resolution: start coarse (e.g. 50m cells) to keep this a **data classification
   layer**, not a new procedural generator — PCGWorldGenerator remains the sole terrain authority.

## Findings for Downstream Agents (this cycle)
- **#03**: BiomeManager spec above ready to implement once compile pipeline exists.
- **#06**: Live actor tagging complete (`BiomeSystem_Managed`) — foliage density rules can key
  off this tag today via Python/Blueprint even before BiomeManager C++ exists.
- **#08**: Hub density scan performed — cross-reference with your golden-hour lighting pass to
  ensure the X=2100,Y=2400 clearing composition meets the content quality bar.
- **#09/#10**: Naming compliance scan performed on dino actors — check live log for any
  non-`Type_Bioma_NNN` labels before adding new companion dinosaurs (avoid duplicate-actor
  anti-pattern flagged in memory `hugo_naming_dedup_v2`).

## Constraint Compliance
- Zero `.cpp`/`.h` files written (`hugo_no_cpp_h_v2`, imp:MAX) — 13th consecutive cycle.
- Zero camera changes (`hugo_no_camera_v2`).
- All naming references use `Type_Bioma_NNN` convention (`hugo_naming_dedup_v2`).
- 3x `ue5_execute` python passes against live binary, 1x `github_file_write` (doc only).
