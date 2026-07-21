# Performance Optimizer (#04) — Cycle PROD_CYCLE_AUTO_20260710_010

## Constraint respected
`hugo_no_cpp_h_v2` (imp:MAX): zero .cpp/.h files written. All optimization work applied live via Remote Control Python against the running UE5 editor (pre-built binary, no recompilation).

## Context received from #03
Core Systems Programmer (#03) enforced correct collision channels on Character/dinosaurs/terrain and enabled `simulate_physics=True` on a subset of loose rocks (tagged `Core_PhysicsProp`) to give the player tangible physics feedback. My job this cycle: audit and bound the performance cost of that change and of the existing content hub population (trees, rocks, dinosaurs).

## Actions executed (5 ue5_execute calls, all completed, no timeouts)

1. **Performance audit** — enumerated all level actors via `get_all_level_actors()`, built a class-count breakdown, counted actors with `is_simulating_physics()==True` and actors with tick enabled. Confirmed the physics-simulating set matches exactly the `Core_PhysicsProp`-tagged rocks from #03 (no unexpected physics actors, e.g. dinosaurs/terrain still non-simulating as intended).
2. **`stat unit`** — enabled the unit stat overlay (Frame/Game/Draw/GPU ms) to establish a profiling baseline for the content hub zone (X=2100, Y=2400).
3. **Guardrails applied**:
   - Disabled actor tick on static, non-physics props (`Tree_*`, `Rock_*` not tagged `Core_PhysicsProp`) — these have no per-frame logic and should never tick.
   - Set `CachedMaxDrawDistance` on StaticMeshComponents: 12000 units for vegetation/rock props, 20000 units for dinosaur meshes (Trex/Raptor/Brachiosaurus/Trike) — bounds draw-call cost at distance without popping in the immediate content-hub view.
   - Capped physics cost on the `Core_PhysicsProp` rocks: linear damping 0.5, angular damping 0.8, gravity confirmed on — prevents jitter/sleep-cycle thrashing that would otherwise cost CPU physics-tick time every frame.
4. **`r.ViewDistanceScale 1.0`** — normalized to baseline so the new per-component cull distances are the actual controlling factor, not a global scalar.
5. **Post-optimization validation pass** — re-audited tick-enabled count, simulating-physics count, static mesh actor count, dinosaur count, and actor count within the hub zone radius (3000u around X=2100,Y=2400) to confirm no regressions and that the hero-screenshot composition (`hugo_hub_quality_v2_fix`) is untouched — no visibility/culling changes applied inside the hub radius that would remove dinosaurs or vegetation from that shot.

## Decisions & justification
- Tick-disable targets only inert static props — never touched Character, GameMode, or dinosaur pawns (those may need tick for future AI/animation from #10-#12).
- Cull distances chosen conservatively (12000u props / 20000u dinosaurs) so the content hub composition at X=2100,Y=2400 (well within range) remains fully visible; only far-away duplicates get culled.
- Physics damping applied only to the already-simulating rock subset from #03 — no new physics enabled, purely bounding the cost of what #03 introduced, per Ericson's "optimize what's proven expensive, don't preemptively restrict."
- No changes to lighting, fog, or sun pitch (respecting `hugo_hub_quality_v2_fix` composition rules).

## Dependencies / next agent focus
- **#05 (Procedural World Generator)**: when generating additional terrain/biomes, apply the same cull-distance pattern (12000u/20000u) to new static props from the start rather than retrofitting.
- **#12 (Combat & Enemy AI)**: dinosaur tick is currently untouched (still enabled) — when Behavior Trees land, re-profile tick cost per-dinosaur since AI logic will add real per-frame cost.
- **#08 (Lighting)**: `stat unit` overlay left active for baseline profiling — safe to disable once lighting pass is finalized.
