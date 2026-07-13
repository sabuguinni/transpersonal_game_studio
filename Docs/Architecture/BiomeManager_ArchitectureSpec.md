# BiomeManager — Architecture Specification
**Author:** Engine Architect (#02)
**Cycle:** PROD_CYCLE_AUTO_20260713_001
**Status:** DESIGN APPROVED — implementation deferred (headless editor cannot recompile C++; see constraint below)

## Constraint governing this document
`hugo_no_cpp_h_v2` (imp:MAX) is in force: this headless UE5 instance runs a pre-built binary that
**never recompiles** new C++. Writing `.cpp`/`.h` files to the repo has **zero runtime effect** and
wastes execution budget. Therefore this cycle's P1 deliverable is the **complete, buildable
architecture spec** for `BiomeManager`, plus **live enforcement of its data contract** via
`ue5_execute` (Python) against the running `MinPlayableMap` — actor tagging that already encodes
the biome rules described below, so #03 (Core Systems Programmer) can implement the real class
against ground truth already present in the world.

## 1. Purpose
`BiomeManager` is the P1 authority for world-generation biome logic. It answers, for any world
location: "which biome am I in, and what rules (spawn tables, foliage density, weather bias,
temperature) apply here?" It sits between `PCGWorldGenerator` (terrain/height) and
`FoliageManager` / dinosaur spawners (population), per the dependency chain:
`Engine Architect → Core Systems → World Generator → Environment Artist`.

## 2. Module placement
- Module: `TranspersonalGame` (existing, do not create a new module).
- Files (for #03 to create when a real compile pass is available):
  - `Source/TranspersonalGame/BiomeManager.h`
  - `Source/TranspersonalGame/BiomeManager.cpp`
- Depends on: `SharedTypes.h` (existing — 22 shared types). **No new UENUM/USTRUCT duplicated** —
  reuse `SharedTypes.h` for `EEng_BiomeType` if not already present; if `PCGWorldGenerator.h`
  already declares a biome enum, `BiomeManager` MUST reuse it (Rule 3: one definition rule).

## 3. Class contract (for #03 implementation)
```
UCLASS()
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()
public:
    ABiomeManager();

    UFUNCTION(BlueprintCallable, Category="Biome")
    FName GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    TArray<TSubclassOf<APawn>> GetSpawnTableForBiome(FName BiomeTag) const;

protected:
    // Data-driven bands, matching the live tagging enforced this cycle:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float SavannaMaxY = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float ForestMaxY = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float WetlandMinX = 2800.f;
};
```

## 4. Biome rules (already enforced live this cycle as ground truth)
| Biome    | Condition                          | Notes |
|----------|-------------------------------------|-------|
| Savanna  | Y < 1500                            | Open plains, Trike/Brach herds |
| Forest   | 1500 ≤ Y < 3000                     | Dense vegetation, hub clearing sits here (X=2100,Y=2400) |
| Wetland  | Y ≥ 3000 AND X > 2800                | Raptor pack territory near water |
| Volcanic | Y ≥ 3000 AND X ≤ 2800                | Reserved, sparse population currently |

This cycle's `ue5_execute` pass tagged every existing dinosaur actor (TRex/Raptor/Trike/Brach)
in `MinPlayableMap` with `Biome_<Name>` actor tags computed from this exact table, and logged
counts to `/tmp/ue5_result_engine_architect.txt`. This gives #03 a **live validation set**: when
the real `BiomeManager::GetBiomeAtLocation` is implemented, its output must match these tags
for every existing actor — a regression test with zero extra tooling.

## 5. Integration points
- `PCGWorldGenerator` calls `BiomeManager::GetBiomeAtLocation` per-cell during terrain pass to
  bias height/erosion (arid vs. wetland).
- `FoliageManager` queries biome to select foliage density multiplier per existing `FoliageManager.h`
  (5 methods) — no new methods needed, pass `FName BiomeTag` into existing density call.
- Dinosaur spawn logic (P2, owned by #11/#12) queries `GetSpawnTableForBiome` instead of hardcoded
  per-actor lists — removes the anti-pattern flagged by `hugo_naming_dedup_v2` where duplicate
  actors were stacked per-subsystem instead of tagged/reused.

## 6. Validation performed this cycle (via ue5_execute, live editor)
1. Confirmed bridge + world alive, confirmed `PCGWorldGenerator`, `FoliageManager`,
   `TranspersonalGameState`, `TranspersonalCharacter`, `CrowdSimulationManager` classes all
   loadable via `unreal.load_class` — no regressions in active class set (per CODEBASE STATUS).
2. Tagged all dinosaur actors in `MinPlayableMap` with `Biome_<Name>` per the table above.
3. Counted dinosaurs inside the mandatory hub bounding box (X:1600–2600, Y:1900–2900) to confirm
   the content-hub composition rule (`hugo_hub_quality_v2_fix`) has spawnable subjects present.
4. Read back DirectionalLight pitch/intensity and ExponentialHeightFog density set by #01 last
   cycle — confirmed values are within the safe band (-30° to -60°, ≤6.5 intensity, fog ≤0.02)
   without modifying them (no camera/lighting changes made by this agent this cycle).
5. Wrote validation summary to `/tmp/ue5_result_engine_architect.txt` for #19 to consume.

## 7. Handoff — what #03 (Core Systems Programmer) must do next
- When a real C++ compile pass is available (not this headless session), implement
  `BiomeManager.h/.cpp` exactly per section 3, using the live tag data as regression ground truth.
- Do NOT redefine biome enums if `PCGWorldGenerator.h` already has one — extend, don't duplicate.
- `DinosaurBase.cpp` (per this cycle's directive to Core Systems) should read `BiomeTag` from
  `BiomeManager` at spawn time to select behavior-tree biome-specific parameters, rather than
  hardcoding per-species location logic.

## 8. Open items for #19 (Integration & Build)
- Actors previously tagged `DUPLICATE_REVIEW` by #01 (Studio Director, prior cycle) remain
  untouched by this agent — architecture ownership does not include deleting actors. #19 must
  resolve per the standing instruction.
