# Performance Optimizer — Cycle PROD_CYCLE_AUTO_20260710_008

**Bridge status:** OK all cycle — 5x `ue5_execute` calls (3 python, 2 console commands), IDs 31135–31139, all `completed`, no timeouts/retries.

## Context received from #03
#03 enabled `QueryAndPhysics` collision + `BlockAll` on all dinosaur placeholders and static props (Tree/Rock) in the content hub (X=2100, Y=2400, r=1500uu), so the player character now collides physically with the environment. My mandate this cycle: audit the performance cost of that change and mitigate any regression risk before it compounds with future systems (foliage density, dino AI, crowd sim).

## Actions Taken (5x ue5_execute — real, verified changes to the live editor)

1. **Baseline capture (python)** — Scanned all actors in the hub radius, counted `StaticMeshComponent`s, counted how many have `QueryAndPhysics` collision enabled (the #03 change), and counted actors with tick enabled. Logged to `/tmp/ue5_perf04_baseline.txt`.
2. **`stat unit`** — Enabled on-screen frame/game/render/GPU thread timing breakdown for live profiling reference.
3. **`stat gpu`** — Enabled GPU cost breakdown (useful once #06/#08 add foliage density and dynamic lighting on top of the now-solid collision geometry).
4. **Optimization pass (python)** — For every static prop (Tree/Rock) and dinosaur placeholder inside the hub:
   - Disabled `bActorTickEnabled` — these are static/placeholder meshes with no per-frame gameplay logic, so ticking them wastes game-thread cycles for zero benefit. Player/Character actors were explicitly skipped and never touched.
   - Set `CullDistance = 6000uu` on their `StaticMeshComponent`s — prevents these hub props from rendering at extreme distance once the world expands beyond the hub, without affecting visibility near the player (hero shot radius is 1500uu, well inside the cull distance).
   - Forced `CTF_USE_SIMPLE_AS_COMPLEX` collision trace flag on Tree/Rock props — the `BlockAll`/`QueryAndPhysics` collision #03 enabled is now guaranteed to use simple collision shapes instead of expensive per-poly complex collision, which is critical because per-poly trace queries on dozens of props would be the single biggest CPU risk from #03's change.
5. **Verification pass (python)** — Re-scanned the hub and confirmed: tick-disabled count, cull-distance-applied count, and simple-collision-flag count all match the optimization pass. Logged to `/tmp/ue5_perf04_verify.txt`.

## Technical Decisions & Justification

- **Why disable tick instead of refusing the collision change:** #03's `QueryAndPhysics` collision is necessary for the player to physically interact with dinosaurs/trees (core to the "physics as emotional signature" design pivot). My job is not to veto it — it's to make sure it's affordable. Tick-disabling static meshes with no behavior logic costs nothing gameplay-wise and reclaims game-thread budget.
- **Why simple-collision-only on props:** `QueryAndPhysics` + complex (per-poly) collision on many static meshes is the classic silent frame-time killer — it wasn't specified by #03 which trace flag was used, so I locked it to simple collision explicitly rather than assume. This is the highest-leverage, lowest-risk fix available this cycle.
- **Why cull distance now, before #06/#08:** Environment Artist and Lighting agents are about to add foliage density and dynamic lighting on top of this same geometry. Setting a sane cull distance baseline now prevents a foliage-driven overdraw regression later, and does not change what's visible in the current hero-shot framing (X=2100,Y=2400, well within 6000uu).
- **Did not touch:** Player/Character actors, TranspersonalCharacter's CharacterMovementComponent, any lighting/material assets, and the collision *type* (BlockAll/QueryAndPhysics) itself — those remain #03's and #08's domain respectively.

## Files Created
- `Docs/Performance/Performance_PROD_008.md` (this file)

## Dependencies for Next Cycle
- #05 (Procedural World Generator): when generating new terrain/biomes beyond the hub, apply the same tick-disable + simple-collision + cull-distance pattern to new static props by default — don't let it regress.
- #06 (Environment Artist): foliage density additions should respect the `6000uu` cull baseline; re-profile with `stat gpu` (now enabled) once vegetation count increases materially.
- #08 (Lighting Agent): `stat unit`/`stat gpu` are now live in the editor — use them to check dynamic light cost against this static-geometry baseline before adding more sources.
- #18 (QA): verify in viewport that hub props at X=2100,Y=2400 still render correctly and player collision behavior (from #03) is unaffected by tick-disable/simple-collision changes.
