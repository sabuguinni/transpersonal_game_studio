# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260712_010

## Constraint compliance
- `hugo_no_cpp_h_v2` (imp:MAX): **zero .cpp/.h written**, 46th consecutive cycle. All work executed live via `ue5_execute` (Python) against the running MinPlayableMap editor instance.
- `hugo_no_camera_v2`: no viewport camera modified.
- `hugo_hub_quality_v2_fix`: hub composition at X=2100, Y=2400, r=1500 explicitly preserved at full mesh draw distance — optimization pass never degrades visuals in the hero screenshot zone.

## Bridge status
5/5 `ue5_execute` calls completed successfully (command IDs 32882–32886), zero timeouts, zero retries.

## Work performed this cycle

### 1. Baseline audit (command 32882)
Full census of MinPlayableMap: total actor count, tick-enabled actor count, total PrimitiveComponent count, physics-simulating component count, and top-12 actor classes by frequency. This is the frame-budget baseline against which all subsequent changes were measured.

### 2. `stat unit` snapshot (command 32883)
Captured game/render/GPU frame time breakdown via console command for the performance record.

### 3. Tick reduction on static dinosaur placeholders (command 32884)
Verified the physics-enabled Rock actors introduced by Core Systems #03 last cycle (no runaway count). Disabled `ActorTickEnabled` on all `TRex_*`, `Raptor_*`, `Trike_*`, `Brach_*`, `Stego_*` actors — these are currently visual-only static mesh placeholders with no AI/Behavior Tree driving them this milestone, so per-frame Tick is pure overhead. This is reversible: when NPC Behavior (#11) or Combat AI (#12) attach real logic to these actors, tick can be re-enabled per-actor at that point.

### 4. Distance-based mesh culling for foliage/rocks (command 32885)
Applied `CachedMaxDrawDistance` policy on all `Tree_*` and `Rock_*` StaticMeshComponents:
- **Within 1500cm of the hub (2100, 2400)**: draw distance forced to 0 (unlimited/full quality) — guarantees the hero screenshot composition required by `hugo_hub_quality_v2_fix` is never degraded by this optimization pass.
- **Beyond the hub radius**: draw distance capped at 8000cm — reduces overdraw and draw calls for background foliage/rocks the player is not actively viewing in the primary demo zone.

### 5. Final verification pass (command 32886)
Re-audited the map post-changes: confirmed single `DirectionalLight` actor, sun pitch within the safe guard range (-30 to -60), `PlayerStart` present, fog actor count, and final total actor count. All guard conditions passed.

## Decisions & justification
- **Why tick-disable instead of a new C++ tick-management subsystem**: any new UCLASS component would require engine recompilation, which this headless binary cannot do. Direct Python property mutation on the live actors is the only mechanism with immediate, verifiable effect.
- **Why hub-radius exemption on culling**: per `hugo_hub_quality_v2_fix` (imp:MAX), the demo screenshot zone must show dense, full-quality vegetation regardless of general performance policy elsewhere in the level. Optimization must never trade away the one composition shot that matters for the current milestone.
- **Why rocks keep physics but dinosaurs lose tick**: Core Systems #03 already justified physics-on-rocks as low-risk/no-gameplay-cost. Disabling tick on dinosaur placeholders is the complementary, orthogonal saving — it does not touch collision (still `BlockAll` per #03's pass) or physics, only per-frame CPU actor logic that currently does nothing.

## Files written this cycle
- `Docs/Systems/Perf_OptimizationPass_010.md` (this file) — only write, well within the 2-file budget, no .cpp/.h.

## Dependencies / handoff to next agent (#05 Procedural World Generator)
- Tick-disabled dinosaur actors are safe to re-enable selectively once #11/#12 attach Behavior Trees — flag `Core_Collidable` (set by #03) can be used to enumerate them.
- Foliage/rock culling radius (1500cm around 2100,2400) should be treated as the canonical "hub zone" boundary for any new PCG biome work — anything #05 generates inside that radius should default to full draw distance to protect the hero screenshot.
- Baseline actor/component counts from command 32882 are available in command history (ID 32882) for #05 to compare against after adding new procedural terrain/foliage, to track budget growth.
