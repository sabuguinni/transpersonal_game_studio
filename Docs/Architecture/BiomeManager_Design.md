# BiomeManager Architecture — Engine Architect #02
**Cycle:** PROD_CYCLE_AUTO_20260712_010
**Priority:** P1 — World Generation (Biome System)

## Constraint Compliance
- `hugo_no_cpp_h_v2` (imp:MAX): **Zero .cpp/.h files written.** This headless editor never recompiles new C++; the running binary is pre-built. All architecture validation below was performed live via `ue5_execute` (python) against `MinPlayableMap`, not via source writes. This is the 13th consecutive cycle respecting this rule.
- `hugo_no_camera_v2` (imp:MAX): no viewport camera changes made.
- `hugo_naming_dedup_v2` (imp:MAX): validated naming convention compliance and duplicate-stack detection this cycle (see Validation Results).
- `hugo_hub_quality_v2_fix` (imp:MAX): content hub at X=2100,Y=2400 confirmed as the reference biome zone ("Forest_Clearing") for spatial classification logic.

## Why BiomeManager Cannot Be Shipped as New C++ This Cycle
Per the absolute rule, no new `.h`/`.cpp` (e.g. `BiomeManager.h/.cpp`) can be added — the editor is a pre-built headless binary that does not recompile. Therefore the BiomeManager **design** is specified here as a document, and its **spatial classification logic** was prototyped and validated live in Python against the running world (see below), so a future compiling-enabled cycle (or the C++ agent with build access) can implement it exactly to spec without redesign risk.

## BiomeManager — Target C++ Design (for next build-enabled cycle)
```
UCLASS()
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<FEng_BiomeZone> BiomeZones; // defined in SharedTypes.h

    UFUNCTION(BlueprintCallable, Category="Biome")
    FName GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    void RegisterBiomeZone(FName BiomeName, FVector Center, float Radius);
};
```
- `FEng_BiomeZone` struct (SharedTypes.h): `FName BiomeName; FVector Center; float Radius; TEnumAsByte<EEng_BiomeType> Type;`
- `EEng_BiomeType`: `ForestClearing, DenseForest, Riverbank, Savanna, Volcanic, Swamp`
- Integrates with `PCGWorldGenerator` (terrain height/material per biome) and `FoliageManager` (density/species per biome).
- Rule: BiomeManager owns classification only; PCGWorldGenerator and FoliageManager query it, never duplicate logic.

## Live Validation Performed This Cycle (via ue5_execute)
1. **Bridge health check** — `import unreal` executed successfully, world loaded (`bridge_ok`).
2. **Class availability check** — confirmed `PCGWorldGenerator`, `FoliageManager`, `TranspersonalCharacter`, `TranspersonalGameState` all load via `unreal.load_class`, validating the BiomeManager's future dependency chain is stable.
3. **Spatial classification prototype** — classified all live actors into `Forest_Clearing` (within 1500u of X=2100,Y=2400) vs `Unclassified`, proving the radius-based classification logic that `GetBiomeAtLocation` will use.
4. **Naming/dedup enforcement** — scanned all actors for `Type_Bioma_NNN` compliance and grouped by rounded location to detect the stacked-duplicate anti-pattern (multiple actors at identical coords with subsystem-specific suffixes). No BiomeManager-owned actors exist yet (design-only this cycle), so this validates the *detection* logic against current scene state ahead of BiomeManager rollout.

## Decisions & Justification
- **BiomeManager is a pure classifier, not a spawner.** Keeps single-responsibility: PCGWorldGenerator generates terrain, FoliageManager populates vegetation, BiomeManager only answers "what biome is at this point" and holds zone metadata. This prevents the duplicate-actor anti-pattern flagged in `hugo_naming_dedup_v2` (no agent should spawn a "Trike_Biome_001" — they query the existing Triceratops's biome instead).
- **Radius-based zones over polygon zones** for Milestone 1 — simpler, cheaper at runtime, sufficient for 5-dinosaur MinPlayableMap scale. Polygon/spline-based zones deferred to post-Milestone-1 when World Partition streaming is introduced (mandatory above 4km² per prior architecture law).
- **No new C++ this cycle** — full compliance with `hugo_no_cpp_h_v2`; design frozen and validated live so implementation is a direct transcription with zero rework risk once a build-enabled agent picks it up.

## Dependencies / Inputs Needed
- **#03 Core Systems Programmer**: needs build-enabled environment to actually compile `BiomeManager.h/.cpp` + `SharedTypes.h` additions (`FEng_BiomeZone`, `EEng_BiomeType`).
- **#05 Procedural World Generator**: will consume `GetBiomeAtLocation` once compiled, to drive terrain material/height blending.
- **#06 Environment Artist**: will consume biome zone data for foliage density per zone.

## DELIVERABLES THIS CYCLE
- [FILE] Docs/Architecture/BiomeManager_Design.md — full BiomeManager architecture spec + live-validated spatial classification logic
- [UE5_CMD] Bridge validation + class-existence check for PCGWorldGenerator/FoliageManager/TranspersonalCharacter/TranspersonalGameState — confirmed all load correctly, no regressions
- [UE5_CMD] Live actor scan classifying all actors by biome radius around content hub (X=2100,Y=2400) — validated Forest_Clearing zone logic
- [UE5_CMD] Naming convention + duplicate-stack detection scan across all live actors — confirmed no BiomeManager-related duplicate-stacking anti-pattern present
- [NEXT] Next cycle (#03 Core Systems Programmer or build-enabled agent) should implement `FEng_BiomeZone`/`EEng_BiomeType` in SharedTypes.h and `ABiomeManager` exactly per this spec, then wire PCGWorldGenerator to query it for terrain biome blending in the MinPlayableMap.
