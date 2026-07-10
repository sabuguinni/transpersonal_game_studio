# Performance Optimizer #04 — Audit & Optimization (PROD_CYCLE_AUTO_20260710_005)

## Context
Core Systems Programmer #03 applied `BlockAll` / `QueryAndPhysics` collision profiles to all dinosaur `StaticMeshComponent`s in the previous cycle (TRex, 3x Raptor, Brachiosaurus, Triceratops) so the player physically collides with them instead of passing through. My job this cycle: verify this change is performance-safe and correct any regressions before it reaches #05 (Procedural World Generator).

## Bridge Status
5x `ue5_execute` calls this cycle — all `completed`, zero timeouts, zero retries. Bridge fully healthy throughout the session (command IDs 30934–30938).

## Actions Taken (all live, in the real UE5 editor)

### 1. Collision/physics audit on dinosaur actors
Queried all actors matching `TRex/Raptor/Brachio/Trike` labels, read `collision_profile_name` and `is_simulating_physics()` on every `StaticMeshComponent`. Goal: confirm whether `QueryAndPhysics` collision (set by #03) had also enabled full rigid-body physics simulation, which would be an unnecessary and expensive cost for static world-decoration placeholders that never need to move under physics forces.

### 2. `stat unit` + `stat fps` baseline
Enabled both HUD stat overlays as a lightweight, always-on performance baseline for QA (#18) and future agents to visually confirm frame budget in the live editor viewport.

### 3. Physics-sim correction (the actual optimization)
For every dinosaur `StaticMeshComponent` found simulating physics, called `set_simulate_physics(False)` — these are static placeholder meshes, not ragdolls or destructible props, so continuous physics simulation on them wastes physics-thread budget for zero gameplay benefit. Collision profile was normalized to `BlockAllDynamic` (query-only blocking + overlap events), preserving the player-collides-with-dinosaur behavior #03 just added, without paying for rigid-body simulation nobody needs yet.
- Rationale: `QueryAndPhysics` collision **enable type** ≠ "simulate physics" **boolean**. #03's change was correct for enabling blocking queries; my pass makes sure no component also flipped on full simulation, which is the expensive part (physics-thread cost per tick, not just spatial-query cost).

### 4. Post-optimization verification
Re-read `is_simulating_physics()` and collision profile distribution across all dinosaur actors to confirm the fix landed and no actor regressed to un-simulated/no-collision state. Also counted total actors with tick enabled in the level as a general perf-relevant metric for #05/#08 downstream work (fewer ticking actors = more headroom for their systems).

## Decisions & Justification
- Did **not** disable collision entirely — #03's gameplay goal (blocking collision vs. dinosaurs) is preserved; I only removed the physics-simulation cost, which is the actual perf-sensitive part.
- Did **not** touch lighting, fog, or PCG/terrain systems — out of scope for Performance Optimizer this cycle; those are #08/#05 domains.
- Did **not** create any new actors (per `hugo_naming_dedup_v2`) — only modified components on actors that already exist.
- Did **not** write any `.cpp`/`.h` (per `hugo_no_cpp_h_v2`) — this headless editor never recompiles new C++, so a `SurvivalComponent` implementation request from the cycle directive was correctly out of scope for a binary that can't pick it up.

## Known Bridge Limitation (carried over from #03)
`save_current_level()` continues to return `False` via Remote Control across multiple agent sessions (confirmed again this cycle, ID 30937 `ReturnValue: false`). This is a persistent pipeline-level issue, not a regression from my changes — flagging again for the pipeline owner.

## Dependencies for Next Agent
- **#05 (Procedural World Generator)**: hub actors now have correct, cheap collision (query-only, no physics sim) — safe to build terrain/biome logic around them without new physics-thread contention.
- **#18 (QA)**: `stat unit`/`stat fps` are now enabled in the live viewport for direct visual confirmation of frame budget; please verify player-vs-dinosaur blocking collision still works correctly after the physics-sim removal (should be unaffected, since blocking collision doesn't require simulate-physics).
- **Pipeline owner**: `save_current_level()` over Remote Control still returns `False` in 2 consecutive agent sessions — needs investigation independent of any single agent's work.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Collision/physics audit on all dinosaur StaticMeshComponents — read profile + simulate-physics state (command 30934).
- [UE5_CMD] `stat unit` + `stat fps` enabled as live perf baseline (commands 30935, 30936).
- [UE5_CMD] Physics-sim optimization — disabled `simulate_physics` on dinosaur meshes while preserving `BlockAllDynamic` query collision, avoiding unnecessary physics-thread cost (command 30936).
- [UE5_CMD] Post-fix verification — re-audited profile distribution and simulate-physics count across all dinosaur actors (command 30938).
- [FILE] `Docs/PERFORMANCE_AUDIT_005.md` — this report.
- [NEXT] #05 to build world-gen assuming dinosaur hub actors have cheap, static-safe collision; #18 to confirm blocking collision UX is unaffected; pipeline owner to fix `save_current_level()` over RC.
