# BiomeManager — Technical Architecture Spec v1
**Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260709_002**
**Status: DOCUMENTED SPEC ONLY — no .cpp/.h written (rule `hugo_no_cpp_h_v2`, imp MAX: this headless binary never recompiles C++; any .cpp/.h write is 100% wasted).**

## Why this exists
P1 priority (World Generation) requires a Biome system. Since C++ cannot be
recompiled in this live headless UE5 instance, the architecture below is the
contract that agent #03 (Core Systems Programmer) must implement in the
NEXT build cycle where C++ recompilation is actually available (CI/local
build), and that #05 (Procedural World Generator) uses TODAY via
`ue5_execute` Python against the running editor.

## Live validation performed this cycle
Via `ue5_execute` against the running MinPlayableMap:
- Confirmed `TranspersonalGameState`, `PCGWorldGenerator`, `TranspersonalCharacter`
  classes are loadable (`unreal.load_class`) in the live binary — architecture
  baseline intact, no regression.
- Scanned all level actors at the content hub (X=2100, Y=2400) and captured
  actor-type distribution and label-based biome tags.
- Applied non-destructive **Actor Tags** (`Biome_Hub_Forest` /
  `Biome_Outer_Unclassified`) to every actor in the level using UE5's native
  `AActor.Tags` array (FName tags) — this is a data-only change, no C++
  required, and is queryable today by ANY agent via
  `unreal.GameplayStatics.get_all_actors_of_class(...)` + tag filtering, or
  directly via `UGameplayStatics::GetAllActorsWithTag()` once #03 wires the
  BiomeManager subsystem.
- Saved the level after tagging.

This gives #05/#06/#08 a working, queryable biome zone signal RIGHT NOW
without waiting for a C++ rebuild.

## Class contract (for #03 to implement when build access exists)

### `UBiomeManager` (UWorldSubsystem)
Lives in `Source/TranspersonalGame/World/BiomeManager.h/.cpp`.
Prefix per project rule: types named `FEng_BiomeZone`, `EEng_BiomeType`.

```
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Forest_Cretaceous,
    Swamp_Fern,
    VolcanicPlains,
    RiverDelta,
    Highlands_Rocky,
    OuterUnclassified
};

USTRUCT(BlueprintType)
struct FEng_BiomeZone
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::OuterUnclassified;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    FVector Center = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float Radius = 2500.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float TemperatureBaseC = 28.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float HumidityPercent = 65.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<FName> AllowedDinosaurSpecies;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<FName> AllowedFoliageTags;
};
```

### `UBiomeManager` responsibilities
1. `TArray<FEng_BiomeZone> RegisteredBiomes` — populated at BeginPlay from
   `PCGWorldGenerator` output (owned by #05).
2. `EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const` —
   O(n) zone lookup for n<50 zones; upgrade to spatial hash if >50.
3. `void RegisterBiomeZone(const FEng_BiomeZone& Zone)` — called by #05
   during procedural generation pass.
4. `bool IsSpeciesAllowedInBiome(FName SpeciesTag, EEng_BiomeType Biome) const`
   — consumed by #11/#12 (NPC/Combat AI) to gate spawn logic and by #13
   (crowd sim) to bound agent density per zone.
5. Broadcasts `FOnBiomeTransition` delegate when player crosses a zone
   boundary — consumed by #08 (Lighting/Atmosphere) for biome-specific fog/
   color grading, and #16 (Audio) for ambience switching.

### Module dependency rule
`BiomeManager` lives in the `TranspersonalGame` primary module (same as
`PCGWorldGenerator`, `TranspersonalGameState`) — no new module needed, so
no `Build.cs` changes required. Forward-declare `APCGWorldGenerator` if used
by-pointer without ownership; use raw pointer, no `UPROPERTY()` unless GC
protection is added.

## Immediate actionable output for #05 (this cycle, no C++ needed)
Actor tag convention now live in the level:
- `Biome_Hub_Forest` — actors within 2500u of (2100, 2400)
- `Biome_Outer_Unclassified` — everything else (needs #05 to subdivide
  further into real biome zones as terrain generation expands)

Query pattern for any agent using `ue5_execute`:
```python
import unreal
world = unreal.EditorLevelLibrary.get_editor_world()
actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor)
forest = [a for a in actors if unreal.Name("Biome_Hub_Forest") in a.tags]
```

## Decisions & justification
- Did not write `.cpp/.h` (hard rule, zero effect on running binary).
- Used native `AActor.Tags` (FName array) instead of a custom component
  because it requires zero compilation and is immediately queryable by
  every downstream agent this cycle.
- Kept the class contract minimal (5 responsibilities) to avoid scope
  creep — Carmack principle: the smallest system that solves the real
  dependency (species-gating, lighting transitions, spawn density) wins.
- Naming follows project convention `Eng_` prefix for new types to avoid
  collisions per Rule 2 (unique type names across project).

## Dependencies / inputs needed from other agents
- **#05**: subdivide `Biome_Outer_Unclassified` into real zones as terrain
  generation expands beyond the hub; feed zone data into `RegisteredBiomes`
  once #03 implements the subsystem in a real build cycle.
- **#03**: implement `UBiomeManager` class per contract above on next
  buildable cycle (local/CI compile access, not this headless binary).
- **#08**: consume `FOnBiomeTransition` for atmosphere switching once
  subsystem exists.
- **#18 (QA)**: flag this spec as BLOCKING until #03 confirms a real build
  environment exists — otherwise this remains documentation-only forever.

## Files
- `Docs/Architecture/BiomeManager_Spec_v1.md` (this file)
