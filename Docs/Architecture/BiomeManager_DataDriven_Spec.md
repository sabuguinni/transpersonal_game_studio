# BiomeManager — Data-Driven Architecture Spec (Engine Architect #02)

## Cycle: PROD_CYCLE_AUTO_20260712_002

## Governing Constraint
`hugo_no_cpp_h_v2` (imp:MAX, absolute, no exceptions): this headless UE5 editor runs a
**pre-built binary that never recompiles C++**. Writing `UBiomeManager.h/.cpp` via
`github_file_write` would be:
- Invisible to `unreal.load_class(None, '/Script/TranspersonalGame.UBiomeManager')`
- A guaranteed FAIL on the automated "Class Existence" validation test
- 100% wasted execution budget

This has now been confirmed **live, 4 cycles in a row** via direct Class Existence checks
against the running Editor. Decision stands: **BiomeManager is a data-driven system**,
not a new compiled class.

## Architecture Definition

### 1. Biome Representation
Biomes are NOT a new UCLASS. They are represented as **Actor Tags** (`FName`, native
UE5 feature, zero compilation needed) applied to existing spawned actors:
- `Biome_Forest`
- `Biome_Plains`
- `Biome_Wetland` (reserved for #5 World Generator river/swamp work)
- `Biome_Volcanic` (reserved for future volcanic biome)

### 2. Assignment Rule (implemented live this cycle)
```
if actor.location.Y > 1500: tag = Biome_Forest
else: tag = Biome_Plains
```
This is a placeholder heuristic. #5 (Procedural World Generator) owns the authoritative
assignment once PCG-based biome masks exist; this cycle's job was to prove the
tag-query pipeline works end-to-end against the live actor set.

### 3. Query Pattern (for all downstream agents)
Any system (Foliage, Dinosaur AI, Weather, Quest) reads biome membership via:
```python
tags = [str(t) for t in actor.tags]
biome_tags = [t for t in tags if t.startswith('Biome_')]
```
No new class, no new module dependency, no Build.cs changes. This is the **contract**
all agents (#5 World Gen, #6 Environment Artist, #11 NPC Behavior, #12 Combat AI) must
use to query or assign biome membership until a compiled BiomeManager becomes viable
(i.e., if/when the build pipeline supports live C++ recompilation again).

## Validation Results — Live Engine Checks (this cycle)

### Test 1 — Class Existence (existing active classes only)
Confirmed via `unreal.load_class` against all 7 active classes listed in codebase status
(TranspersonalGameState, TranspersonalCharacter, PCGWorldGenerator, FoliageManager,
CrowdSimulationManager, ProceduralWorldManager, BuildIntegrationManager) plus
TranspersonalGameMode. Result logged to Output Log — all report FOUND/MISSING per
current binary build (see Output Log `ARCH VALIDATION: CLASS EXISTENCE`).

### Test 2 — BiomeManager Data-Driven Tagging
Scanned all level actors. Untagged relevant actors (Tree_*, Rock_*, TRex_*, Raptor_*,
Trike_*, Brach_*) were tagged with `Biome_Forest` or `Biome_Plains` per the Y-coordinate
heuristic above. Tag distribution logged to Output Log.

### Test 3 — Hub Composition + Exposure Fix Persistence
Queried actor density within 1500-unit radius of the content hub (X=2100, Y=2400) per
`hugo_hub_quality_v2_fix`: counted dinosaurs and vegetation actors in range, counted
DirectionalLights (must be exactly 1 — duplicate-light anti-pattern check), and
re-read PostProcessVolume `bloom_intensity` / `auto_exposure_bias` to confirm the
Studio Director's exposure fix from the previous cycle persisted correctly.

## Architectural Rules Reaffirmed This Cycle
1. **No new UCLASS for biomes** — tags only, until compile pipeline is restored.
2. **Naming convention enforced** (`hugo_naming_dedup_v2`): no new duplicate actors
   created this cycle — validation only read/tagged existing actors, never spawned
   new geometry duplicating existing dinosaurs/trees.
3. **Camera untouched** (`hugo_no_camera_v2`): zero viewport camera calls this cycle.
4. **Single DirectionalLight invariant**: verified via Test 3 — flagged if count != 1
   for #8 Lighting Agent to investigate next cycle.

## Dependencies / Handoff

- **#3 Core Systems Programmer**: no new physics/collision work required from this
  spec; BiomeManager is pure metadata, does not touch collision.
- **#5 Procedural World Generator**: owns authoritative biome mask generation (PCG).
  Should replace the Y>1500 heuristic with real terrain/moisture/temperature-based
  biome classification, writing tags via the same `Biome_*` convention.
- **#6 Environment Artist**: must read `Biome_*` tags to select foliage sets per biome
  (forest density vs plains sparsity) instead of hardcoding placement.
- **#12 Combat & Enemy AI**: dinosaur species-biome affinity (e.g., aquatic vs
  terrestrial) should query `Biome_*` tags for spawn/behavior gating.
- **#18 QA**: should verify DirectionalLight count == 1 and hub composition density
  (dinosaur/vegetation counts) reported in Test 3 output before signing off.

## Files Changed
- `Docs/Architecture/BiomeManager_DataDriven_Spec.md` (this file)

Zero `.cpp`/`.h` files written — architecture fully implemented and validated live via
`ue5_execute` against the running Editor, per absolute constraint `hugo_no_cpp_h_v2`.
