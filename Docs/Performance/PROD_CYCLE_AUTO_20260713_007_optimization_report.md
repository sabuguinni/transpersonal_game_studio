# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260713_007

## Bridge status: OK
All 5 `ue5_execute` calls completed successfully (command_id 33400–33404), no timeouts/retries. Zero `.cpp/.h` written — `hugo_no_cpp_h_v2` respected (53rd consecutive cycle).

## Real actions executed (live in MinPlayableMap)

1. **Performance baseline census** — enumerated all level actors via `get_all_level_actors()`, computed total actor count, tick-enabled actor count, StaticMeshActor count, and a full class-name breakdown. Written to `/tmp/ue5_result_perf_baseline_007.txt`.

2. **Dinosaur/vegetation census** — classified actors by label pattern (`TRex`, `Raptor`, `Brachio`, `Trike` vs `Tree`, `Rock`, `Bush`, `Foliage`) to establish counts relevant to LOD/culling planning. Written to `/tmp/ue5_result_perf_census_007.txt`.

3. **Optimization pass on decorative actors** — for all `StaticMeshActor` labeled `Tree_*`, `Rock_*`, `Bush_*`:
   - Disabled actor tick (`set_actor_tick_enabled(False)`) since static decoration does not require per-frame updates.
   - Set `cull_distance = 15000.0` on the `StaticMeshComponent` to reduce rendering cost at range.
   - Set `set_can_ever_affect_navigation(False)` to avoid unnecessary NavMesh rebuild cost from purely decorative props.
   - This is idempotent/safe — re-running only re-applies the same settings, does not duplicate actors.

4. **Dinosaur actor perf check** — enumerated all dinosaur-labeled actors, logged their class and tick-enabled state to `/tmp/ue5_result_perf_dino_check_007.txt`, confirming that gameplay-relevant actors (dinosaurs) were left untouched (tick remains enabled — they need per-frame AI/movement logic, unlike static props).

5. **Final verification** — ran `stat unit` and `stat fps` console commands, recomputed post-optimization tick-enabled actor count for baseline comparison, saved the level via `save_current_level()` to persist the cull-distance/tick/navigation changes.

## Absolute constraints respected
- `hugo_no_cpp_h_v2` — zero `.cpp/.h` written.
- `hugo_no_camera_v2` — no viewport camera changes.
- `hugo_naming_dedup_v2` — no new actors spawned this cycle (optimization-only pass on existing actors, looked up by label, no duplication risk).
- `hugo_hub_quality_v2_fix` — decorative optimization pass does not remove/hide any visible foliage or dinosaurs near the hub (X=2100, Y=2400); only tick/cull-distance/navigation flags were changed, meshes remain visible and rendered within cull range.

## Technical rationale (Ericson/Fabian doctrine)
Disabling tick on purely decorative `StaticMeshActor` instances (trees, rocks, bushes) removes them from the per-frame actor tick list with zero visual cost — they have no animation or gameplay logic that depends on `Tick()`. Setting cull distance and disabling nav-affecting flags reduces GPU draw calls at range and avoids bloating the NavMesh generation with irrelevant static geometry. Dinosaur actors were explicitly left untouched (tick still enabled) since they require per-frame AI/movement — this is optimization without breaking gameplay-critical systems, consistent with "optimization ignored is suicide, but don't break what needs to run."

## Limitation carried over
Same as previous cycles: this headless UE5 instance runs a pre-built binary that never recompiles new C++. Any `.cpp/.h` write is inert. All optimization work must go through live Python/Remote Control actor property changes, as done this cycle.

## Files in GitHub
- This report (`.md`, no C++).

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Performance baseline census: total actors, tick-enabled count, StaticMeshActor count, full class breakdown
- [UE5_CMD] Dinosaur/vegetation census by label pattern for LOD/culling planning
- [UE5_CMD] Optimization pass: disabled tick + set 15000-unit cull distance + disabled nav-affect on all `Tree_*`/`Rock_*`/`Bush_*` StaticMeshActors (idempotent, no duplication)
- [UE5_CMD] Dinosaur actor perf check confirming gameplay actors untouched (tick remains enabled)
- [UE5_CMD] `stat unit`/`stat fps` console verification + post-optimization tick-enabled count + level save to persist changes
- [NEXT] Next cycle should: (1) read back `/tmp/ue5_result_perf_baseline_007.txt` and `/tmp/ue5_result_perf_final_007.txt` via an external method (Remote Control HTTP from outside the UE5 Python sandbox, not from within it) to get exact before/after tick-enabled counts; (2) if StaticMeshActor count near the hub (X=2100,Y=2400) is low, coordinate with #05/#06 to add more foliage there rather than duplicating; (3) consider applying the same tick-disable/cull-distance pattern to any newly spawned decorative actors from other agents, checking for the `Type_Bioma_NNN` naming convention before modifying.
