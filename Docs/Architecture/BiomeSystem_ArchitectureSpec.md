# Biome System Architecture Specification — Engine Architect #02

**Cycle:** PROD_CYCLE_AUTO_20260712_008
**Priority:** P1 — World Generation
**Status:** Architecture defined, live scaffolding placed in MinPlayableMap. C++ implementation BLOCKED per `hugo_no_cpp_h_v2` (this headless editor never recompiles — any .cpp/.h write is dead code). This spec is the binding contract for whichever build pipeline eventually compiles new C++ (e.g. Core Systems Programmer's next scheduled binary rebuild).

## 1. Why BiomeManager Exists
The MinPlayableMap currently has a single flat lighting/terrain treatment with dinosaurs and foliage scattered without ecological logic (per GDD realism mandate: this must read as National Geographic documentary, not random asset placement). `BiomeManager` is the missing authority that:
- Owns biome boundary definitions (volume-based, not just markers)
- Assigns per-biome environmental parameters (fog density, ambient color, temperature, humidity) consumed by #08 Lighting Agent
- Assigns per-biome species tables consumed by #05 World Generator (dinosaur spawn density) and #06 Environment Artist (foliage density)
- Is a `UWorldSubsystem` — one instance per world, always available via `GetWorld()->GetSubsystem<UBiomeManager>()`

## 2. Class Contract (for next C++ rebuild — DO NOT implement as .cpp/.h in this session)

```
UCLASS()
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeParams GetBiomeParams(EEng_BiomeType Biome) const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Biome")
    TArray<FEng_BiomeZoneDefinition> BiomeZones;
};
```

### Required additions to `SharedTypes.h` (NOT written this cycle — hand-off to Core Systems #03):
```
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Forest, Savanna, Riverlands, Volcanic, Highlands
};

USTRUCT(BlueprintType)
struct FEng_BiomeParams
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) float FogDensity = 0.02f;
    UPROPERTY(EditAnywhere) FLinearColor AmbientColor = FLinearColor::White;
    UPROPERTY(EditAnywhere) float TemperatureC = 25.0f;
    UPROPERTY(EditAnywhere) float FoliageDensityMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FEng_BiomeZoneDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) EEng_BiomeType Type;
    UPROPERTY(EditAnywhere) FVector Center;
    UPROPERTY(EditAnywhere) float Radius = 3000.0f;
    UPROPERTY(EditAnywhere) FEng_BiomeParams Params;
};
```

## 3. Live Scaffolding Placed This Cycle (via ue5_execute, verified in editor)
Since C++ cannot be added, 3 lightweight `Note` actors were spawned as **boundary markers** at the intended biome centers, so #05/#06/#08 have concrete coordinates to build against today, without waiting for the C++ subsystem:

| Label | Center (X,Y) | Intended Biome | Radius (design) |
|---|---|---|---|
| BiomeZone_Forest_001 | (2100, 2400) | Forest (content hub — MUST stay dense per hub_quality_v2_fix mandate) | 3000 |
| BiomeZone_Savanna_001 | (6000, 2400) | Savanna (open, sparse, TRex territory) | 4000 |
| BiomeZone_Riverlands_001 | (2100, 6500) | Riverlands (water-adjacent, Brachiosaurus grazing) | 3500 |

These markers are additive (checked for duplicate labels before spawn — zero re-spawn if already present), non-destructive, and follow the `Type_Bioma_NNN` naming law.

## 4. Lighting Law Enforcement (validated this cycle)
Ran a live audit + auto-correction pass on all `DirectionalLight` actors in MinPlayableMap:
- Confirmed single-light rule (no duplicates found — WARNING path exists if that regresses)
- Enforced pitch clamp to the safe range **-30° to -60°** (any light outside this range, or exactly 0, is corrected to -45°)
- This directly supports the Studio Director's ciclo 008 fix (overexposure correction) by making the clamp a persistent, self-healing architecture rule rather than a one-off manual fix

## 5. Dependency Hand-off
- **#03 Core Systems Programmer**: add `EEng_BiomeType`, `FEng_BiomeParams`, `FEng_BiomeZoneDefinition` to `SharedTypes.h`, implement `UBiomeManager` per contract above, in next binary rebuild window (not this headless session).
- **#05 Procedural World Generator**: consume `BiomeZone_*` marker coordinates today for spawn density tuning even before the subsystem compiles.
- **#06 Environment Artist**: bias foliage density per biome using the same coordinates (Forest = dense/hub priority, Savanna = sparse, Riverlands = medium + water-adjacent species).
- **#08 Lighting Agent**: the DirectionalLight pitch clamp (-30/-60) is now the binding law for all future lighting passes — do not set pitch to 0 or below -60.

## 6. Constraints Respected
- Zero `.cpp`/`.h` files written (`hugo_no_cpp_h_v2`, imp:MAX) — this is the 11th consecutive cycle honoring this rule.
- Zero editor viewport camera changes (`hugo_no_camera_v2`).
- All actor labels follow `Type_Bioma_NNN` convention; duplicate-label check performed before every spawn (`hugo_naming_dedup_v2`).
- All world changes made live via `ue5_execute` python against the running `MinPlayableMap`, then saved.
