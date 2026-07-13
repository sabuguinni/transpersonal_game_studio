# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260713_002

## Bridge status
OK — 5/5 `ue5_execute` calls completed without timeout/retry (IDs 33025–33029). Zero `.cpp/.h` written — `hugo_no_cpp_h_v2` constraint respected (48th consecutive cycle).

## Context: input from #03 Core Systems Programmer
#03 hardened collision on ~20+ dinosaur/tree/rock StaticMeshComponents (`QueryAndPhysics` + `BlockAll` profile) and tuned `CharacterMovementComponent` (MaxWalkSpeed=600, JumpZVelocity=420, GravityScale=1.0). My mandate this cycle: audit the frame-time impact of that change and optimize without touching the hero-screenshot hub composition (X=2100, Y=2400 per `hugo_hub_quality_v2_fix`).

## Work performed live (MinPlayableMap)

### 1. Baseline census (call 33025)
Counted total actors, tick-enabled actors, StaticMeshComponents, physics-simulating components, and BlockAll-profile components in the level — established the cost surface introduced by #03's collision hardening pass.

### 2. `stat unit` enabled (call 33026)
Activated frame/game/draw/GPU thread timing overlay for ongoing visual profiling in the live editor session.

### 3. Hub census + distance-based tick culling (call 33027)
- Censused actors within 3000 units of the hero-screenshot hub (X=2100, Y=2400): counted dinosaurs (TRex/Raptor/Brachio/Trike) and vegetation (Tree/Rock) present — confirmed hub composition data available to #08 Lighting for screenshot framing.
- **Disabled Tick on decorative actors (trees/rocks, non-dinosaur) beyond 6000 units from the hub.** This reduces per-frame CPU cost from static background props that never need per-tick logic, without touching any actor inside or near the hub composition.
- Saved level.

### 4. Hub integrity verification + LOD audit + far collision culling (call 33028)
- Verified hub actors (within 3000 units) were untouched by the tick-disable pass and remain ticking/visible.
- Audited StaticMesh LOD coverage on hub meshes (flagged single-LOD meshes for #06 Environment Artist / #19 Integration as future LOD-chain candidates).
- **Disabled collision on decorative (non-dinosaur) StaticMeshComponents beyond 8000 units from the hub**, directly mitigating physics query cost added by #03's blanket `BlockAll` pass — collision is now distance-scoped instead of uniform across the whole level.
- Saved level.

### 5. Final sanity check (call 33029)
Confirmed after optimization: `CharacterMovementComponent` still present and intact, `NavMeshBoundsVolume` still present, and **all dinosaur actors remain tick-enabled** (fully animated/visible) — the optimization pass targeted only background decoration, never gameplay-critical or hero-composition actors.

## Technical decisions & justification
- **Distance-based tick/collision culling instead of blanket disable**: preserves #03's collision hardening exactly where it matters (near player, near hub) while removing its cost where it doesn't (far background props). This is the Ericson/Fabian principle in practice — the visual/physics fidelity the Core Systems pass wanted is kept where it's seen, cut where it isn't.
- **Hub composition (X=2100,Y=2400) explicitly protected**: per `hugo_hub_quality_v2_fix`, this zone feeds the hero screenshot. No tick, collision, or LOD change was applied inside the 3000-unit radius.
- **No new actors, no camera changes**: respected `hugo_no_camera_v2` and existing `hugo_naming_dedup_v2` — no duplicate actors created; only property changes on existing actors.

## Dependencies / handoff
- **#05 Procedural World Generator**: distance thresholds (6000/8000 units) used here are placeholders based on current map scale — should be revisited once world size/biome layout is finalized.
- **#06 Environment Artist**: flagged single-LOD StaticMeshes near the hub for LOD-chain creation (perf gain without visual loss in the hero shot).
- **#19 Integration/QA**: recommend a follow-up `stat unit` capture during actual player movement (not just editor idle) to validate real frame-time gain from this cycle's culling pass.

## Files
- `Docs/Performance/Cycle_PROD_20260713_002_Report.md` (this report)
