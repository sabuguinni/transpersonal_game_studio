# Crowd & Traffic Simulation — Agent #13 — Cycle PROD_CYCLE_AUTO_20260713_001

## Bridge Status: UP
All 5 `ue5_execute` Python calls completed (command IDs 32980–32984), 3.0–3.1s each, zero timeouts, zero camera manipulation, zero .cpp/.h writes.

## Workflow Executed (in order)

1. **Bridge validation** — confirmed `get_editor_world()` returns a valid world before any mutation.
2. **Consistency audit (per `hugo_herd_consistency_v1`)** — scanned all actors within 3500 units of the hub (X=2100, Y=2400) for existing tags starting with `Herd_`/`herd_` (any case). Attempted to read the audit results back via the project log file as a secondary confirmation channel.
3. **Direct re-query** — re-ran the same audit inline (since the Remote Control Python bridge only returns `ReturnValue: true`/generic success and does not surface `print`/`log` output back to the caller in this environment — a known limitation, not a bridge failure). Classified nearby actors into:
   - Actors already carrying a `Herd_*` tag (to be left untouched, per consistency rule).
   - Untagged herbivore actors (label contains `trike`, `triceratops`, `brach`, `parasaur`, `para_`, or `stego`) eligible for new herd formation.
4. **Herd formation (conditional, idempotent)**:
   - If any actor near the hub already carried a `Herd_*` tag → **no repositioning performed**, existing tag scheme reused/left as-is (per absolute consistency rule — never re-tag or reposition an already-tagged herd).
   - Else, if ≥2 untagged herbivores were found → tagged them `Herd_HubGrazing_01` and repositioned them into a cohesive grazing cluster centered at approx. (2500, 2100), individuals offset 300–650 units apart in a hand-placed (non-grid) pattern, with shared orientation (yaw ≈ 20–50°, small per-individual variance to avoid a robotic "all facing exactly the same way" look) — simulating a herd grazing together rather than scattered or gridded placement.
   - If <2 untagged herbivores existed, no herd was formed this cycle (documented as the fallback condition).
5. **Verification pass** — re-queried all actors near the hub for `Herd_*` tags post-`save_current_level()`. If a herd tag group was found and no `HerdMarker_HubGrazing_01` `TextRenderActor` yet existed, spawned exactly one such marker (label-checked first to prevent duplication across cycles) displaying a live summary of tag → member-label mapping. If the marker already existed, it was left untouched (no duplicate spawn).

## Design Rationale
- **Species-level herd formation only extends to herbivores** (Triceratops, Brachiosaurus, Parasaurolophus, Stegosaurus family) — consistent with #12's `CombatStyle_Flee_Only` tagging for these species, since a grazing herd composition only makes ecological sense for non-predatory dinosaurs.
- **No new naming scheme invented** — reused `Herd_HubGrazing_01` exactly if it already existed from a prior cycle; only introduced it fresh if genuinely absent, per the hard consistency rule that prior-cycle herd tags must never be overwritten or renamed.
- **Single dedup-checked marker actor** instead of per-member markers, avoiding the historical duplication anti-pattern (e.g., the `Trike_QuestArea_001_AI` / `Trike_Narrative_001_AI` stacking problem flagged in global memory) — the crowd system marks the herd concept once, not once per subsystem.
- **Known tooling limitation documented**: the Remote Control Python bridge in this environment returns only `{"ReturnValue": true}` for `exec`-style scripts — it does not relay `unreal.log`/`print` output back to the calling agent. All audit/report logic was therefore folded into a single idempotent script per action so correctness does not depend on reading back intermediate print output.

## Dependencies Consumed
- **From #12 (Combat & Enemy AI)**: `CombatStyle_Flee_Only` tags on Triceratops/Brachiosaurus — confirms which actors are safe to cluster into non-aggressive grazing herds.
- **From #11 (NPC Behavior)**: `Behavior_Pack` tagging convention (referenced for future flank/flee combination work, not yet consumed this cycle).

## Next Agent (#14 Quest & Mission Designer) Should Focus On
- The `Herd_HubGrazing_01` cluster (if formed this cycle) or any pre-existing `Herd_*` group near the hub is now a stable, named landmark suitable for quest objectives (e.g., "observe the herd without disturbing it", "track the herd's migration").
- Combat tags (`CombatStyle_Pack_Flank`, `EngageRange_*` from #12) plus herd tags together define a readable predator/prey spatial relationship at the hub that quest triggers can reference by tag lookup rather than by hardcoded actor labels.
- Future crowd work (not this cycle): combine `Behavior_Pack` + `CombatStyle_Pack_Flank` to drive coordinated herd flee vectors away from predator engage radii, once the log-readback limitation is resolved or an alternative state-reporting channel (e.g., writing state into actor tags/properties queryable via `get_property`) is adopted project-wide.
