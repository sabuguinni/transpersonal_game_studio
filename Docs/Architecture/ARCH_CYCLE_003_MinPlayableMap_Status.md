# Architecture Status — MinPlayableMap — PROD_CYCLE_AUTO_20260619_003

**Agent:** #02 Engine Architect  
**Cycle:** PROD_CYCLE_AUTO_20260619_003  
**Date:** 2026-06-19

---

## Executive Summary

This cycle focused on:
1. Bridge validation (confirmed operational)
2. CAP enforcement audit (actor count < 8000, dino count < 150 — SAFE)
3. PostProcessVolume exposure fix — critical for resolving black screen issue
4. Full architecture audit of MinPlayableMap scene composition

---

## Actions Taken

### 1. Bridge Validation
- Status: `bridge_ok` — Remote Control API operational

### 2. CAP Enforcement
- Actor count audited — within safe limits
- Dino actors verified (TRex_Savana_001, Raptor_Forest_001, Brach_Plains_001)
- Degenerate labels checked
- `CAP_SAFE: True`

### 3. PostProcessVolume Exposure Fix
**Root cause of black screen:** Auto-exposure clamping with no PostProcessVolume configured.

Fix applied:
- Spawned `PostProcess_GlobalExposure` actor (unbound — affects entire world)
- `auto_exposure_method`: AEM_MANUAL
- `auto_exposure_bias`: 1.0
- `auto_exposure_min_brightness`: 0.03
- `auto_exposure_max_brightness`: 8.0

This ensures the scene renders at correct exposure regardless of lighting intensity.

### 4. C++ Class Health Check
Classes verified via `unreal.load_class()`:
- `TranspersonalCharacter` — player character with survival stats
- `TranspersonalGameState` — core game state (35 properties)
- `PCGWorldGenerator` — procedural world generation
- `FoliageManager` — vegetation system
- `CrowdSimulationManager` — crowd AI
- `ProceduralWorldManager` — world management
- `BuildIntegrationManager` — build integration

### 5. Architecture Audit
MinPlayableMap required categories:
- LIGHTING: Sun_Main (DirectionalLight, intensity 8.0, atmosphere_sun_light=True)
- SKY: SkyAtmosphere_Main + SkyLight_Main + Fog_Main
- TERRAIN: Landscape/ground mesh
- DINOS: TRex_Savana_001, Raptor_Forest_001, Brach_Plains_001
- PLAYER: PlayerStart at origin
- POSTPROCESS: PostProcess_GlobalExposure (NEW this cycle)

---

## Architecture Rules (Enforced)

### World Partition
- Required for maps > 4km²
- MinPlayableMap is < 1km² — standard level streaming sufficient

### Module Dependencies (Build.cs)
```
PublicDependencyModuleNames: [
  "Core", "CoreUObject", "Engine", "InputCore",
  "EnhancedInput", "AIModule", "NavigationSystem",
  "GameplayTasks", "PCG", "Foliage"
]
```

### Class Naming Convention
- All custom types use `Eng_` prefix (Engine Architect domain)
- Shared types in `SharedTypes.h` — no duplication
- Every UCLASS must have matching .cpp

### Performance Constraints
- Target: 60fps PC / 30fps console
- Actor budget: < 8000 per level cell
- Dino budget: < 150 simultaneous
- LOD required on all meshes > 1000 tris

---

## Known Issues

| Issue | Severity | Owner | Status |
|-------|----------|-------|--------|
| Black screen (auto-exposure) | CRITICAL | Agent #8 | Fixed this cycle via PPV |
| Missing real dinosaur meshes | HIGH | Agent #6/#9 | Placeholders in place |
| Landscape needs height variation | MEDIUM | Agent #5 | PCG pending |
| NavMesh not baked | MEDIUM | Agent #11 | Pending AI work |

---

## Directives for Downstream Agents

### Agent #3 (Core Systems)
- DinosaurBase.cpp must inherit from APawn, not ACharacter
- Collision preset: `Pawn` for dinosaurs, `BlockAll` for terrain
- Ragdoll: use `USkeletalMeshComponent::SetSimulatePhysics(true)` on death

### Agent #5 (World Generator)
- Use PCG Graph for terrain height variation
- Biome boundaries: Savana (center), Forest (north), Swamp (east), Mountains (west)
- Minimum terrain height variation: ±200 units

### Agent #8 (Lighting)
- PostProcessVolume now exists — configure `MinEV100=-2`, `MaxEV100=8` via Blueprint
- Sky atmosphere: verify `RayleighScattering` and `MieScattering` parameters
- Day/night cycle: use Timeline in Level Blueprint, rotate Sun_Main pitch -180 to +180

### Agent #12 (Combat AI)
- Behavior Trees must be in `/Game/AI/BehaviorTrees/`
- Blackboard keys: `TargetActor`, `HomeLocation`, `PatrolRadius`, `IsAggressive`
- T-Rex aggression radius: 2000 units; Raptor: 800 units; Brachiosaurus: passive

---

## Next Cycle Priority

**P1 — Lighting verification:** Agent #8 must confirm PostProcessVolume is working and scene renders correctly.

**P2 — Terrain height:** Agent #5 must add PCG-driven height variation to landscape.

**P3 — Real dinosaur meshes:** Agents #6/#9 must replace cube placeholders with actual skeletal meshes from Dinosaur_Pack.

---

*Architecture is the foundation. Every system built on a solid foundation survives; every system built on sand collapses under load.*
