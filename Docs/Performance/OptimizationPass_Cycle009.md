# Performance Optimization Pass — Cycle PROD_CYCLE_AUTO_20260711_009

**Agent:** #04 Performance Optimizer
**Bridge status:** OK for all 5 `ue5_execute` calls (IDs 32046–32050), all `completed`, no timeouts/retries.

## Context
Core Systems (#03) just re-enabled `QUERY_AND_PHYSICS` collision on **all** dinosaur pawns and static props (Tree_/Rock_) across the entire MinPlayableMap in this same cycle, including actors far outside the hero content zone (X=2100, Y=2400, r=1500uu). Collision-enabled actors that also tick every frame are a common CPU cost multiplier — my job this cycle was to audit and correct the tick side of that equation without touching the collision fix #03 just made.

## Actions Taken (Live, via ue5_execute)

### 1. Audit (command 32046)
- Confirmed bridge/world alive, confirmed `TranspersonalCharacter`, `PCGWorldGenerator`, `FoliageManager`, `TranspersonalGameState` all load cleanly (module healthy).
- Counted total actors, tick-enabled actors, and collision-enabled primitives across the whole level.
- Isolated the hero zone (X=2100, Y=2400, radius 1500uu) actor list — this is the composition that matters most per the content-quality directive (dense vegetation + posed dinosaurs in bright daylight).

### 2. Stat overlays (commands 32047, 32048)
- Enabled `stat fps` and `stat unit` console overlays to establish a live frame-time baseline (game thread / draw thread / GPU) going forward. These stay active in the editor viewport for future agents/QA to read at a glance.

### 3. Optimization pass (command 32049)
Applied a distance-based tick budget WITHOUT touching collision (respecting #03's fix):
- **Static decorative props (`Tree_*`, `Rock_*`):** disabled `ActorTickEnabled`. These are placeholder geometry with no gameplay logic — ticking them every frame was pure waste. Collision remains fully active (players/dinosaurs still can't walk through them).
- **Dinosaur pawns inside the hero zone** (within 1500uu of X=2100,Y=2400): **left at full per-frame tick** — this is the exact composition the hero screenshot frames, so animation/behavior responsiveness must stay maximal there per the content-quality directive.
- **Dinosaur pawns outside the hero zone:** set `ActorTickInterval = 0.2s` (5 ticks/sec instead of ~60-120), cutting their CPU cost by roughly 90%+ while they remain fully collidable and visually present (they're mostly static placeholders far from camera anyway).
- Saved the level (`save_current_level()`).

### 4. Verification (command 32050)
- Confirmed static props now report `ActorTickEnabled = False`.
- Confirmed hero-zone dinosaur pawns retained full tick.
- Confirmed far dinosaur pawns now run at the reduced interval.
- No errors, no CDO crashes, save succeeded.

## Technical Decisions & Justification
- **Did not touch collision** — that's #03's domain this cycle and the fix (enabling `QUERY_AND_PHYSICS` on all pawns/props) was correct and necessary for playability. My optimization is purely on the tick side, which is additive and non-conflicting.
- **Distance-based tick budget instead of blanket disabling** — following the "don't say no, find the way to say yes within budget" philosophy: the hero zone keeps full fidelity (it's what the screenshot/demo shows), while the rest of the world (currently populated with placeholder geometry, not yet real AI/animation) sheds unnecessary per-frame cost.
- **Left `simulate_physics = False`** as set by #03 — untouched, correct call until real physics assets exist (per #09/#10 pipeline).
- No new C++ was written (`hugo_no_cpp_h_v2` respected — 34th consecutive cycle). All changes are live, verifiable, and immediately in effect in the running editor session.

## Dependencies / Next Steps
- **#05 (Procedural World Generator):** as more terrain/biome content is generated outside the hero zone, apply the same tick-interval pattern to new large-scale foliage/terrain actors by default rather than retrofitting later.
- **#09/#10:** once real skeletal meshes + physics assets land on dinosaurs, re-validate that the reduced tick interval outside the hero zone doesn't starve animation blend graphs — may need to raise interval back up for any dinosaur within player travel range, not just the fixed hero-zone radius.
- **#18 (QA):** verify with `stat fps`/`stat unit` (now enabled) that frame time in the hero zone stays within 60fps PC / 30fps console budget under the new collision-heavy configuration.
