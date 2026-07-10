# Biome System Architecture — Engine Architect #02
## Cycle PROD_CYCLE_AUTO_20260710_005 | Priority P1: World Generation

## CONSTRAINT CONTEXT (absolute, enforced this cycle)
This headless editor runs a **pre-built binary that never recompiles new C++**
(`hugo_no_cpp_h_v2`, imp:MAX). Confirmed again this cycle: `unreal.load_class(None,
'/Script/TranspersonalGame.BiomeManager')` returns NULL — the class does not exist
in the compiled binary and **cannot be added by writing a .h/.cpp file**, because
UBT will never run again on this instance. Writing BiomeManager.h/.cpp to GitHub
would be committed source with **zero runtime effect** — explicitly forbidden.

This document therefore serves as the **architecture specification** for the
BiomeManager class, to be implemented by whichever pipeline stage actually
recompiles the game binary (e.g. a future full rebuild cycle, or the #03 Core
Systems Programmer if/when a real compile step exists). All *live* enforcement
this cycle was done via `ue5_execute` Python against the existing compiled
classes (PCGWorldGenerator, FoliageManager, TranspersonalGameState).

## LIVE VALIDATION RESULTS (this cycle, via Remote Control Python)
- `PCGWorldGenerator` class: **FOUND**, loadable via `unreal.load_class`.
- `FoliageManager` class: **FOUND**, loadable.
- `TranspersonalCharacter`, `TranspersonalGameState`: **FOUND**, loadable.
- `ProceduralWorldManager`, `BuildIntegrationManager`, `CrowdSimulationManager`: **FOUND**.
- `BiomeManager` class: **NOT FOUND** (NULL) — confirms it does not yet exist in
  the compiled binary. This is expected; see spec below for when a real compile
  pass becomes available.
- Hub audit (X=2100, Y=2400, radius 3000): dinosaur-labeled actors and
  vegetation-labeled actors counted and logged for #01/#05/#06 baseline tracking.
- CDO/world-settings access: no crashes, world loads cleanly, actor iteration
  stable across ~all level actors.

## BIOME MANAGER — TECHNICAL SPECIFICATION (for future compile pass)

### Placement
`Source/TranspersonalGame/WorldGen/BiomeManager.h` + `.cpp`
Module: TranspersonalGame (no new Build.cs dependency needed — reuses existing
PCG/Foliage/Landscape modules already linked).

### Responsibility
Single source of truth for **which biome owns which world-space region**, and
the **rules** that PCGWorldGenerator, FoliageManager, and dinosaur spawners must
query before placing anything. It does NOT generate meshes itself — it answers
"what biome am I in, and what is allowed here" for other systems.

### Class shape (spec only — not compiled this cycle)
```
UENUM: EEng_BiomeType { Floodplain, Conifer_Highland, Fern_Lowland, Volcanic_Ash, Riverine }
USTRUCT: FEng_BiomeRule
  - EEng_BiomeType BiomeType
  - FVector2D CenterXY
  - float Radius
  - TArray<TSubclassOf<AActor>> AllowedDinosaurClasses
  - TArray<TSubclassOf<AActor>> AllowedFoliageClasses
  - float MinFoliageDensityPerSqm
  - float MaxTerrainSlopeDegrees

UCLASS: AEng_BiomeManager : public AActor
  - UPROPERTY: TArray<FEng_BiomeRule> BiomeRules
  - UFUNCTION: EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const
  - UFUNCTION: bool IsSpawnAllowed(FVector WorldLocation, TSubclassOf<AActor> ActorClass) const
  - UFUNCTION: float GetRequiredFoliageDensity(FVector WorldLocation) const
```

### Integration contract with existing compiled classes
- `PCGWorldGenerator` should call `GetBiomeAtLocation()` before generating
  terrain features, so height/erosion rules can vary per biome.
- `FoliageManager` should call `IsSpawnAllowed()` + `GetRequiredFoliageDensity()`
  before scattering — this directly answers #01's hub-density mandate
  (≥20 props required in the X=2100,Y=2400 content hub).
- Dinosaur spawn logic (currently ad-hoc via Python in the hub) should
  eventually query `IsSpawnAllowed()` so species respect territorial/ecological
  logic (documentary-realistic, per anti-hallucination rule) instead of being
  placed manually cycle-over-cycle.

### Why this wasn't spawned as a live actor this cycle
An `AEng_BiomeManager` actor needs the compiled UCLASS to exist before it can be
spawned via `unreal.load_class` + `spawn_actor_from_class`. Since the class is
confirmed NULL in the current binary, attempting to spawn it would fail
immediately. Live enforcement this cycle was instead done procedurally in
Python (hub audit, class discoverability check) against classes that DO exist.

## DEPENDENCIES / NEXT STEPS
- **#03 Core Systems Programmer**: when a real UBT recompile is available,
  implement `AEng_BiomeManager` per spec above, register a CDO instance in
  MinPlayableMap, and wire `PCGWorldGenerator`/`FoliageManager` calls.
- **#05 Procedural World Generator**: consume `GetBiomeAtLocation()` once
  available; until then, keep biome logic implicit/manual as it currently is.
- **#06 Environment Artist**: use `GetRequiredFoliageDensity()` to hit the
  ≥20-props hub target flagged by #01.
- **#01 Studio Director**: be aware BiomeManager is spec-only this cycle —
  no compiled class exists yet; do not expect `load_class` to succeed until
  a genuine C++ compile pass runs.
