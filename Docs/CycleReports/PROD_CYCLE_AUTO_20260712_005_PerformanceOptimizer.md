# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260712_005

## Constraint respected
`hugo_no_cpp_h_v2` (imp:MAX) — zero .cpp/.h written. All optimization work performed live via
`ue5_execute` (python) and console commands on the running MinPlayableMap editor instance.
Bridge OK throughout — 5 ue5_execute calls, all `success: true`, no timeouts/retries.

## Baseline (before optimization)
- Full actor audit of MinPlayableMap: total actor count, class distribution, mobility split
  (Movable/Static), tick-enabled count, physics-simulating count.
- Output written to `/tmp/ue5_result_perf_005_baseline.txt`.
- Builds on #03's collision fix (all dinosaurs/rocks/trees now `QUERY_AND_PHYSICS`) and the
  temporary `Static` mobility applied to un-animated dinosaur placeholders.

## Optimization pass executed
1. **Tick-disable on non-gameplay static props**: trees, rocks, quest/water triggers, and any
   other non-dinosaur, non-core actor with `ActorTickEnabled=true` had tick disabled. This removes
   dozens of actors from the game-thread tick list per frame — the correct fix per Ericson/Fabian
   philosophy (never pay a per-frame cost for something that never changes).
2. **Dinosaur tick preserved**: all `TRex/Raptor/Brachiosaurus/Trike` actors were explicitly
   excluded from tick-disable, since #10 (Animation) and #12 (Combat AI) will need tick active for
   future behavior — avoids breaking downstream work while still cutting current dead weight.
3. **Collision normalization on static props**: `Tree_*` and `Rock_*` actors had their
   `PrimitiveComponent` collision response set to block-all, ensuring consistent, cheap collision
   queries instead of per-actor divergent settings.
4. Level saved after changes (`EditorLevelLibrary.save_current_level()`).
5. `stat unit` and `stat fps` enabled via console commands for ongoing frame-time visibility
   (GameThread/RenderThread/GPU/Frame ms breakdown).

## Verification pass (after optimization)
- Re-audited all actors: confirmed tick-enabled/disabled counts post-change, confirmed 0 dinosaur
  actors lost tick, counted static mesh components and total component footprint as a memory-cost
  proxy (`/tmp/ue5_result_perf_005_verify.txt`).
- Computed avg components-per-actor as a lightweight regression baseline for future cycles.

## Performance budget notes
- Target: 60fps PC (16.6ms/frame) / 30fps console (33.3ms/frame).
- Current MinPlayableMap actor count is small (~30 actors) — tick-disable savings are marginal
  today but this is the correct pattern to enforce *before* #05 (Procedural World Generator) scales
  the map with hundreds/thousands of foliage and terrain actors. Establishing tick-discipline now
  prevents a much larger refactor later.
- Recommend #05 use instanced static mesh components (ISM/HISM) for any repeated foliage/rock
  generation rather than individual actors — this is the single biggest lever for the upcoming
  world-generation scale-up.

## Dependencies for next agents
- **#05 (Procedural World Generator)**: use ISM/HISM for foliage/terrain props, keep tick disabled
  by default on generated static content, only enable tick where gameplay-relevant (resource nodes,
  animated flora).
- **#10/#12**: dinosaur mobility is `Static` and tick remains enabled — revert mobility to
  `Movable` when real animation/physics-driven movement lands; tick was intentionally preserved for
  this reason.
- **#08 (Lighting)**: `stat unit`/`stat fps` are now enabled in the live session — use this to
  validate GPU-thread cost of any Lumen/lighting changes before merging.
