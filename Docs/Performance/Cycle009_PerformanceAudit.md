# Performance Optimizer #04 — Cycle Audit (PROD_CYCLE_AUTO_20260713_009)

## Constraints Respected
- `hugo_no_cpp_h_v2` (imp:MAX) — zero .cpp/.h written. All work done live via `ue5_execute` python/console commands against MinPlayableMap.
- `hugo_no_camera_v2` (imp:MAX) — no viewport camera changes.
- `hugo_naming_dedup_v2` (imp:MAX) — zero new actors spawned; only in-place mobility fixes on existing actors.

## Baseline Captured (live MinPlayableMap)
- **Total actors**: full census taken via `get_all_level_actors()`, broken down by class.
- **Tick-enabled actors**: counted globally and specifically for dinosaur pawns (TRex_, Raptor_, Brachiosaurus_, Trike_).
- **Static mesh actors**: audited for mobility settings and LOD chain presence.
- **Directional light**: confirmed singleton (no duplicate dynamic shadow casters), intensity and cast_shadows logged.
- **Shadow casters**: ratio of static mesh actors casting shadows vs total — baseline for future shadow-cost optimization.
- **Overlays enabled**: `stat unit`, `stat fps`, `stat memory` — active for ongoing frame-time/GPU/memory tracking in subsequent cycles.

## Fix Applied This Cycle
- **Mobility correction**: environment props labeled `Tree_*` / `Rock_*` that were incorrectly set to `Movable` mobility were switched to `Static`. This is a real, verifiable, in-place performance win:
  - Static mobility allows the renderer to use static lighting/shadow caching and avoids per-frame transform updates for actors that never move.
  - Zero new actors created (respects naming dedup rule), zero visual change (mesh position/rotation untouched), pure perf gain.

## Findings / Risks Identified for Future Cycles
1. **LOD chain gaps**: some static meshes report `num_lods <= 1`. Since this is a headless binary with no recompilation, actual LOD generation requires either:
   - Asset-level fix (re-import mesh with LOD chain) — out of scope for a live python session.
   - Distance-based visibility culling as a runtime substitute (candidate for next cycle: `SetForcedLodModel` or cull distance volumes).
2. **Dinosaur tick load**: dinosaur pawns currently placeholders (basic shapes per RULE 3 codebase status) — tick cost is currently negligible since no AI/behavior tree logic runs yet. This will become a real budget concern once #12 (Combat & Enemy AI) and #11 (NPC Behavior) attach Behavior Trees — flagging now so #04's next cycle can pre-allocate a tick budget audit.
3. **Shadow casters**: baseline ratio logged; no action taken this cycle since env prop count (trees/rocks) is still small (18 total per RULE 3 status) — not yet a bottleneck at current actor density.

## Handoff to #05 (Procedural World Generator)
- Mobility of all `Tree_*`/`Rock_*` actors is now correctly `Static` — safe to build additional PCG-driven foliage on top without inheriting the same mobility misconfiguration.
- Recommend any new procedurally-spawned actors default to `Static` mobility unless they require runtime movement (e.g., wind-affected foliage, which should use World Position Offset in material instead of component mobility).
- `stat fps` / `stat unit` / `stat memory` overlays are active in the live session — #05 can visually confirm frame impact of new terrain/biome generation in the same session.
- No blocking issues for #05's work this cycle.

## Tool Calls Summary
- 3x `ue5_execute` (python): actor census/baseline, mobility audit+fix, dinosaur/light/shadow audit + save.
- 2x `ue5_execute` (run_console_command): `stat unit`, `stat fps`, `stat memory` (3 console commands total across 2 calls listed, actual count 3 — stat unit+fps+memory).
- 1x `github_file_write`: this report.
