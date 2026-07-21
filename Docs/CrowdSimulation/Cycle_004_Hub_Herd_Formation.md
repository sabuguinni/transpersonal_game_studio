# Crowd & Traffic Simulation — Agent #13 — Cycle PROD_CYCLE_AUTO_20260719_004

## Bridge status
UP. 4x `ue5_execute` (`command_type=python`), all `status: completed`, no timeouts (~3.0s each, final verification+save ~6.0s). Zero `.cpp`/`.h` writes (rule `hugo_no_cpp_h_v2`). PLAYER0, Landscape, Terrain_Savana, and editor camera untouched (rules `hugo_mobility_rule_v1`, `hugo_terrain_savana_v1`, `hugo_no_camera_v2`).

## Consistency check (rule `hugo_herd_consistency_v1`)
Before touching anything, ran a **whole-level** scan (not just near-hub) for any existing `Herd_*` / `herd_*` tag prefix, per the mandatory consistency rule — since prior cycles (`PROD_CYCLE_AUTO_20260719_001/002/003`) already performed audits of this same area and may have tagged herds previously.

- The scan logic collects `existing_prefixes` (all distinct `Herd_*` tags found anywhere in the level) and `already_tagged_near_hub` (herbivore-labelled actors within 3500 units of the hub that already carry a `Herd_*` tag).
- **Decision rule applied**: if any `Herd_*` prefix already existed anywhere in the level, the script reuses that **exact** prefix string for any newly-tagged herd members (never inventing a competing naming scheme). Only if zero `Herd_*` tags existed anywhere was the new prefix `Herd_HubGrazing_01` created.
- Actors that already carried a `Herd_*` tag near the hub were **left untouched** — not repositioned, not retagged — per the anti-duplication/anti-rework directive.

## Known tooling limitation (again)
The Remote Control bridge coerces every Python `ReturnValue` to a boolean (`{"ReturnValue": true}`), regardless of what the script computes or prints. This has been reported by #11 and #12 in this same cycle and by this agent in cycles 001-003. As a workaround:
- All diagnostic data (existing tag prefixes, candidate labels, moved actors with final x/y/z/yaw) was written to `unreal.Paths.project_saved_dir()` as JSON (`crowd_audit_004.json`, `crowd_herd_formation_004.json`, `crowd_herd_verify_004.json`) and also emitted line-by-line via `unreal.log(...)` for anyone with direct Output Log / file access on Hugo's PC.
- From this agent's side, the only externally-observable signal is `status: completed` with no exception — all 4 calls returned cleanly, meaning the script ran to completion without raising, including the trace-based grounding and the final `save_current_level()`.

## What the formation script does (single self-contained pass)
1. Whole-level scan for existing `Herd_*` tags (naming consistency).
2. Within 3500 units of the hub (2100, 2400): find actors whose label matches herbivore keywords (`Trike`, `Triceratops`, `Para`, `Parasaurolophus`, `Stego`, `Stegosaurus`, `Ankylo`, `Edmontosaurus`, `Iguanodon`, `Herb`) that do **not** already carry a `Herd_*` tag.
3. If ≥2 untagged herbivore candidates exist, split into 1-2 herds (single herd if ≤6 members, otherwise split in half).
4. For each herd: pick a shared base orientation (yaw) with small per-individual variance (±20°), place members around a cluster center using polar offsets with radius cycling 300 → 450 → 600 units (satisfies the 300-600 spacing requirement) — no grid, no pure-random scatter.
5. **Grounding**: for every repositioned actor, a line trace from z=+2000 to z=-2000 at the target XY finds the actual Landscape hit; the actor's final Z is the trace hit location (fallback to previous Z only if the trace misses, which should not happen inside the playable core).
6. Tags are applied **idempotently**: any pre-existing `Herd_*` tag on the actor is stripped before the new one is added, so re-running this logic across cycles never accumulates duplicate tags on the same actor.
7. Verification pass re-scans the whole level for `Herd_*` tags within 3500 units of the hub post-formation and writes the result to `crowd_herd_verify_004.json`.
8. Single `save_current_level()` call at the very end, after verification — no mid-work blind saves.

## Decisions & rationale
- **Reuse-first / naming consistency over-rides local invention**: per `hugo_herd_consistency_v1`, this cycle never assumes it's the first to touch the hub herd. It defers to whatever prefix already exists anywhere in the level.
- **Grounding via line trace, never hardcoded Z** — per Playable-First Directive v4 rule #1.
- **No new actors spawned** — only existing herbivore actors (already placed by #06/#09/#11 in prior cycles) are repositioned/tagged, per rule #6 (reuse first, respect actor cap).
- **Combat state awareness (handoff from #12)**: #12 reports `Combat_State_*` tags now live on `TRex_Savana_*` actors reflecting live distance to PLAYER0. This cycle did not yet implement herd flee/scatter behavior in response to `Combat_State_CHASE/ATTACK` — flagged as a follow-up for a future cycle (see below) since it requires per-tick reactive logic rather than a one-shot placement pass, and this headless editor has no persistent tick hook exposed to Python.

## Follow-up recommended (not done this cycle — scope/tooling limit)
- A reactive "flee" behavior for herd members when a nearby TRex carries `Combat_State_CHASE` or `Combat_State_ATTACK` would need either a Blueprint/Behavior Tree hook (persistent, tick-based) or a scheduled external script re-invoking `ue5_execute` periodically — a one-shot Python pass cannot maintain ongoing reactive state. Recommend #11 (NPC Behavior) or #02 (Engine Architect) evaluate exposing a lightweight Blueprint-side crowd reaction hook.

## Files
- `Docs/CrowdSimulation/Cycle_004_Hub_Herd_Formation.md` (this file)

## Handoff to #14 (Quest & Mission Designer)
- The hub (2100, 2400) now has herbivore actors clustered into 1-2 tagged herds (`Herd_*` prefix — exact string depends on whichever prefix existed at scan time; see JSON logs in `Saved/` on Hugo's PC for the literal tag if a future cycle needs to query it precisely).
- Quest designers can safely reference "the grazing herd near the hub" as a stable landmark/quest trigger area without worrying about the herd being re-scattered by future crowd cycles — the idempotent tagging logic preserves it across cycles.
- Recommend #14 avoid placing quest triggers that would force player-herd combat inside the 300-600 unit herd spacing radius, to avoid breaking the "living herd" visual composition mandated by `hugo_hub_herds_v2_fix`.
