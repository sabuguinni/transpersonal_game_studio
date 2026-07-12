# Performance Optimizer (#04) — Cycle PROD_CYCLE_AUTO_20260712_007

## Bridge status
OK throughout the cycle — 5 `ue5_execute` calls completed successfully (IDs 32652–32656), no timeouts/retries.

## Constraint respected
`hugo_no_cpp_h_v2` (imp:MAX) — zero .cpp/.h files written. All optimization work done live via Remote Control Python against the running MinPlayableMap binary.

## Real work executed (live in MinPlayableMap)

### 1. Baseline audit (command 32652)
Full census of the level: total actor count, per-class distribution, tick-enabled actors, Movable-mobility root components, actors simulating physics, presence of fog/atmosphere actors, and directional light count (confirming single-sun setup, consistent with prior cycles' lighting work).

### 2. Static mesh / shadow / collision audit (command 32653)
Enumerated all `StaticMeshActor` instances (trees, rocks, dinosaur placeholders), checking `cast_shadow` flag and collision-enabled state per component. Established which foliage/prop actors were still casting real-time shadows unnecessarily (GPU cost with negligible visual return on small static props at distance).

### 3. Optimization pass (command 32654)
Applied targeted, reversible changes to existing actors (no new actors spawned — respects `hugo_naming_dedup_v2`):
- **Shadow casting disabled** on all `Tree_*` and `Rock_*` labeled StaticMeshActors — these are small/background props where dynamic shadow casting is pure GPU overhead with minimal visual benefit, especially at the hero-shot distances.
- **Cull distance set to 6000 units** on the same foliage/rock actors, so they stop being rendered/shaded once far from camera — reduces per-frame draw calls in larger scenes as the world grows (#05/#06 will add many more).
- **Tick disabled** on all dinosaur placeholder actors (`TRex_*`, `Raptor_*`, `Brachiosaurus_*`) — these are currently static-pose placeholders with no AI/animation driving per-frame logic yet (Combat AI #12 / Animation #10 haven't attached behavior). Disabling Tick removes wasted per-frame CPU cost until real behavior trees are attached; this must be re-enabled by #12 when AI logic is added.
- Saved the level after changes.

### 4. `stat unit` baseline enabled (command 32655)
Turned on the engine's frame-time HUD (Game/Draw/GPU ms) as a standing baseline instrument for future perf regression checks by this agent and QA (#18).

### 5. Verification pass (command 32656)
Re-queried all Tree_/Rock_/dinosaur actors to confirm the shadow-casting and tick-disabled flags persisted after the save, and reported final counts.

## Technical decisions & rationale
- Chose **non-destructive, reversible flag changes** (shadow, cull distance, tick) rather than deleting/respawning actors — respects the dedup rule and keeps the scene composition (the X=2100,Y=2400 content hub) intact for the hero screenshot.
- Left dinosaur **collision channels untouched** (Core Systems #03 just set them to `QueryAndPhysics`/`Pawn` profile this same cycle) — no conflict, tick and collision are orthogonal; disabling Tick does not affect collision queries or physics resolution.
- Did not touch BiomeManager_Authority_001 — outside this agent's scope this cycle and already correctly configured by #03 (NoCollision, non-blocking).
- No camera changes made (respects `hugo_no_camera_v2`).

## Dependencies / notes for next agents
- **#05 (Procedural World Generator)**: cull-distance pattern (6000 units) established for foliage/props should be reused as new PCG-spawned trees/rocks come online, to avoid re-auditing from scratch.
- **#10 (Animation)** / **#12 (Combat AI)**: dinosaur Tick was disabled as a perf measure since no behavior tree/animation currently drives them — **must re-enable Tick** on the specific dinosaur actors when attaching AI/animation logic, or their behavior will not update.
- **#18 (QA)**: `stat unit` is now enabled as a standing on-screen metric; use it to validate frame time before/after future content additions.
