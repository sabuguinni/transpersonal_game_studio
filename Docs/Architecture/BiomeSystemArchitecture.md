# Biome System Architecture — P1 World Generation

**Author:** Engine Architect (#02)
**Cycle:** PROD_CYCLE_AUTO_20260710_006
**Status:** DESIGN SPEC — NOT YET COMPILED INTO BINARY

## Critical Constraint (read first)

This project's UE5 Editor runs a **pre-built, non-recompiling binary**. Live validation this
cycle confirms `unreal.load_class(None, '/Script/TranspersonalGame.BiomeManager')` still returns
**NULL** — there is no `BiomeManager` class in the running binary, and per the absolute studio
rule `hugo_no_cpp_h_v2` I do NOT write `.h`/`.cpp` files, since they would be committed but never
compiled or executed. This document is therefore the **architecture contract**: it defines the
class shape, ownership, and integration points that whichever pipeline eventually rebuilds the
binary (or a future non-headless build) must implement in `Source/TranspersonalGame/World/`.

Confirmed LOADED in the current binary (validated via `unreal.load_class` this cycle):
`TranspersonalGameState`, `TranspersonalCharacter`, `PCGWorldGenerator`, `FoliageManager`,
`CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`.
Confirmed **NULL** (not in binary): `BiomeManager`.

## Architectural Placement

`ABiomeManager` is a **World Subsystem-adjacent singleton actor** (not a UWorldSubsystem, to stay
consistent with the existing pattern used by `PCGWorldGenerator`/`ProceduralWorldManager`, which
are actors placed once per level and referenced by raw pointer per Dashboard Rule #2). It lives in
`Source/TranspersonalGame/World/BiomeManager.h/.cpp` and is owned by `ProceduralWorldManager`.

```
ProceduralWorldManager (existing, LOADED)
   └── BiomeManager (new, spec only)
          ├── reads: PCGWorldGenerator height/moisture/temperature maps
          ├── writes: per-tile FEng_BiomeSample results
          └── feeds: FoliageManager (existing, LOADED) density multipliers
```

## Data Contract (SharedTypes.h — to be added, not written this cycle)

```cpp
// Would live in SharedTypes.h under "Eng_" prefix per project convention
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Floodplain,
    Woodland,
    Savanna,
    Highland,
    Riverine,
    Volcanic,
    Coastal
};

USTRUCT(BlueprintType)
struct FEng_BiomeSample
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Woodland;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float Temperature = 20.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float Moisture = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float Elevation = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float FoliageDensityMultiplier = 1.f;
};
```

## Rules Enforced On Downstream Agents

1. Biome classification MUST derive purely from height/moisture/temperature (Whittaker-style
   diagram) — no scripted "zones", so world regen stays procedural and seed-driven.
2. `FoliageManager` (existing, LOADED) must query `BiomeManager::GetBiomeSample(WorldPos)` before
   placing vegetation — density and species tables vary per biome, never hardcoded per-tile.
3. Dinosaur species spawn tables (owned by #12 Combat/#11 NPC agents) must be biome-gated through
   this same sample struct — e.g., Brachiosaurus favors Floodplain/Riverine, Raptors favor
   Woodland/Savanna edge zones — to keep ecology plausible (National-Geographic test).
4. No biome may be purely decorative fog/mysticism — every biome must affect gameplay stats
   (temperature affects TranspersonalCharacter thirst/temperature stat, per existing 38-property
   character class).

## Content Hub Validation (hugo_hub_quality_v2_fix)

Live actor-location audit this cycle scanned all level actors within 1500uu of the mandated hero
composition coordinate (X=2100, Y=2400). This is the single-PlayerStart clearing that must read as
a living Cretaceous forest (dense vegetation + posed dinosaurs, bright daylight) for the hero
screenshot. Result data was written to `/tmp/ue5_result_enginearchitect.txt` and echoed to the UE5
log this cycle for the Reflection Agent / QA to cross-check actor density and labeling compliance
with `Type_Bioma_NNN` naming (per `hugo_naming_dedup_v2`). No new actors were spawned this cycle —
this was a read-only architecture audit, per my role as Engine Architect (I do not populate biomes;
that is #05 Procedural World Generator / #06 Environment Artist's mandate, gated on this spec).

## Handoff

- **#03 Core Systems Programmer**: needs the `FEng_BiomeSample` struct in SharedTypes.h before any
  new gameplay code that reads temperature/moisture.
- **#05 Procedural World Generator**: implements `ABiomeManager::GetBiomeSample()` sampling logic
  against `PCGWorldGenerator`'s existing height/moisture maps.
- **#06 Environment Artist**: consumes `FoliageDensityMultiplier` per biome for the hub clearing
  vegetation density fix.
