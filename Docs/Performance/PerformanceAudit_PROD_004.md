# Performance Optimizer Audit — Cycle PROD_CYCLE_AUTO_20260710_004

## Bridge Status
OK for the entire cycle — 5 `ue5_execute` calls (3 python, 2 console commands), all `completed`, no timeouts/retries. IDs 30862–30866.

## Context Received from #03
Core Systems corrected collision on dinosaur placeholders (TRex, Raptor x3, Brachiosaurus, Trike) from `NoCollision` to `QueryAndPhysics`, so the player can no longer walk through them. #03 explicitly flagged this as a cost increase for #04 to re-profile, and confirmed no `SimulatePhysics` was enabled on those actors (no ragdoll behavior expected from static/AI-driven placeholders).

## Work Performed (5x ue5_execute)

1. **Baseline Actor/Tick Audit** — Enumerated all level actors, counted total actors and how many have Tick enabled, and cross-checked collision component state on all dinosaur-labeled actors post-#03 fix. Confirmed all dinosaur primitive components now report `QueryAndPhysics` collision (no more `NoCollision` regressions).
2. **stat unit** — Enabled on-screen frame time breakdown (Game/Draw/GPU ms) as a live profiling baseline for the next agent (#05 Procedural World Generator) to reference when adding terrain/biome density.
3. **stat fps** — Enabled FPS counter alongside `stat unit` for continuous visibility during subsequent world-generation work.
4. **Tick Optimization Pass** — Static decorative actors (`Tree_*`, `Rock_*`) had Tick disabled where it was unnecessarily enabled (these are non-animated placeholders with no per-frame logic needed). Re-verified zero dinosaur placeholders have `SimulatePhysics` active, avoiding unwanted physics-simulation overhead now that collision is `QueryAndPhysics` (query-only, not simulation).
5. **Post-Optimization Verification** — Re-counted total actors, tick-enabled actors after the pass, and confirmed the static-actor tick-disable change persisted and was saved with the level.

## Technical Decisions
- Disabling Tick on static foliage/rocks reduces per-frame actor tick overhead without affecting visuals, since these actors have no animation or gameplay logic — pure cost reduction with zero risk.
- Left dinosaur collision as `QueryAndPhysics` (per #03's fix) since this is required for player-blocking behavior; confirmed this does NOT imply physics simulation (no ragdoll cost), only collision queries — the correct low-cost choice for static/AI-driven pawns.
- Did not enable `SimulatePhysics` anywhere — that remains scoped to future ragdoll/destruction work (#03/#10/#12), out of budget for this cycle.
- Kept `stat unit`/`stat fps` on as a persistent lightweight profiling overlay for downstream agents (#05 World Generator) who will add terrain complexity — they should watch GPU ms as biome density increases.

## Files Created/Modified
- `Docs/Performance/PerformanceAudit_PROD_004.md` (this file)
- Live level: Tick disabled on static Tree/Rock actors; level saved via `EditorLevelLibrary.save_current_level()`.

## Dependencies / Next Inputs
- **#05 (Procedural World Generator)**: `stat unit`/`stat fps` are live on-screen — monitor GPU ms as terrain/biome density is added; keep static prop tick disabled by default for any newly spawned foliage.
- **#12 (Combat AI)**: Dinosaur collision is confirmed `QueryAndPhysics`, no `SimulatePhysics` — hit detection can rely on collision queries; do not enable physics simulation without a performance review.
- **#03**: If ragdoll/destruction work begins, flag #04 again to budget the physics-simulation tick cost separately from the current query-only baseline.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Baseline audit of actor count, tick-enabled count, and dinosaur collision component states post-#03 fix.
- [UE5_CMD] Enabled `stat unit` for live frame-time breakdown.
- [UE5_CMD] Enabled `stat fps` for live FPS counter.
- [UE5_CMD] Disabled Tick on static Tree/Rock actors (cost reduction); re-verified zero dinosaurs have SimulatePhysics enabled; level saved.
- [UE5_CMD] Post-optimization verification pass confirming tick-disable persisted and actor counts.
- [FILE] Docs/Performance/PerformanceAudit_PROD_004.md
- [NEXT] #05 should monitor stat unit/fps GPU ms as terrain/biome density increases; keep new static props with Tick disabled by default.
