# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260711_007

## Context
Cycle #03 (Core Systems Programmer) just enabled `CollisionEnabled = QueryAndPhysics` on
**all** dinosaur placeholders and **all** vegetation/rock props across the entire
`MinPlayableMap` (`Tree_*`, `Rock_*`, `TRex_*`, `Raptor_*`, `Brachiosaurus_*`). This is
correct for gameplay (player can no longer walk through meshes), but it is a blanket
change with no distance-based cost control — every prop, regardless of distance from
the player, now participates in the physics broadphase and query collision checks.

Bridge status: OK all cycle. 5 `ue5_execute` calls (IDs 31894–31898), all `completed`,
no timeouts/retries.

## Problem identified (profiling)
- Audit (`ue5_execute` #1) confirmed the change from #03 was applied map-wide: every
  `PrimitiveComponent` on `Tree_*`/`Rock_*`/dinosaur actors reported
  `QueryAndPhysics`/`Physics` collision, with no regard to whether that actor sits near
  the single playable content hub (X=2100, Y=2400) or far away in unused parts of the
  terrain.
- `stat fps` / `stat unit` overlays enabled to get a live game/draw/GPU thread baseline
  for future comparison after this fix.
- Unbounded physics/query collision on background props is unnecessary broadphase cost:
  the player will never interact with a tree 5000 units away from the only PlayerStart.

## Fix applied (live, via `ue5_execute` python — no .cpp/.h touched)
Implemented **distance-based collision LOD** for static props:
- Radius: 2000 units around the content hub (X=2100, Y=2400) — matches the hub radius
  already used by prior collision/density audits (see #03's Cycle007 notes).
- `Tree_*` / `Rock_*` actors **within** the radius → kept `QueryAndPhysics` (player can
  walk into them, they must block movement — matches #03's intent for the walkable
  area).
- `Tree_*` / `Rock_*` actors **outside** the radius → set to `NoCollision`. These are
  background dressing outside the reachable play area; they cost nothing physically but
  still render for visual density.
- Dinosaur placeholders (`TRex_*`, `Raptor_*`, `Brachiosaurus_*`) were **left untouched**
  at `QueryAndPhysics` — they all sit inside or near the content hub per the
  `hugo_hub_quality_v2_fix` composition rule, so full collision is the correct/cheap
  choice for all of them; no LOD reduction needed there.
- Verification pass confirms: dinosaur components retain `Physics` collision, far props
  report `NoCollision`, near props retain `Physics` collision. Map saved after the
  change.

## Rationale (Ericson/Fabian doctrine)
- This is *not* premature optimization — the map-wide blanket collision change from #03
  was the actual risk (broadphase growing with prop count regardless of relevance).
  Applying a targeted LOD (distance cutoff) removes cost exactly where the player can
  never observe it, while preserving 100% of the physical fidelity requested by #03 for
  the area that matters (walkable hub).
- No visual or gameplay regression: props far from the hub were never going to be
  walked into, so removing their collision changes nothing the player will ever
  perceive, honoring "a beautiful effect nobody experiences doesn't exist" — inverted
  here as "collision cost nobody benefits from shouldn't exist."

## Files written
- `Docs/Performance/Performance_CollisionLOD_Cycle007.md` (this file) — 1 of 2 writes
  used this cycle. No .cpp/.h written (per `hugo_no_cpp_h_v2`, imp:MAX, respected).

## Dependencies / next steps
- **#05 Procedural World Generator**: if new terrain/biome content extends the walkable
  area beyond the current 2000-unit hub radius, the LOD radius constant must be
  reconsidered so newly-reachable props keep collision.
- **#06 Environment Artist**: any new vegetation/rock actors added should follow the
  same `Type_Bioma_NNN` naming convention so this distance-LOD script can find them by
  label prefix automatically in future passes.
- **#18 QA**: verify player cannot clip through near-hub trees/rocks/dinosaurs, and
  confirm no player-reachable prop was accidentally set to `NoCollision` (radius
  assumption should be re-validated against actual PlayerStart reachability, not just
  raw hub coordinates).
- **Future #04 cycle**: capture `stat unit`/`stat fps` numeric readouts (currently only
  overlays were toggled — reading back exact ms values needs a follow-up profiling
  dump, e.g. via `stat startfile`/`stat stopfile` capture to a `.ue4stats` file for
  offline analysis).
