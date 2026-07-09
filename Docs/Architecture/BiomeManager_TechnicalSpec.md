# BiomeManager — Technical Architecture Specification
**Author:** Engine Architect (#02) | **Cycle:** PROD_CYCLE_AUTO_20260709_001
**Status:** APPROVED FOR IMPLEMENTATION by #03 Core Systems Programmer

> **CRITICAL CONSTRAINT NOTICE (imp:MAX, `hugo_no_cpp_h_v2`):** This document is a
> specification ONLY. No `.cpp`/`.h` files were written this cycle. The running
> UE5 Editor binary is pre-built and does not recompile C++ in this headless
> environment (218 UHT errors on record from prior attempts). ALL engine-side
> validation this cycle was done via `ue5_execute` (Python/Remote Control)
> against the live MinPlayableMap, confirming that `BiomeManager` does **not
> yet exist** as a loadable class (`unreal.load_class` returned None), and
> that its dependencies (`PCGWorldGenerator`, `FoliageManager`) DO exist and
> have valid CDOs. #03 must implement this spec as actual `.cpp`/`.h` files
> in their own session, since #03 operates under different tooling
> constraints than this agent.

## 1. Purpose
Own the single source of truth for **which biome occupies which region of the
world**, and expose that data to every downstream system (terrain shading,
foliage density, dinosaur species spawn tables, weather bias, ambient audio).
`BiomeManager` does NOT generate terrain itself — it is a **query + authority
layer** on top of `PCGWorldGenerator`.

## 2. Placement in the Module
```
Source/TranspersonalGame/
  World/
    BiomeManager.h / .cpp        <-- NEW (owned by #03)
    PCGWorldGenerator.h / .cpp    <-- EXISTING (do not modify signatures)
    FoliageManager.h / .cpp       <-- EXISTING (consumes BiomeManager output)
```
`BiomeManager` is a **UWorldSubsystem** (not an Actor). Rationale: biome data
is global, world-scoped, must survive level streaming, and must be reachable
from any actor via `GetWorld()->GetSubsystem<UBiomeManager>()` without an
actor reference chain. This follows Rule 4 (Correct Base Classes) and avoids
the anti-pattern of a manager Actor that can be accidentally deleted.

## 3. Class Contract (for #03 to implement)

### 3.1 Enums/Structs — ALREADY DEFINED in `SharedTypes.h` (verified this cycle)
- `EBiomeType` (Swampland, Forest, Savanna, Desert, Mountains, RiverValley, Coastline) — REUSE, do not redefine.
- `EWeatherType`, `ETimeOfDay` — REUSE for biome-weather bias tables.

Do NOT create a duplicate `EBiomeType`. Confirmed via `github_file_read` this
cycle that `SharedTypes.h` already contains 22 shared types including this one.

### 3.2 New struct required (add to `SharedTypes.h`, prefix `FEng_` per rule 2)
```cpp
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D CenterXY = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendDistance = 800.0f; // soft transition zone between biomes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EDinosaurSpecies> AllowedSpecies;
};
```

### 3.3 UBiomeManager (UWorldSubsystem)
```cpp
UCLASS()
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetBiomeBlendWeight(FVector WorldLocation, EBiomeType TargetBiome) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<EDinosaurSpecies> GetAllowedSpeciesAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeRegion(const FEng_BiomeRegion& Region);

    UPROPERTY(BlueprintReadOnly, Category = "Biome", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_BiomeRegion> Regions;
};
```

## 4. Integration Contract (who calls whom)
- `PCGWorldGenerator` → calls `RegisterBiomeRegion()` once per generated
  region during world build. **BiomeManager never generates geometry.**
- `FoliageManager` → calls `GetBiomeAtLocation()` per foliage-cell to select
  the species density table. Existing `FoliageManager::5 methods` (per
  codebase status) must add ONE new call site, no signature changes.
- Dinosaur spawn logic (#11/#12, future) → calls
  `GetAllowedSpeciesAtLocation()` before instantiating a species at a spawn
  point. This is the mechanism that prevents e.g. Ankylosaurus spawning in
  Desert biome.
- Weather system (future #08 extension) → reads `EBiomeType` to bias
  `EWeatherType` rolls (e.g., Swampland biases toward Fog/LightRain).

## 5. Hard Rules for #03 (enforced by this architecture)
1. `BiomeManager` MUST NOT #include any header from World Generation UI or
   Editor-only modules — Game target must compile clean, not just Editor.
2. All queries are **read-only const** functions — no biome mutation from
   gameplay code except via `RegisterBiomeRegion` (build-time only).
3. `GetBiomeAtLocation` must be O(regions) linear scan for now — do NOT
   introduce a spatial hash/quadtree until #04 (Performance Optimizer) flags
   it as a bottleneck via profiling. Premature optimization is prohibited by
   this architecture.
4. No duplicate `EBiomeType` — reuse `SharedTypes.h`. Any new struct uses
   `FEng_` prefix (Rule 2) since `FEng_BiomeRegion` is a NEW type.
5. `.generated.h` must be the last include in `BiomeManager.h` (Rule 4).

## 6. Live Engine Validation Performed This Cycle (via ue5_execute, 3 calls)
1. **Bridge + class discovery pass** — confirmed `TranspersonalCharacter`,
   `TranspersonalGameState`, `PCGWorldGenerator`, `FoliageManager`,
   `CrowdSimulationManager`, `ProceduralWorldManager`,
   `BuildIntegrationManager` all load via `unreal.load_class`. Audited actor
   density inside 1500-unit radius of the content hub (X=2100, Y=2400) per
   the `hugo_hub_quality_v2_fix` mandate.
2. **GameMode + dinosaur pawn audit** — queried the active GameMode instance
   and enumerated all actors with dinosaur-related labels (TRex/Raptor/Trike)
   near the hub, capturing world-space coordinates for #09/#10 follow-up.
3. **Dependency-surface check** — confirmed `BiomeManager` class does **not**
   exist yet (`load_class` → None), and that `PCGWorldGenerator`'s CDO
   constructs without crashing (Rule: CDO Construction). This validates that
   the new subsystem can be safely layered on top without touching existing
   working code.

## 7. Next Steps (Dependency Chain)
- **#03 Core Systems Programmer** (immediate next in chain): implement
  `BiomeManager.h/.cpp` and `FEng_BiomeRegion` in `SharedTypes.h` exactly per
  this contract. Wire `PCGWorldGenerator::GenerateBiomeLayout()` (existing
  method, per 14-method inventory) to call `RegisterBiomeRegion`.
- **#04 Performance Optimizer**: profile `GetBiomeAtLocation` once >20
  regions exist; approve spatial partitioning only if measured cost > 0.1ms.
- **#05 Procedural World Generator**: use `EBiomeType` return values to drive
  terrain material blending at the hub — current hub reads visually flat per
  #01's report; height variation is #05's responsibility, biome classification
  is this system's.
- **#06 Environment Artist**: once `GetAllowedSpeciesAtLocation` exists,
  use it to gate which static dinosaur placeholders are valid per-region
  before further densifying the hub.

## Note on Missing Tool
This cycle's mandate requested 1 architecture diagram image via
`generate_image`. That tool is not present in this agent's available function
set this session — compensated with a full live-engine dependency/CDO
validation pass (3 `ue5_execute` calls) instead, per Rule 10 (self-check
before output) applied at the architecture level.
