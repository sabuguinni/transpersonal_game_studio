# Biome System Architecture — Engine Architect #02
### Cycle: PROD_CYCLE_AUTO_20260710_011

## Constraint Compliance
Per `hugo_no_cpp_h_v2` (imp:MAX, GLOBAL): **zero .cpp/.h files written**, 6th consecutive cycle.
All architecture validation this cycle was performed LIVE against the running pre-built
UE5 binary via `ue5_execute` (python), confirming the well-established fact that this
headless editor instance never recompiles new C++ additions.

## What Was Executed This Cycle (3x ue5_execute, live UE5)
1. **Class compilation audit**: Confirmed `unreal.load_class(None, '/Script/TranspersonalGame.BiomeManager')`
   still returns `None` — BiomeManager (my P1 priority target) is NOT part of the compiled
   binary and cannot be instantiated, registered, or tested live. Also audited the 7 active
   classes listed in CODEBASE STATUS (TranspersonalGameState, TranspersonalCharacter,
   PCGWorldGenerator, FoliageManager, CrowdSimulationManager, ProceduralWorldManager,
   BuildIntegrationManager) for load-class success.
2. **Hub composition + naming-dedup enforcement**: Audited actors within 800uu radius of the
   content hub (X=2100, Y=2400) per `hugo_hub_quality_v2_fix`, separating dinosaur-labeled vs
   vegetation-labeled vs other actors — read-only architecture audit, no duplicate spawns
   (respecting `hugo_naming_dedup_v2`). Also enforced the single-DirectionalLight /
   sun-pitch-safety architecture rule (clamped pitch to -45° only if found outside the
   -30°/-60° safe range — did not touch camera, per `hugo_no_camera_v2`).
3. **Final verification + persistence**: Full actor/class distribution audit across the level,
   confirmed `TranspersonalGameMode` class loads and `PlayerStart` count, then
   `save_current_level()` to persist any pitch corrections made this cycle.

## Architecture Decision (Standing, Unchanged): BiomeManager Design
Since the binary cannot be recompiled in this headless environment, the BiomeManager
architecture remains a **design contract** for the next real compile pass (owned by #03
Core Systems Programmer, who has authority to modify Build.cs and trigger a rebuild):

- **Class**: `UEng_BiomeManager` (UObject-based World Subsystem, not an Actor — avoids
  per-instance actor overhead; one instance per world).
- **Data-driven**: `FEng_BiomeDefinition` (USTRUCT in SharedTypes.h) holds per-biome
  temperature range, moisture range, dinosaur spawn table (weighted), foliage density
  multiplier, and ground material reference. NO hardcoded biome logic in C++ — biomes are
  DataTable rows so #05/#06 can iterate without recompiling.
- **Query API**: `EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation)` — samples a
  biome noise/height map (owned by PCGWorldGenerator) and returns the current active class.
  This is the ONLY entry point other systems (FoliageManager, CrowdSimulationManager,
  spawners) may call — enforces single-source-of-truth for "what biome am I standing in."
- **Ownership boundary**: BiomeManager OWNS classification only. It does NOT spawn actors,
  place foliage, or move dinosaurs — those remain #06 (Environment Artist) and #11/#12 (AI)
  responsibilities, consuming BiomeManager's query API. This prevents the actor-duplication
  anti-pattern already flagged in `hugo_naming_dedup_v2` (e.g. Trike_QuestArea_001_AI stacking
  on existing Triceratops).

## Standing Recommendation
Since I (Engine Architect) am constrained from writing .cpp/.h, this BiomeManager design
is documentation-only until #03 performs an actual engine rebuild outside this headless
session. Until then, biome-like behavior in the live map should be approximated via
`ue5_execute` python (e.g., zone-based checks using actor location bounds) rather than
waiting on a class that cannot be tested here.

## Files Modified This Cycle
- `Docs/Architecture/BiomeManager_Architecture_Cycle011.md` (this file, new)

## Next Agent (#03 Core Systems Programmer)
- If a genuine engine rebuild window becomes available, implement `UEng_BiomeManager` per
  the contract above (SharedTypes.h additions: `FEng_BiomeDefinition`, `EEng_BiomeType`).
- Until then, continue live `ue5_execute` fixes only — do not write new .cpp/.h in this session.
- Confirm PlayerStart/TranspersonalGameMode DefaultPawnClass wiring (checked read-only this
  cycle, class loads successfully) still resolves to TranspersonalCharacter at runtime.
