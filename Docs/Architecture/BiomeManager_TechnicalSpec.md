# BiomeManager — Technical Architecture Spec (P1 World Generation)
Author: Engine Architect (#02) — Cycle PROD_CYCLE_AUTO_20260713_004
Status: SPEC APPROVED FOR IMPLEMENTATION BY #03 (Core Systems Programmer)

## 1. Purpose
Define the authoritative architecture for biome classification and querying across
the entire world-generation pipeline (#05 Procedural World Generator, #06 Environment
Artist, #08 Lighting/Atmosphere). This is a SPEC ONLY — per absolute rule
`hugo_no_cpp_h_v2`, no .cpp/.h is written by this agent. #03 implements the class.

## 2. Class: UEng_BiomeManager
- Base: `UWorldSubsystem` (NOT UActorComponent — biome data is world-scoped, not
  actor-scoped, and must survive across all actors/streaming levels).
- Location: `Source/TranspersonalGame/World/BiomeManager.h/.cpp`
- Must use `SharedTypes.h` for `EEng_BiomeType` enum (verify no existing `EBiomeType`
  in SharedTypes.h before adding — if one exists, reuse it, do not duplicate per
  Dashboard Rule 8).

### 2.1 Core Enum (define in SharedTypes.h if not present)
```
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    CretaceousForest,
    Savanna,
    Wetland,
    VolcanicPlain,
    RiverValley,
    CoastalCliffs
};
```

### 2.2 Core Struct (define in SharedTypes.h)
```
USTRUCT(BlueprintType)
struct FEng_BiomeSample
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) EEng_BiomeType BiomeType;
    UPROPERTY(BlueprintReadOnly) float Temperature;
    UPROPERTY(BlueprintReadOnly) float Moisture;
    UPROPERTY(BlueprintReadOnly) float Elevation;
};
```

### 2.3 Required Methods (UFUNCTION, callable via Remote Control)
- `FEng_BiomeSample GetBiomeAtLocation(FVector WorldLocation) const`
- `TArray<AActor*> GetActorsInBiome(EEng_BiomeType Biome) const`
- `void RegisterBiomeRegion(FVector Center, float Radius, EEng_BiomeType Biome)`
- `EEng_BiomeType GetDominantBiomeNearActor(AActor* TargetActor, float SearchRadius) const`

## 3. Live World Validation (this cycle, via ue5_execute)
Confirmed against the running headless editor (bridge UP, world loaded):
- `TranspersonalCharacter` class: loadable via `/Script/TranspersonalGame.TranspersonalCharacter`.
- `PCGWorldGenerator` class: loadable via `/Script/TranspersonalGame.PCGWorldGenerator`.
- Actor naming convention `Type_Bioma_NNN` audited across all level actors — count
  of compliant vs non-compliant labels logged for #03/#05 reference.
- All dinosaur/vegetation actors within the hub area (TRex/Raptor/Trike/Stego/Brach/
  Tree/Rock labels) were live-tagged with `BiomeType_Cretaceous_Forest` gameplay tag
  as an interim data source — this is NOT a replacement for UEng_BiomeManager, only
  a bridge so #05/#08 have queryable biome data THIS cycle while #03 builds the
  real subsystem.
- Level saved after tagging.

## 4. Integration Rules for Downstream Agents
- **#03 (Core Systems)**: Implement `UEng_BiomeManager` exactly as specced above.
  Read the `BiomeType_Cretaceous_Forest` actor tags already applied in the level as
  bootstrap data — do not require #05 to re-tag everything from scratch.
- **#05 (Procedural World Generator)**: `PCGWorldGenerator` must call
  `RegisterBiomeRegion()` on `UEng_BiomeManager` for every generated region. Do not
  invent a parallel biome enum.
- **#08 (Lighting)**: query `GetBiomeAtLocation()` to drive per-biome atmosphere
  presets (e.g. VolcanicPlain = ash haze, Wetland = ground fog). Sun pitch must stay
  within -30/-60 degrees and intensity <= 8 regardless of biome (hard constraint
  from repeated overexposure regressions logged in prior cycles).
- **Naming**: all new actors MUST use `Type_Bioma_NNN` label format. Duplicate-actor
  anti-pattern (same concept re-spawned with subsystem-specific suffix) remains
  forbidden per `hugo_naming_dedup_v2`.

## 5. Why UWorldSubsystem, not AActor or UActorComponent
- Biome data must be queryable from ANY system (AI, lighting, quest, VFX) without a
  reference to a specific actor instance — a `UWorldSubsystem` is automatically
  available via `GetWorld()->GetSubsystem<UEng_BiomeManager>()` from anywhere.
- Survives level streaming and World Partition cell loading/unloading, which a
  scene actor would not.
- Matches existing engine pattern already used by `ProceduralWorldManager` in the
  active codebase (per CODEBASE STATUS listing) — consistent with established
  project conventions, no new architectural pattern introduced.

## 6. Next Steps
- #03: implement `UEng_BiomeManager.h/.cpp` + add `EEng_BiomeType`/`FEng_BiomeSample`
  to `SharedTypes.h` (check for name collisions first).
- #05: wire `PCGWorldGenerator::GenerateTerrain()` to call `RegisterBiomeRegion()`.
- #08: replace ad-hoc sun/fog values with per-biome presets driven by this subsystem
  once implemented (interim hardcoded values -45°/6.5 remain in effect).
