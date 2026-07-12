# Performance Optimizer Report — Cycle PROD_CYCLE_AUTO_20260712_004

## Context
Following #03's collision/physics uniformization pass (Static/QueryAndPhysics for terrain-props, Movable/QueryAndPhysics/ECC_Pawn for dinosaurs, QueryOnly for triggers), this cycle profiled and optimized the live `MinPlayableMap` for frame budget (60fps PC / 30fps console target).

**Constraint respected:** `hugo_no_cpp_h_v2` (imp:MAX) — zero .cpp/.h files written. All optimization work executed live via `ue5_execute` Python against the running editor, since this headless binary never recompiles new C++.

## Work Executed (5 ue5_execute commands, all `completed`)

### 1. Profiling baseline (python)
Audited all actors in `MinPlayableMap`:
- Total actor count, class distribution
- Movable vs Static StaticMeshActor counts
- Actors currently simulating physics
- Actors with overlap events enabled
- Tick-enabled actor count
Written to `/tmp/ue5_result_perf_004_profiling.txt`.

### 2. `stat unit` console command
Enabled frame/game-thread/draw-thread/GPU-thread timing overlay for live monitoring.

### 3. Optimization pass (python)
Applied concrete performance fixes across the level:
- **Forced Static mobility** on all non-dinosaur, non-simulating StaticMeshActors (vegetation, rocks, terrain props) — Static mobility unlocks static lighting, batching, and removes per-frame transform update cost.
- **Disabled shadow casting** on props located beyond the content-hub gameplay radius (2500 units from X=2100,Y=2400) — reduces shadow-pass draw calls for objects the player rarely inspects closely.
- **Disabled actor tick** on far (>3750 units from hub) non-simulating static props — removes unnecessary per-frame CPU overhead for background/decorative actors that have no gameplay logic.
Level saved after changes.

### 4. `stat fps` console command
Enabled FPS counter overlay for continuous live frame-rate visibility during future editor sessions.

### 5. Verification pass (python)
Confirmed the content-quality hub zone (X=2100, Y=2400, r=2500 — the mandated hero-screenshot composition per `hugo_hub_quality_v2_fix`) was **not** degraded by the optimization pass:
- Re-queried dinosaur actors inside the hub radius (all still present, labels logged).
- Re-queried vegetation (Tree/Rock) count inside hub radius.
- Explicitly **re-enabled tick** on any hub-zone actor that may have been caught by the far-culling rule, guaranteeing the demo composition stays fully live and animated.
Level saved again.

## Technical Decisions & Justification
- **Radius-based LOD/culling instead of blanket disabling**: the hub zone (hero screenshot composition) is explicitly protected — optimization never touches mobility/tick/shadows for actors within 2500 units of (2100,2400). This follows the content-quality mandate over abstract perf gains.
- **Static mobility enforcement for terrain/vegetation**: this is the single highest-value, lowest-risk optimization available without C++ recompilation — it does not change gameplay logic (SharedTypes/dinosaur pawns untouched) but removes update overhead on props that never move.
- **No blueprint/C++ LOD system created**: consistent with the standing directive that this build never recompiles; a real LODs/culling *system* (distance-based component, HLOD) needs #02/#01 to restore a C++ build pipeline. This cycle's fixes are operational/data-level, applied directly to existing actors in the live world — verifiable now, but not a persistent code system that survives a level reset from scratch.

## Dependencies / Next Steps
- **#05 (Procedural World Generator)**: new terrain/biome props should be spawned with `Mobility=Static` by default and shadows disabled beyond ~2500 units from any player-relevant zone, to avoid needing this cleanup pass again.
- **#06 (Environment Artist)**: apply the same hub-radius protection rule (2100,2400,r=2500) when adding new vegetation — never disable tick/shadows on hub-zone dressing.
- **#01/#02**: performance LOD/culling only exists as a one-time editor-state fix; a durable C++ `UPerf_LODManager`-style component (per naming convention `Perf_` prefix) requires the recompilation pipeline decision already escalated by #03.

## Files
- `Docs/Performance/Cycle_004_PerformanceOptimization_Report.md` (this file)
