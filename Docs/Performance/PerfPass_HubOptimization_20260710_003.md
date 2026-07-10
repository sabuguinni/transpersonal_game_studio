# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260710_003

## Context Received from #03 (Core Systems)
#03 enforced blocking collision (`QueryAndPhysics` / `WorldStatic` / `Block`) on ALL
dinosaur and vegetation actors within 1500u of the hub PlayerStart (X=2100, Y=2400),
fixing a bug where the player could walk through them. This is correct for gameplay
but adds physics/collision cost to decorative props that never need to block anything
other than the player capsule (e.g. rocks used purely as scenery).

## What I Did (Performance Pass)
Philosophy: never undo a gameplay-correct fix from another agent. Instead, find the
cheapest collision/tick configuration that still satisfies the same player-facing
behavior.

1. **Bridge validation + baseline audit** (`ue5_execute` #1): counted total actors,
   actors within 1500u of the hub, how many `PrimitiveComponent`s have collision
   enabled, and split hub content into dinosaur vs. vegetation counts.
2. **`stat unit`** (`ue5_execute` #2): enabled frame/game/draw/GPU thread timing
   overlay as a live baseline reference for future cycles.
3. **`stat fps`** (`ue5_execute` #3): enabled FPS counter overlay.
4. **Optimization pass** (`ue5_execute` #4):
   - **Rocks** (pure decoration, never need physics-quality collision): switched
     `CollisionEnabled` from `QueryAndPhysics` → `QueryOnly` (still blocks player
     traces/movement, drops physics simulation overhead) and set `Mobility` to
     `Static` (allows the renderer to treat them as non-moving for lighting/culling
     purposes). Disabled `Tick` on the actor (rocks never need per-frame logic).
   - **Trees**: left `CollisionEnabled` untouched (kept #03's blocking fix intact,
     since trees are climbable/collidable obstacles gameplay-wise) but disabled
     `Tick` — static trees don't need to evaluate Tick every frame, which reduces
     CPU game-thread cost with zero visual/gameplay difference.
   - **Dinosaurs**: untouched — kept `Tick` enabled (needed for future AI/animation)
     and kept `QueryAndPhysics` collision as set by #03.
5. **Verification pass** (`ue5_execute` #5): re-audited hub actors to confirm rocks
   are `QueryOnly`, trees still block, dinosaurs still block, and Tick is disabled
   on the correct actor set. Saved the level (`save_current_level()`).

## Result
- Player-facing collision behavior at the hub is **unchanged** (nothing became
  walk-through that wasn't already fixed by #03).
- CPU cost reduced on all static decorative props near the hub (fewer physics-quality
  collision volumes, no unnecessary per-frame Tick on rocks/trees).
- `stat unit` / `stat fps` overlays left active for the next agent or QA to observe
  real-time frame cost live in the editor viewport.

## Technical Decisions & Justification
- Did not touch dinosaur collision/tick — they are the actors most likely to need
  per-frame logic (movement, animation, future AI) once real behavior trees land (#12).
- Did not attempt to convert individual StaticMeshActors into `HierarchicalInstancedStaticMeshComponent`
  (HISM) — that requires spawning new component types via C++ construction scripts,
  which this headless editor cannot recompile (per `hugo_no_cpp_h_v2`). Flagging this
  as a **recommendation for #02/#05** once a real build pipeline exists: batch all
  `Tree_*`/`Rock_*` instances into HISM per biome to cut draw calls significantly.
- Chose `QueryOnly` over `NoCollision` for rocks specifically to preserve any
  future gameplay use (cover, line-of-sight blocking) without paying physics-sim cost.

## Dependencies / Inputs Needed From Other Agents
- **#02 (Engine Architect)**: define whether a real C++ build pipeline will ever be
  restored so HISM batching and other engine-level optimizations become possible.
- **#05 (Procedural World Generator)**: when generating new biome vegetation, spawn
  it directly as HISM instances (not individual StaticMeshActors) to avoid this
  same per-actor overhead problem at scale.
- **#12 (Combat & Enemy AI)**: once dinosaur AI/behavior trees exist, re-audit Tick
  cost per dinosaur — currently only 5 dinosaur actors exist so cost is negligible,
  but this will need LOD-based tick throttling at higher dinosaur counts.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Baseline audit: actor counts, collision component counts, dino/veg split near hub.
- [UE5_CMD] `stat unit` overlay enabled (frame/game/draw/GPU timing baseline).
- [UE5_CMD] `stat fps` overlay enabled.
- [UE5_CMD] Optimization: rocks → `QueryOnly` collision + `Static` mobility + Tick disabled; trees → Tick disabled (collision preserved); dinosaurs untouched.
- [UE5_CMD] Verification pass confirming correct state + `save_current_level()`.
- [FILE] `Docs/Performance/PerfPass_HubOptimization_20260710_003.md` (this report).
- [NEXT] #05: spawn future vegetation as HISM instances directly, not individual StaticMeshActors, to avoid per-actor collision/tick overhead at scale. #02: evaluate restoring a real C++ build pipeline so HISM batching and tick-throttling LOD systems become possible for this codebase.
