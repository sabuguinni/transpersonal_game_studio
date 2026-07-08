# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260708_005

## Bridge status: OK
All 5 `ue5_execute` Python calls completed cleanly against the live `MinPlayableMap` editor world (command IDs 29856–29860). No timeouts, no retries needed.

## Rule compliance
Per GLOBAL brain memory `hugo_no_cpp_h_v2` (importance MAX, ABSOLUTE): **zero `.cpp`/`.h` files written this cycle.** All optimization work executed live via Python/Remote Control against the pre-built running binary, not via source that would require recompilation.
Per `hugo_no_camera_v2`: viewport camera was never touched.

## Baseline audit (call 1)
- Confirmed live world (`MinPlayableMap`) loaded and reachable.
- Enumerated all `StaticMeshComponent`s across all actors: counted total vs. collidable (collision_enabled != NoCollision).
- This gives #03's collision-enforcement work (Cycle_005 Physics audit) a measurable before/after baseline for next cycle's re-check.

## Tick / hub density scan (call 2)
- Counted actors with `actor_tick_enabled=True` across the whole level.
- Counted actors inside the hero clearing radius (X=2100, Y=2400, r=1500uu) per `hugo_hub_quality_v2_fix`, confirming this is the actor-density hotspot that must stay performant since it's the framed screenshot composition.

## Optimization pass A — Tick reduction (call 3)
- Disabled `actor_tick_enabled` on all static prop actors labeled `Tree_*` / `Rock_*` (non-dinosaur, purely decorative geometry that has no per-frame logic).
- Explicitly preserved tick on all dinosaur actors (`TRex_*`, `Raptor_*`, `Brachio_*`, `Trike_*`, `Stego_*`, `Anky_*`, `Para_*`) since AI/behavior systems (#11, #12) depend on their tick.
- Rationale: static vegetation/rocks have no gameplay logic requiring per-frame CPU; disabling tick removes wasted actor-tick overhead at scale without affecting visuals or the hero clearing composition.

## Optimization pass B — Mobility & shadow cost (call 4)
- Forced `Static` mobility on all vegetation/rock `StaticMeshComponent`s that weren't already Static — enables static lighting/shadow caching instead of per-frame dynamic shadow recompute, a standard Ericson-style "pay once, not every frame" win.
- Disabled `CastShadow` only on props **outside** the hero clearing radius (>1500uu from X=2100,Y=2400) — keeps full shadow fidelity in the framed screenshot area (per `hugo_hub_quality_v2_fix`) while cutting shadow-draw cost in peripheral/background zones the camera doesn't prioritize.

## Verification (call 5)
- Ran `stat unit` console command to sample live frame timing (GameThread/RenderThread/GPU ms) post-optimization.
- Re-counted `actor_tick_enabled` actors to confirm the tick-disable pass from call 3 took effect and persisted.

## Decisions & justification
- Prioritized zero-risk, reversible Editor-state optimizations (tick flags, mobility, shadow casting) over any source-level change, consistent with the standing headless-binary constraint.
- Preserved full visual/gameplay fidelity in the hero clearing (content-quality zone) while trimming cost in peripheral areas — aligns Optimizer mandate ("say yes without breaking budget") with the content-quality directive instead of fighting it.
- Did not touch collision (already handled by #03 this cycle) or camera (forbidden).

## Dependencies / next steps for #05 (Procedural World Generator)
- Static mobility now enforced on vegetation — any new PCG-spawned props should be spawned with `Static` mobility and tick disabled by default going forward, not fixed up after the fact.
- Hub-radius (X=2100, Y=2400, r=1500uu) should be treated as a "full fidelity" zone in any new procedural generation pass: full shadows, full tick, higher prop density is acceptable there per content-quality mandate.
- Recommend #05 keep new terrain/biome tiles outside the hub radius lower density or use instanced static meshes (ISM/HISM) from the start to avoid a repeat optimization pass.
- Recommend #01/#02 resolve the recurring cycle-directive vs. headless-binary conflict (C++ file requests every cycle) at the generator level, per #03's note — this consumes budget without effect and duplicates the same report every cycle.
