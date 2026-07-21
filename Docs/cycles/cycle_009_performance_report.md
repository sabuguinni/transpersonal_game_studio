# Performance Optimizer (#04) Report — Cycle PROD_CYCLE_AUTO_20260712_009

## Bridge Status
OK — 5 `ue5_execute` calls completed, zero timeouts/retries (IDs 32805–32809 + stat unit).

## Constraint Compliance
`hugo_no_cpp_h_v2` (imp:MAX) respected — **zero .cpp/.h writes**. All optimization work applied live via `ue5_execute` Python against the running `MinPlayableMap`, since the headless editor binary is pre-built and never recompiles new C++.

## Work Performed (live, MinPlayableMap)

### 1. Baseline Audit
Full actor census: total actor count, tick-enabled count, StaticMeshComponent count, Movable-mobility component count, physics-simulating component count, DirectionalLight count, Fog/SkyAtmosphere actor count, and top-15 class distribution. Logged to `/tmp/ue5_result_perf_baseline.txt`.

### 2. Mobility & Physics Cleanup
Iterated all level actors and, for anything **not** in the gameplay-critical keyword set (`Character`, `Pawn`, `TRex`, `Raptor`, `Brachiosaurus`, `Trike`, `Water`, `Trigger`, `PlayerStart`):
- Forced `StaticMeshComponent.Mobility = STATIC` (enables static lighting/batching, avoids per-frame transform updates).
- Disabled `SimulatePhysics` on any decor mesh that had it erroneously enabled (trees/rocks have no gameplay reason to simulate physics).
- Disabled actor Tick on static decor actors with no per-frame logic requirement.

This directly reduces CPU game-thread cost (tick overhead) and render-thread cost (movable-light interaction, dynamic shadow re-evaluation) without touching any actor the player, dinosaurs, or interaction system depend on.

### 3. LOD / Culling Distance Pass
Set `CachedMaxDrawDistance` on StaticMeshComponents by category:
- Decor (trees/rocks): 15000 units cull distance.
- Large dinosaur meshes (TRex/Raptor/Brachiosaurus/Trike): 25000 units cull distance (kept visible further since they're the hero content per `hugo_hub_quality_v2_fix`).

Also audited DirectionalLight components for `DynamicShadowDistanceMovableLight` and cascade count — logged for future shadow-cost tuning, no destructive change made (light itself untouched per `hugo_no_camera_v2`/lighting caution).

### 4. Persistence
Called `EditorLevelLibrary.save_current_level()` after all changes to ensure mobility/tick/cull edits persist across editor sessions, not just the live in-memory state.

## Technical Decisions & Justification
- **Static > Movable for decor**: Movable components force dynamic lighting recompute and skip static batching — pure cost with zero visual benefit for immobile trees/rocks in this scene.
- **Physics sim disabled on decor**: Simulating physics on static props costs per-frame physics-thread work; none of these objects are meant to react to forces in this milestone.
- **Differentiated cull distances**: Hero content (dinosaurs) gets a longer draw distance than background decor, aligning perf budget with the content-quality priority (`hugo_hub_quality_v2_fix` — the hub at X=2100,Y=2400 must read as a living forest with visible dinosaurs).
- **No lighting/fog removal performed this cycle** — that audit was informational only; changes to sun/fog are out of scope for Performance Optimizer unless a measured cost is found (none confirmed this cycle beyond logging shadow settings).

## Files Modified
- `Docs/cycles/cycle_009_performance_report.md` (this report)

## Dependencies / Next Steps for Other Agents
- **#05 (Procedural World Generator)**: New terrain chunks or biome props should be spawned with `Mobility=STATIC` and appropriate `CachedMaxDrawDistance` from the start — don't rely on #04 to retrofit every future prop.
- **#03 (Core Systems)**: `USurvivalComponent` spec is pending compile window; once implemented, ensure any tick logic added to `TranspersonalCharacter` doesn't regress the tick-count baseline logged here.
- **#08 (Lighting)**: Directional light shadow-cascade settings were logged but not modified — review `/tmp/ue5_result_perf_culling.txt` data if shadow cost needs tuning.
- **#18 (QA)**: Baseline + fix counts logged in `/tmp/ue5_result_perf_baseline.txt`, `/tmp/ue5_result_perf_fixes.txt`, `/tmp/ue5_result_perf_culling.txt` — use as regression reference for next cycle's audit (compare tick-enabled count, movable-component count before/after future changes).
