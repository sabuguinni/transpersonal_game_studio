# BiomeManager — Technical Architecture (P1: World Generation)
**Author:** Engine Architect #02 | **Cycle:** PROD_CYCLE_AUTO_20260712_009
**Status:** DESIGN APPROVED — awaiting compile-capable environment for #03 to implement `.h/.cpp`

---

## 0. Constraint Context (why this is a design doc, not code)
Per absolute rule `hugo_no_cpp_h_v2` (imp:MAX): this headless editor instance runs a
**pre-built binary that never recompiles new C++**. Any `.cpp/.h` written to GitHub in this
state is inert — zero effect on the live game, 218 UHT errors on record. Therefore this
cycle delivers:
1. A complete, compile-ready architecture spec (this document) for `UBiomeManager`.
2. A **live runtime stopgap** applied directly via `ue5_execute` (Actor Tags), so biome
   membership is queryable in `MinPlayableMap` TODAY, without needing a new compiled class.
3. Explicit integration contract for #03 (Core Systems), #05 (World Generator), #06
   (Environment Artist) to implement once a compile pass is available.

---

## 1. Purpose
`UBiomeManager` is a `UWorldSubsystem` that owns the authoritative mapping between world
location and biome classification, and exposes it to every dependent system (PCG terrain,
foliage density, dinosaur species spawn tables, weather/lighting presets, survival stat
modifiers like temperature).

## 2. Class Placement (Law)
- **Module:** `TranspersonalGame` (no new module — avoid Build.cs churn)
- **Base class:** `UWorldSubsystem` (NOT `UActorComponent` — biome data is world-global,
  not per-actor)
- **File location:** `Source/TranspersonalGame/World/BiomeManager.h` / `.cpp`
- **Naming:** `UBiomeManager`, `FBiomeDefinition` (struct), `EBiomeType` (enum) — all types
  MUST be added to `SharedTypes.h` per Rule 8 (Shared Types), not defined locally, since
  #05, #06, #08, #11 all need `EBiomeType`.

## 3. Data Contract (SharedTypes.h additions — for #03 to add when compiling)
```cpp
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Forest      UMETA(DisplayName="Dense Forest"),
    Savanna     UMETA(DisplayName="Savanna Plains"),
    Wetland     UMETA(DisplayName="River Wetland"),
    Highland    UMETA(DisplayName="Volcanic Highland"),
    Coastal     UMETA(DisplayName="Coastal Shore")
};

USTRUCT(BlueprintType)
struct FBiomeDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EBiomeType BiomeType;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float TemperatureBaseC = 20.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float HumidityPercent = 50.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float FoliageDensityMultiplier = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FName> DominantSpeciesTags;
};
```

## 4. API Surface (UBiomeManager)
| Function | Signature | Consumer |
|---|---|---|
| `GetBiomeAtLocation` | `EBiomeType GetBiomeAtLocation(FVector WorldLocation) const` | #05 PCGWorldGenerator, #11 NPC Behavior |
| `GetBiomeDefinition` | `FBiomeDefinition GetBiomeDefinition(EBiomeType Type) const` | #06 Environment Artist, #08 Lighting |
| `RegisterBiomeZone` | `void RegisterBiomeZone(FBox ZoneBounds, EBiomeType Type)` | #05 at world-gen time |
| `GetTemperatureModifier` | `float GetTemperatureModifier(FVector Location) const` | #03 survival stats (TranspersonalCharacter) |

## 5. Integration Order (Law — no agent skips ahead)
`#02 (this doc)` → `#03 implements UBiomeManager.h/.cpp + SharedTypes.h enum/struct` →
`#05 calls RegisterBiomeZone during PCG terrain pass` → `#06 reads GetBiomeDefinition for
foliage density` → `#08 reads biome for lighting/fog presets` → `#11 reads biome for
species habitat rules`.

## 6. Live Stopgap Applied This Cycle (verified via ue5_execute)
Since `UBiomeManager` cannot be compiled right now, a **quadrant-based biome tagging pass**
was applied directly to all actors in `MinPlayableMap` via Python `AActor.tags`:
- Quadrant split around the content hub (X=2100, Y=2400):
  - X≥2100, Y≥2400 → `Biome_Forest`
  - X<2100, Y≥2400 → `Biome_Savanna`
  - X≥2100, Y<2400 → `Biome_Wetland`
  - X<2100, Y<2400 → `Biome_Highland`
- Every actor was tagged (idempotent — no duplicate tags added on re-run).
- Level saved after tagging.
- This makes biome membership queryable NOW via `Actor.tags` for any Blueprint or Python
  script, without waiting for a compile pass — unblocks #05/#06/#11 prototyping.

## 7. Validation Performed This Cycle
- `unreal.load_class(None, '/Script/TranspersonalGame.X')` checked for all 7 active core
  classes (TranspersonalCharacter, TranspersonalGameState, PCGWorldGenerator,
  FoliageManager, CrowdSimulationManager, ProceduralWorldManager,
  BuildIntegrationManager) — confirms module loads cleanly, no CDO crash blocking editor.
- Hub composition query (X=2100/Y=2400, radius 1500) — actor count, dinosaur count,
  foliage count, light count — feeds #08/#18 validation of the hero screenshot composition.
- Quadrant biome tagging pass — applied and saved live.

## 8. Open Dependency for Next Cycle
- **#03**: implement `UBiomeManager` + `SharedTypes.h` additions per Section 3 as soon as
  a compile-capable session is available (flag to Studio Director when that window opens).
- **#05**: once compiled, replace quadrant stopgap with real PCG-driven biome boundaries
  (rivers, elevation, moisture noise maps) — quadrant tags remain as fallback/debug layer.
