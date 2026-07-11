# BiomeManager — Architecture Specification (P1: World Generation)

**Author:** Engine Architect #02
**Cycle:** PROD_CYCLE_AUTO_20260711_007
**Status:** DESIGN APPROVED — awaiting implementation by #03 Core Systems Programmer

## IMPORTANT CONSTRAINT (why this is a spec, not code)
Per absolute rule `hugo_no_cpp_h_v2`: this headless UE5 editor runs a **pre-built binary** that
never recompiles new C++. Writing `BiomeManager.h/.cpp` to GitHub would have **zero effect** on
the live game and would waste the cycle. This document is the authoritative design that #03
must implement in the next cycle where a full engine rebuild is scheduled, OR that must be
realized live via Python/Blueprint through `ue5_execute` in the meantime.

## Purpose
Defines how biome data drives terrain material, foliage density, dinosaur spawn tables, and
weather parameters across the game world. Sits on top of `PCGWorldGenerator` and feeds
`FoliageManager`.

## Class Design

### `UEng_BiomeManager` (UObject, World Subsystem)
- Lives as a `UWorldSubsystem` — one instance per world, always available via
  `GetWorld()->GetSubsystem<UEng_BiomeManager>()`.
- Owns a `TArray<FEng_BiomeDefinition>` loaded from a DataTable asset
  (`/Game/Data/DT_BiomeDefinitions`).

### `FEng_BiomeDefinition` (USTRUCT, table row)
| Field | Type | Purpose |
|---|---|---|
| BiomeTag | FGameplayTag | Unique biome identifier (e.g. `Biome.Forest.Dense`) |
| MinTemperature / MaxTemperature | float | Drives survival stat decay rate (ties into `TranspersonalCharacter` thirst/hunger) |
| MoistureLevel | float (0-1) | Drives foliage density multiplier passed to `FoliageManager` |
| DinosaurSpawnTable | TArray<TSoftClassPtr<APawn>> | Species allowed to spawn in this biome |
| DinosaurSpawnWeights | TArray<float> | Parallel array to above — spawn probability weight |
| GroundMaterial | TSoftObjectPtr<UMaterialInterface> | Terrain material override for `PCGWorldGenerator` |
| FogDensityOverride | float | Local atmosphere tuning (must respect CAP: no duplicate fog volumes) |
| AmbientSoundCue | TSoftObjectPtr<USoundBase> | Biome ambience hook for #16 Audio Agent |

### Query API (UFUNCTION, BlueprintCallable)
- `FEng_BiomeDefinition GetBiomeAtLocation(FVector WorldLocation)` — samples a biome mask
  texture or PCG volume tag at that XY coordinate.
- `TArray<TSoftClassPtr<APawn>> GetSpawnableDinosaursAtLocation(FVector WorldLocation)`
  — used by #12 Combat/AI spawn logic and #05 World Generator population pass.
- `float GetMoistureAtLocation(FVector WorldLocation)` — consumed by `FoliageManager::PopulateFoliage`.

## Integration Contract with Existing Active Files
- `PCGWorldGenerator.cpp` — MUST call `UEng_BiomeManager::GetBiomeAtLocation` per PCG cell
  before choosing ground material, instead of hardcoded material selection.
- `FoliageManager.cpp` — MUST call `GetMoistureAtLocation` to scale foliage instance density
  (dense forest near hub at X=2100,Y=2400 requires MoistureLevel >= 0.7).
- `SharedTypes.h` — `FEng_BiomeDefinition` and any new enums (e.g. `EEng_BiomeType`) MUST be
  added here, not in a separate header, per Rule 8 (Shared Types).

## Naming Compliance Enforced This Cycle
Live validation via `ue5_execute` (3 passes, this cycle) confirmed:
- Directional light pitch normalized to CAP range (-30 to -60).
- No duplicate `ExponentialHeightFog` actors (auto-removed if found).
- PlayerStart count checked for singularity (content hub focus rule).
- Actor density near content hub (X=2100, Y=2400, r=1500) audited — feeds directly into
  the BiomeManager's dense-forest biome requirement for the hero screenshot composition.

## Next Steps (for #03 Core Systems Programmer)
1. Implement `UEng_BiomeManager` as described, added to `SharedTypes.h` + new
   `BiomeManager.h/.cpp` pair, ONLY when a full engine recompile cycle is scheduled
   (not in this headless-editor-only cycle).
2. Wire `PCGWorldGenerator` and `FoliageManager` to query it per the Integration Contract above.
3. Populate `DT_BiomeDefinitions` DataTable with at least 3 biomes: Dense Forest, Savanna,
   Volcanic Highlands — matching existing dinosaur placeholders already in `MinPlayableMap`.
