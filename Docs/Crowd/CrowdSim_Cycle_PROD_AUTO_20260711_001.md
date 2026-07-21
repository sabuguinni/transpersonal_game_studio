# Crowd & Herd Simulation — Cycle PROD_CYCLE_AUTO_20260711_001
Agent #13 — Crowd & Traffic Simulation

## Bridge Status
Fully operational this cycle — 5/5 `ue5_execute` Python calls returned `"success": true` with no timeouts
(command IDs 31497–31501). This breaks a 3-cycle streak (PROD_010, PROD_011, PROD_012) where the
audit step consistently FAILED or timed out. Note: the Remote Control bridge on this instance only
returns a generic `{"ReturnValue": true}` wrapper per call — it does NOT relay `print()`/`unreal.log()`
output back to the calling session. All in-editor logging (`unreal.log`) was still executed for
Hugo/anyone tailing the UE5 Output Log directly, but this agent cannot visually confirm the exact
printed counts within its own session. The script logic was therefore written to be **fully
idempotent and self-contained** — all decisions (skip vs. form) are made by the script itself using
live data queried in the same execution, not by a human reading printed output between calls.

## Compliance with standing MAX-importance memories
- **hugo_herd_consistency_v1**: Before touching anything, the script queries ALL actors within 3500
  units of the hub (2100, 2400) and checks for any tag starting with `herd_`/`Herd_` (case-insensitive).
  If ANY such tag is found on ANY actor in range, the script takes the `SKIP_NO_NEW_HERDS_EXISTING_TAGS_REUSED`
  branch and does **not** invent a new naming scheme or reposition already-tagged actors.
- **hugo_hub_herds_v2_fix / hugo_hub_herds_v2 (crowd directive)**: Only if NO existing `Herd_*` tag is
  found does the script form new herds. It groups up to two clusters of untagged herbivore-labeled
  actors (label contains `trike`, `triceratops`, `brachio`, `parasaur`, or `stego`) found near the hub:
  - `Herd_HubGrazing_Alpha` — centered ~(1500, 2700), up to 5 members, offsets of 300–600 units apart,
    shared orientation ~35° yaw with ±20° jitter per individual (natural grazing variance, not a grid).
  - `Herd_HubGrazing_Beta` — centered ~(2800, 1900), up to 5 members, same spacing/jitter logic,
    shared orientation ~200° yaw.
  Each member actor is repositioned, re-oriented, and tagged exactly once (append-only tag list).
- **hugo_naming_dedup_v2**: No new actors were spawned. Zero duplicate label actors created — the
  script exclusively repositions/tags pre-existing actors already present in `MinPlayableMap`.
- **hugo_no_cpp_h_v2**: Zero `.cpp`/`.h` files written. All changes are live Python mutations via
  `ue5_execute`, followed by `EditorLevelLibrary.save_current_level()`.
- **hugo_no_camera_v2**: No viewport camera was touched.

## What the script actually does (single atomic pass, command_id 31500)
1. Scans all level actors, filters to those within 3500 units of hub (2100, 2400).
2. Splits them into `already_tagged` (has any `Herd_*` tag) vs. `untagged_herbivores` (label matches
   a herbivore species and has no `Herd_*` tag yet).
3. If `already_tagged` is non-empty → **no new herds formed**, existing tags are left untouched
   (per consistency rule — reuse, never re-invent).
4. If `already_tagged` is empty → forms up to 2 herds from the untagged herbivore pool, moving each
   member to a clustered position (300–600 unit offsets from a shared center), applying a shared
   base yaw with small per-individual jitter so the group reads as "grazing together" rather than a
   grid or random scatter, and appends the herd tag to each member exactly once.
5. Saves the level (`EditorLevelLibrary.save_current_level()`).
6. A separate verification pass (command_id 31501) re-queries all `Herd_*` tags on all actors and logs
   counts + sample member labels to the UE5 Output Log for external confirmation by Hugo/QA.

## Known Limitation / Handback
Because this bridge does not return stdout/print payloads to the calling agent, I cannot state with
certainty in this document whether the `SKIP` branch or the `FORM` branch executed this cycle — that
depends on whether any prior cycle (010/011/012, which all FAILED before reaching the audit) had
actually left `Herd_*` tags in the world. Given those three prior cycles show FAIL/timeout status,
it is likely no `Herd_*` tags existed yet, so this cycle's `FORM` branch (creating
`Herd_HubGrazing_Alpha`/`Herd_HubGrazing_Beta`) most likely executed. **Next cycle must verify by
reading the UE5 Output Log directly (not via this session) or by using a `get_property`/tag-count
Remote Control call that surfaces the return value rather than relying on print.**

## Interaction with Combat AI (#12)
Agent #12 this cycle attached native Actor Tags (`ApexPredator_Ambush`, `PackHunter_Flank`,
`DefensiveCharger_Herbivore`, `PassiveGiant_NoAggro`) plus aggro-radius tags to existing dinosaur
pawns. This crowd/herd script did not touch or remove those tags (only appended new `Herd_*` tags),
so both tag sets should coexist independently on the same actors.

## Files
- `Docs/Crowd/CrowdSim_Cycle_PROD_AUTO_20260711_001.md` (this file) — only file written this cycle.
  No `.cpp`/`.h` files created per hard rule.

## Next Agent (#14 Quest & Mission Designer)
- Read `Herd_HubGrazing_Alpha` / `Herd_HubGrazing_Beta` tags via `GetActorsWithTag` to design a
  "track/observe the herd" objective near the hub, reinforcing the living-world composition.
- Confirm via UE5 Output Log (search for `HERD_TAG_COUNT`) which branch executed this cycle before
  building any quest logic that assumes specific herd member counts.
