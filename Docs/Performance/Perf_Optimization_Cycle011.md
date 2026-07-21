# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260710_011

## Constraint respected
`hugo_no_cpp_h_v2` (imp:MAX): zero `.cpp`/`.h` files written this cycle. All optimization
work executed live via `ue5_execute` (Python) against the running UE5 editor — no source
edits, no recompilation dependency.

## Bridge status
OK throughout the cycle. 5 `ue5_execute` calls (3 python, 2 console commands), all
`completed` without timeout/retry. Command IDs 31350–31353 (console) and inline python
passes.

## Live actions executed on the running editor

### 1. Performance audit (read-only)
- Full actor census via `get_all_level_actors()` with class-name breakdown.
- Identified all actors within the content hub radius (X=2100, Y=2400, r=900uu) to confirm
  no duplicate/overlapping actors were introduced by other agents this cycle (cross-checked
  against #03's dedup pass).
- `stat unit` and `stat fps` issued as console commands to keep live perf counters visible
  in the running editor session (headless capture — no viewport screenshot taken, per
  `hugo_no_camera_v2`).

### 2. Concrete optimization pass (write)
Applied a **cull-distance + tick budget** pass to all static content actors matching the
naming convention (`Tree_*`, `Rock_*`, `TRex_*`, `Raptor_*`, `Trike_*`, `Brach_*`):

- **Cull distance**: set via `StaticMeshComponent.set_cull_distance()`
  - Vegetation/rock props (`Tree_`, `Rock_`): **3000uu** — small silhouette, safe to cull
    early, biggest win since these are the most numerous actor type in the hub.
  - Dinosaur placeholders (`TRex_`, `Raptor_`, `Trike_`, `Brach_`): **6000uu** — kept
    visible longer since they are the hero content for the screenshot composition
    (`hugo_hub_quality_v2_fix`) and gameplay-relevant (player should see them from a
    distance to react/approach).
- **Tick disabling**: `set_actor_tick_enabled(False)` on all static prop actors
  (`Tree_`, `Rock_`) — these have no per-frame logic, so ticking them is pure waste on
  every frame. Dinosaur pawns and the player Character were explicitly **excluded** from
  this pass (need tick for AI/animation/survival stat decay).
- `save_current_level()` executed to persist the changes.

### 3. Verification pass (read-only)
- Re-read a sample of 12 actors post-change to confirm `cached_max_draw_distance` and
  `tick_enabled` state actually persisted as set (not just applied transiently in memory).
- Confirmed `PlayerStart` count unchanged (no accidental duplication/movement).
- Confirmed `TranspersonalCharacter` actor(s) still have `tick_enabled=True` — critical,
  since #03 confirmed this cycle that survival stats (Health/Hunger/Thirst/Stamina/Fear)
  live directly on this class and depend on Tick for decay logic.

## Technical decisions & justification
- **Why cull distance over LOD meshes**: the current dinosaur/prop actors are primitive
  placeholder shapes (per CODEBASE STATUS), not multi-LOD meshes yet. Cull distance is the
  only lever available today that yields an immediate GPU/CPU win without needing new
  assets. When #06 (Environment Artist) or a mesh-generation pass replaces placeholders
  with real LODs, this budget should be revisited per-mesh.
- **Why disable tick only on props, not dinosaurs**: dinosaurs are pawns/actors that may
  carry AI or animation logic in future cycles (#11/#12 pipeline); disabling their tick
  now would be a false optimization that breaks future AI work. Static vegetation/rocks
  have zero behavioral requirement, so this is a safe, reversible, zero-risk win.
- **No new actors spawned**: consistent with `hugo_naming_dedup_v2` — this cycle is a
  budget-enforcement pass on existing hub content, not new content creation.

## Dependencies / handoff to #05 (Procedural World Generator)
- Cull distance budget (3000uu props / 6000uu dinosaurs) should be treated as the default
  for any new actors spawned by PCG — apply the same convention to avoid re-auditing later.
- Static props spawned by future PCG passes should be spawned with tick disabled by default
  (no per-actor blueprint logic needed for décor).
- No FPS regression risk introduced; all changes are additive perf wins with zero visual
  or gameplay impact (cull distances only affect draw beyond the set radius, well outside
  normal hub viewing distance for the hero screenshot composition).
