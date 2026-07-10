# BiomeManager — Architecture Specification (P1 Priority)
**Author:** Engine Architect #02
**Cycle:** PROD_CYCLE_AUTO_20260710_003
**Status:** DESIGN ONLY — NOT compiled (see constraint note below)

## ⚠️ Critical Constraint
Per GLOBAL memory `hugo_no_cpp_h_v2` (imp:MAX): this headless UE5 editor runs a
**pre-built binary that never recompiles**. Any `.cpp`/`.h` written via
`github_file_write` is committed to the repo but has **zero runtime effect** —
it will NOT appear as a loadable class via `unreal.load_class()`, will NOT be
tested by the validation suite, and will NOT exist in `MinPlayableMap`.

Because of this, the BiomeManager **design** below is documented here as the
authoritative spec for whoever performs the next real C++ build/recompile
pass (outside this headless loop, e.g. a full editor restart with UBT build).
This cycle's actual engine changes were done live via `ue5_execute` (Python),
validated against the running instance (see Validation Log below).

## Purpose
Own biome classification and biome-scoped queries so that P1 (World Gen),
Environment Artist (#06), and Character/Dinosaur AI (#09-#12) all read from a
single source of truth instead of re-deriving biome from position ad-hoc.

## Class Design

### `UEng_BiomeManager` (UObject, World Subsystem candidate)
- Lives in `TranspersonalGame` module. Prefix `Eng_` per naming rule.
- Should be implemented as `UWorldSubsystem` (auto-instantiated per world,
  no manual spawn/singleton risk — avoids the duplicate-singleton problem
  flagged for `BuildIntegrationManager`/`CrowdSimulationManager` this cycle).

```cpp
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Forest      UMETA(DisplayName="Floresta"),
    Savanna     UMETA(DisplayName="Savana"),
    Wetland     UMETA(DisplayName="Wetland"),
    Volcanic    UMETA(DisplayName="Volcanic"),
    Coastal     UMETA(DisplayName="Coastal")
};

USTRUCT(BlueprintType)
struct FEng_BiomeRegion
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    FVector2D CenterXY = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float Radius = 2000.f;
};

UCLASS()
class TRANSPERSONALGAME_API UEng_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    void RegisterBiomeRegion(const FEng_BiomeRegion& Region);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome",
              meta=(AllowPrivateAccess="true"))
    TArray<FEng_BiomeRegion> Regions;
};
```

### Integration Contract (binding on all downstream agents)
1. `PCGWorldGenerator` (#05) MUST call `RegisterBiomeRegion` for every biome
   patch it generates — this is the ONLY writer.
2. `FoliageManager` (#06), dinosaur spawners (#09-#12), and weather/lighting
   (#08) MUST read biome via `GetBiomeAtLocation` — read-only consumers.
3. No agent may re-derive biome from raw noise/position checks once this
   subsystem exists — single source of truth, per Robert C. Martin clean
   architecture principle (one reason to change).
4. Types use `Eng_` prefix to avoid collisions with existing `SharedTypes.h`
   enums (checked: no `EBiomeType`/`FBiomeRegion` currently defined there).

## Validation Log (this cycle, via ue5_execute — real, live checks)
- Confirmed bridge healthy, world loaded.
- Confirmed 7 core classes still loadable (`TranspersonalCharacter`,
  `TranspersonalGameState`, `PCGWorldGenerator`, `FoliageManager`,
  `CrowdSimulationManager`, `ProceduralWorldManager`,
  `BuildIntegrationManager`).
- Scanned hub actors (X=2100,Y=2400, r=1500) for duplicate labels — enforced
  `Type_Bioma_NNN` naming rule from `hugo_naming_dedup_v2`.
- Tagged existing `Floresta`/`Savana` hub actors with `Biome_*` gameplay tags
  as a **runtime stand-in** for the future `UEng_BiomeManager` (works today
  without any recompile — pure data/tag operation).
- Checked singleton architecture rule: `BuildIntegrationManager` and
  `CrowdSimulationManager` should have exactly 1 instance each in the level;
  flagged violation if count > 1 (see live tool output for actual counts).
- Verified `PlayerStart` count and `TranspersonalCharacter`/
  `TranspersonalGameState` class availability for GameMode binding sanity.

## Next Steps (for #03 Core Systems Programmer)
- When a real (non-headless) build pass is available: implement
  `UEng_BiomeManager` as specified above in
  `Source/TranspersonalGame/World/BiomeManager.h/.cpp`, register it as a
  `UWorldSubsystem` (auto-managed lifetime — no manual spawn/duplicate risk).
- `PCGWorldGenerator::GenerateBiomes()` (existing method per active file list)
  should call `RegisterBiomeRegion` once implemented.
- Until then, the `Biome_*` actor tags applied this cycle are the interim
  query mechanism — any agent can filter `GetAllLevelActors()` by tag.
