# Performance Optimization Log — Cycle 008 (Performance Optimizer #04)

## Bridge Status
OK — 5 `ue5_execute` commands completed, all `success: true`, zero timeouts/retries (IDs 32728–32731 + stat unit console command). No .cpp/.h written this cycle (constraint `hugo_no_cpp_h_v2` respected — headless binary never recompiles).

## Live Work Executed (MinPlayableMap)

### 1. Baseline Performance Audit (python)
Census of all actors in the live world:
- Total actor count
- Tick-enabled actor count
- StaticMeshComponent count
- Movable-mobility component count
- Physics-simulating component count
- DirectionalLight count
- Class distribution (top 15)

Result written to `/tmp/ue5_result_perf04.txt` for cross-agent verification.

### 2. Frame Timing Baseline (console command)
Enabled `stat unit` — establishes game thread / draw thread / GPU thread ms baseline visible in the live editor viewport for ongoing frame-budget tracking across cycles.

### 3. LOD / Culling Budget Enforcement (python)
Applied concrete optimization pass to existing actors (no duplicates spawned — reuse-over-duplicate rule respected):
- **Static decorative props** (`Tree_*`, `Rock_*`): tick disabled (`set_actor_tick_enabled(False)`) — these never need per-frame updates — and `cull_distance` set to 6000uu on their StaticMeshComponent to reduce draw calls at distance.
- **Dinosaur pawns** (`TRex_*`, `Raptor_*`, `Brachio_*`, `Trike_*`): tick left enabled (required for movement/AI) but `cull_distance` set to 15000uu to bound their rendering cost without popping too aggressively given their size/importance to the hero shot composition.
- Confirmed directional light count (target: single light, no duplicate/competing light actors).
- Level saved after changes.

### 4. Final Sanity Sweep (python)
- Confirmed `TranspersonalCharacter` instance present, read live `CharacterMovementComponent` values (`gravity_scale`, `max_walk_speed`) to confirm no regression from #03's Cycle008 physics pass.
- Scanned for any StaticMeshComponent still simulating physics unnecessarily (perf watch list — flagged for next cycle if non-empty).
- Re-confirmed clean CDO load for all 7 active core classes (`TranspersonalCharacter`, `TranspersonalGameState`, `PCGWorldGenerator`, `FoliageManager`, `CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`) — zero crashes, baseline of 134 classes intact.

## Technical Decisions
- **Tick-disable on static props**: Trees and rocks are non-interactive decoration; disabling their tick is a zero-risk, zero-visual-impact perf win (no gameplay logic depends on their per-frame update).
- **Differentiated cull distances**: Dinosaurs get a longer cull distance (15000uu) than static props (6000uu) because they are the primary visual/gameplay focus (per `hugo_hub_quality_v2_fix` content quality mandate) and must remain visible longer for the hero composition at world coords X=2100,Y=2400.
- **No new actors spawned**: Followed `hugo_naming_dedup_v2` — this cycle is a hardening/optimization pass on existing actors, not asset creation. No duplicate Type_Bioma_NNN actors created.
- **No camera modification**: Respected `hugo_no_camera_v2` absolute rule — zero viewport camera changes.

## Dependencies / Next Cycle (#05 Procedural World Generator)
- Baseline actor/tick/physics census from `/tmp/ue5_result_perf04.txt` is available for reference before generating additional terrain/biome content — new PCG-spawned actors should follow the same tick-disable pattern for static decoration established this cycle.
- Physics-simulating actor watch list (if non-empty) should be reviewed by #03 next physics-focused cycle.
- `stat unit` is now enabled in the live viewport — subsequent agents can layer `stat fps` / `stat gpu` on top without re-enabling base frame timing.
- Recommend #05 keep new biome props' cull distances consistent with this cycle's 6000uu (static) / 15000uu (creature) convention for a coherent LOD budget across the whole level.
