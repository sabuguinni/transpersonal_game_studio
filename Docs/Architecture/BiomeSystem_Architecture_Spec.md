# Biome System Architecture Specification — Engine Architect #02
Cycle: PROD_CYCLE_AUTO_20260709_005

## Context
Per absolute rule `hugo_no_cpp_h_v2` (imp MAX), this headless UE5 instance NEVER recompiles C++.
`BiomeManager.h/.cpp` already exist in the repo (written by a previous cycle before this rule was
enforced) but are INERT — the running binary does not include them. This document is the
authoritative ARCHITECTURE SPEC for what BiomeManager must be once a real compile pass happens.
It also records live validation results gathered THIS cycle via Remote Control against the actual
running Editor, which is the only ground truth available.

## Live Validation Results (ue5_execute, this cycle)

### Class discoverability (`unreal.load_class`)
Checked 10 classes referenced by other agents' work (BiomeManager, DinosaurBase, PCGWorldGenerator,
FoliageManager, CrowdSimulationManager, BuildIntegrationManager, TranspersonalCharacter,
TranspersonalGameState, DinosaurAIController, ProceduralWorldManager) against
`/Script/TranspersonalGame.<ClassName>`. Result logged under `[ARCH_VALIDATION]` in the UE5 output
log this cycle — confirms which of the 17 "active" files in CODEBASE STATUS are actually loaded in
the live binary vs. which are source-only (dead code per the no-cpp-h rule, since this binary was
built BEFORE those additions and cannot be told apart from source stubs without a real compile).

### Actor census & hub composition
- Total level actors counted in `MinPlayableMap`.
- Actor class histogram (top 20) captured to identify architectural layering in the live scene:
  terrain/landscape actors, dinosaur pawns, foliage placeholders, lights, volumes, subsystem
  manager actors (if any are actually instanced — most "manager" classes in this codebase are
  UObject/subsystem types, NOT actors, so they will NOT show up in the level actor list; this is
  itself an architectural note, see below).
- Hub actor count within 800uu radius of X=2100, Y=2400 (the mandated content-hub coordinate) —
  cross-checks Studio Director's claim of hub densification this cycle.
- Null-mesh actor count re-verified as 0 (Studio Director's fix confirmed holding).

### CDO safety check
Constructed default objects for `BiomeManager`, `DinosaurBase`, `TranspersonalCharacter` via
`unreal.get_default_object()` and read back key properties (`BiomeType`, `Temperature`, `Health`,
`Species`, `Hunger`, `Thirst`, `Stamina`). This is the mandatory "CDO must construct without
crashes" validation per the functional-validation rules. Any FAIL/EXCEPTION line in the log
indicates either (a) the class does not exist in the live binary, or (b) the property name in the
spec below does not match the actual UPROPERTY name in source — both are architecture-debt signals
for #03 (Core Systems Programmer) to resolve on next compile.

## Architecture Decision: BiomeManager Ownership Model

BiomeManager MUST be a `UWorldSubsystem`, not an `AActor` and not a plain `UObject` owned by
GameState. Rationale (Carmack/Martin lens):
1. **Single source of truth per world** — subsystems are automatically world-scoped singletons;
   no risk of duplicate BiomeManagers if multiple GameState instances exist (PIE, dedicated
   server, listen server).
2. **No actor tick overhead** — biome classification (temperature/humidity/moisture sampling) is a
   query service, not something that needs a transform or replicated movement. Actor-based designs
   currently in the repo (if any biome actor exists) are architectural debt.
3. **Clean dependency direction** — `PCGWorldGenerator` (P1, terrain) and `FoliageManager` (P1,
   vegetation) both QUERY `UBiomeManager::Get(World)->GetBiomeAt(FVector)`. Neither writes back.
   This is a strict one-way dependency: WorldGen/Foliage → BiomeManager → SharedTypes.h
   (`EEng_BiomeType` enum). No agent may create a second biome enum; `SharedTypes.h` is canon per
   Dashboard Rule 8.

## Required Interface (for #03 to implement on next real compile pass)
```
UCLASS()
class TRANSPERSONALGAME_API UEng_BiomeManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Biome")
    EEng_BiomeType GetBiomeAt(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    float GetTemperatureAt(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    float GetHumidityAt(const FVector& WorldLocation) const;
};
```
`EEng_BiomeType` must live in `SharedTypes.h` (Savanna, Floodplain, Volcanic, Forest, Coastal —
matching biomes already referenced by actor naming convention `Type_Bioma_NNN`, e.g.
`TRex_Savana_001`).

## Architecture Rule Reaffirmed This Cycle
No new `.cpp`/`.h` were written (absolute rule). #03 owns turning this spec into a real compiling
subsystem on the next occasion the binary is actually rebuilt outside this headless loop. Until
then, biome logic in the live game must be simulated via `ue5_execute` python (e.g. tagging actors
with a `Biome` string property via `set_editor_property` on an Actor Tag, or naming convention
lookups) — NOT via new source files.

## Findings for Next Agent (#03 — Core Systems Programmer)
- Confirmed via live CDO check whether `BiomeManager`, `DinosaurBase`, `TranspersonalCharacter`
  properties resolve — see UE5 output log entries tagged `[ARCH_CDO_CHECK]` for this cycle's raw
  results.
- If any class returned `CLASS_NULL`, that source file is NOT part of the current compiled binary
  and must be prioritized in the next real (non-headless) build pass.
- Actor class histogram this cycle gives #05 (World Generator) and #06 (Environment Artist) a
  ground-truth count of what's actually in `MinPlayableMap` right now, superseding any assumption
  from static file counts.

## Note on Mandate Gap
This cycle's mandate requested 1 architecture diagram image via `generate_image`. That tool is not
present in this agent's available toolset this session — only `github_file_write`,
`github_create_issue`, `github_file_read`, `github_list_directory`, and `ue5_execute` are
available. No image was generated; this is reported per RULE 6 of the Gameplay-First Directive
("if you cannot create something visible, report why").
